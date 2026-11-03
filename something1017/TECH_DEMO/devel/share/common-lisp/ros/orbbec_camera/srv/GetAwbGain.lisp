; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-srv)


;//! \htmlinclude GetAwbGain-request.msg.html

(cl:defclass <GetAwbGain-request> (roslisp-msg-protocol:ros-message)
  ()
)

(cl:defclass GetAwbGain-request (<GetAwbGain-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GetAwbGain-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GetAwbGain-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<GetAwbGain-request> is deprecated: use orbbec_camera-srv:GetAwbGain-request instead.")))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GetAwbGain-request>) ostream)
  "Serializes a message object of type '<GetAwbGain-request>"
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GetAwbGain-request>) istream)
  "Deserializes a message object of type '<GetAwbGain-request>"
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GetAwbGain-request>)))
  "Returns string type for a service object of type '<GetAwbGain-request>"
  "orbbec_camera/GetAwbGainRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GetAwbGain-request)))
  "Returns string type for a service object of type 'GetAwbGain-request"
  "orbbec_camera/GetAwbGainRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GetAwbGain-request>)))
  "Returns md5sum for a message object of type '<GetAwbGain-request>"
  "0942635903184795f22b4ac62ff74930")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GetAwbGain-request)))
  "Returns md5sum for a message object of type 'GetAwbGain-request"
  "0942635903184795f22b4ac62ff74930")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GetAwbGain-request>)))
  "Returns full string definition for message of type '<GetAwbGain-request>"
  (cl:format cl:nil "~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GetAwbGain-request)))
  "Returns full string definition for message of type 'GetAwbGain-request"
  (cl:format cl:nil "~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GetAwbGain-request>))
  (cl:+ 0
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GetAwbGain-request>))
  "Converts a ROS message object to a list"
  (cl:list 'GetAwbGain-request
))
;//! \htmlinclude GetAwbGain-response.msg.html

(cl:defclass <GetAwbGain-response> (roslisp-msg-protocol:ros-message)
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

(cl:defclass GetAwbGain-response (<GetAwbGain-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GetAwbGain-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GetAwbGain-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<GetAwbGain-response> is deprecated: use orbbec_camera-srv:GetAwbGain-response instead.")))

(cl:ensure-generic-function 'r_gain-val :lambda-list '(m))
(cl:defmethod r_gain-val ((m <GetAwbGain-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:r_gain-val is deprecated.  Use orbbec_camera-srv:r_gain instead.")
  (r_gain m))

(cl:ensure-generic-function 'b_gain-val :lambda-list '(m))
(cl:defmethod b_gain-val ((m <GetAwbGain-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:b_gain-val is deprecated.  Use orbbec_camera-srv:b_gain instead.")
  (b_gain m))

(cl:ensure-generic-function 'g_gain-val :lambda-list '(m))
(cl:defmethod g_gain-val ((m <GetAwbGain-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:g_gain-val is deprecated.  Use orbbec_camera-srv:g_gain instead.")
  (g_gain m))

(cl:ensure-generic-function 'success-val :lambda-list '(m))
(cl:defmethod success-val ((m <GetAwbGain-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:success-val is deprecated.  Use orbbec_camera-srv:success instead.")
  (success m))

(cl:ensure-generic-function 'message-val :lambda-list '(m))
(cl:defmethod message-val ((m <GetAwbGain-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:message-val is deprecated.  Use orbbec_camera-srv:message instead.")
  (message m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GetAwbGain-response>) ostream)
  "Serializes a message object of type '<GetAwbGain-response>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'r_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'r_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'b_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'b_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'g_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'g_gain)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'success) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'message))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'message))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GetAwbGain-response>) istream)
  "Deserializes a message object of type '<GetAwbGain-response>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'r_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'r_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'b_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'b_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'g_gain)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'g_gain)) (cl:read-byte istream))
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
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GetAwbGain-response>)))
  "Returns string type for a service object of type '<GetAwbGain-response>"
  "orbbec_camera/GetAwbGainResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GetAwbGain-response)))
  "Returns string type for a service object of type 'GetAwbGain-response"
  "orbbec_camera/GetAwbGainResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GetAwbGain-response>)))
  "Returns md5sum for a message object of type '<GetAwbGain-response>"
  "0942635903184795f22b4ac62ff74930")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GetAwbGain-response)))
  "Returns md5sum for a message object of type 'GetAwbGain-response"
  "0942635903184795f22b4ac62ff74930")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GetAwbGain-response>)))
  "Returns full string definition for message of type '<GetAwbGain-response>"
  (cl:format cl:nil "# Raw Q8.8 AWB channel gains.~%uint16 r_gain~%uint16 b_gain~%uint16 g_gain~%bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GetAwbGain-response)))
  "Returns full string definition for message of type 'GetAwbGain-response"
  (cl:format cl:nil "# Raw Q8.8 AWB channel gains.~%uint16 r_gain~%uint16 b_gain~%uint16 g_gain~%bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GetAwbGain-response>))
  (cl:+ 0
     2
     2
     2
     1
     4 (cl:length (cl:slot-value msg 'message))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GetAwbGain-response>))
  "Converts a ROS message object to a list"
  (cl:list 'GetAwbGain-response
    (cl:cons ':r_gain (r_gain msg))
    (cl:cons ':b_gain (b_gain msg))
    (cl:cons ':g_gain (g_gain msg))
    (cl:cons ':success (success msg))
    (cl:cons ':message (message msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'GetAwbGain)))
  'GetAwbGain-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'GetAwbGain)))
  'GetAwbGain-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GetAwbGain)))
  "Returns string type for a service object of type '<GetAwbGain>"
  "orbbec_camera/GetAwbGain")