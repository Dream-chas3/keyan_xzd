;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::SetAwbGain)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'SetAwbGain (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::SETAWBGAIN")
  (make-package "ORBBEC_CAMERA::SETAWBGAIN"))
(unless (find-package "ORBBEC_CAMERA::SETAWBGAINREQUEST")
  (make-package "ORBBEC_CAMERA::SETAWBGAINREQUEST"))
(unless (find-package "ORBBEC_CAMERA::SETAWBGAINRESPONSE")
  (make-package "ORBBEC_CAMERA::SETAWBGAINRESPONSE"))

(in-package "ROS")





(defclass orbbec_camera::SetAwbGainRequest
  :super ros::object
  :slots (_r_gain _b_gain _g_gain ))

(defmethod orbbec_camera::SetAwbGainRequest
  (:init
   (&key
    ((:r_gain __r_gain) 0)
    ((:b_gain __b_gain) 0)
    ((:g_gain __g_gain) 0)
    )
   (send-super :init)
   (setq _r_gain (round __r_gain))
   (setq _b_gain (round __b_gain))
   (setq _g_gain (round __g_gain))
   self)
  (:r_gain
   (&optional __r_gain)
   (if __r_gain (setq _r_gain __r_gain)) _r_gain)
  (:b_gain
   (&optional __b_gain)
   (if __b_gain (setq _b_gain __b_gain)) _b_gain)
  (:g_gain
   (&optional __g_gain)
   (if __g_gain (setq _g_gain __g_gain)) _g_gain)
  (:serialization-length
   ()
   (+
    ;; uint16 _r_gain
    2
    ;; uint16 _b_gain
    2
    ;; uint16 _g_gain
    2
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint16 _r_gain
       (write-word _r_gain s)
     ;; uint16 _b_gain
       (write-word _b_gain s)
     ;; uint16 _g_gain
       (write-word _g_gain s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint16 _r_gain
     (setq _r_gain (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint16 _b_gain
     (setq _b_gain (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint16 _g_gain
     (setq _g_gain (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;;
   self)
  )

(defclass orbbec_camera::SetAwbGainResponse
  :super ros::object
  :slots (_success _message ))

(defmethod orbbec_camera::SetAwbGainResponse
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

(defclass orbbec_camera::SetAwbGain
  :super ros::object
  :slots ())

(setf (get orbbec_camera::SetAwbGain :md5sum-) "78d4a958be234c52a0c5e570d6402ff4")
(setf (get orbbec_camera::SetAwbGain :datatype-) "orbbec_camera/SetAwbGain")
(setf (get orbbec_camera::SetAwbGain :request) orbbec_camera::SetAwbGainRequest)
(setf (get orbbec_camera::SetAwbGain :response) orbbec_camera::SetAwbGainResponse)

(defmethod orbbec_camera::SetAwbGainRequest
  (:response () (instance orbbec_camera::SetAwbGainResponse :init)))

(setf (get orbbec_camera::SetAwbGainRequest :md5sum-) "78d4a958be234c52a0c5e570d6402ff4")
(setf (get orbbec_camera::SetAwbGainRequest :datatype-) "orbbec_camera/SetAwbGainRequest")
(setf (get orbbec_camera::SetAwbGainRequest :definition-)
      "# Raw Q8.8 AWB channel gains.
uint16 r_gain
uint16 b_gain
uint16 g_gain
---
bool success
string message

")

(setf (get orbbec_camera::SetAwbGainResponse :md5sum-) "78d4a958be234c52a0c5e570d6402ff4")
(setf (get orbbec_camera::SetAwbGainResponse :datatype-) "orbbec_camera/SetAwbGainResponse")
(setf (get orbbec_camera::SetAwbGainResponse :definition-)
      "# Raw Q8.8 AWB channel gains.
uint16 r_gain
uint16 b_gain
uint16 g_gain
---
bool success
string message

")



(provide :orbbec_camera/SetAwbGain "78d4a958be234c52a0c5e570d6402ff4")


