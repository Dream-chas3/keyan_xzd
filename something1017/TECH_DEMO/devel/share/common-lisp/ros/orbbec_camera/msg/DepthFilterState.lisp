; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-msg)


;//! \htmlinclude DepthFilterState.msg.html

(cl:defclass <DepthFilterState> (roslisp-msg-protocol:ros-message)
  ((filter_name
    :reader filter_name
    :initarg :filter_name
    :type cl:string
    :initform "")
   (enabled
    :reader enabled
    :initarg :enabled
    :type cl:boolean
    :initform cl:nil)
   (params
    :reader params
    :initarg :params
    :type (cl:vector orbbec_camera-msg:DepthFilterParam)
   :initform (cl:make-array 0 :element-type 'orbbec_camera-msg:DepthFilterParam :initial-element (cl:make-instance 'orbbec_camera-msg:DepthFilterParam))))
)

(cl:defclass DepthFilterState (<DepthFilterState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DepthFilterState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DepthFilterState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-msg:<DepthFilterState> is deprecated: use orbbec_camera-msg:DepthFilterState instead.")))

(cl:ensure-generic-function 'filter_name-val :lambda-list '(m))
(cl:defmethod filter_name-val ((m <DepthFilterState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:filter_name-val is deprecated.  Use orbbec_camera-msg:filter_name instead.")
  (filter_name m))

(cl:ensure-generic-function 'enabled-val :lambda-list '(m))
(cl:defmethod enabled-val ((m <DepthFilterState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:enabled-val is deprecated.  Use orbbec_camera-msg:enabled instead.")
  (enabled m))

(cl:ensure-generic-function 'params-val :lambda-list '(m))
(cl:defmethod params-val ((m <DepthFilterState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:params-val is deprecated.  Use orbbec_camera-msg:params instead.")
  (params m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DepthFilterState>) ostream)
  "Serializes a message object of type '<DepthFilterState>"
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'filter_name))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'filter_name))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'enabled) 1 0)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'params))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'params))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DepthFilterState>) istream)
  "Deserializes a message object of type '<DepthFilterState>"
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'filter_name) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'filter_name) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:slot-value msg 'enabled) (cl:not (cl:zerop (cl:read-byte istream))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'params) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'params)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'orbbec_camera-msg:DepthFilterParam))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DepthFilterState>)))
  "Returns string type for a message object of type '<DepthFilterState>"
  "orbbec_camera/DepthFilterState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DepthFilterState)))
  "Returns string type for a message object of type 'DepthFilterState"
  "orbbec_camera/DepthFilterState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DepthFilterState>)))
  "Returns md5sum for a message object of type '<DepthFilterState>"
  "a4c5e50af4025d2a64d42b41c1cb6b04")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DepthFilterState)))
  "Returns md5sum for a message object of type 'DepthFilterState"
  "a4c5e50af4025d2a64d42b41c1cb6b04")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DepthFilterState>)))
  "Returns full string definition for message of type '<DepthFilterState>"
  (cl:format cl:nil "string filter_name~%bool enabled~%DepthFilterParam[] params~%~%================================================================================~%MSG: orbbec_camera/DepthFilterParam~%string name~%string value~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DepthFilterState)))
  "Returns full string definition for message of type 'DepthFilterState"
  (cl:format cl:nil "string filter_name~%bool enabled~%DepthFilterParam[] params~%~%================================================================================~%MSG: orbbec_camera/DepthFilterParam~%string name~%string value~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DepthFilterState>))
  (cl:+ 0
     4 (cl:length (cl:slot-value msg 'filter_name))
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'params) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DepthFilterState>))
  "Converts a ROS message object to a list"
  (cl:list 'DepthFilterState
    (cl:cons ':filter_name (filter_name msg))
    (cl:cons ':enabled (enabled msg))
    (cl:cons ':params (params msg))
))
