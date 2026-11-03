; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-msg)


;//! \htmlinclude DepthFiltersStatus.msg.html

(cl:defclass <DepthFiltersStatus> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (filters
    :reader filters
    :initarg :filters
    :type (cl:vector orbbec_camera-msg:DepthFilterState)
   :initform (cl:make-array 0 :element-type 'orbbec_camera-msg:DepthFilterState :initial-element (cl:make-instance 'orbbec_camera-msg:DepthFilterState))))
)

(cl:defclass DepthFiltersStatus (<DepthFiltersStatus>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DepthFiltersStatus>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DepthFiltersStatus)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-msg:<DepthFiltersStatus> is deprecated: use orbbec_camera-msg:DepthFiltersStatus instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <DepthFiltersStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:header-val is deprecated.  Use orbbec_camera-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'filters-val :lambda-list '(m))
(cl:defmethod filters-val ((m <DepthFiltersStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-msg:filters-val is deprecated.  Use orbbec_camera-msg:filters instead.")
  (filters m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DepthFiltersStatus>) ostream)
  "Serializes a message object of type '<DepthFiltersStatus>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'filters))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'filters))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DepthFiltersStatus>) istream)
  "Deserializes a message object of type '<DepthFiltersStatus>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'filters) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'filters)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'orbbec_camera-msg:DepthFilterState))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DepthFiltersStatus>)))
  "Returns string type for a message object of type '<DepthFiltersStatus>"
  "orbbec_camera/DepthFiltersStatus")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DepthFiltersStatus)))
  "Returns string type for a message object of type 'DepthFiltersStatus"
  "orbbec_camera/DepthFiltersStatus")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DepthFiltersStatus>)))
  "Returns md5sum for a message object of type '<DepthFiltersStatus>"
  "7e84f3ab9e3323f2de9c2ef408d5df13")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DepthFiltersStatus)))
  "Returns md5sum for a message object of type 'DepthFiltersStatus"
  "7e84f3ab9e3323f2de9c2ef408d5df13")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DepthFiltersStatus>)))
  "Returns full string definition for message of type '<DepthFiltersStatus>"
  (cl:format cl:nil "std_msgs/Header header~%DepthFilterState[] filters~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: orbbec_camera/DepthFilterState~%string filter_name~%bool enabled~%DepthFilterParam[] params~%~%================================================================================~%MSG: orbbec_camera/DepthFilterParam~%string name~%string value~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DepthFiltersStatus)))
  "Returns full string definition for message of type 'DepthFiltersStatus"
  (cl:format cl:nil "std_msgs/Header header~%DepthFilterState[] filters~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: orbbec_camera/DepthFilterState~%string filter_name~%bool enabled~%DepthFilterParam[] params~%~%================================================================================~%MSG: orbbec_camera/DepthFilterParam~%string name~%string value~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DepthFiltersStatus>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'filters) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DepthFiltersStatus>))
  "Converts a ROS message object to a list"
  (cl:list 'DepthFiltersStatus
    (cl:cons ':header (header msg))
    (cl:cons ':filters (filters msg))
))
