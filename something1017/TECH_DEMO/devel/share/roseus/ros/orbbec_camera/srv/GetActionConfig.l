;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::GetActionConfig)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'GetActionConfig (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::GETACTIONCONFIG")
  (make-package "ORBBEC_CAMERA::GETACTIONCONFIG"))
(unless (find-package "ORBBEC_CAMERA::GETACTIONCONFIGREQUEST")
  (make-package "ORBBEC_CAMERA::GETACTIONCONFIGREQUEST"))
(unless (find-package "ORBBEC_CAMERA::GETACTIONCONFIGRESPONSE")
  (make-package "ORBBEC_CAMERA::GETACTIONCONFIGRESPONSE"))

(in-package "ROS")





(defclass orbbec_camera::GetActionConfigRequest
  :super ros::object
  :slots (_selector ))

(defmethod orbbec_camera::GetActionConfigRequest
  (:init
   (&key
    ((:selector __selector) 0)
    )
   (send-super :init)
   (setq _selector (round __selector))
   self)
  (:selector
   (&optional __selector)
   (if __selector (setq _selector __selector)) _selector)
  (:serialization-length
   ()
   (+
    ;; uint32 _selector
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint32 _selector
       (write-long _selector s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint32 _selector
     (setq _selector (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(defclass orbbec_camera::GetActionConfigResponse
  :super ros::object
  :slots (_action_signal_count _device_key _group_key _group_mask _success _message ))

(defmethod orbbec_camera::GetActionConfigResponse
  (:init
   (&key
    ((:action_signal_count __action_signal_count) 0)
    ((:device_key __device_key) 0)
    ((:group_key __group_key) 0)
    ((:group_mask __group_mask) 0)
    ((:success __success) nil)
    ((:message __message) "")
    )
   (send-super :init)
   (setq _action_signal_count (round __action_signal_count))
   (setq _device_key (round __device_key))
   (setq _group_key (round __group_key))
   (setq _group_mask (round __group_mask))
   (setq _success __success)
   (setq _message (string __message))
   self)
  (:action_signal_count
   (&optional __action_signal_count)
   (if __action_signal_count (setq _action_signal_count __action_signal_count)) _action_signal_count)
  (:device_key
   (&optional __device_key)
   (if __device_key (setq _device_key __device_key)) _device_key)
  (:group_key
   (&optional __group_key)
   (if __group_key (setq _group_key __group_key)) _group_key)
  (:group_mask
   (&optional __group_mask)
   (if __group_mask (setq _group_mask __group_mask)) _group_mask)
  (:success
   (&optional (__success :null))
   (if (not (eq __success :null)) (setq _success __success)) _success)
  (:message
   (&optional __message)
   (if __message (setq _message __message)) _message)
  (:serialization-length
   ()
   (+
    ;; uint32 _action_signal_count
    4
    ;; uint32 _device_key
    4
    ;; uint32 _group_key
    4
    ;; uint32 _group_mask
    4
    ;; bool _success
    1
    ;; string _message
    4 (length _message)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint32 _action_signal_count
       (write-long _action_signal_count s)
     ;; uint32 _device_key
       (write-long _device_key s)
     ;; uint32 _group_key
       (write-long _group_key s)
     ;; uint32 _group_mask
       (write-long _group_mask s)
     ;; bool _success
       (if _success (write-byte -1 s) (write-byte 0 s))
     ;; string _message
       (write-long (length _message) s) (princ _message s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint32 _action_signal_count
     (setq _action_signal_count (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _device_key
     (setq _device_key (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _group_key
     (setq _group_key (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _group_mask
     (setq _group_mask (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; bool _success
     (setq _success (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _message
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _message (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(defclass orbbec_camera::GetActionConfig
  :super ros::object
  :slots ())

(setf (get orbbec_camera::GetActionConfig :md5sum-) "b599f0e1250bf719f4e715cab6e1e366")
(setf (get orbbec_camera::GetActionConfig :datatype-) "orbbec_camera/GetActionConfig")
(setf (get orbbec_camera::GetActionConfig :request) orbbec_camera::GetActionConfigRequest)
(setf (get orbbec_camera::GetActionConfig :response) orbbec_camera::GetActionConfigResponse)

(defmethod orbbec_camera::GetActionConfigRequest
  (:response () (instance orbbec_camera::GetActionConfigResponse :init)))

(setf (get orbbec_camera::GetActionConfigRequest :md5sum-) "b599f0e1250bf719f4e715cab6e1e366")
(setf (get orbbec_camera::GetActionConfigRequest :datatype-) "orbbec_camera/GetActionConfigRequest")
(setf (get orbbec_camera::GetActionConfigRequest :definition-)
      "uint32 selector
---
uint32 action_signal_count
uint32 device_key
uint32 group_key
uint32 group_mask
bool success
string message

")

(setf (get orbbec_camera::GetActionConfigResponse :md5sum-) "b599f0e1250bf719f4e715cab6e1e366")
(setf (get orbbec_camera::GetActionConfigResponse :datatype-) "orbbec_camera/GetActionConfigResponse")
(setf (get orbbec_camera::GetActionConfigResponse :definition-)
      "uint32 selector
---
uint32 action_signal_count
uint32 device_key
uint32 group_key
uint32 group_mask
bool success
string message

")



(provide :orbbec_camera/GetActionConfig "b599f0e1250bf719f4e715cab6e1e366")


