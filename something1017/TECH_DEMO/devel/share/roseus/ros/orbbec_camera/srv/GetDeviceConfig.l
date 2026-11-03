;; Auto-generated. Do not edit!


(when (boundp 'orbbec_camera::GetDeviceConfig)
  (if (not (find-package "ORBBEC_CAMERA"))
    (make-package "ORBBEC_CAMERA"))
  (shadow 'GetDeviceConfig (find-package "ORBBEC_CAMERA")))
(unless (find-package "ORBBEC_CAMERA::GETDEVICECONFIG")
  (make-package "ORBBEC_CAMERA::GETDEVICECONFIG"))
(unless (find-package "ORBBEC_CAMERA::GETDEVICECONFIGREQUEST")
  (make-package "ORBBEC_CAMERA::GETDEVICECONFIGREQUEST"))
(unless (find-package "ORBBEC_CAMERA::GETDEVICECONFIGRESPONSE")
  (make-package "ORBBEC_CAMERA::GETDEVICECONFIGRESPONSE"))

(in-package "ROS")





(defclass orbbec_camera::GetDeviceConfigRequest
  :super ros::object
  :slots ())

(defmethod orbbec_camera::GetDeviceConfigRequest
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

(defclass orbbec_camera::GetDeviceConfigResponse
  :super ros::object
  :slots (_schema_version _device_preset _preset_version _preset_depth_work_mode_version _color_preset _depth_precision _disparity_to_depth_mode _exposure_range_mode _depth_registration _align_mode _align_target_stream _frame_aggregate_mode _enable_frame_sync _time_domain _sync_mode _intra_camera_sync_reference _data_json _success _message ))

(defmethod orbbec_camera::GetDeviceConfigResponse
  (:init
   (&key
    ((:schema_version __schema_version) "")
    ((:device_preset __device_preset) "")
    ((:preset_version __preset_version) "")
    ((:preset_depth_work_mode_version __preset_depth_work_mode_version) "")
    ((:color_preset __color_preset) "")
    ((:depth_precision __depth_precision) "")
    ((:disparity_to_depth_mode __disparity_to_depth_mode) "")
    ((:exposure_range_mode __exposure_range_mode) "")
    ((:depth_registration __depth_registration) nil)
    ((:align_mode __align_mode) "")
    ((:align_target_stream __align_target_stream) "")
    ((:frame_aggregate_mode __frame_aggregate_mode) "")
    ((:enable_frame_sync __enable_frame_sync) nil)
    ((:time_domain __time_domain) "")
    ((:sync_mode __sync_mode) "")
    ((:intra_camera_sync_reference __intra_camera_sync_reference) "")
    ((:data_json __data_json) "")
    ((:success __success) nil)
    ((:message __message) "")
    )
   (send-super :init)
   (setq _schema_version (string __schema_version))
   (setq _device_preset (string __device_preset))
   (setq _preset_version (string __preset_version))
   (setq _preset_depth_work_mode_version (string __preset_depth_work_mode_version))
   (setq _color_preset (string __color_preset))
   (setq _depth_precision (string __depth_precision))
   (setq _disparity_to_depth_mode (string __disparity_to_depth_mode))
   (setq _exposure_range_mode (string __exposure_range_mode))
   (setq _depth_registration __depth_registration)
   (setq _align_mode (string __align_mode))
   (setq _align_target_stream (string __align_target_stream))
   (setq _frame_aggregate_mode (string __frame_aggregate_mode))
   (setq _enable_frame_sync __enable_frame_sync)
   (setq _time_domain (string __time_domain))
   (setq _sync_mode (string __sync_mode))
   (setq _intra_camera_sync_reference (string __intra_camera_sync_reference))
   (setq _data_json (string __data_json))
   (setq _success __success)
   (setq _message (string __message))
   self)
  (:schema_version
   (&optional __schema_version)
   (if __schema_version (setq _schema_version __schema_version)) _schema_version)
  (:device_preset
   (&optional __device_preset)
   (if __device_preset (setq _device_preset __device_preset)) _device_preset)
  (:preset_version
   (&optional __preset_version)
   (if __preset_version (setq _preset_version __preset_version)) _preset_version)
  (:preset_depth_work_mode_version
   (&optional __preset_depth_work_mode_version)
   (if __preset_depth_work_mode_version (setq _preset_depth_work_mode_version __preset_depth_work_mode_version)) _preset_depth_work_mode_version)
  (:color_preset
   (&optional __color_preset)
   (if __color_preset (setq _color_preset __color_preset)) _color_preset)
  (:depth_precision
   (&optional __depth_precision)
   (if __depth_precision (setq _depth_precision __depth_precision)) _depth_precision)
  (:disparity_to_depth_mode
   (&optional __disparity_to_depth_mode)
   (if __disparity_to_depth_mode (setq _disparity_to_depth_mode __disparity_to_depth_mode)) _disparity_to_depth_mode)
  (:exposure_range_mode
   (&optional __exposure_range_mode)
   (if __exposure_range_mode (setq _exposure_range_mode __exposure_range_mode)) _exposure_range_mode)
  (:depth_registration
   (&optional (__depth_registration :null))
   (if (not (eq __depth_registration :null)) (setq _depth_registration __depth_registration)) _depth_registration)
  (:align_mode
   (&optional __align_mode)
   (if __align_mode (setq _align_mode __align_mode)) _align_mode)
  (:align_target_stream
   (&optional __align_target_stream)
   (if __align_target_stream (setq _align_target_stream __align_target_stream)) _align_target_stream)
  (:frame_aggregate_mode
   (&optional __frame_aggregate_mode)
   (if __frame_aggregate_mode (setq _frame_aggregate_mode __frame_aggregate_mode)) _frame_aggregate_mode)
  (:enable_frame_sync
   (&optional (__enable_frame_sync :null))
   (if (not (eq __enable_frame_sync :null)) (setq _enable_frame_sync __enable_frame_sync)) _enable_frame_sync)
  (:time_domain
   (&optional __time_domain)
   (if __time_domain (setq _time_domain __time_domain)) _time_domain)
  (:sync_mode
   (&optional __sync_mode)
   (if __sync_mode (setq _sync_mode __sync_mode)) _sync_mode)
  (:intra_camera_sync_reference
   (&optional __intra_camera_sync_reference)
   (if __intra_camera_sync_reference (setq _intra_camera_sync_reference __intra_camera_sync_reference)) _intra_camera_sync_reference)
  (:data_json
   (&optional __data_json)
   (if __data_json (setq _data_json __data_json)) _data_json)
  (:success
   (&optional (__success :null))
   (if (not (eq __success :null)) (setq _success __success)) _success)
  (:message
   (&optional __message)
   (if __message (setq _message __message)) _message)
  (:serialization-length
   ()
   (+
    ;; string _schema_version
    4 (length _schema_version)
    ;; string _device_preset
    4 (length _device_preset)
    ;; string _preset_version
    4 (length _preset_version)
    ;; string _preset_depth_work_mode_version
    4 (length _preset_depth_work_mode_version)
    ;; string _color_preset
    4 (length _color_preset)
    ;; string _depth_precision
    4 (length _depth_precision)
    ;; string _disparity_to_depth_mode
    4 (length _disparity_to_depth_mode)
    ;; string _exposure_range_mode
    4 (length _exposure_range_mode)
    ;; bool _depth_registration
    1
    ;; string _align_mode
    4 (length _align_mode)
    ;; string _align_target_stream
    4 (length _align_target_stream)
    ;; string _frame_aggregate_mode
    4 (length _frame_aggregate_mode)
    ;; bool _enable_frame_sync
    1
    ;; string _time_domain
    4 (length _time_domain)
    ;; string _sync_mode
    4 (length _sync_mode)
    ;; string _intra_camera_sync_reference
    4 (length _intra_camera_sync_reference)
    ;; string _data_json
    4 (length _data_json)
    ;; bool _success
    1
    ;; string _message
    4 (length _message)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; string _schema_version
       (write-long (length _schema_version) s) (princ _schema_version s)
     ;; string _device_preset
       (write-long (length _device_preset) s) (princ _device_preset s)
     ;; string _preset_version
       (write-long (length _preset_version) s) (princ _preset_version s)
     ;; string _preset_depth_work_mode_version
       (write-long (length _preset_depth_work_mode_version) s) (princ _preset_depth_work_mode_version s)
     ;; string _color_preset
       (write-long (length _color_preset) s) (princ _color_preset s)
     ;; string _depth_precision
       (write-long (length _depth_precision) s) (princ _depth_precision s)
     ;; string _disparity_to_depth_mode
       (write-long (length _disparity_to_depth_mode) s) (princ _disparity_to_depth_mode s)
     ;; string _exposure_range_mode
       (write-long (length _exposure_range_mode) s) (princ _exposure_range_mode s)
     ;; bool _depth_registration
       (if _depth_registration (write-byte -1 s) (write-byte 0 s))
     ;; string _align_mode
       (write-long (length _align_mode) s) (princ _align_mode s)
     ;; string _align_target_stream
       (write-long (length _align_target_stream) s) (princ _align_target_stream s)
     ;; string _frame_aggregate_mode
       (write-long (length _frame_aggregate_mode) s) (princ _frame_aggregate_mode s)
     ;; bool _enable_frame_sync
       (if _enable_frame_sync (write-byte -1 s) (write-byte 0 s))
     ;; string _time_domain
       (write-long (length _time_domain) s) (princ _time_domain s)
     ;; string _sync_mode
       (write-long (length _sync_mode) s) (princ _sync_mode s)
     ;; string _intra_camera_sync_reference
       (write-long (length _intra_camera_sync_reference) s) (princ _intra_camera_sync_reference s)
     ;; string _data_json
       (write-long (length _data_json) s) (princ _data_json s)
     ;; bool _success
       (if _success (write-byte -1 s) (write-byte 0 s))
     ;; string _message
       (write-long (length _message) s) (princ _message s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; string _schema_version
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _schema_version (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _device_preset
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _device_preset (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _preset_version
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _preset_version (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _preset_depth_work_mode_version
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _preset_depth_work_mode_version (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _color_preset
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _color_preset (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _depth_precision
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _depth_precision (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _disparity_to_depth_mode
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _disparity_to_depth_mode (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _exposure_range_mode
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _exposure_range_mode (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; bool _depth_registration
     (setq _depth_registration (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _align_mode
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _align_mode (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _align_target_stream
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _align_target_stream (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _frame_aggregate_mode
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _frame_aggregate_mode (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; bool _enable_frame_sync
     (setq _enable_frame_sync (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _time_domain
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _time_domain (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _sync_mode
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _sync_mode (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _intra_camera_sync_reference
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _intra_camera_sync_reference (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _data_json
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _data_json (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; bool _success
     (setq _success (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _message
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _message (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(defclass orbbec_camera::GetDeviceConfig
  :super ros::object
  :slots ())

(setf (get orbbec_camera::GetDeviceConfig :md5sum-) "5df94601ebc864c7f2d9a8d4b93c986e")
(setf (get orbbec_camera::GetDeviceConfig :datatype-) "orbbec_camera/GetDeviceConfig")
(setf (get orbbec_camera::GetDeviceConfig :request) orbbec_camera::GetDeviceConfigRequest)
(setf (get orbbec_camera::GetDeviceConfig :response) orbbec_camera::GetDeviceConfigResponse)

(defmethod orbbec_camera::GetDeviceConfigRequest
  (:response () (instance orbbec_camera::GetDeviceConfigResponse :init)))

(setf (get orbbec_camera::GetDeviceConfigRequest :md5sum-) "5df94601ebc864c7f2d9a8d4b93c986e")
(setf (get orbbec_camera::GetDeviceConfigRequest :datatype-) "orbbec_camera/GetDeviceConfigRequest")
(setf (get orbbec_camera::GetDeviceConfigRequest :definition-)
      "---
# Schema version for this service response. Increment only when field semantics change.
string schema_version

# Effective device configuration state that is not provided by existing device/version services.
string device_preset
# Legacy preset package version reported by the device extension information.
string preset_version
# Current preset's target depth work mode version. Empty when unsupported or unavailable.
string preset_depth_work_mode_version
string color_preset
string depth_precision
string disparity_to_depth_mode
string exposure_range_mode

bool depth_registration
string align_mode
string align_target_stream
string frame_aggregate_mode
bool enable_frame_sync

string time_domain
string sync_mode
string intra_camera_sync_reference

# Reserved for future non-breaking, device-specific extensions.
string data_json

bool success
string message

")

(setf (get orbbec_camera::GetDeviceConfigResponse :md5sum-) "5df94601ebc864c7f2d9a8d4b93c986e")
(setf (get orbbec_camera::GetDeviceConfigResponse :datatype-) "orbbec_camera/GetDeviceConfigResponse")
(setf (get orbbec_camera::GetDeviceConfigResponse :definition-)
      "---
# Schema version for this service response. Increment only when field semantics change.
string schema_version

# Effective device configuration state that is not provided by existing device/version services.
string device_preset
# Legacy preset package version reported by the device extension information.
string preset_version
# Current preset's target depth work mode version. Empty when unsupported or unavailable.
string preset_depth_work_mode_version
string color_preset
string depth_precision
string disparity_to_depth_mode
string exposure_range_mode

bool depth_registration
string align_mode
string align_target_stream
string frame_aggregate_mode
bool enable_frame_sync

string time_domain
string sync_mode
string intra_camera_sync_reference

# Reserved for future non-breaking, device-specific extensions.
string data_json

bool success
string message

")



(provide :orbbec_camera/GetDeviceConfig "5df94601ebc864c7f2d9a8d4b93c986e")


