;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::DepthFiltersStatus)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'DepthFiltersStatus (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::DEPTHFILTERSSTATUS")
  (make-package "ORBBEC_CAMERA::DEPTHFILTERSSTATUS"))

(in-package "ROS")
;;//! \htmlinclude DepthFiltersStatus.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass orbbec_camera::DepthFiltersStatus
  :super ros::object
  :slots (_header _filters ))

(defmethod orbbec_camera::DepthFiltersStatus
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:filters __filters) ())
    )
   (send-super :init)
   (setq _header __header)
   (setq _filters __filters)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:filters
   (&rest __filters)
   (if (keywordp (car __filters))
       (send* _filters __filters)
     (progn
       (if __filters (setq _filters (car __filters)))
       _filters)))
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; orbbec_camera/DepthFilterState[] _filters
    (apply #'+ (send-all _filters :serialization-length)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; orbbec_camera/DepthFilterState[] _filters
     (write-long (length _filters) s)
     (dolist (elem _filters)
       (send elem :serialize s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; orbbec_camera/DepthFilterState[] _filters
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _filters (let (r) (dotimes (i n) (push (instance orbbec_camera::DepthFilterState :init) r)) r))
     (dolist (elem- _filters)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;;
   self)
  )

(setf (get orbbec_camera::DepthFiltersStatus :md5sum-) "7e84f3ab9e3323f2de9c2ef408d5df13")
(setf (get orbbec_camera::DepthFiltersStatus :datatype-) "orbbec_camera/DepthFiltersStatus")
(setf (get orbbec_camera::DepthFiltersStatus :definition-)
      "std_msgs/Header header
DepthFilterState[] filters

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
string frame_id

================================================================================
MSG: orbbec_camera/DepthFilterState
string filter_name
bool enabled
DepthFilterParam[] params

================================================================================
MSG: orbbec_camera/DepthFilterParam
string name
string value

")



(provide :orbbec_camera/DepthFiltersStatus "7e84f3ab9e3323f2de9c2ef408d5df13")


