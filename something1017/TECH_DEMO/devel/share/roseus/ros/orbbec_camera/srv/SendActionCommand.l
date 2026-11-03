;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::SendActionCommand)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'SendActionCommand (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::SENDACTIONCOMMAND")
  (make-package "ORBBEC_CAMERA::SENDACTIONCOMMAND"))
(unless (find-package "ORBBEC_CAMERA::SENDACTIONCOMMANDREQUEST")
  (make-package "ORBBEC_CAMERA::SENDACTIONCOMMANDREQUEST"))
(unless (find-package "ORBBEC_CAMERA::SENDACTIONCOMMANDRESPONSE")
  (make-package "ORBBEC_CAMERA::SENDACTIONCOMMANDRESPONSE"))

(in-package "ROS")





(intern "*TRIGGER_MODE_IMMEDIATE*" (find-package "ORBBEC_CAMERA::SENDACTIONCOMMANDREQUEST"))
(shadow '*TRIGGER_MODE_IMMEDIATE* (find-package "ORBBEC_CAMERA::SENDACTIONCOMMANDREQUEST"))
(defconstant orbbec_camera::SendActionCommandRequest::*TRIGGER_MODE_IMMEDIATE* 0)
(intern "*TRIGGER_MODE_DELAYED*" (find-package "ORBBEC_CAMERA::SENDACTIONCOMMANDREQUEST"))
(shadow '*TRIGGER_MODE_DELAYED* (find-package "ORBBEC_CAMERA::SENDACTIONCOMMANDREQUEST"))
(defconstant orbbec_camera::SendActionCommandRequest::*TRIGGER_MODE_DELAYED* 1)
(intern "*TRIGGER_MODE_TIMESTAMP*" (find-package "ORBBEC_CAMERA::SENDACTIONCOMMANDREQUEST"))
(shadow '*TRIGGER_MODE_TIMESTAMP* (find-package "ORBBEC_CAMERA::SENDACTIONCOMMANDREQUEST"))
(defconstant orbbec_camera::SendActionCommandRequest::*TRIGGER_MODE_TIMESTAMP* 2)

(defun orbbec_camera::SendActionCommandRequest-to-symbol (const)
  (cond
        ((= const 0) 'orbbec_camera::SendActionCommandRequest::*TRIGGER_MODE_IMMEDIATE*)
        ((= const 1) 'orbbec_camera::SendActionCommandRequest::*TRIGGER_MODE_DELAYED*)
        ((= const 2) 'orbbec_camera::SendActionCommandRequest::*TRIGGER_MODE_TIMESTAMP*)
        (t nil)))

(defclass orbbec_camera::SendActionCommandRequest
  :super ros::object
  :slots (_device_key _group_key _group_mask _broadcast_ip _trigger_mode _delay_ms _scheduled_time ))

(defmethod orbbec_camera::SendActionCommandRequest
  (:init
   (&key
    ((:device_key __device_key) 0)
    ((:group_key __group_key) 0)
    ((:group_mask __group_mask) 0)
    ((:broadcast_ip __broadcast_ip) "")
    ((:trigger_mode __trigger_mode) 0)
    ((:delay_ms __delay_ms) 0)
    ((:scheduled_time __scheduled_time) 0)
    )
   (send-super :init)
   (setq _device_key (round __device_key))
   (setq _group_key (round __group_key))
   (setq _group_mask (round __group_mask))
   (setq _broadcast_ip (string __broadcast_ip))
   (setq _trigger_mode (round __trigger_mode))
   (setq _delay_ms (round __delay_ms))
   (setq _scheduled_time (round __scheduled_time))
   self)
  (:device_key
   (&optional __device_key)
   (if __device_key (setq _device_key __device_key)) _device_key)
  (:group_key
   (&optional __group_key)
   (if __group_key (setq _group_key __group_key)) _group_key)
  (:group_mask
   (&optional __group_mask)
   (if __group_mask (setq _group_mask __group_mask)) _group_mask)
  (:broadcast_ip
   (&optional __broadcast_ip)
   (if __broadcast_ip (setq _broadcast_ip __broadcast_ip)) _broadcast_ip)
  (:trigger_mode
   (&optional __trigger_mode)
   (if __trigger_mode (setq _trigger_mode __trigger_mode)) _trigger_mode)
  (:delay_ms
   (&optional __delay_ms)
   (if __delay_ms (setq _delay_ms __delay_ms)) _delay_ms)
  (:scheduled_time
   (&optional __scheduled_time)
   (if __scheduled_time (setq _scheduled_time __scheduled_time)) _scheduled_time)
  (:serialization-length
   ()
   (+
    ;; uint32 _device_key
    4
    ;; uint32 _group_key
    4
    ;; uint32 _group_mask
    4
    ;; string _broadcast_ip
    4 (length _broadcast_ip)
    ;; uint8 _trigger_mode
    1
    ;; uint32 _delay_ms
    4
    ;; uint64 _scheduled_time
    8
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint32 _device_key
       (write-long _device_key s)
     ;; uint32 _group_key
       (write-long _group_key s)
     ;; uint32 _group_mask
       (write-long _group_mask s)
     ;; string _broadcast_ip
       (write-long (length _broadcast_ip) s) (princ _broadcast_ip s)
     ;; uint8 _trigger_mode
       (write-byte _trigger_mode s)
     ;; uint32 _delay_ms
       (write-long _delay_ms s)
     ;; uint64 _scheduled_time
#+(or :alpha :irix6 :x86_64)
       (progn (sys::poke _scheduled_time (send s :buffer) (send s :count) :long) (incf (stream-count s) 8))
#-(or :alpha :irix6 :x86_64)
       (cond ((and (class _scheduled_time) (= (length (_scheduled_time . bv)) 2)) ;; bignum
              (write-long (ash (elt (_scheduled_time . bv) 0) 0) s)
              (write-long (ash (elt (_scheduled_time . bv) 1) -1) s))
             ((and (class _scheduled_time) (= (length (_scheduled_time . bv)) 1)) ;; big1
              (write-long (elt (_scheduled_time . bv) 0) s)
              (write-long (if (>= _scheduled_time 0) 0 #xffffffff) s))
             (t                                         ;; integer
              (write-long _scheduled_time s)(write-long (if (>= _scheduled_time 0) 0 #xffffffff) s)))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint32 _device_key
     (setq _device_key (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _group_key
     (setq _group_key (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _group_mask
     (setq _group_mask (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; string _broadcast_ip
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _broadcast_ip (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; uint8 _trigger_mode
     (setq _trigger_mode (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint32 _delay_ms
     (setq _delay_ms (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint64 _scheduled_time
     
#+(or :alpha :irix6 :x86_64)
      (setf _scheduled_time (prog1 (sys::peek buf ptr- :long) (incf ptr- 8)))
#-(or :alpha :irix6 :x86_64)
      (setf _scheduled_time (let ((b0 (prog1 (sys::peek buf ptr- :integer) (incf ptr- 4)))
                  (b1 (prog1 (sys::peek buf ptr- :integer) (incf ptr- 4))))
              (cond ((= b1 -1) b0)
                     ((and (= b1  0)
                           (<= lisp::most-negative-fixnum b0 lisp::most-positive-fixnum))
                      b0)
                    ((= b1  0) (make-instance bignum :size 1 :bv (integer-vector b0)))
                    (t (make-instance bignum :size 2 :bv (integer-vector b0 (ash b1 1)))))))
   ;;
   self)
  )

(defclass orbbec_camera::SendActionCommandResponse
  :super ros::object
  :slots (_success _encoded_scheduled_time _message ))

(defmethod orbbec_camera::SendActionCommandResponse
  (:init
   (&key
    ((:success __success) nil)
    ((:encoded_scheduled_time __encoded_scheduled_time) 0)
    ((:message __message) "")
    )
   (send-super :init)
   (setq _success __success)
   (setq _encoded_scheduled_time (round __encoded_scheduled_time))
   (setq _message (string __message))
   self)
  (:success
   (&optional (__success :null))
   (if (not (eq __success :null)) (setq _success __success)) _success)
  (:encoded_scheduled_time
   (&optional __encoded_scheduled_time)
   (if __encoded_scheduled_time (setq _encoded_scheduled_time __encoded_scheduled_time)) _encoded_scheduled_time)
  (:message
   (&optional __message)
   (if __message (setq _message __message)) _message)
  (:serialization-length
   ()
   (+
    ;; bool _success
    1
    ;; uint64 _encoded_scheduled_time
    8
    ;; string _message
    4 (length _message)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _success
       (if _success (write-byte -1 s) (write-byte 0 s))
     ;; uint64 _encoded_scheduled_time
#+(or :alpha :irix6 :x86_64)
       (progn (sys::poke _encoded_scheduled_time (send s :buffer) (send s :count) :long) (incf (stream-count s) 8))
#-(or :alpha :irix6 :x86_64)
       (cond ((and (class _encoded_scheduled_time) (= (length (_encoded_scheduled_time . bv)) 2)) ;; bignum
              (write-long (ash (elt (_encoded_scheduled_time . bv) 0) 0) s)
              (write-long (ash (elt (_encoded_scheduled_time . bv) 1) -1) s))
             ((and (class _encoded_scheduled_time) (= (length (_encoded_scheduled_time . bv)) 1)) ;; big1
              (write-long (elt (_encoded_scheduled_time . bv) 0) s)
              (write-long (if (>= _encoded_scheduled_time 0) 0 #xffffffff) s))
             (t                                         ;; integer
              (write-long _encoded_scheduled_time s)(write-long (if (>= _encoded_scheduled_time 0) 0 #xffffffff) s)))
     ;; string _message
       (write-long (length _message) s) (princ _message s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _success
     (setq _success (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; uint64 _encoded_scheduled_time
     
#+(or :alpha :irix6 :x86_64)
      (setf _encoded_scheduled_time (prog1 (sys::peek buf ptr- :long) (incf ptr- 8)))
#-(or :alpha :irix6 :x86_64)
      (setf _encoded_scheduled_time (let ((b0 (prog1 (sys::peek buf ptr- :integer) (incf ptr- 4)))
                  (b1 (prog1 (sys::peek buf ptr- :integer) (incf ptr- 4))))
              (cond ((= b1 -1) b0)
                     ((and (= b1  0)
                           (<= lisp::most-negative-fixnum b0 lisp::most-positive-fixnum))
                      b0)
                    ((= b1  0) (make-instance bignum :size 1 :bv (integer-vector b0)))
                    (t (make-instance bignum :size 2 :bv (integer-vector b0 (ash b1 1)))))))
   ;; string _message
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _message (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(defclass orbbec_camera::SendActionCommand
  :super ros::object
  :slots ())

(setf (get orbbec_camera::SendActionCommand :md5sum-) "c725e105b4f2d4dfe7d74a3ac74cd5d7")
(setf (get orbbec_camera::SendActionCommand :datatype-) "orbbec_camera/SendActionCommand")
(setf (get orbbec_camera::SendActionCommand :request) orbbec_camera::SendActionCommandRequest)
(setf (get orbbec_camera::SendActionCommand :response) orbbec_camera::SendActionCommandResponse)

(defmethod orbbec_camera::SendActionCommandRequest
  (:response () (instance orbbec_camera::SendActionCommandResponse :init)))

(setf (get orbbec_camera::SendActionCommandRequest :md5sum-) "c725e105b4f2d4dfe7d74a3ac74cd5d7")
(setf (get orbbec_camera::SendActionCommandRequest :datatype-) "orbbec_camera/SendActionCommandRequest")
(setf (get orbbec_camera::SendActionCommandRequest :definition-)
      "uint8 TRIGGER_MODE_IMMEDIATE=0
uint8 TRIGGER_MODE_DELAYED=1
uint8 TRIGGER_MODE_TIMESTAMP=2

uint32 device_key
uint32 group_key
uint32 group_mask
# IPv4 destination. Leave empty to use the SDK broadcast address (255.255.255.255).
string broadcast_ip
# Select immediate, relative-delay, or absolute-PTP-time triggering.
uint8 trigger_mode
# Used only with TRIGGER_MODE_DELAYED. Must be positive.
uint32 delay_ms
# Used only with TRIGGER_MODE_TIMESTAMP. Upper 32 bits are seconds and lower 32 bits are nanoseconds.
uint64 scheduled_time
---
# Success means that the SDK dispatched the command; Action Command has no device acknowledgment.
bool success
# Exact GVCP/PTP value sent to the SDK. Zero is used for an immediate command.
uint64 encoded_scheduled_time
string message

")

(setf (get orbbec_camera::SendActionCommandResponse :md5sum-) "c725e105b4f2d4dfe7d74a3ac74cd5d7")
(setf (get orbbec_camera::SendActionCommandResponse :datatype-) "orbbec_camera/SendActionCommandResponse")
(setf (get orbbec_camera::SendActionCommandResponse :definition-)
      "uint8 TRIGGER_MODE_IMMEDIATE=0
uint8 TRIGGER_MODE_DELAYED=1
uint8 TRIGGER_MODE_TIMESTAMP=2

uint32 device_key
uint32 group_key
uint32 group_mask
# IPv4 destination. Leave empty to use the SDK broadcast address (255.255.255.255).
string broadcast_ip
# Select immediate, relative-delay, or absolute-PTP-time triggering.
uint8 trigger_mode
# Used only with TRIGGER_MODE_DELAYED. Must be positive.
uint32 delay_ms
# Used only with TRIGGER_MODE_TIMESTAMP. Upper 32 bits are seconds and lower 32 bits are nanoseconds.
uint64 scheduled_time
---
# Success means that the SDK dispatched the command; Action Command has no device acknowledgment.
bool success
# Exact GVCP/PTP value sent to the SDK. Zero is used for an immediate command.
uint64 encoded_scheduled_time
string message

")



(provide :orbbec_camera/SendActionCommand "c725e105b4f2d4dfe7d74a3ac74cd5d7")


