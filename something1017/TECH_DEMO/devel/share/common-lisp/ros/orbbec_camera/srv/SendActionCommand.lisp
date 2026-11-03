; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-srv)


;//! \htmlinclude SendActionCommand-request.msg.html

(cl:defclass <SendActionCommand-request> (roslisp-msg-protocol:ros-message)
  ((device_key
    :reader device_key
    :initarg :device_key
    :type cl:integer
    :initform 0)
   (group_key
    :reader group_key
    :initarg :group_key
    :type cl:integer
    :initform 0)
   (group_mask
    :reader group_mask
    :initarg :group_mask
    :type cl:integer
    :initform 0)
   (broadcast_ip
    :reader broadcast_ip
    :initarg :broadcast_ip
    :type cl:string
    :initform "")
   (trigger_mode
    :reader trigger_mode
    :initarg :trigger_mode
    :type cl:fixnum
    :initform 0)
   (delay_ms
    :reader delay_ms
    :initarg :delay_ms
    :type cl:integer
    :initform 0)
   (scheduled_time
    :reader scheduled_time
    :initarg :scheduled_time
    :type cl:integer
    :initform 0))
)

(cl:defclass SendActionCommand-request (<SendActionCommand-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SendActionCommand-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SendActionCommand-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<SendActionCommand-request> is deprecated: use orbbec_camera-srv:SendActionCommand-request instead.")))

(cl:ensure-generic-function 'device_key-val :lambda-list '(m))
(cl:defmethod device_key-val ((m <SendActionCommand-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:device_key-val is deprecated.  Use orbbec_camera-srv:device_key instead.")
  (device_key m))

(cl:ensure-generic-function 'group_key-val :lambda-list '(m))
(cl:defmethod group_key-val ((m <SendActionCommand-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:group_key-val is deprecated.  Use orbbec_camera-srv:group_key instead.")
  (group_key m))

(cl:ensure-generic-function 'group_mask-val :lambda-list '(m))
(cl:defmethod group_mask-val ((m <SendActionCommand-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:group_mask-val is deprecated.  Use orbbec_camera-srv:group_mask instead.")
  (group_mask m))

(cl:ensure-generic-function 'broadcast_ip-val :lambda-list '(m))
(cl:defmethod broadcast_ip-val ((m <SendActionCommand-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:broadcast_ip-val is deprecated.  Use orbbec_camera-srv:broadcast_ip instead.")
  (broadcast_ip m))

(cl:ensure-generic-function 'trigger_mode-val :lambda-list '(m))
(cl:defmethod trigger_mode-val ((m <SendActionCommand-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:trigger_mode-val is deprecated.  Use orbbec_camera-srv:trigger_mode instead.")
  (trigger_mode m))

(cl:ensure-generic-function 'delay_ms-val :lambda-list '(m))
(cl:defmethod delay_ms-val ((m <SendActionCommand-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:delay_ms-val is deprecated.  Use orbbec_camera-srv:delay_ms instead.")
  (delay_ms m))

(cl:ensure-generic-function 'scheduled_time-val :lambda-list '(m))
(cl:defmethod scheduled_time-val ((m <SendActionCommand-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:scheduled_time-val is deprecated.  Use orbbec_camera-srv:scheduled_time instead.")
  (scheduled_time m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<SendActionCommand-request>)))
    "Constants for message type '<SendActionCommand-request>"
  '((:TRIGGER_MODE_IMMEDIATE . 0)
    (:TRIGGER_MODE_DELAYED . 1)
    (:TRIGGER_MODE_TIMESTAMP . 2))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'SendActionCommand-request)))
    "Constants for message type 'SendActionCommand-request"
  '((:TRIGGER_MODE_IMMEDIATE . 0)
    (:TRIGGER_MODE_DELAYED . 1)
    (:TRIGGER_MODE_TIMESTAMP . 2))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SendActionCommand-request>) ostream)
  "Serializes a message object of type '<SendActionCommand-request>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'device_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'device_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'device_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'device_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'group_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'group_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'group_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_mask)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'group_mask)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'group_mask)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'group_mask)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'broadcast_ip))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'broadcast_ip))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'trigger_mode)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'delay_ms)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'delay_ms)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'delay_ms)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'delay_ms)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 32) (cl:slot-value msg 'scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 40) (cl:slot-value msg 'scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 48) (cl:slot-value msg 'scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 56) (cl:slot-value msg 'scheduled_time)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SendActionCommand-request>) istream)
  "Deserializes a message object of type '<SendActionCommand-request>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'device_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'device_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'device_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'device_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'group_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'group_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'group_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_mask)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'group_mask)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'group_mask)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'group_mask)) (cl:read-byte istream))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'broadcast_ip) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'broadcast_ip) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'trigger_mode)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'delay_ms)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'delay_ms)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'delay_ms)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'delay_ms)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 32) (cl:slot-value msg 'scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 40) (cl:slot-value msg 'scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 48) (cl:slot-value msg 'scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 56) (cl:slot-value msg 'scheduled_time)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SendActionCommand-request>)))
  "Returns string type for a service object of type '<SendActionCommand-request>"
  "orbbec_camera/SendActionCommandRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SendActionCommand-request)))
  "Returns string type for a service object of type 'SendActionCommand-request"
  "orbbec_camera/SendActionCommandRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SendActionCommand-request>)))
  "Returns md5sum for a message object of type '<SendActionCommand-request>"
  "c725e105b4f2d4dfe7d74a3ac74cd5d7")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SendActionCommand-request)))
  "Returns md5sum for a message object of type 'SendActionCommand-request"
  "c725e105b4f2d4dfe7d74a3ac74cd5d7")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SendActionCommand-request>)))
  "Returns full string definition for message of type '<SendActionCommand-request>"
  (cl:format cl:nil "uint8 TRIGGER_MODE_IMMEDIATE=0~%uint8 TRIGGER_MODE_DELAYED=1~%uint8 TRIGGER_MODE_TIMESTAMP=2~%~%uint32 device_key~%uint32 group_key~%uint32 group_mask~%# IPv4 destination. Leave empty to use the SDK broadcast address (255.255.255.255).~%string broadcast_ip~%# Select immediate, relative-delay, or absolute-PTP-time triggering.~%uint8 trigger_mode~%# Used only with TRIGGER_MODE_DELAYED. Must be positive.~%uint32 delay_ms~%# Used only with TRIGGER_MODE_TIMESTAMP. Upper 32 bits are seconds and lower 32 bits are nanoseconds.~%uint64 scheduled_time~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SendActionCommand-request)))
  "Returns full string definition for message of type 'SendActionCommand-request"
  (cl:format cl:nil "uint8 TRIGGER_MODE_IMMEDIATE=0~%uint8 TRIGGER_MODE_DELAYED=1~%uint8 TRIGGER_MODE_TIMESTAMP=2~%~%uint32 device_key~%uint32 group_key~%uint32 group_mask~%# IPv4 destination. Leave empty to use the SDK broadcast address (255.255.255.255).~%string broadcast_ip~%# Select immediate, relative-delay, or absolute-PTP-time triggering.~%uint8 trigger_mode~%# Used only with TRIGGER_MODE_DELAYED. Must be positive.~%uint32 delay_ms~%# Used only with TRIGGER_MODE_TIMESTAMP. Upper 32 bits are seconds and lower 32 bits are nanoseconds.~%uint64 scheduled_time~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SendActionCommand-request>))
  (cl:+ 0
     4
     4
     4
     4 (cl:length (cl:slot-value msg 'broadcast_ip))
     1
     4
     8
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SendActionCommand-request>))
  "Converts a ROS message object to a list"
  (cl:list 'SendActionCommand-request
    (cl:cons ':device_key (device_key msg))
    (cl:cons ':group_key (group_key msg))
    (cl:cons ':group_mask (group_mask msg))
    (cl:cons ':broadcast_ip (broadcast_ip msg))
    (cl:cons ':trigger_mode (trigger_mode msg))
    (cl:cons ':delay_ms (delay_ms msg))
    (cl:cons ':scheduled_time (scheduled_time msg))
))
;//! \htmlinclude SendActionCommand-response.msg.html

(cl:defclass <SendActionCommand-response> (roslisp-msg-protocol:ros-message)
  ((success
    :reader success
    :initarg :success
    :type cl:boolean
    :initform cl:nil)
   (encoded_scheduled_time
    :reader encoded_scheduled_time
    :initarg :encoded_scheduled_time
    :type cl:integer
    :initform 0)
   (message
    :reader message
    :initarg :message
    :type cl:string
    :initform ""))
)

(cl:defclass SendActionCommand-response (<SendActionCommand-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SendActionCommand-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SendActionCommand-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<SendActionCommand-response> is deprecated: use orbbec_camera-srv:SendActionCommand-response instead.")))

(cl:ensure-generic-function 'success-val :lambda-list '(m))
(cl:defmethod success-val ((m <SendActionCommand-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:success-val is deprecated.  Use orbbec_camera-srv:success instead.")
  (success m))

(cl:ensure-generic-function 'encoded_scheduled_time-val :lambda-list '(m))
(cl:defmethod encoded_scheduled_time-val ((m <SendActionCommand-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:encoded_scheduled_time-val is deprecated.  Use orbbec_camera-srv:encoded_scheduled_time instead.")
  (encoded_scheduled_time m))

(cl:ensure-generic-function 'message-val :lambda-list '(m))
(cl:defmethod message-val ((m <SendActionCommand-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:message-val is deprecated.  Use orbbec_camera-srv:message instead.")
  (message m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SendActionCommand-response>) ostream)
  "Serializes a message object of type '<SendActionCommand-response>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'success) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'encoded_scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'encoded_scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'encoded_scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'encoded_scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 32) (cl:slot-value msg 'encoded_scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 40) (cl:slot-value msg 'encoded_scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 48) (cl:slot-value msg 'encoded_scheduled_time)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 56) (cl:slot-value msg 'encoded_scheduled_time)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'message))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'message))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SendActionCommand-response>) istream)
  "Deserializes a message object of type '<SendActionCommand-response>"
    (cl:setf (cl:slot-value msg 'success) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'encoded_scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'encoded_scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'encoded_scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'encoded_scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 32) (cl:slot-value msg 'encoded_scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 40) (cl:slot-value msg 'encoded_scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 48) (cl:slot-value msg 'encoded_scheduled_time)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 56) (cl:slot-value msg 'encoded_scheduled_time)) (cl:read-byte istream))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'message) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'message) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SendActionCommand-response>)))
  "Returns string type for a service object of type '<SendActionCommand-response>"
  "orbbec_camera/SendActionCommandResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SendActionCommand-response)))
  "Returns string type for a service object of type 'SendActionCommand-response"
  "orbbec_camera/SendActionCommandResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SendActionCommand-response>)))
  "Returns md5sum for a message object of type '<SendActionCommand-response>"
  "c725e105b4f2d4dfe7d74a3ac74cd5d7")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SendActionCommand-response)))
  "Returns md5sum for a message object of type 'SendActionCommand-response"
  "c725e105b4f2d4dfe7d74a3ac74cd5d7")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SendActionCommand-response>)))
  "Returns full string definition for message of type '<SendActionCommand-response>"
  (cl:format cl:nil "# Success means that the SDK dispatched the command; Action Command has no device acknowledgment.~%bool success~%# Exact GVCP/PTP value sent to the SDK. Zero is used for an immediate command.~%uint64 encoded_scheduled_time~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SendActionCommand-response)))
  "Returns full string definition for message of type 'SendActionCommand-response"
  (cl:format cl:nil "# Success means that the SDK dispatched the command; Action Command has no device acknowledgment.~%bool success~%# Exact GVCP/PTP value sent to the SDK. Zero is used for an immediate command.~%uint64 encoded_scheduled_time~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SendActionCommand-response>))
  (cl:+ 0
     1
     8
     4 (cl:length (cl:slot-value msg 'message))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SendActionCommand-response>))
  "Converts a ROS message object to a list"
  (cl:list 'SendActionCommand-response
    (cl:cons ':success (success msg))
    (cl:cons ':encoded_scheduled_time (encoded_scheduled_time msg))
    (cl:cons ':message (message msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'SendActionCommand)))
  'SendActionCommand-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'SendActionCommand)))
  'SendActionCommand-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SendActionCommand)))
  "Returns string type for a service object of type '<SendActionCommand>"
  "orbbec_camera/SendActionCommand")