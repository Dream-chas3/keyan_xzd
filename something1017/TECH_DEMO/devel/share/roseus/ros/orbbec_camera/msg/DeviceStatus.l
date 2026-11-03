;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::DeviceStatus)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'DeviceStatus (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::DEVICESTATUS")
  (make-package "ORBBEC_CAMERA::DEVICESTATUS"))

(in-package "ROS")
;;//! \htmlinclude DeviceStatus.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass orbbec_camera::DeviceStatus
  :super ros::object
  :slots (_header _device_online _connection_type _customer_calibration_ready _calibration_from_factory _calibration_from_launch_param _streams ))

(defmethod orbbec_camera::DeviceStatus
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:device_online __device_online) nil)
    ((:connection_type __connection_type) "")
    ((:customer_calibration_ready __customer_calibration_ready) nil)
    ((:calibration_from_factory __calibration_from_factory) nil)
    ((:calibration_from_launch_param __calibration_from_launch_param) nil)
    ((:streams __streams) ())
    )
   (send-super :init)
   (setq _header __header)
   (setq _device_online __device_online)
   (setq _connection_type (string __connection_type))
   (setq _customer_calibration_ready __customer_calibration_ready)
   (setq _calibration_from_factory __calibration_from_factory)
   (setq _calibration_from_launch_param __calibration_from_launch_param)
   (setq _streams __streams)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:device_online
   (&optional (__device_online :null))
   (if (not (eq __device_online :null)) (setq _device_online __device_online)) _device_online)
  (:connection_type
   (&optional __connection_type)
   (if __connection_type (setq _connection_type __connection_type)) _connection_type)
  (:customer_calibration_ready
   (&optional (__customer_calibration_ready :null))
   (if (not (eq __customer_calibration_ready :null)) (setq _customer_calibration_ready __customer_calibration_ready)) _customer_calibration_ready)
  (:calibration_from_factory
   (&optional (__calibration_from_factory :null))
   (if (not (eq __calibration_from_factory :null)) (setq _calibration_from_factory __calibration_from_factory)) _calibration_from_factory)
  (:calibration_from_launch_param
   (&optional (__calibration_from_launch_param :null))
   (if (not (eq __calibration_from_launch_param :null)) (setq _calibration_from_launch_param __calibration_from_launch_param)) _calibration_from_launch_param)
  (:streams
   (&rest __streams)
   (if (keywordp (car __streams))
       (send* _streams __streams)
     (progn
       (if __streams (setq _streams (car __streams)))
       _streams)))
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; bool _device_online
    1
    ;; string _connection_type
    4 (length _connection_type)
    ;; bool _customer_calibration_ready
    1
    ;; bool _calibration_from_factory
    1
    ;; bool _calibration_from_launch_param
    1
    ;; orbbec_camera/StreamStatus[] _streams
    (apply #'+ (send-all _streams :serialization-length)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; bool _device_online
       (if _device_online (write-byte -1 s) (write-byte 0 s))
     ;; string _connection_type
       (write-long (length _connection_type) s) (princ _connection_type s)
     ;; bool _customer_calibration_ready
       (if _customer_calibration_ready (write-byte -1 s) (write-byte 0 s))
     ;; bool _calibration_from_factory
       (if _calibration_from_factory (write-byte -1 s) (write-byte 0 s))
     ;; bool _calibration_from_launch_param
       (if _calibration_from_launch_param (write-byte -1 s) (write-byte 0 s))
     ;; orbbec_camera/StreamStatus[] _streams
     (write-long (length _streams) s)
     (dolist (elem _streams)
       (send elem :serialize s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; bool _device_online
     (setq _device_online (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _connection_type
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _connection_type (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; bool _customer_calibration_ready
     (setq _customer_calibration_ready (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _calibration_from_factory
     (setq _calibration_from_factory (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _calibration_from_launch_param
     (setq _calibration_from_launch_param (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; orbbec_camera/StreamStatus[] _streams
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _streams (let (r) (dotimes (i n) (push (instance orbbec_camera::StreamStatus :init) r)) r))
     (dolist (elem- _streams)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;;
   self)
  )

(setf (get orbbec_camera::DeviceStatus :md5sum-) "c5afe24864ffc8299459142b55ca2268")
(setf (get orbbec_camera::DeviceStatus :datatype-) "orbbec_camera/DeviceStatus")
(setf (get orbbec_camera::DeviceStatus :definition-)
      "std_msgs/Header header
bool device_online
string connection_type
bool customer_calibration_ready
bool calibration_from_factory
bool calibration_from_launch_param
orbbec_camera/StreamStatus[] streams

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
string frame_id

================================================================================
MSG: orbbec_camera/StreamStatus
string topic_name
bool has_subscribers
float64 publish_rate_hz
float64 delay_ms_avg

")



(provide :orbbec_camera/DeviceStatus "c5afe24864ffc8299459142b55ca2268")


