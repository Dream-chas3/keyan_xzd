; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-srv)


;//! \htmlinclude SetBagRecording-request.msg.html

(cl:defclass <SetBagRecording-request> (roslisp-msg-protocol:ros-message)
  ((enable
    :reader enable
    :initarg :enable
    :type cl:boolean
    :initform cl:nil)
   (file_path
    :reader file_path
    :initarg :file_path
    :type cl:string
    :initform ""))
)

(cl:defclass SetBagRecording-request (<SetBagRecording-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SetBagRecording-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SetBagRecording-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<SetBagRecording-request> is deprecated: use orbbec_camera-srv:SetBagRecording-request instead.")))

(cl:ensure-generic-function 'enable-val :lambda-list '(m))
(cl:defmethod enable-val ((m <SetBagRecording-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:enable-val is deprecated.  Use orbbec_camera-srv:enable instead.")
  (enable m))

(cl:ensure-generic-function 'file_path-val :lambda-list '(m))
(cl:defmethod file_path-val ((m <SetBagRecording-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:file_path-val is deprecated.  Use orbbec_camera-srv:file_path instead.")
  (file_path m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SetBagRecording-request>) ostream)
  "Serializes a message object of type '<SetBagRecording-request>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'enable) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'file_path))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'file_path))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SetBagRecording-request>) istream)
  "Deserializes a message object of type '<SetBagRecording-request>"
    (cl:setf (cl:slot-value msg 'enable) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'file_path) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'file_path) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SetBagRecording-request>)))
  "Returns string type for a service object of type '<SetBagRecording-request>"
  "orbbec_camera/SetBagRecordingRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SetBagRecording-request)))
  "Returns string type for a service object of type 'SetBagRecording-request"
  "orbbec_camera/SetBagRecordingRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SetBagRecording-request>)))
  "Returns md5sum for a message object of type '<SetBagRecording-request>"
  "7047bf76906efb3e9ffd85b3a4c07eb6")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SetBagRecording-request)))
  "Returns md5sum for a message object of type 'SetBagRecording-request"
  "7047bf76906efb3e9ffd85b3a4c07eb6")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SetBagRecording-request>)))
  "Returns full string definition for message of type '<SetBagRecording-request>"
  (cl:format cl:nil "bool enable          # true starts recording, false stops recording~%string file_path     # path to save the .bag file when enable is true; empty uses current directory~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SetBagRecording-request)))
  "Returns full string definition for message of type 'SetBagRecording-request"
  (cl:format cl:nil "bool enable          # true starts recording, false stops recording~%string file_path     # path to save the .bag file when enable is true; empty uses current directory~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SetBagRecording-request>))
  (cl:+ 0
     1
     4 (cl:length (cl:slot-value msg 'file_path))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SetBagRecording-request>))
  "Converts a ROS message object to a list"
  (cl:list 'SetBagRecording-request
    (cl:cons ':enable (enable msg))
    (cl:cons ':file_path (file_path msg))
))
;//! \htmlinclude SetBagRecording-response.msg.html

(cl:defclass <SetBagRecording-response> (roslisp-msg-protocol:ros-message)
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

(cl:defclass SetBagRecording-response (<SetBagRecording-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SetBagRecording-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SetBagRecording-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<SetBagRecording-response> is deprecated: use orbbec_camera-srv:SetBagRecording-response instead.")))

(cl:ensure-generic-function 'success-val :lambda-list '(m))
(cl:defmethod success-val ((m <SetBagRecording-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:success-val is deprecated.  Use orbbec_camera-srv:success instead.")
  (success m))

(cl:ensure-generic-function 'message-val :lambda-list '(m))
(cl:defmethod message-val ((m <SetBagRecording-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:message-val is deprecated.  Use orbbec_camera-srv:message instead.")
  (message m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SetBagRecording-response>) ostream)
  "Serializes a message object of type '<SetBagRecording-response>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'success) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'message))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'message))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SetBagRecording-response>) istream)
  "Deserializes a message object of type '<SetBagRecording-response>"
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
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SetBagRecording-response>)))
  "Returns string type for a service object of type '<SetBagRecording-response>"
  "orbbec_camera/SetBagRecordingResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SetBagRecording-response)))
  "Returns string type for a service object of type 'SetBagRecording-response"
  "orbbec_camera/SetBagRecordingResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SetBagRecording-response>)))
  "Returns md5sum for a message object of type '<SetBagRecording-response>"
  "7047bf76906efb3e9ffd85b3a4c07eb6")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SetBagRecording-response)))
  "Returns md5sum for a message object of type 'SetBagRecording-response"
  "7047bf76906efb3e9ffd85b3a4c07eb6")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SetBagRecording-response>)))
  "Returns full string definition for message of type '<SetBagRecording-response>"
  (cl:format cl:nil "bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SetBagRecording-response)))
  "Returns full string definition for message of type 'SetBagRecording-response"
  (cl:format cl:nil "bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SetBagRecording-response>))
  (cl:+ 0
     1
     4 (cl:length (cl:slot-value msg 'message))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SetBagRecording-response>))
  "Converts a ROS message object to a list"
  (cl:list 'SetBagRecording-response
    (cl:cons ':success (success msg))
    (cl:cons ':message (message msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'SetBagRecording)))
  'SetBagRecording-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'SetBagRecording)))
  'SetBagRecording-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SetBagRecording)))
  "Returns string type for a service object of type '<SetBagRecording>"
  "orbbec_camera/SetBagRecording")