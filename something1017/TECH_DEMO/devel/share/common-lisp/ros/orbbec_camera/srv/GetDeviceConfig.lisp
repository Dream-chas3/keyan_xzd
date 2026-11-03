; Auto-generated. Do not edit!


(cl:in-package orbbec_camera-srv)


;//! \htmlinclude GetDeviceConfig-request.msg.html

(cl:defclass <GetDeviceConfig-request> (roslisp-msg-protocol:ros-message)
  ()
)

(cl:defclass GetDeviceConfig-request (<GetDeviceConfig-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GetDeviceConfig-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GetDeviceConfig-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<GetDeviceConfig-request> is deprecated: use orbbec_camera-srv:GetDeviceConfig-request instead.")))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GetDeviceConfig-request>) ostream)
  "Serializes a message object of type '<GetDeviceConfig-request>"
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GetDeviceConfig-request>) istream)
  "Deserializes a message object of type '<GetDeviceConfig-request>"
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GetDeviceConfig-request>)))
  "Returns string type for a service object of type '<GetDeviceConfig-request>"
  "orbbec_camera/GetDeviceConfigRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GetDeviceConfig-request)))
  "Returns string type for a service object of type 'GetDeviceConfig-request"
  "orbbec_camera/GetDeviceConfigRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GetDeviceConfig-request>)))
  "Returns md5sum for a message object of type '<GetDeviceConfig-request>"
  "5df94601ebc864c7f2d9a8d4b93c986e")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GetDeviceConfig-request)))
  "Returns md5sum for a message object of type 'GetDeviceConfig-request"
  "5df94601ebc864c7f2d9a8d4b93c986e")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GetDeviceConfig-request>)))
  "Returns full string definition for message of type '<GetDeviceConfig-request>"
  (cl:format cl:nil "~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GetDeviceConfig-request)))
  "Returns full string definition for message of type 'GetDeviceConfig-request"
  (cl:format cl:nil "~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GetDeviceConfig-request>))
  (cl:+ 0
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GetDeviceConfig-request>))
  "Converts a ROS message object to a list"
  (cl:list 'GetDeviceConfig-request
))
;//! \htmlinclude GetDeviceConfig-response.msg.html

(cl:defclass <GetDeviceConfig-response> (roslisp-msg-protocol:ros-message)
  ((schema_version
    :reader schema_version
    :initarg :schema_version
    :type cl:string
    :initform "")
   (device_preset
    :reader device_preset
    :initarg :device_preset
    :type cl:string
    :initform "")
   (preset_version
    :reader preset_version
    :initarg :preset_version
    :type cl:string
    :initform "")
   (preset_depth_work_mode_version
    :reader preset_depth_work_mode_version
    :initarg :preset_depth_work_mode_version
    :type cl:string
    :initform "")
   (color_preset
    :reader color_preset
    :initarg :color_preset
    :type cl:string
    :initform "")
   (depth_precision
    :reader depth_precision
    :initarg :depth_precision
    :type cl:string
    :initform "")
   (disparity_to_depth_mode
    :reader disparity_to_depth_mode
    :initarg :disparity_to_depth_mode
    :type cl:string
    :initform "")
   (exposure_range_mode
    :reader exposure_range_mode
    :initarg :exposure_range_mode
    :type cl:string
    :initform "")
   (depth_registration
    :reader depth_registration
    :initarg :depth_registration
    :type cl:boolean
    :initform cl:nil)
   (align_mode
    :reader align_mode
    :initarg :align_mode
    :type cl:string
    :initform "")
   (align_target_stream
    :reader align_target_stream
    :initarg :align_target_stream
    :type cl:string
    :initform "")
   (frame_aggregate_mode
    :reader frame_aggregate_mode
    :initarg :frame_aggregate_mode
    :type cl:string
    :initform "")
   (enable_frame_sync
    :reader enable_frame_sync
    :initarg :enable_frame_sync
    :type cl:boolean
    :initform cl:nil)
   (time_domain
    :reader time_domain
    :initarg :time_domain
    :type cl:string
    :initform "")
   (sync_mode
    :reader sync_mode
    :initarg :sync_mode
    :type cl:string
    :initform "")
   (intra_camera_sync_reference
    :reader intra_camera_sync_reference
    :initarg :intra_camera_sync_reference
    :type cl:string
    :initform "")
   (data_json
    :reader data_json
    :initarg :data_json
    :type cl:string
    :initform "")
   (success
    :reader success
    :initarg :success
    :type cl:boolean
    :initform cl:nil)
   (message
    :reader message
    :initarg :message
    :type cl:string
    :initform ""))
)

