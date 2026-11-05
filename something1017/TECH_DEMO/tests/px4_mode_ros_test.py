#!/usr/bin/env python3
"""Run compiled controllers against a mock FCU on a new PRIVATE loopback master.

Usage: python3 px4_mode_ros_test.py /tmp/mode_ws
Never starts MAVROS, a serial driver, a camera, or any hardware interface.
"""
import os
from pathlib import Path
import signal
import socket
import subprocess
import sys
import tempfile
import threading
import time


def wait_for(check, message, timeout=12):
    end = time.monotonic() + timeout
    while time.monotonic() < end:
        if check():
            return
        time.sleep(.03)
    raise AssertionError(message)


def main():
    workspace = Path(sys.argv[1]).resolve()
    with socket.socket() as sock:
        sock.bind(('127.0.0.1', 0))
        port = sock.getsockname()[1]
    os.environ.update(ROS_MASTER_URI=f'http://127.0.0.1:{port}', ROS_IP='127.0.0.1')
    os.environ.pop('ROS_HOSTNAME', None)
    import rosgraph
    import rospy
    from geometry_msgs.msg import PoseStamped, TwistStamped
    from mavros_msgs.msg import State, ExtendedState, PositionTarget, RCIn
    from mavros_msgs.srv import SetMode, SetModeResponse, CommandBool, CommandBoolResponse
    from std_msgs.msg import String

    processes, logs = [], []
    running = threading.Event()
    running.set()
    data = dict(mode='OFFBOARD', armed=False, ground=True, z=.7, feed=True, target=None, pwm=1000)
    requests, positions, velocities, phases = [], [], [], []
    with tempfile.TemporaryDirectory(prefix='px4_mode_private_') as directory:
        os.environ['ROS_LOG_DIR'] = directory + '/ros'

        def spawn(args, label):
            log = open(Path(directory)/(label+'.log'), 'w')
            logs.append(log)
            p = subprocess.Popen(args, stdout=log, stderr=subprocess.STDOUT,
                                 env=os.environ.copy(), start_new_session=True)
            processes.append(p)
            return p

        def stop(p):
            if p.poll() is None:
                os.killpg(p.pid, signal.SIGINT)
                try:
                    p.wait(timeout=8)
                except subprocess.TimeoutExpired:
                    os.killpg(p.pid, signal.SIGKILL)
                    p.wait(timeout=3)

        def save_position(m):
            positions.append((time.monotonic(), m))
            data['target'] = m.pose.position.z

        def mode(request):
            requests.append(('mode', request.custom_mode))
            data['mode'] = request.custom_mode
            return SetModeResponse(mode_sent=True)

        def arm(request):
            requests.append(('arm', request.value))
            data['armed'] = request.value
            return CommandBoolResponse(success=True, result=0)

        try:
            spawn(['roscore', '-p', str(port)], 'master')
            wait_for(rosgraph.is_master_online, 'private master unavailable')
            rospy.init_node('px4_mode_mock', disable_signals=True)
            publishers = [rospy.Publisher('/mavros/state', State, queue_size=1),
                          rospy.Publisher('/mavros/local_position/pose', PoseStamped, queue_size=1),
                          rospy.Publisher('/mavros/local_position/velocity_local', TwistStamped, queue_size=1),
                          rospy.Publisher('/mavros/extended_state', ExtendedState, queue_size=1),
                          rospy.Publisher('/mavros/rc/in', RCIn, queue_size=1)]
            subscribers = [rospy.Subscriber('/mavros/setpoint_position/local', PoseStamped, save_position),
                           rospy.Subscriber('/mavros/setpoint_raw/local', PositionTarget,
                                            lambda m: velocities.append((time.monotonic(), m))),
                           rospy.Subscriber('/aruco_landing/status', String, lambda m: phases.append(m.data.split(',')[2]))]
            services = [rospy.Service('/mavros/set_mode', SetMode, mode),
                        rospy.Service('/mavros/cmd/arming', CommandBool, arm)]

            def feed():
                while running.is_set() and not rospy.is_shutdown():
                    vz = 0.0
                    if data['mode'] == 'AUTO.LAND':
                        data['z'] = max(.7, data['z']-.02)
                        if data['z'] <= .701:
                            data.update(ground=True, armed=False)
                    elif data['armed'] and data['mode'] == 'OFFBOARD' and data['target'] is not None:
                        dz = max(-.01, min(.01, data['target']-data['z']))
                        data['z'] += dz
                        vz = dz*30
                        if data['z'] > .8:
                            data['ground'] = False
                    if data['feed']:
                        stamp = rospy.Time.now()
                        s = State(); s.header.stamp = stamp; s.connected = True
                        s.mode = data['mode']; s.armed = data['armed']
                        p = PoseStamped(); p.header.stamp = stamp; p.header.frame_id = 'map'
                        p.pose.position.x = 4; p.pose.position.y = -3; p.pose.position.z = data['z']; p.pose.orientation.w = 1
                        v = TwistStamped(); v.header.stamp = stamp; v.twist.linear.z = vz
                        e = ExtendedState(); e.header.stamp = stamp; e.landed_state = 1 if data['ground'] else 2
                        rc = RCIn(); rc.header.stamp = stamp; rc.rssi = 0; rc.channels = [data['pwm']]*18
                        for pub, msg in zip(publishers, (s, p, v, e, rc)):
                            pub.publish(msg)
                    time.sleep(1/30)

            thread = threading.Thread(target=feed, daemon=True)
            thread.start()
            demo = workspace/'devel/lib/offboard_ctl/offboard_ctl'
            land = workspace/'devel/lib/land_test/Safina_land_test'

            # Ground demo: startup already OFFBOARD cannot move or arm.
            controller = spawn([str(demo)], 'demo')
            time.sleep(2)
            assert controller.poll() is None
            assert not positions and not requests, 'OFFBOARD startup triggered demo'
            data['mode'] = 'POSCTL'
            wait_for(lambda: len(positions) > 45, 'POSCTL standby prestream absent')
            assert not requests
            assert all(abs(m.pose.position.z-.7) < 1e-6 for _, m in positions)
            # Nonsense RC PWM/RSSI must have no influence on the PX4 mode gate.
            data['pwm'] = 2000
            time.sleep(.3)
            assert not requests
            data['mode'] = 'OFFBOARD'
            wait_for(lambda: ('arm', True) in requests, 'actual OFFBOARD did not arm demo')
            wait_for(lambda: any(m.pose.position.z > 1.6 for _, m in positions), 'no relative +1 m target')
            assert all(abs(m.pose.position.x-4) < 1e-6 and abs(m.pose.position.y+3) < 1e-6 for _, m in positions)
            wait_for(lambda: ('mode', 'AUTO.LAND') in requests, 'two-second hover never handed off', timeout=16)
            assert ('mode', 'OFFBOARD') not in requests
            wait_for(lambda: not data['armed'], 'mock landing failed')
            time.sleep(.2)
            before = len(positions); count = len(requests)
            data['mode'] = 'OFFBOARD'
            time.sleep(1)
            assert len(positions) == before and len(requests) == count, 'completed demo restarted'
            data.update(mode='POSCTL', target=None)
            wait_for(lambda: len(positions) > before+45, 'demo cannot prepare second explicit attempt')
            data['mode'] = 'OFFBOARD'
            wait_for(lambda: data['armed'] and data['z'] > 1.0, 'second deliberate demo edge did not start')
            data['mode'] = 'POSCTL'
            time.sleep(.15)
            data['mode'] = 'OFFBOARD'  # Brief mode bounce cannot re-enable a cancelled task.
            time.sleep(.2)
            before = len(positions); count = len(requests)
            time.sleep(1)
            assert len(positions) == before and len(requests) == count, 'cancelled demo reclaimed control'
            stop(controller)

            # Default land workflow stays airborne/manual: no automatic arming.
            for key, value in dict(start_trigger='px4_mode', auto_takeoff=False, command_gimbal_down=False,
                                   gimbal_down_on_startup=False, initial_target_timeout=60., diagnostic_directory=directory).items():
                rospy.set_param('/aruco_landing/'+key, value)
            # Old contradictory values must be ignored, even when still on the parameter server.
            rospy.set_param('/aruco_landing/rc_channel', 99)
            rospy.set_param('/aruco_landing/rc_offboard_slot', -7)
            data.update(mode='OFFBOARD', armed=True, ground=False, z=1.5, target=None)
            velocities.clear(); requests.clear(); phases.clear()
            controller = spawn([str(land)], 'land')
            wait_for(lambda: phases, 'land controller did not start')
            time.sleep(1.6)
            assert not velocities and not requests, 'OFFBOARD startup triggered landing'
            data['mode'] = 'POSCTL'
            wait_for(lambda: len(velocities) > 45, 'land standby prestream absent')
            assert not requests and all(abs(m.velocity.z) < 1e-9 for _, m in velocities)
            data['mode'] = 'OFFBOARD'
            wait_for(lambda: phases and phases[-1] == 'ACQUIRE', 'actual OFFBOARD did not trigger airborne landing')
            assert not requests, 'manual landing requested arming or reclaimed OFFBOARD'
            data['mode'] = 'POSCTL'
            wait_for(lambda: phases[-1] == 'CANCELLED', 'PX4 mode override did not cancel landing')
            time.sleep(.1)
            data['mode'] = 'OFFBOARD'
            time.sleep(.2)
            before = len(velocities)
            time.sleep(1)
            assert len(velocities) == before and not requests, 'landing restarted on a mode bounce'
            data['mode'] = 'POSCTL'
            wait_for(lambda: len(velocities) > before+45, 'fresh POSCTL wait could not re-arm mode gate')
            data['mode'] = 'OFFBOARD'
            wait_for(lambda: phases[-1] == 'ACQUIRE', 'new deliberate mode edge did not restart landing')
            data['feed'] = False
            wait_for(lambda: phases[-1] == 'CANCELLED', 'telemetry loss did not cancel')
            time.sleep(.2)
            before = len(velocities)
            data['feed'] = True
            time.sleep(1)
            assert len(velocities) == before and not requests, 'telemetry recovery restarted landing'
            stop(controller)
            assert not requests, 'controller shutdown changed mode or armed'
            print('PRIVATE ROS integration passed: mode authority despite contradictory RC configuration, '
                  'startup lock, standby prestream, relative +1 m, 2 s hover, AUTO.LAND, '
                  'pilot override, bounce/restart interlock, manual airborne landing and telemetry-loss cancellation.')
        except Exception:
            for log in logs:
                log.flush()
            for log in Path(directory).glob('*.log'):
                print('\nLOG', log.name, '\n', log.read_text(errors='replace')[-9000:])
            raise
        finally:
            running.clear()
            for process in reversed(processes):
                stop(process)
            for log in logs:
                log.close()


if __name__ == '__main__':
    main()
