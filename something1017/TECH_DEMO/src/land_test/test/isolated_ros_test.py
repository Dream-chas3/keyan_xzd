#!/usr/bin/env python3
"""Integration check on a PRIVATE localhost ROS master, never on the aircraft.

Usage: source /opt/ros/noetic/setup.bash; python3 isolated_ros_test.py /tmp/v2_ws
The fixture supplies simulated MAVROS state/services and generated marker images.
"""
import csv
import json
import os
from pathlib import Path
import signal
import socket
import subprocess
import sys
import tempfile
import threading
import time


def wait_for(predicate, message, timeout=12):
    until = time.monotonic()+timeout
    while time.monotonic() < until:
        if predicate():
            return
        time.sleep(0.05)
    raise AssertionError(message)


def main():
    workspace = Path(sys.argv[1]).resolve()
    package = Path(__file__).resolve().parents[1]
    sock = socket.socket();sock.bind(("127.0.0.1", 0));port=sock.getsockname()[1];sock.close()
    os.environ["ROS_MASTER_URI"] = "http://127.0.0.1:%d" % port
    os.environ["ROS_IP"] = "127.0.0.1"
    os.environ.pop("ROS_HOSTNAME", None)
    import rosgraph
    import rospy
    import cv2
    import numpy as np
    import yaml
    from geometry_msgs.msg import PoseStamped, TwistStamped
    from mavros_msgs.msg import State, ExtendedState, PositionTarget, RCIn
    from mavros_msgs.srv import SetMode, SetModeResponse, CommandBool, CommandBoolResponse
    from sensor_msgs.msg import Image
    from nav_msgs.msg import Path as RosPath
    from std_msgs.msg import String, Int32
    from std_srvs.srv import SetBool
    temp = tempfile.TemporaryDirectory(prefix="keyan_aruco_isolated_")
    os.environ["ROS_LOG_DIR"] = str(Path(temp.name)/"ros")
    processes=[];logs=[];running=threading.Event();running.set()
    requests=[];commands=[];phases=[];detections=[];ids=[];headers=[];metrics=[];paths=[];previews=[]
    simulated={"mode":"POSCTL", "armed":True, "frame":None, "sequence":0,
               "z":1.2,"landed":2,"rc":1514,"rc_feed":True,"image_lag":0.0}

    def spawn(args, label):
        log=open(Path(temp.name)/(label+".log"),"w");logs.append(log)
        p=subprocess.Popen(args,stdout=log,stderr=subprocess.STDOUT,env=os.environ.copy(),start_new_session=True)
        processes.append(p);return p

    def stop(p):
        if p.poll() is None:
            os.killpg(p.pid,signal.SIGINT)
            try:p.wait(timeout=8)
            except subprocess.TimeoutExpired:os.killpg(p.pid,signal.SIGTERM);p.wait(timeout=5)

    try:
        master=spawn(["roscore","-p",str(port)],"master")
        wait_for(lambda: rosgraph.is_master_online(),"private ROS master unavailable")
        rospy.init_node("isolated_aruco_fixture",disable_signals=True)
        settings=yaml.safe_load((package/"config/takeoff_land.yaml").read_text())
        for key,value in settings.items():rospy.set_param("/"+key,value)
        rospy.set_param("/aruco_landing/command_gimbal_down",False)
        rospy.set_param("/aruco_landing/start_trigger","service")
        rospy.set_param("/aruco_landing/auto_takeoff",False)
        rospy.set_param("/aruco_landing/diagnostic_directory",temp.name)
        # The fixture camera image is static in the body frame and does not move
        # with the aircraft, so any camera offset becomes a constant unreachable
        # target error. Zero it for this integration test only.
        rospy.set_param("/aruco_landing/camera_offset_x",0.0)
        rospy.set_param("/aruco_landing/camera_offset_y",0.0)
        rospy.set_param("/aruco_landing/camera_offset_z",0.0)
        # This fixture also checks the strict "heading disabled means no turn"
        # contract; the new soft yaw correction is tested in the C++ suite.
        rospy.set_param("/aruco_landing/soft_yaw_enabled",False)
        # Non-default IDs prove the detector consumes YAML parameters.
        rospy.set_param("/aruco_marker/original_enabled",True)
        rospy.set_param("/aruco_marker/five_enabled",True)
        rospy.set_param("/aruco_marker/original_id",7)
        rospy.set_param("/aruco_marker/five_id",23)
        pub_state=rospy.Publisher("/mavros/state",State,queue_size=1)
        pub_pose=rospy.Publisher("/mavros/local_position/pose",PoseStamped,queue_size=1)
        pub_velocity=rospy.Publisher("/mavros/local_position/velocity_local",TwistStamped,queue_size=1)
        pub_extended=rospy.Publisher("/mavros/extended_state",ExtendedState,queue_size=1)
        pub_image=rospy.Publisher("/rtsp_cam/image_raw",Image,queue_size=1)
        pub_rc=rospy.Publisher("/mavros/rc/in",RCIn,queue_size=1)

        def mode(request):
            requests.append(("mode",request.custom_mode,time.monotonic()))
            simulated["mode"]=request.custom_mode
            return SetModeResponse(mode_sent=True)
        def arm(request):
            requests.append(("arm",request.value,time.monotonic()))
            simulated["armed"]=request.value
            return CommandBoolResponse(success=True,result=0)
        services=[rospy.Service("/mavros/set_mode",SetMode,mode),
                  rospy.Service("/mavros/cmd/arming",CommandBool,arm)]
        subs=[rospy.Subscriber("/mavros/setpoint_raw/local",PositionTarget,lambda m: commands.append((time.monotonic(),m))),
              rospy.Subscriber("/aruco_landing/status",String,lambda m: phases.append(next(csv.reader([m.data])))),
              rospy.Subscriber("/aruco_poseresult",PoseStamped,lambda m:detections.append(m)),
              rospy.Subscriber("/aruco_target_id",Int32,lambda m:ids.append(m.data)),
              rospy.Subscriber("/aruco_landing/status_header",String,lambda m:headers.append(m.data)),
              rospy.Subscriber("/aruco_detector/metrics",String,lambda m:metrics.append(json.loads(m.data))),
              rospy.Subscriber("/aruco_landing/guidance_path",RosPath,lambda m:paths.append(m)),
              rospy.Subscriber("/aruco_imgresult",Image,lambda m:previews.append((m.width,m.height)))]

        def marker(dictionary, target, size=80):
            return cv2.aruco.drawMarker(cv2.aruco.getPredefinedDictionary(dictionary),target,size)
        def frame_for(original=None, five=None, wrong_original=False, extra_five=False):
            frame=np.full((1080,1920),255,np.uint8)
            if original is not None:frame[428:508,919:999]=marker(cv2.aruco.DICT_ARUCO_ORIGINAL,original)
            if wrong_original:frame[150:270,100:220]=marker(cv2.aruco.DICT_ARUCO_ORIGINAL,0,120)
            if five is not None:
                # The delivered pad's 5X5 marker is detected on inverted grey.
                frame[350:590,840:1080]=0
                frame[428:508,919:999]=255-marker(cv2.aruco.DICT_5X5_250,five)
            if extra_five:
                frame[700:940,1300:1540]=0
                frame[780:860,1380:1460]=255-marker(cv2.aruco.DICT_5X5_250,33)
            return cv2.cvtColor(frame,cv2.COLOR_GRAY2BGR)
        simulated["frame"]=frame_for(original=7)
        def feed():
            n=0
            while running.is_set() and not rospy.is_shutdown():
                stamp=rospy.Time.now()
                s=State();s.header.stamp=stamp;s.connected=True;s.armed=simulated["armed"];s.mode=simulated["mode"]
                p=PoseStamped();p.header.stamp=stamp;p.header.frame_id="map";p.pose.position.z=simulated["z"];p.pose.orientation.w=1
                v=TwistStamped();v.header.stamp=stamp;v.header.frame_id="map"
                e=ExtendedState();e.header.stamp=stamp;e.landed_state=simulated["landed"]
                pub_state.publish(s);pub_pose.publish(p);pub_velocity.publish(v);pub_extended.publish(e)
                if simulated["rc_feed"]:
                    rc=RCIn();rc.header.stamp=stamp;rc.rssi=100;rc.channels=[1500]*18;rc.channels[4]=simulated["rc"];pub_rc.publish(rc)
                if n%3==0:
                    f=simulated["frame"];image=Image();image.header.stamp=stamp-rospy.Duration(simulated["image_lag"]);image.height=f.shape[0];image.width=f.shape[1]
                    image.encoding="bgr8";image.step=f.shape[1]*3;image.data=f.tobytes();pub_image.publish(image)
                n+=1;time.sleep(1/30)
        feeder=threading.Thread(target=feed,daemon=True);feeder.start()
        detector=spawn([str(workspace/"devel/lib/aruco_test/Safina_aruco_test"),"__name:=Safina_aruco_test"],"detector")
        controller=spawn([str(workspace/"devel/lib/land_test/Safina_land_test"),"__name:=Safina_land_test"],"controller")
        rospy.wait_for_service("/aruco_landing/enable",timeout=10)
        wait_for(lambda:ids and ids[-1]==7,"configured ORIGINAL ID not detected")
        time.sleep(1.5)
        assert not requests and not commands,"Startup performed flight control actions"
        assert phases[-1][2]=="IDLE",phases[-1]
        assert headers and "marker_reason" in headers[-1]
        assert metrics and metrics[-1]["selected_id"]==7
        assert previews and previews[-1]==(960,540),previews[-1:]
        assert paths and not paths[-1].poses,"Idle published a guidance path"
        assert detections[-1].header.stamp.to_sec()>0
        assert "/mavros/setpoint_position/local" not in dict(rospy.get_published_topics())
        simulated["image_lag"]=.65
        def diagnostic_value(key):
            return dict(zip(headers[-1].split(','),phases[-1])).get(key)
        wait_for(lambda:diagnostic_value("marker_reason")=="source_stamp_stale", "stale source frame not diagnosed")
        assert not requests and not commands,"Diagnostic probes performed startup actions"
        simulated["image_lag"]=0.
        simulated["frame"]=np.full((1080,1920,3),255,np.uint8)
        wait_for(lambda:diagnostic_value("marker_reason")=="detector_no_selected_target", "missing selected target not diagnosed")
        simulated["frame"]=frame_for(original=7)
        wait_for(lambda:diagnostic_value("marker_reason")=="accepted", "fresh target not accepted after recovery")
        rospy.set_param("/aruco_landing/align_yaw",False)
        # Node loaded align_yaw=false from YAML. A 90-degree pad must not command a turn.
        rotated=frame_for(original=7)
        rotated[428:508,919:999]=cv2.cvtColor(cv2.rotate(marker(cv2.aruco.DICT_ARUCO_ORIGINAL,7),cv2.ROTATE_90_CLOCKWISE),cv2.COLOR_GRAY2BGR)
        simulated["frame"]=rotated
        wait_for(lambda:abs(detections[-1].pose.orientation.z)>.5,"rotated test target not observed")
        enable=rospy.ServiceProxy("/aruco_landing/enable",SetBool)
        response=enable(True);assert response.success,response.message
        wait_for(lambda:any(r[0]=="mode" and r[1]=="OFFBOARD" for r in requests),"OFFBOARD handover not requested")
        wait_for(lambda:any(r[2] in ("STABILIZE","DESCEND") for r in phases),"no alignment phase reached")
        assert not any(r[0]=="arm" for r in requests),"Manual mode tried to arm"
        assert all(abs(m.yaw_rate)<1e-9 for t,m in commands),"Heading-disabled task commanded a turn"
        assert any(len(p.poses)==2 for p in paths),"Active controller guidance not published"
        diag=dict(zip(headers[-1].split(','),phases[-1]))
        assert diag["marker_reason"]=="accepted",diag
        assert int(diag["marker_accepted"])>0
        samples=[t for t,m in commands][-30:]
        assert len(samples)>10 and (len(samples)-1)/(samples[-1]-samples[0])>20,"Setpoint timer too slow"
        simulated["mode"]="POSCTL"
        wait_for(lambda:phases[-1][2]=="CANCELLED","pilot override not latched")
        time.sleep(.2);before=len(commands);request_count=len(requests)
        simulated["mode"]="OFFBOARD";time.sleep(1.2)
        assert len(commands)==before and len(requests)==request_count,"Task reclaimed control without re-enable"
        stop(controller);time.sleep(.2)
        assert len(requests)==request_count,"Destructor changed mode"
        # PX4 mode authority: even contradictory raw RC values cannot trigger.
        rospy.set_param("/aruco_landing/start_trigger","px4_mode")
        rospy.set_param("/aruco_landing/auto_takeoff",True)
        rospy.set_param("/aruco_landing/rc_channel",99)  # Deliberately invalid legacy settings are ignored.
        rospy.set_param("/aruco_landing/rc_offboard_slot",-7)
        simulated.update(mode="OFFBOARD",armed=False,z=0.,landed=1,rc=1514)
        before=len(commands);request_count=len(requests);phases.clear()
        controller=spawn([str(workspace/"devel/lib/land_test/Safina_land_test"),"__name:=Safina_land_test"],"mode_controller")
        wait_for(lambda:phases,"PX4 mode controller failed to start")
        time.sleep(1.3)
        assert len(commands)==before and len(requests)==request_count,"Already-OFFBOARD startup triggered task"
        response=enable(True);assert not response.success,"Service bypassed PX4 mode gate"
        simulated["mode"]="POSCTL";time.sleep(1.7)
        assert len(commands)>before and len(requests)==request_count,"POSCTL prestream missing or requested flight action"
        simulated["rc"]=2000;time.sleep(.3);simulated["rc"]=1514;time.sleep(.3)
        assert len(requests)==request_count,"Raw RC value bypassed PX4 mode authority"
        before=len(commands);simulated["mode"]="OFFBOARD"
        wait_for(lambda:any(m.velocity.z>.01 for t,m in commands[before:]),"PX4 OFFBOARD edge did not start automatic climb",timeout=10)
        assert any(r[0]=="arm" and r[1] for r in requests[request_count:]),"Automatic takeoff did not request arming"
        assert not any(r[0]=="mode" and r[1]=="OFFBOARD" for r in requests[request_count:]),"Mode gate requested OFFBOARD itself"
        simulated["mode"]="POSCTL"
        wait_for(lambda:phases[-1][2]=="CANCELLED","PX4 POSCTL did not cancel")
        time.sleep(.1);simulated["mode"]="OFFBOARD";time.sleep(.2)
        before=len(commands);request_count=len(requests);time.sleep(.8)
        assert len(commands)==before and len(requests)==request_count,"Mode bounce relaunched task"
        simulated["mode"]="POSCTL";simulated["armed"]=False
        time.sleep(1.7);before=len(commands);simulated["mode"]="OFFBOARD"
        wait_for(lambda:any(m.velocity.z>.01 for t,m in commands[before:]),"Second deliberate mode edge never climbed")
        simulated["mode"]="ALTCTL"
        wait_for(lambda:phases[-1][2]=="CANCELLED","mode override did not cancel task")
        time.sleep(.2);before=len(commands);request_count=len(requests);time.sleep(.7)
        assert len(commands)==before and len(requests)==request_count,"Mode override was reclaimed"
        stop(controller)
        # Vision-loss message must clear stale target coordinates and orientation.
        simulated["frame"]=np.full((1080,1920,3),255,np.uint8)
        wait_for(lambda:detections[-1].pose.position.z==-1,"missing marker did not invalidate pose")
        assert detections[-1].pose.position.x==0 and detections[-1].pose.orientation.w==1
        wait_for(lambda:metrics[-1]["reason"]=="no_configured_id","missing target not diagnosed")
        simulated["frame"]=frame_for(five=23,wrong_original=True,extra_five=True)
        wait_for(lambda:ids[-1]==23 and detections[-1].pose.position.z>0,"wrong IDs blocked/overwrote selected 5X5 marker")
        wait_for(lambda:metrics[-1]["selected_id"]==23,"selected ID metrics missing")
        wait_for(lambda:any(any(x["id"]==33 for x in m["detected"]) for m in metrics[-12:]),"periodic full search missed coexisting ID")
        expected_depth=2042.5*settings["aruco_marker"]["five_size_m"]/80.0
        assert expected_depth*.9<detections[-1].pose.position.z<expected_depth*1.1,"Detector did not apply configured physical marker size"
        # Uneven lighting and moderate blur; real IDs must still be decoded.
        base=frame_for(five=23)
        illumination=np.linspace(.45,1.0,1920,dtype=np.float32)[None,:,None]
        simulated["frame"]=cv2.GaussianBlur((base.astype(np.float32)*illumination).astype(np.uint8),(3,3),.8)
        before_detections=len(detections)
        wait_for(lambda:len(detections)>before_detections+2 and ids[-1]==23,"configured marker lost under synthetic blur/illumination")
        assert any(m.get("search_mode")=="roi" for m in metrics),"ROI tracking path never exercised"
        simulated["frame"]=np.full((480,640,3),255,np.uint8)
        wait_for(lambda:detections[-1].pose.position.z==-1,"resolution mismatch was not rejected")
        wait_for(lambda:metrics[-1]["reason"]=="resolution_mismatch","resolution diagnostic missing")
        stop(detector)
        # Startup camera action uses a loopback mock only; FCU remains untouched.
        simulated.update(mode="OFFBOARD",armed=False,z=0.,landed=1,rc=2000)
        rospy.set_param("/aruco_landing/command_gimbal_down",True)
        rospy.set_param("/aruco_landing/gimbal_down_on_startup",True)
        rospy.set_param("/aruco_landing/gimbal_ip","127.0.0.1")
        camera_socket=socket.socket();camera_socket.bind(("127.0.0.1",0));camera_socket.listen(1);camera_socket.settimeout(5)
        camera_port=camera_socket.getsockname()[1];rospy.set_param("/aruco_landing/gimbal_port",camera_port)
        camera_requests=[]
        def camera_reply():
            client,_=camera_socket.accept();client.settimeout(2)
            with client:
                packet=b""
                while len(packet)<72:
                    part=client.recv(72-len(packet))
                    if not part:break
                    packet+=part
                camera_requests.append(packet)
                client.sendall(bytes([0,0])) # CRC of an empty mock ACK payload.
            camera_socket.close()
        camera_thread=threading.Thread(target=camera_reply,daemon=True);camera_thread.start()
        request_count=len(requests);command_count=len(commands);phases.clear();headers.clear()
        controller=spawn([str(workspace/"devel/lib/land_test/Safina_land_test"),"__name:=Safina_land_test"],"startup_camera")
        wait_for(lambda:headers and phases and dict(zip(headers[-1].split(','),phases[-1])).get("gimbal_status")=="startup_down_acknowledged","startup camera command not acknowledged")
        time.sleep(1.3)
        assert len(camera_requests)==1 and len(camera_requests[0])==72
        assert int.from_bytes(camera_requests[0][7:9],byteorder="little",signed=True)==-9000,"Camera startup pitch was not -90 degrees"
        assert phases[-1][2]=="IDLE" and len(requests)==request_count and len(commands)==command_count,"Startup camera action performed flight control"
        stop(controller)
        # Failed camera setup also leaves the aircraft and task untouched.
        phases.clear();headers.clear();request_count=len(requests);command_count=len(commands)
        controller=spawn([str(workspace/"devel/lib/land_test/Safina_land_test"),"__name:=Safina_land_test"],"startup_camera_failure")
        wait_for(lambda:headers and phases and dict(zip(headers[-1].split(','),phases[-1])).get("gimbal_status")=="startup_down_failed","startup camera failure not diagnosed")
        assert phases[-1][2]=="IDLE" and len(requests)==request_count and len(commands)==command_count
        stop(controller)
        print("Isolated ROS checks passed: idle side effects=0, service compatibility, PX4 mode edge starts auto takeoff, "
              "already-ON startup blocked, mode-exit cancellation, mode-bounce blocked, deliberate mode re-enable, 30 Hz stream, "
              "manual override latch, no destructor mode changes, configurable IDs, invalidation, ID coexistence, resolution guard, "
              "startup camera -90 deg loopback ACK and failure with zero FCU actions")
        print("Private master:",os.environ["ROS_MASTER_URI"],"; logs:",temp.name)
    except Exception:
        for log in logs:log.flush()
        for p in Path(temp.name).glob("*.log"):
            print("\nLOG",p, "\n",p.read_text(errors="replace")[-7000:])
        raise
    finally:
        running.clear()
        for p in reversed(processes):stop(p)
        for log in logs:log.close()
        temp.cleanup()


if __name__=="__main__":main()