(cl:defclass GetDeviceConfig-response (<GetDeviceConfig-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GetDeviceConfig-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GetDeviceConfig-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name orbbec_camera-srv:<GetDeviceConfig-response> is deprecated: use orbbec_camera-srv:GetDeviceConfig-response instead.")))

(cl:ensure-generic-function 'schema_version-val :lambda-list '(m))
(cl:defmethod schema_version-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:schema_version-val is deprecated.  Use orbbec_camera-srv:schema_version instead.")
  (schema_version m))

(cl:ensure-generic-function 'device_preset-val :lambda-list '(m))
(cl:defmethod device_preset-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:device_preset-val is deprecated.  Use orbbec_camera-srv:device_preset instead.")
  (device_preset m))

(cl:ensure-generic-function 'preset_version-val :lambda-list '(m))
(cl:defmethod preset_version-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:preset_version-val is deprecated.  Use orbbec_camera-srv:preset_version instead.")
  (preset_version m))

(cl:ensure-generic-function 'preset_depth_work_mode_version-val :lambda-list '(m))
(cl:defmethod preset_depth_work_mode_version-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:preset_depth_work_mode_version-val is deprecated.  Use orbbec_camera-srv:preset_depth_work_mode_version instead.")
  (preset_depth_work_mode_version m))

(cl:ensure-generic-function 'color_preset-val :lambda-list '(m))
(cl:defmethod color_preset-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:color_preset-val is deprecated.  Use orbbec_camera-srv:color_preset instead.")
  (color_preset m))

(cl:ensure-generic-function 'depth_precision-val :lambda-list '(m))
(cl:defmethod depth_precision-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:depth_precision-val is deprecated.  Use orbbec_camera-srv:depth_precision instead.")
  (depth_precision m))

(cl:ensure-generic-function 'disparity_to_depth_mode-val :lambda-list '(m))
(cl:defmethod disparity_to_depth_mode-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:disparity_to_depth_mode-val is deprecated.  Use orbbec_camera-srv:disparity_to_depth_mode instead.")
  (disparity_to_depth_mode m))

(cl:ensure-generic-function 'exposure_range_mode-val :lambda-list '(m))
(cl:defmethod exposure_range_mode-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:exposure_range_mode-val is deprecated.  Use orbbec_camera-srv:exposure_range_mode instead.")
  (exposure_range_mode m))

(cl:ensure-generic-function 'depth_registration-val :lambda-list '(m))
(cl:defmethod depth_registration-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:depth_registration-val is deprecated.  Use orbbec_camera-srv:depth_registration instead.")
  (depth_registration m))

(cl:ensure-generic-function 'align_mode-val :lambda-list '(m))
(cl:defmethod align_mode-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:align_mode-val is deprecated.  Use orbbec_camera-srv:align_mode instead.")
  (align_mode m))

(cl:ensure-generic-function 'align_target_stream-val :lambda-list '(m))
(cl:defmethod align_target_stream-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:align_target_stream-val is deprecated.  Use orbbec_camera-srv:align_target_stream instead.")
  (align_target_stream m))

(cl:ensure-generic-function 'frame_aggregate_mode-val :lambda-list '(m))
(cl:defmethod frame_aggregate_mode-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:frame_aggregate_mode-val is deprecated.  Use orbbec_camera-srv:frame_aggregate_mode instead.")
  (frame_aggregate_mode m))

(cl:ensure-generic-function 'enable_frame_sync-val :lambda-list '(m))
(cl:defmethod enable_frame_sync-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:enable_frame_sync-val is deprecated.  Use orbbec_camera-srv:enable_frame_sync instead.")
  (enable_frame_sync m))

(cl:ensure-generic-function 'time_domain-val :lambda-list '(m))
(cl:defmethod time_domain-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:time_domain-val is deprecated.  Use orbbec_camera-srv:time_domain instead.")
  (time_domain m))

(cl:ensure-generic-function 'sync_mode-val :lambda-list '(m))
(cl:defmethod sync_mode-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:sync_mode-val is deprecated.  Use orbbec_camera-srv:sync_mode instead.")
  (sync_mode m))

(cl:ensure-generic-function 'intra_camera_sync_reference-val :lambda-list '(m))
(cl:defmethod intra_camera_sync_reference-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:intra_camera_sync_reference-val is deprecated.  Use orbbec_camera-srv:intra_camera_sync_reference instead.")
  (intra_camera_sync_reference m))

(cl:ensure-generic-function 'data_json-val :lambda-list '(m))
(cl:defmethod data_json-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:data_json-val is deprecated.  Use orbbec_camera-srv:data_json instead.")
  (data_json m))

(cl:ensure-generic-function 'success-val :lambda-list '(m))
(cl:defmethod success-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:success-val is deprecated.  Use orbbec_camera-srv:success instead.")
  (success m))

(cl:ensure-generic-function 'message-val :lambda-list '(m))
(cl:defmethod message-val ((m <GetDeviceConfig-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader orbbec_camera-srv:message-val is deprecated.  Use orbbec_camera-srv:message instead.")
  (message m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GetDeviceConfig-response>) ostream)
  "Serializes a message object of type '<GetDeviceConfig-response>"
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'schema_version))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'schema_version))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'device_preset))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'device_preset))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'preset_version))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'preset_version))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'preset_depth_work_mode_version))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'preset_depth_work_mode_version))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'color_preset))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'color_preset))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'depth_precision))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'depth_precision))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'disparity_to_depth_mode))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'disparity_to_depth_mode))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'exposure_range_mode))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'exposure_range_mode))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'depth_registration) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'align_mode))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'align_mode))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'align_target_stream))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'align_target_stream))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'frame_aggregate_mode))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'frame_aggregate_mode))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'enable_frame_sync) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'time_domain))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'time_domain))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'sync_mode))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'sync_mode))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'intra_camera_sync_reference))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'intra_camera_sync_reference))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'data_json))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'data_json))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'success) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'message))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'message))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GetDeviceConfig-response>) istream)
  "Deserializes a message object of type '<GetDeviceConfig-response>"
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'schema_version) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'schema_version) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'device_preset) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'device_preset) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'preset_version) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'preset_version) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'preset_depth_work_mode_version) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'preset_depth_work_mode_version) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'color_preset) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'color_preset) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'depth_precision) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'depth_precision) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'disparity_to_depth_mode) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'disparity_to_depth_mode) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'exposure_range_mode) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'exposure_range_mode) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:slot-value msg 'depth_registration) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'align_mode) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'align_mode) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'align_target_stream) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'align_target_stream) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'frame_aggregate_mode) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'frame_aggregate_mode) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:slot-value msg 'enable_frame_sync) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'time_domain) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'time_domain) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'sync_mode) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'sync_mode) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'intra_camera_sync_reference) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'intra_camera_sync_reference) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'data_json) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'data_json) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:slot-value msg 'success) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'message) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'message) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GetDeviceConfig-response>)))
  "Returns string type for a service object of type '<GetDeviceConfig-response>"
  "orbbec_camera/GetDeviceConfigResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GetDeviceConfig-response)))
  "Returns string type for a service object of type 'GetDeviceConfig-response"
  "orbbec_camera/GetDeviceConfigResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GetDeviceConfig-response>)))
  "Returns md5sum for a message object of type '<GetDeviceConfig-response>"
  "5df94601ebc864c7f2d9a8d4b93c986e")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GetDeviceConfig-response)))
  "Returns md5sum for a message object of type 'GetDeviceConfig-response"
  "5df94601ebc864c7f2d9a8d4b93c986e")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GetDeviceConfig-response>)))
  "Returns full string definition for message of type '<GetDeviceConfig-response>"
  (cl:format cl:nil "# Schema version for this service response. Increment only when field semantics change.~%string schema_version~%~%# Effective device configuration state that is not provided by existing device/version services.~%string device_preset~%# Legacy preset package version reported by the device extension information.~%string preset_version~%# Current preset's target depth work mode version. Empty when unsupported or unavailable.~%string preset_depth_work_mode_version~%string color_preset~%string depth_precision~%string disparity_to_depth_mode~%string exposure_range_mode~%~%bool depth_registration~%string align_mode~%string align_target_stream~%string frame_aggregate_mode~%bool enable_frame_sync~%~%string time_domain~%string sync_mode~%string intra_camera_sync_reference~%~%# Reserved for future non-breaking, device-specific extensions.~%string data_json~%~%bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GetDeviceConfig-response)))
  "Returns full string definition for message of type 'GetDeviceConfig-response"
  (cl:format cl:nil "# Schema version for this service response. Increment only when field semantics change.~%string schema_version~%~%# Effective device configuration state that is not provided by existing device/version services.~%string device_preset~%# Legacy preset package version reported by the device extension information.~%string preset_version~%# Current preset's target depth work mode version. Empty when unsupported or unavailable.~%string preset_depth_work_mode_version~%string color_preset~%string depth_precision~%string disparity_to_depth_mode~%string exposure_range_mode~%~%bool depth_registration~%string align_mode~%string align_target_stream~%string frame_aggregate_mode~%bool enable_frame_sync~%~%string time_domain~%string sync_mode~%string intra_camera_sync_reference~%~%# Reserved for future non-breaking, device-specific extensions.~%string data_json~%~%bool success~%string message~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GetDeviceConfig-response>))
  (cl:+ 0
     4 (cl:length (cl:slot-value msg 'schema_version))
     4 (cl:length (cl:slot-value msg 'device_preset))
     4 (cl:length (cl:slot-value msg 'preset_version))
     4 (cl:length (cl:slot-value msg 'preset_depth_work_mode_version))
     4 (cl:length (cl:slot-value msg 'color_preset))
     4 (cl:length (cl:slot-value msg 'depth_precision))
     4 (cl:length (cl:slot-value msg 'disparity_to_depth_mode))
     4 (cl:length (cl:slot-value msg 'exposure_range_mode))
     1
     4 (cl:length (cl:slot-value msg 'align_mode))
     4 (cl:length (cl:slot-value msg 'align_target_stream))
     4 (cl:length (cl:slot-value msg 'frame_aggregate_mode))
     1
     4 (cl:length (cl:slot-value msg 'time_domain))
     4 (cl:length (cl:slot-value msg 'sync_mode))
     4 (cl:length (cl:slot-value msg 'intra_camera_sync_reference))
     4 (cl:length (cl:slot-value msg 'data_json))
     1
     4 (cl:length (cl:slot-value msg 'message))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GetDeviceConfig-response>))
  "Converts a ROS message object to a list"
  (cl:list 'GetDeviceConfig-response
    (cl:cons ':schema_version (schema_version msg))
    (cl:cons ':device_preset (device_preset msg))
    (cl:cons ':preset_version (preset_version msg))
    (cl:cons ':preset_depth_work_mode_version (preset_depth_work_mode_version msg))
    (cl:cons ':color_preset (color_preset msg))
    (cl:cons ':depth_precision (depth_precision msg))
    (cl:cons ':disparity_to_depth_mode (disparity_to_depth_mode msg))
    (cl:cons ':exposure_range_mode (exposure_range_mode msg))
    (cl:cons ':depth_registration (depth_registration msg))
    (cl:cons ':align_mode (align_mode msg))
    (cl:cons ':align_target_stream (align_target_stream msg))
    (cl:cons ':frame_aggregate_mode (frame_aggregate_mode msg))
    (cl:cons ':enable_frame_sync (enable_frame_sync msg))
    (cl:cons ':time_domain (time_domain msg))
    (cl:cons ':sync_mode (sync_mode msg))
    (cl:cons ':intra_camera_sync_reference (intra_camera_sync_reference msg))
    (cl:cons ':data_json (data_json msg))
    (cl:cons ':success (success msg))
    (cl:cons ':message (message msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'GetDeviceConfig)))
  'GetDeviceConfig-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'GetDeviceConfig)))
  'GetDeviceConfig-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GetDeviceConfig)))
  "Returns string type for a service object of type '<GetDeviceConfig>"
  "orbbec_camera/GetDeviceConfig")