; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-srv)


;//! \htmlinclude SetActionConfig-request.msg.html

(cl:defclass <SetActionConfig-request> (roslisp-msg-protocol:ros-message)
  ((device_key
    :reader device_key
    :initarg :device_key
    :type cl:integer
    :initform 0)
   (selector
    :reader selector
    :initarg :selector
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
    :initform 0))
)

(cl:defclass SetActionConfig-request (<SetActionConfig-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SetActionConfig-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SetActionConfig-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<SetActionConfig-request> is deprecated: use orbbec_camera-srv:SetActionConfig-request instead.")))

(cl:ensure-generic-function 'device_key-val :lambda-list '(m))
(cl:defmethod device_key-val ((m <SetActionConfig-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:device_key-val is deprecated.  Use orbbec_camera-srv:device_key instead.")
  (device_key m))

(cl:ensure-generic-function 'selector-val :lambda-list '(m))
(cl:defmethod selector-val ((m <SetActionConfig-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:selector-val is deprecated.  Use orbbec_camera-srv:selector instead.")
  (selector m))

(cl:ensure-generic-function 'group_key-val :lambda-list '(m))
(cl:defmethod group_key-val ((m <SetActionConfig-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:group_key-val is deprecated.  Use orbbec_camera-srv:group_key instead.")
  (group_key m))

(cl:ensure-generic-function 'group_mask-val :lambda-list '(m))
(cl:defmethod group_mask-val ((m <SetActionConfig-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:group_mask-val is deprecated.  Use orbbec_camera-srv:group_mask instead.")
  (group_mask m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SetActionConfig-request>) ostream)
  "Serializes a message object of type '<SetActionConfig-request>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'device_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'device_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'device_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'device_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'selector)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'selector)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'selector)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'selector)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'group_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'group_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'group_key)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_mask)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'group_mask)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'group_mask)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'group_mask)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SetActionConfig-request>) istream)
  "Deserializes a message object of type '<SetActionConfig-request>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'device_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'device_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'device_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'device_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'selector)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'selector)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'selector)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'selector)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'group_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'group_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'group_key)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_mask)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'group_mask)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'group_mask)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'group_mask)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SetActionConfig-request>)))
  "Returns string type for a service object of type '<SetActionConfig-request>"
  "orbbec_camera/SetActionConfigRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SetActionConfig-request)))
  "Returns string type for a service object of type 'SetActionConfig-request"
  "orbbec_camera/SetActionConfigRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SetActionConfig-request>)))
  "Returns md5sum for a message object of type '<SetActionConfig-request>"
  "6df9d533c9f8d312e2eb4484d8d2f98b")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SetActionConfig-request)))
  "Returns md5sum for a message object of type 'SetActionConfig-request"
  "6df9d533c9f8d312e2eb4484d8d2f98b")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SetActionConfig-request>)))
  "Returns full string definition for message of type '<SetActionConfig-request>"
  (cl:format cl:nil "uint32 device_key~%uint32 selector~%uint32 group_key~%uint32 group_mask~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SetActionConfig-request)))
  "Returns full string definition for message of type 'SetActionConfig-request"
  (cl:format cl:nil "uint32 device_key~%uint32 selector~%uint32 group_key~%uint32 group_mask~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SetActionConfig-request>))
  (cl:+ 0
     4
     4
     4
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SetActionConfig-request>))
  "Converts a ROS message object to a list"
  (cl:list 'SetActionConfig-request
    (cl:cons ':device_key (device_key msg))
    (cl:cons ':selector (selector msg))
    (cl:cons ':group_key (group_key msg))
    (cl:cons ':group_mask (group_mask msg))
))
;//! \htmlinclude SetActionConfig-response.msg.html

(cl:defclass <SetActionConfig-response> (roslisp-msg-protocol:ros-message)
  ((success
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

(cl:defclass SetActionConfig-response (<SetActionConfig-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SetActionConfig-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SetActionConfig-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<SetActionConfig-response> is deprecated: use orbbec_camera-srv:SetActionConfig-response instead.")))

(cl:ensure-generic-function 'success-val :lambda-list '(m))
(cl:defmethod success-val ((m <SetActionConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:success-val is deprecated.  Use orbbec_camera-srv:success instead.")
  (success m))

(cl:ensure-generic-function 'message-val :lambda-list '(m))
(cl:defmethod message-val ((m <SetActionConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:message-val is deprecated.  Use orbbec_camera-srv:message instead.")
  (message m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SetActionConfig-response>) ostream)
  "Serializes a message object of type '<SetActionConfig-response>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'success) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'message))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'message))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SetActionConfig-response>) istream)
  "Deserializes a message object of type '<SetActionConfig-response>"
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
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SetActionConfig-response>)))
  "Returns string type for a service object of type '<SetActionConfig-response>"
  "orbbec_camera/SetActionConfigResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SetActionConfig-response)))
  "Returns string type for a service object of type 'SetActionConfig-response"
  "orbbec_camera/SetActionConfigResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SetActionConfig-response>)))
  "Returns md5sum for a message object of type '<SetActionConfig-response>"
  "6df9d533c9f8d312e2eb4484d8d2f98b")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SetActionConfig-response)))
  "Returns md5sum for a message object of type 'SetActionConfig-response"
  "6df9d533c9f8d312e2eb4484d8d2f98b")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SetActionConfig-response>)))
  "Returns full string definition for message of type '<SetActionConfig-response>"
  (cl:format cl:nil "bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SetActionConfig-response)))
  "Returns full string definition for message of type 'SetActionConfig-response"
  (cl:format cl:nil "bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SetActionConfig-response>))
  (cl:+ 0
     1
     4 (cl:length (cl:slot-value msg 'message))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SetActionConfig-response>))
  "Converts a ROS message object to a list"
  (cl:list 'SetActionConfig-response
    (cl:cons ':success (success msg))
    (cl:cons ':message (message msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'SetActionConfig)))
  'SetActionConfig-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'SetActionConfig)))
  'SetActionConfig-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SetActionConfig)))
  "Returns string type for a service object of type '<SetActionConfig>"
  "orbbec_camera/SetActionConfig")