; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-msg)


;//! \htmlinclude DeviceStatus.msg.html

(cl:defclass <DeviceStatus> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (device_online
    :reader device_online
    :initarg :device_online
    :type cl:boolean
    :initform cl:nil)
   (connection_type
    :reader connection_type
    :initarg :connection_type
    :type cl:string
    :initform "")
   (customer_calibration_ready
    :reader customer_calibration_ready
    :initarg :customer_calibration_ready
    :type cl:boolean
    :initform cl:nil)
   (calibration_from_factory
    :reader calibration_from_factory
    :initarg :calibration_from_factory
    :type cl:boolean
    :initform cl:nil)
   (calibration_from_launch_param
    :reader calibration_from_launch_param
    :initarg :calibration_from_launch_param
    :type cl:boolean
    :initform cl:nil)
   (streams
    :reader streams
    :initarg :streams
    :type (cl:vector orbbec_camera-msg:StreamStatus)
   :initform (cl:make-array 0 :element-type 'orbbec_camera-msg:StreamStatus :initial-element (cl:make-instance 'orbbec_camera-msg:StreamStatus))))
)

(cl:defclass DeviceStatus (<DeviceStatus>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DeviceStatus>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DeviceStatus)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-msg:<DeviceStatus> is deprecated: use orbbec_camera-msg:DeviceStatus instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <DeviceStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:header-val is deprecated.  Use orbbec_camera-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'device_online-val :lambda-list '(m))
(cl:defmethod device_online-val ((m <DeviceStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:device_online-val is deprecated.  Use orbbec_camera-msg:device_online instead.")
  (device_online m))

(cl:ensure-generic-function 'connection_type-val :lambda-list '(m))
(cl:defmethod connection_type-val ((m <DeviceStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:connection_type-val is deprecated.  Use orbbec_camera-msg:connection_type instead.")
  (connection_type m))

(cl:ensure-generic-function 'customer_calibration_ready-val :lambda-list '(m))
(cl:defmethod customer_calibration_ready-val ((m <DeviceStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:customer_calibration_ready-val is deprecated.  Use orbbec_camera-msg:customer_calibration_ready instead.")
  (customer_calibration_ready m))

(cl:ensure-generic-function 'calibration_from_factory-val :lambda-list '(m))
(cl:defmethod calibration_from_factory-val ((m <DeviceStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:calibration_from_factory-val is deprecated.  Use orbbec_camera-msg:calibration_from_factory instead.")
  (calibration_from_factory m))

(cl:ensure-generic-function 'calibration_from_launch_param-val :lambda-list '(m))
(cl:defmethod calibration_from_launch_param-val ((m <DeviceStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:calibration_from_launch_param-val is deprecated.  Use orbbec_camera-msg:calibration_from_launch_param instead.")
  (calibration_from_launch_param m))

(cl:ensure-generic-function 'streams-val :lambda-list '(m))
(cl:defmethod streams-val ((m <DeviceStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:streams-val is deprecated.  Use orbbec_camera-msg:streams instead.")
  (streams m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DeviceStatus>) ostream)
  "Serializes a message object of type '<DeviceStatus>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'device_online) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'connection_type))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'connection_type))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'customer_calibration_ready) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'calibration_from_factory) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'calibration_from_launch_param) 1 0)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'streams))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'streams))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DeviceStatus>) istream)
  "Deserializes a message object of type '<DeviceStatus>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:slot-value msg 'device_online) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'connection_type) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'connection_type) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:slot-value msg 'customer_calibration_ready) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'calibration_from_factory) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'calibration_from_launch_param) (cl:not (cl:zerop (cl:read-byte istream))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'streams) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'streams)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'orbbec_camera-msg:StreamStatus))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DeviceStatus>)))
  "Returns string type for a message object of type '<DeviceStatus>"
  "orbbec_camera/DeviceStatus")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DeviceStatus)))
  "Returns string type for a message object of type 'DeviceStatus"
  "orbbec_camera/DeviceStatus")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DeviceStatus>)))
  "Returns md5sum for a message object of type '<DeviceStatus>"
  "c5afe24864ffc8299459142b55ca2268")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DeviceStatus)))
  "Returns md5sum for a message object of type 'DeviceStatus"
  "c5afe24864ffc8299459142b55ca2268")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DeviceStatus>)))
  "Returns full string definition for message of type '<DeviceStatus>"
  (cl:format cl:nil "std_msgs/Header header~%bool device_online~%string connection_type~%bool customer_calibration_ready~%bool calibration_from_factory~%bool calibration_from_launch_param~%orbbec_camera/StreamStatus[] streams~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: orbbec_camera/StreamStatus~%string topic_name~%bool has_subscribers~%float64 publish_rate_hz~%float64 delay_ms_avg~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DeviceStatus)))
  "Returns full string definition for message of type 'DeviceStatus"
  (cl:format cl:nil "std_msgs/Header header~%bool device_online~%string connection_type~%bool customer_calibration_ready~%bool calibration_from_factory~%bool calibration_from_launch_param~%orbbec_camera/StreamStatus[] streams~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: orbbec_camera/StreamStatus~%string topic_name~%bool has_subscribers~%float64 publish_rate_hz~%float64 delay_ms_avg~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DeviceStatus>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     1
     4 (cl:length (cl:slot-value msg 'connection_type))
     1
     1
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'streams) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DeviceStatus>))
  "Converts a ROS message object to a list"
  (cl:list 'DeviceStatus
    (cl:cons ':header (header msg))
    (cl:cons ':device_online (device_online msg))
    (cl:cons ':connection_type (connection_type msg))
    (cl:cons ':customer_calibration_ready (customer_calibration_ready msg))
    (cl:cons ':calibration_from_factory (calibration_from_factory msg))
    (cl:cons ':calibration_from_launch_param (calibration_from_launch_param msg))
    (cl:cons ':streams (streams msg))
))
