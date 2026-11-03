; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-srv)


;//! \htmlinclude SetAwbGain-request.msg.html

(cl:defclass <SetAwbGain-request> (roslisp-msg-protocol:ros-message)
  ((r_gain
    :reader r_gain
    :initarg :r_gain
    :type cl:fixnum
    :initform 0)
   (b_gain
    :reader b_gain
    :initarg :b_gain
    :type cl:fixnum
    :initform 0)
   (g_gain
    :reader g_gain
    :initarg :g_gain
    :type cl:fixnum
    :initform 0))
)

(cl:defclass SetAwbGain-request (<SetAwbGain-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SetAwbGain-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SetAwbGain-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<SetAwbGain-request> is deprecated: use orbbec_camera-srv:SetAwbGain-request instead.")))

(cl:ensure-generic-function 'r_gain-val :lambda-list '(m))
(cl:defmethod r_gain-val ((m <SetAwbGain-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:r_gain-val is deprecated.  Use orbbec_camera-srv:r_gain instead.")
  (r_gain m))

(cl:ensure-generic-function 'b_gain-val :lambda-list '(m))
(cl:defmethod b_gain-val ((m <SetAwbGain-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:b_gain-val is deprecated.  Use orbbec_camera-srv:b_gain instead.")
  (b_gain m))

(cl:ensure-generic-function 'g_gain-val :lambda-list '(m))
(cl:defmethod g_gain-val ((m <SetAwbGain-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:g_gain-val is deprecated.  Use orbbec_camera-srv:g_gain instead.")
  (g_gain m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SetAwbGain-request>) ostream)
  "Serializes a message object of type '<SetAwbGain-request>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'r_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'r_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'b_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'b_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'g_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'g_gain)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SetAwbGain-request>) istream)
  "Deserializes a message object of type '<SetAwbGain-request>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'r_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'r_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'b_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'b_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'g_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'g_gain)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SetAwbGain-request>)))
  "Returns string type for a service object of type '<SetAwbGain-request>"
  "orbbec_camera/SetAwbGainRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SetAwbGain-request)))
  "Returns string type for a service object of type 'SetAwbGain-request"
  "orbbec_camera/SetAwbGainRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SetAwbGain-request>)))
  "Returns md5sum for a message object of type '<SetAwbGain-request>"
  "78d4a958be234c52a0c5e570d6402ff4")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SetAwbGain-request)))
  "Returns md5sum for a message object of type 'SetAwbGain-request"
  "78d4a958be234c52a0c5e570d6402ff4")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SetAwbGain-request>)))
  "Returns full string definition for message of type '<SetAwbGain-request>"
  (cl:format cl:nil "# Raw Q8.8 AWB channel gains.~%uint16 r_gain~%uint16 b_gain~%uint16 g_gain~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SetAwbGain-request)))
  "Returns full string definition for message of type 'SetAwbGain-request"
  (cl:format cl:nil "# Raw Q8.8 AWB channel gains.~%uint16 r_gain~%uint16 b_gain~%uint16 g_gain~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SetAwbGain-request>))
  (cl:+ 0
     2
     2
     2
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SetAwbGain-request>))
  "Converts a ROS message object to a list"
  (cl:list 'SetAwbGain-request
    (cl:cons ':r_gain (r_gain msg))
    (cl:cons ':b_gain (b_gain msg))
    (cl:cons ':g_gain (g_gain msg))
))
;//! \htmlinclude SetAwbGain-response.msg.html

(cl:defclass <SetAwbGain-response> (roslisp-msg-protocol:ros-message)
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

(cl:defclass SetAwbGain-response (<SetAwbGain-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SetAwbGain-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SetAwbGain-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<SetAwbGain-response> is deprecated: use orbbec_camera-srv:SetAwbGain-response instead.")))

(cl:ensure-generic-function 'success-val :lambda-list '(m))
(cl:defmethod success-val ((m <SetAwbGain-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:success-val is deprecated.  Use orbbec_camera-srv:success instead.")
  (success m))

(cl:ensure-generic-function 'message-val :lambda-list '(m))
(cl:defmethod message-val ((m <SetAwbGain-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:message-val is deprecated.  Use orbbec_camera-srv:message instead.")
  (message m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SetAwbGain-response>) ostream)
  "Serializes a message object of type '<SetAwbGain-response>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'success) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'message))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'message))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SetAwbGain-response>) istream)
  "Deserializes a message object of type '<SetAwbGain-response>"
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
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SetAwbGain-response>)))
  "Returns string type for a service object of type '<SetAwbGain-response>"
  "orbbec_camera/SetAwbGainResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SetAwbGain-response)))
  "Returns string type for a service object of type 'SetAwbGain-response"
  "orbbec_camera/SetAwbGainResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SetAwbGain-response>)))
  "Returns md5sum for a message object of type '<SetAwbGain-response>"
  "78d4a958be234c52a0c5e570d6402ff4")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SetAwbGain-response)))
  "Returns md5sum for a message object of type 'SetAwbGain-response"
  "78d4a958be234c52a0c5e570d6402ff4")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SetAwbGain-response>)))
  "Returns full string definition for message of type '<SetAwbGain-response>"
  (cl:format cl:nil "bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SetAwbGain-response)))
  "Returns full string definition for message of type 'SetAwbGain-response"
  (cl:format cl:nil "bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SetAwbGain-response>))
  (cl:+ 0
     1
     4 (cl:length (cl:slot-value msg 'message))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SetAwbGain-response>))
  "Converts a ROS message object to a list"
  (cl:list 'SetAwbGain-response
    (cl:cons ':success (success msg))
    (cl:cons ':message (message msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'SetAwbGain)))
  'SetAwbGain-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'SetAwbGain)))
  'SetAwbGain-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SetAwbGain)))
  "Returns string type for a service object of type '<SetAwbGain>"
  "orbbec_camera/SetAwbGain")