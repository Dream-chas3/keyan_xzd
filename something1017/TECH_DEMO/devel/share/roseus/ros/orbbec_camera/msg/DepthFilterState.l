;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::DepthFilterState)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'DepthFilterState (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::DEPTHFILTERSTATE")
  (make-package "ORBBEC_CAMERA::DEPTHFILTERSTATE"))

(in-package "ROS")
;;//! \htmlinclude DepthFilterState.msg.html


(defclass orbbec_camera::DepthFilterState
  :super ros::object
  :slots (_filter_name _enabled _params ))

(defmethod orbbec_camera::DepthFilterState
  (:init
   (&key
    ((:filter_name __filter_name) "")
    ((:enabled __enabled) nil)
    ((:params __params) ())
    )
   (send-super :init)
   (setq _filter_name (string __filter_name))
   (setq _enabled __enabled)
   (setq _params __params)
   self)
  (:filter_name
   (&optional __filter_name)
   (if __filter_name (setq _filter_name __filter_name)) _filter_name)
  (:enabled
   (&optional (__enabled :null))
   (if (not (eq __enabled :null)) (setq _enabled __enabled)) _enabled)
  (:params
   (&rest __params)
   (if (keywordp (car __params))
       (send* _params __params)
     (progn
       (if __params (setq _params (car __params)))
       _params)))
  (:serialization-length
   ()
   (+
    ;; string _filter_name
    4 (length _filter_name)
    ;; bool _enabled
    1
    ;; orbbec_camera/DepthFilterParam[] _params
    (apply #'+ (send-all _params :serialization-length)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; string _filter_name
       (write-long (length _filter_name) s) (princ _filter_name s)
     ;; bool _enabled
       (if _enabled (write-byte -1 s) (write-byte 0 s))
     ;; orbbec_camera/DepthFilterParam[] _params
     (write-long (length _params) s)
     (dolist (elem _params)
       (send elem :serialize s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; string _filter_name
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _filter_name (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; bool _enabled
     (setq _enabled (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; orbbec_camera/DepthFilterParam[] _params
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _params (let (r) (dotimes (i n) (push (instance orbbec_camera::DepthFilterParam :init) r)) r))
     (dolist (elem- _params)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;;
   self)
  )

(setf (get orbbec_camera::DepthFilterState :md5sum-) "a4c5e50af4025d2a64d42b41c1cb6b04")
(setf (get orbbec_camera::DepthFilterState :datatype-) "orbbec_camera/DepthFilterState")
(setf (get orbbec_camera::DepthFilterState :definition-)
      "string filter_name
bool enabled
DepthFilterParam[] params

================================================================================
MSG: orbbec_camera/DepthFilterParam
string name
string value

")



(provide :orbbec_camera/DepthFilterState "a4c5e50af4025d2a64d42b41c1cb6b04")


