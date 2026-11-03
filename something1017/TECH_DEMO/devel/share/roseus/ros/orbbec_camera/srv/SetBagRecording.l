;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::SetBagRecording)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'SetBagRecording (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::SETBAGRECORDING")
  (make-package "ORBBEC_CAMERA::SETBAGRECORDING"))
(unless (find-package "ORBBEC_CAMERA::SETBAGRECORDINGREQUEST")
  (make-package "ORBBEC_CAMERA::SETBAGRECORDINGREQUEST"))
(unless (find-package "ORBBEC_CAMERA::SETBAGRECORDINGRESPONSE")
  (make-package "ORBBEC_CAMERA::SETBAGRECORDINGRESPONSE"))

(in-package "ROS")





(defclass orbbec_camera::SetBagRecordingRequest
  :super ros::object
  :slots (_enable _file_path ))

(defmethod orbbec_camera::SetBagRecordingRequest
  (:init
   (&key
    ((:enable __enable) nil)
    ((:file_path __file_path) "")
    )
   (send-super :init)
   (setq _enable __enable)
   (setq _file_path (string __file_path))
   self)
  (:enable
   (&optional (__enable :null))
   (if (not (eq __enable :null)) (setq _enable __enable)) _enable)
  (:file_path
   (&optional __file_path)
   (if __file_path (setq _file_path __file_path)) _file_path)
  (:serialization-length
   ()
   (+
    ;; bool _enable
    1
    ;; string _file_path
    4 (length _file_path)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _enable
       (if _enable (write-byte -1 s) (write-byte 0 s))
     ;; string _file_path
       (write-long (length _file_path) s) (princ _file_path s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _enable
     (setq _enable (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _file_path
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _file_path (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(defclass orbbec_camera::SetBagRecordingResponse
  :super ros::object
  :slots (_success _message ))

(defmethod orbbec_camera::SetBagRecordingResponse
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

(defclass orbbec_camera::SetBagRecording
  :super ros::object
  :slots ())

(setf (get orbbec_camera::SetBagRecording :md5sum-) "7047bf76906efb3e9ffd85b3a4c07eb6")
(setf (get orbbec_camera::SetBagRecording :datatype-) "orbbec_camera/SetBagRecording")
(setf (get orbbec_camera::SetBagRecording :request) orbbec_camera::SetBagRecordingRequest)
(setf (get orbbec_camera::SetBagRecording :response) orbbec_camera::SetBagRecordingResponse)

(defmethod orbbec_camera::SetBagRecordingRequest
  (:response () (instance orbbec_camera::SetBagRecordingResponse :init)))

(setf (get orbbec_camera::SetBagRecordingRequest :md5sum-) "7047bf76906efb3e9ffd85b3a4c07eb6")
(setf (get orbbec_camera::SetBagRecordingRequest :datatype-) "orbbec_camera/SetBagRecordingRequest")
(setf (get orbbec_camera::SetBagRecordingRequest :definition-)
      "bool enable          # true starts recording, false stops recording
string file_path     # path to save the .bag file when enable is true; empty uses current directory
---
bool success
string message

")

(setf (get orbbec_camera::SetBagRecordingResponse :md5sum-) "7047bf76906efb3e9ffd85b3a4c07eb6")
(setf (get orbbec_camera::SetBagRecordingResponse :datatype-) "orbbec_camera/SetBagRecordingResponse")
(setf (get orbbec_camera::SetBagRecordingResponse :definition-)
      "bool enable          # true starts recording, false stops recording
string file_path     # path to save the .bag file when enable is true; empty uses current directory
---
bool success
string message

")



(provide :orbbec_camera/SetBagRecording "7047bf76906efb3e9ffd85b3a4c07eb6")


