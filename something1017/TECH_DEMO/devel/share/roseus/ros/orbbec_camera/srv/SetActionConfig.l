;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::SetActionConfig)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'SetActionConfig (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::SETACTIONCONFIG")
  (make-package "ORBBEC_CAMERA::SETACTIONCONFIG"))
(unless (find-package "ORBBEC_CAMERA::SETACTIONCONFIGREQUEST")
  (make-package "ORBBEC_CAMERA::SETACTIONCONFIGREQUEST"))
(unless (find-package "ORBBEC_CAMERA::SETACTIONCONFIGRESPONSE")
  (make-package "ORBBEC_CAMERA::SETACTIONCONFIGRESPONSE"))

(in-package "ROS")





(defclass orbbec_camera::SetActionConfigRequest
  :super ros::object
  :slots (_device_key _selector _group_key _group_mask ))

(defmethod orbbec_camera::SetActionConfigRequest
  (:init
   (&key
    ((:device_key __device_key) 0)
    ((:selector __selector) 0)
    ((:group_key __group_key) 0)
    ((:group_mask __group_mask) 0)
    )
   (send-super :init)
   (setq _device_key (round __device_key))
   (setq _selector (round __selector))
   (setq _group_key (round __group_key))
   (setq _group_mask (round __group_mask))
   self)
  (:device_key
   (&optional __device_key)
   (if __device_key (setq _device_key __device_key)) _device_key)
  (:selector
   (&optional __selector)
   (if __selector (setq _selector __selector)) _selector)
  (:group_key
   (&optional __group_key)
   (if __group_key (setq _group_key __group_key)) _group_key)
  (:group_mask
   (&optional __group_mask)
   (if __group_mask (setq _group_mask __group_mask)) _group_mask)
  (:serialization-length
   ()
   (+
    ;; uint32 _device_key
    4
    ;; uint32 _selector
    4
    ;; uint32 _group_key
    4
    ;; uint32 _group_mask
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint32 _device_key
       (write-long _device_key s)
     ;; uint32 _selector
       (write-long _selector s)
     ;; uint32 _group_key
       (write-long _group_key s)
     ;; uint32 _group_mask
       (write-long _group_mask s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint32 _device_key
     (setq _device_key (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _selector
     (setq _selector (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _group_key
     (setq _group_key (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _group_mask
     (setq _group_mask (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(defclass orbbec_camera::SetActionConfigResponse
  :super ros::object
  :slots (_success _message ))

(defmethod orbbec_camera::SetActionConfigResponse
  (:init
   (&key
    ((:success __success) nil)
    ((:message __message) "")
    )
   (send-super :init)
   (setq _success __success)
   (setq _message (string __message))
   self)
  (:success
   (&optional (__success :null))
   (if (not (eq __success :null)) (setq _success __success)) _success)
  (:message
   (&optional __message)
   (if __message (setq _message __message)) _message)
  (:serialization-length
   ()
   (+
    ;; bool _success
    1
    ;; string _message
    4 (length _message)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _success
       (if _success (write-byte -1 s) (write-byte 0 s))
     ;; string _message
       (write-long (length _message) s) (princ _message s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _success
     (setq _success (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _message
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _message (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(defclass orbbec_camera::SetActionConfig
  :super ros::object
  :slots ())

(setf (get orbbec_camera::SetActionConfig :md5sum-) "6df9d533c9f8d312e2eb4484d8d2f98b")
(setf (get orbbec_camera::SetActionConfig :datatype-) "orbbec_camera/SetActionConfig")
(setf (get orbbec_camera::SetActionConfig :request) orbbec_camera::SetActionConfigRequest)
(setf (get orbbec_camera::SetActionConfig :response) orbbec_camera::SetActionConfigResponse)

(defmethod orbbec_camera::SetActionConfigRequest
  (:response () (instance orbbec_camera::SetActionConfigResponse :init)))

(setf (get orbbec_camera::SetActionConfigRequest :md5sum-) "6df9d533c9f8d312e2eb4484d8d2f98b")
(setf (get orbbec_camera::SetActionConfigRequest :datatype-) "orbbec_camera/SetActionConfigRequest")
(setf (get orbbec_camera::SetActionConfigRequest :definition-)
      "uint32 device_key
uint32 selector
uint32 group_key
uint32 group_mask
---
bool success
string message

")

(setf (get orbbec_camera::SetActionConfigResponse :md5sum-) "6df9d533c9f8d312e2eb4484d8d2f98b")
(setf (get orbbec_camera::SetActionConfigResponse :datatype-) "orbbec_camera/SetActionConfigResponse")
(setf (get orbbec_camera::SetActionConfigResponse :definition-)
      "uint32 device_key
uint32 selector
uint32 group_key
uint32 group_mask
---
bool success
string message

")



(provide :orbbec_camera/SetActionConfig "6df9d533c9f8d312e2eb4484d8d2f98b")


