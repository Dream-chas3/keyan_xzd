; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-msg)


;//! \htmlinclude StreamStatus.msg.html

(cl:defclass <StreamStatus> (roslisp-msg-protocol:ros-message)
  ((topic_name
    :reader topic_name
    :initarg :topic_name
    :type cl:string
    :initform "")
   (has_subscribers
    :reader has_subscribers
    :initarg :has_subscribers
    :type cl:boolean
    :initform cl:nil)
   (publish_rate_hz
    :reader publish_rate_hz
    :initarg :publish_rate_hz
    :type cl:float
    :initform 0.0)
   (delay_ms_avg
    :reader delay_ms_avg
    :initarg :delay_ms_avg
    :type cl:float
    :initform 0.0))
)

(cl:defclass StreamStatus (<StreamStatus>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <StreamStatus>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'StreamStatus)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-msg:<StreamStatus> is deprecated: use orbbec_camera-msg:StreamStatus instead.")))

(cl:ensure-generic-function 'topic_name-val :lambda-list '(m))
(cl:defmethod topic_name-val ((m <StreamStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:topic_name-val is deprecated.  Use orbbec_camera-msg:topic_name instead.")
  (topic_name m))

(cl:ensure-generic-function 'has_subscribers-val :lambda-list '(m))
(cl:defmethod has_subscribers-val ((m <StreamStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:has_subscribers-val is deprecated.  Use orbbec_camera-msg:has_subscribers instead.")
  (has_subscribers m))

(cl:ensure-generic-function 'publish_rate_hz-val :lambda-list '(m))
(cl:defmethod publish_rate_hz-val ((m <StreamStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:publish_rate_hz-val is deprecated.  Use orbbec_camera-msg:publish_rate_hz instead.")
  (publish_rate_hz m))

(cl:ensure-generic-function 'delay_ms_avg-val :lambda-list '(m))
(cl:defmethod delay_ms_avg-val ((m <StreamStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:delay_ms_avg-val is deprecated.  Use orbbec_camera-msg:delay_ms_avg instead.")
  (delay_ms_avg m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <StreamStatus>) ostream)
  "Serializes a message object of type '<StreamStatus>"
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'topic_name))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'topic_name))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'has_subscribers) 1 0)) ostream)
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'publish_rate_hz))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'delay_ms_avg))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <StreamStatus>) istream)
  "Deserializes a message object of type '<StreamStatus>"
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'topic_name) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'topic_name) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:slot-value msg 'has_subscribers) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'publish_rate_hz) (roslisp-utils:decode-double-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'delay_ms_avg) (roslisp-utils:decode-double-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<StreamStatus>)))
  "Returns string type for a message object of type '<StreamStatus>"
  "orbbec_camera/StreamStatus")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'StreamStatus)))
  "Returns string type for a message object of type 'StreamStatus"
  "orbbec_camera/StreamStatus")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<StreamStatus>)))
  "Returns md5sum for a message object of type '<StreamStatus>"
  "921f032866beb1e18363e1b44b10b8c1")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'StreamStatus)))
  "Returns md5sum for a message object of type 'StreamStatus"
  "921f032866beb1e18363e1b44b10b8c1")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<StreamStatus>)))
  "Returns full string definition for message of type '<StreamStatus>"
  (cl:format cl:nil "string topic_name~%bool has_subscribers~%float64 publish_rate_hz~%float64 delay_ms_avg~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'StreamStatus)))
  "Returns full string definition for message of type 'StreamStatus"
  (cl:format cl:nil "string topic_name~%bool has_subscribers~%float64 publish_rate_hz~%float64 delay_ms_avg~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <StreamStatus>))
  (cl:+ 0
     4 (cl:length (cl:slot-value msg 'topic_name))
     1
     8
     8
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <StreamStatus>))
  "Converts a ROS message object to a list"
  (cl:list 'StreamStatus
    (cl:cons ':topic_name (topic_name msg))
    (cl:cons ':has_subscribers (has_subscribers msg))
    (cl:cons ':publish_rate_hz (publish_rate_hz msg))
    (cl:cons ':delay_ms_avg (delay_ms_avg msg))
))
