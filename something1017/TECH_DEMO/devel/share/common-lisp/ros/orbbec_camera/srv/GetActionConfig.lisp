; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-srv)


;//! \htmlinclude GetActionConfig-request.msg.html

(cl:defclass <GetActionConfig-request> (roslisp-msg-protocol:ros-message)
  ((selector
    :reader selector
    :initarg :selector
    :type cl:integer
    :initform 0))
)

(cl:defclass GetActionConfig-request (<GetActionConfig-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GetActionConfig-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GetActionConfig-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<GetActionConfig-request> is deprecated: use orbbec_camera-srv:GetActionConfig-request instead.")))

(cl:ensure-generic-function 'selector-val :lambda-list '(m))
(cl:defmethod selector-val ((m <GetActionConfig-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:selector-val is deprecated.  Use orbbec_camera-srv:selector instead.")
  (selector m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GetActionConfig-request>) ostream)
  "Serializes a message object of type '<GetActionConfig-request>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'selector)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'selector)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'selector)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'selector)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GetActionConfig-request>) istream)
  "Deserializes a message object of type '<GetActionConfig-request>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'selector)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'selector)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'selector)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'selector)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GetActionConfig-request>)))
  "Returns string type for a service object of type '<GetActionConfig-request>"
  "orbbec_camera/GetActionConfigRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GetActionConfig-request)))
  "Returns string type for a service object of type 'GetActionConfig-request"
  "orbbec_camera/GetActionConfigRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GetActionConfig-request>)))
  "Returns md5sum for a message object of type '<GetActionConfig-request>"
  "b599f0e1250bf719f4e715cab6e1e366")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GetActionConfig-request)))
  "Returns md5sum for a message object of type 'GetActionConfig-request"
  "b599f0e1250bf719f4e715cab6e1e366")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GetActionConfig-request>)))
  "Returns full string definition for message of type '<GetActionConfig-request>"
  (cl:format cl:nil "uint32 selector~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GetActionConfig-request)))
  "Returns full string definition for message of type 'GetActionConfig-request"
  (cl:format cl:nil "uint32 selector~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GetActionConfig-request>))
  (cl:+ 0
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GetActionConfig-request>))
  "Converts a ROS message object to a list"
  (cl:list 'GetActionConfig-request
    (cl:cons ':selector (selector msg))
))
;//! \htmlinclude GetActionConfig-response.msg.html

(cl:defclass <GetActionConfig-response> (roslisp-msg-protocol:ros-message)
  ((action_signal_count
    :reader action_signal_count
    :initarg :action_signal_count
    :type cl:integer
    :initform 0)
   (device_key
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
   (success
    :reader success
    :initarg :success
    :type cl:boolean
    :initform cl:nil)
   (message
    :reader message
    :initarg :message
    :type cl:string
    :initform ""))
)

(cl:defclass GetActionConfig-response (<GetActionConfig-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GetActionConfig-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GetActionConfig-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<GetActionConfig-response> is deprecated: use orbbec_camera-srv:GetActionConfig-response instead.")))

(cl:ensure-generic-function 'action_signal_count-val :lambda-list '(m))
(cl:defmethod action_signal_count-val ((m <GetActionConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:action_signal_count-val is deprecated.  Use orbbec_camera-srv:action_signal_count instead.")
  (action_signal_count m))

(cl:ensure-generic-function 'device_key-val :lambda-list '(m))
(cl:defmethod device_key-val ((m <GetActionConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:device_key-val is deprecated.  Use orbbec_camera-srv:device_key instead.")
  (device_key m))

(cl:ensure-generic-function 'group_key-val :lambda-list '(m))
(cl:defmethod group_key-val ((m <GetActionConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:group_key-val is deprecated.  Use orbbec_camera-srv:group_key instead.")
  (group_key m))

(cl:ensure-generic-function 'group_mask-val :lambda-list '(m))
(cl:defmethod group_mask-val ((m <GetActionConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:group_mask-val is deprecated.  Use orbbec_camera-srv:group_mask instead.")
  (group_mask m))

(cl:ensure-generic-function 'success-val :lambda-list '(m))
(cl:defmethod success-val ((m <GetActionConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:success-val is deprecated.  Use orbbec_camera-srv:success instead.")
  (success m))

(cl:ensure-generic-function 'message-val :lambda-list '(m))
(cl:defmethod message-val ((m <GetActionConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:message-val is deprecated.  Use orbbec_camera-srv:message instead.")
  (message m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GetActionConfig-response>) ostream)
  "Serializes a message object of type '<GetActionConfig-response>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'action_signal_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'action_signal_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'action_signal_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'action_signal_count)) ostream)
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
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'success) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'message))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'message))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GetActionConfig-response>) istream)
  "Deserializes a message object of type '<GetActionConfig-response>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'action_signal_count)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'action_signal_count)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'action_signal_count)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'action_signal_count)) (cl:read-byte istream))
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
    (cl:setf (cl:slot-value msg 'success) (cl:not (cl:zerop (cl:read-byte istream))))
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
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GetActionConfig-response>)))
  "Returns string type for a service object of type '<GetActionConfig-response>"
  "orbbec_camera/GetActionConfigResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GetActionConfig-response)))
  "Returns string type for a service object of type 'GetActionConfig-response"
  "orbbec_camera/GetActionConfigResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GetActionConfig-response>)))
  "Returns md5sum for a message object of type '<GetActionConfig-response>"
  "b599f0e1250bf719f4e715cab6e1e366")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GetActionConfig-response)))
  "Returns md5sum for a message object of type 'GetActionConfig-response"
  "b599f0e1250bf719f4e715cab6e1e366")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GetActionConfig-response>)))
  "Returns full string definition for message of type '<GetActionConfig-response>"
  (cl:format cl:nil "uint32 action_signal_count~%uint32 device_key~%uint32 group_key~%uint32 group_mask~%bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GetActionConfig-response)))
  "Returns full string definition for message of type 'GetActionConfig-response"
  (cl:format cl:nil "uint32 action_signal_count~%uint32 device_key~%uint32 group_key~%uint32 group_mask~%bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GetActionConfig-response>))
  (cl:+ 0
     4
     4
     4
     4
     1
     4 (cl:length (cl:slot-value msg 'message))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GetActionConfig-response>))
  "Converts a ROS message object to a list"
  (cl:list 'GetActionConfig-response
    (cl:cons ':action_signal_count (action_signal_count msg))
    (cl:cons ':device_key (device_key msg))
    (cl:cons ':group_key (group_key msg))
    (cl:cons ':group_mask (group_mask msg))
    (cl:cons ':success (success msg))
    (cl:cons ':message (message msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'GetActionConfig)))
  'GetActionConfig-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'GetActionConfig)))
  'GetActionConfig-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GetActionConfig)))
  "Returns string type for a service object of type '<GetActionConfig>"
  "orbbec_camera/GetActionConfig")