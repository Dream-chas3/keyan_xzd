;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::GetAwbGain)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'GetAwbGain (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::GETAWBGAIN")
  (make-package "ORBBEC_CAMERA::GETAWBGAIN"))
(unless (find-package "ORBBEC_CAMERA::GETAWBGAINREQUEST")
  (make-package "ORBBEC_CAMERA::GETAWBGAINREQUEST"))
(unless (find-package "ORBBEC_CAMERA::GETAWBGAINRESPONSE")
  (make-package "ORBBEC_CAMERA::GETAWBGAINRESPONSE"))

(in-package "ROS")





(defclass orbbec_camera::GetAwbGainRequest
  :super ros::object
  :slots ())

(defmethod orbbec_camera::GetAwbGainRequest
  (:init
   (&key
    )
   (send-super :init)
   self)
  (:serialization-length
   ()
   (+
    0
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;;
   self)
  )

(defclass orbbec_camera::GetAwbGainResponse
  :super ros::object
  :slots (_r_gain _b_gain _g_gain _success _message ))

(defmethod orbbec_camera::GetAwbGainResponse
  (:init
   (&key
    ((:r_gain __r_gain) 0)
    ((:b_gain __b_gain) 0)
    ((:g_gain __g_gain) 0)
    ((:success __success) nil)
    ((:message __message) "")
    )
   (send-super :init)
   (setq _r_gain (round __r_gain))
   (setq _b_gain (round __b_gain))
   (setq _g_gain (round __g_gain))
   (setq _success __success)
   (setq _message (string __message))
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
  (:success
   (&optional (__success :null))
   (if (not (eq __success :null)) (setq _success __success)) _success)
  (:message
   (&optional __message)
   (if __message (setq _message __message)) _message)
  (:serialization-length
   ()
   (+
    ;; uint16 _r_gain
    2
    ;; uint16 _b_gain
    2
    ;; uint16 _g_gain
    2
    ;; bool _success
    1
    ;; string _message
    4 (length _message)
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
     ;; bool _success
       (if _success (write-byte -1 s) (write-byte 0 s))
     ;; string _message
       (write-long (length _message) s) (princ _message s)
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
   ;; bool _success
     (setq _success (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _message
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _message (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(defclass orbbec_camera::GetAwbGain
  :super ros::object
  :slots ())

(setf (get orbbec_camera::GetAwbGain :md5sum-) "0942635903184795f22b4ac62ff74930")
(setf (get orbbec_camera::GetAwbGain :datatype-) "orbbec_camera/GetAwbGain")
(setf (get orbbec_camera::GetAwbGain :request) orbbec_camera::GetAwbGainRequest)
(setf (get orbbec_camera::GetAwbGain :response) orbbec_camera::GetAwbGainResponse)

(defmethod orbbec_camera::GetAwbGainRequest
  (:response () (instance orbbec_camera::GetAwbGainResponse :init)))

(setf (get orbbec_camera::GetAwbGainRequest :md5sum-) "0942635903184795f22b4ac62ff74930")
(setf (get orbbec_camera::GetAwbGainRequest :datatype-) "orbbec_camera/GetAwbGainRequest")
(setf (get orbbec_camera::GetAwbGainRequest :definition-)
      "---
# Raw Q8.8 AWB channel gains.
uint16 r_gain
uint16 b_gain
uint16 g_gain
bool success
string message

")

(setf (get orbbec_camera::GetAwbGainResponse :md5sum-) "0942635903184795f22b4ac62ff74930")
(setf (get orbbec_camera::GetAwbGainResponse :datatype-) "orbbec_camera/GetAwbGainResponse")
(setf (get orbbec_camera::GetAwbGainResponse :definition-)
      "---
# Raw Q8.8 AWB channel gains.
uint16 r_gain
uint16 b_gain
uint16 g_gain
bool success
string message

")



(provide :orbbec_camera/GetAwbGain "0942635903184795f22b4ac62ff74930")


