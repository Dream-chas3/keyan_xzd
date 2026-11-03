;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::StreamStatus)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'StreamStatus (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::STREAMSTATUS")
  (make-package "ORBBEC_CAMERA::STREAMSTATUS"))

(in-package "ROS")
;;//! \htmlinclude StreamStatus.msg.html


(defclass orbbec_camera::StreamStatus
  :super ros::object
  :slots (_topic_name _has_subscribers _publish_rate_hz _delay_ms_avg ))

(defmethod orbbec_camera::StreamStatus
  (:init
   (&key
    ((:topic_name __topic_name) "")
    ((:has_subscribers __has_subscribers) nil)
    ((:publish_rate_hz __publish_rate_hz) 0.0)
    ((:delay_ms_avg __delay_ms_avg) 0.0)
    )
   (send-super :init)
   (setq _topic_name (string __topic_name))
   (setq _has_subscribers __has_subscribers)
   (setq _publish_rate_hz (float __publish_rate_hz))
   (setq _delay_ms_avg (float __delay_ms_avg))
   self)
  (:topic_name
   (&optional __topic_name)
   (if __topic_name (setq _topic_name __topic_name)) _topic_name)
  (:has_subscribers
   (&optional (__has_subscribers :null))
   (if (not (eq __has_subscribers :null)) (setq _has_subscribers __has_subscribers)) _has_subscribers)
  (:publish_rate_hz
   (&optional __publish_rate_hz)
   (if __publish_rate_hz (setq _publish_rate_hz __publish_rate_hz)) _publish_rate_hz)
  (:delay_ms_avg
   (&optional __delay_ms_avg)
   (if __delay_ms_avg (setq _delay_ms_avg __delay_ms_avg)) _delay_ms_avg)
  (:serialization-length
   ()
   (+
    ;; string _topic_name
    4 (length _topic_name)
    ;; bool _has_subscribers
    1
    ;; float64 _publish_rate_hz
    8
    ;; float64 _delay_ms_avg
    8
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; string _topic_name
       (write-long (length _topic_name) s) (princ _topic_name s)
     ;; bool _has_subscribers
       (if _has_subscribers (write-byte -1 s) (write-byte 0 s))
     ;; float64 _publish_rate_hz
       (sys::poke _publish_rate_hz (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _delay_ms_avg
       (sys::poke _delay_ms_avg (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; string _topic_name
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _topic_name (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; bool _has_subscribers
     (setq _has_subscribers (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; float64 _publish_rate_hz
     (setq _publish_rate_hz (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _delay_ms_avg
     (setq _delay_ms_avg (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;;
   self)
  )

(setf (get orbbec_camera::StreamStatus :md5sum-) "921f032866beb1e18363e1b44b10b8c1")
(setf (get orbbec_camera::StreamStatus :datatype-) "orbbec_camera/StreamStatus")
(setf (get orbbec_camera::StreamStatus :definition-)
      "string topic_name
bool has_subscribers
float64 publish_rate_hz
float64 delay_ms_avg

")



(provide :orbbec_camera/StreamStatus "921f032866beb1e18363e1b44b10b8c1")


