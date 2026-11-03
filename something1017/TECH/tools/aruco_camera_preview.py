#!/usr/bin/env python3
"""On-demand, bounded JPEG preview. Subscribes to annotation only while viewed.
No flight publishers or service clients. Encoding runs outside ROS callbacks.
"""
import argparse
import json
import threading
import time
from http.server import BaseHTTPRequestHandler,ThreadingHTTPServer

class Preview:
    def __init__(self,width=640,fps=3,quality=60):
        self.width=width;self.period=1/fps;self.quality=quality;self.lock=threading.RLock()
        self.raw=None;self.sequence=0;self.encoded_sequence=-1;self.jpeg=None;self.meta={};self.requested=0;self.completed=0
        self.lease_until=0;self.subscriber=None;self.stop=threading.Event();self.attach=None
        self.thread=threading.Thread(target=self.work,daemon=True);self.thread.start()
    def receive(self,message):
        with self.lock:self.raw=(message,time.monotonic());self.sequence+=1
    def request(self):
        with self.lock:
            self.lease_until=time.monotonic()+2;self.requested+=1
            if self.subscriber is None and self.attach:self.subscriber=self.attach(self.receive)
            return self.requested
    def frame(self,ticket=None):
        with self.lock:
            if ticket is not None and self.completed<ticket:return None
            if not self.jpeg or not self.meta or time.monotonic()-self.meta['received_at']>1.2:return None
            return self.jpeg,{**self.meta,'age_ms':(time.monotonic()-self.meta['received_at'])*1000}
    def work(self):
        encoder=None;last_encode=-1
        while not self.stop.wait(.025):
            retired=None
            with self.lock:
                active=time.monotonic()<self.lease_until
                if not active:
                    retired=self.subscriber;self.subscriber=None
                    self.raw=None;self.jpeg=None
                raw=self.raw;seq=self.sequence;ticket=self.requested;completed=self.completed
            if retired:retired.unregister()
            if not active or not raw or ticket<=completed or time.monotonic()-last_encode<self.period:continue
            message,received=raw
            if time.monotonic()-received>1.2:continue
            try:
                if encoder is None:
                    import cv2
                    from cv_bridge import CvBridge
                    cv2.setNumThreads(1);encoder=(cv2,CvBridge())
                cv2,bridge=encoder;start=time.monotonic();image=bridge.imgmsg_to_cv2(message,'bgr8')
                height,width=image.shape[:2];scale=min(1,self.width/width)
                if scale<1:image=cv2.resize(image,(round(width*scale),round(height*scale)),interpolation=cv2.INTER_AREA)
                ok,encoded=cv2.imencode('.jpg',image,[cv2.IMWRITE_JPEG_QUALITY,self.quality])
                if not ok:continue
                stamp=message.header.stamp.to_sec()
                with self.lock:
                    self.jpeg=encoded.tobytes();self.encoded_sequence=seq;self.completed=ticket
                    self.meta={'received_at':received,'encode_ms':(time.monotonic()-start)*1000,
                        'width':image.shape[1],'height':image.shape[0],
                        'host_image_age_ms':max(0,(time.time()-stamp)*1000) if stamp>0 else -1,
                        'capture_latency_known':False}
                last_encode=time.monotonic()
            except Exception:
                self.stop.wait(.1)
    def close(self):
        self.stop.set();self.thread.join(timeout=3)
        if self.subscriber:self.subscriber.unregister()


def handler(preview):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self,*_):pass
        def do_GET(self):
            if self.path!='/frame.jpg':self.send_error(404);return
            ticket=preview.request();until=time.monotonic()+.8;frame=preview.frame(ticket)
            while frame is None and time.monotonic()<until:
                time.sleep(.025);frame=preview.frame(ticket)
            if frame is None:self.send_error(503,'No fresh annotation image; start the existing vision terminal');return
            body,meta=frame;self.send_response(200);self.send_header('Content-Type','image/jpeg')
            self.send_header('Content-Length',str(len(body)));self.send_header('Cache-Control','no-store')
            self.send_header('X-Frame-Age-Ms',str(round(meta['age_ms'],2)))
            self.send_header('X-Encode-Ms',str(round(meta['encode_ms'],2)))
            self.send_header('X-Capture-Latency-Known','false');self.end_headers()
            try:self.wfile.write(body)
            except (BrokenPipeError,ConnectionResetError):pass
    return Handler


def main():
    import rospy
    from sensor_msgs.msg import Image
    p=argparse.ArgumentParser();p.add_argument('--port',type=int,default=8768);p.add_argument('--width',type=int,default=640)
    p.add_argument('--fps',type=float,default=3);p.add_argument('--quality',type=int,default=60);args=p.parse_args()
    if not 160<=args.width<=960 or not .5<=args.fps<=5 or not 30<=args.quality<=85:raise ValueError('Invalid preview limits')
    rospy.init_node('aruco_camera_preview',disable_signals=True)
    preview=Preview(args.width,args.fps,args.quality)
    preview.attach=lambda callback:rospy.Subscriber('/aruco_imgresult',Image,callback,queue_size=1,buff_size=8*1024*1024,tcp_nodelay=True)
    server=ThreadingHTTPServer(('0.0.0.0',args.port),handler(preview));server.daemon_threads=True
    try:server.serve_forever(poll_interval=.2)
    except KeyboardInterrupt:pass
    finally:server.server_close();preview.close()
if __name__=='__main__':main()
