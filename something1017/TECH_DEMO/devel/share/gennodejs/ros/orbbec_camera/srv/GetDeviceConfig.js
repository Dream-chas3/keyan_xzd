// Auto-generated. Do not edit!

// (in-package orbbec_camera.srv)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------


//-----------------------------------------------------------

class GetDeviceConfigRequest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
    }
    else {
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GetDeviceConfigRequest
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GetDeviceConfigRequest
    let len;
    let data = new GetDeviceConfigRequest(null);
    return data;
  }

  static getMessageSize(object) {
    return 0;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/GetDeviceConfigRequest';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'd41d8cd98f00b204e9800998ecf8427e';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GetDeviceConfigRequest(null);
    return resolved;
    }
};

class GetDeviceConfigResponse {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.schema_version = null;
      this.device_preset = null;
      this.preset_version = null;
      this.preset_depth_work_mode_version = null;
      this.color_preset = null;
      this.depth_precision = null;
      this.disparity_to_depth_mode = null;
      this.exposure_range_mode = null;
      this.depth_registration = null;
      this.align_mode = null;
      this.align_target_stream = null;
      this.frame_aggregate_mode = null;
      this.enable_frame_sync = null;
      this.time_domain = null;
      this.sync_mode = null;
      this.intra_camera_sync_reference = null;
      this.data_json = null;
      this.success = null;
      this.message = null;
    }
    else {
      if (initObj.hasOwnProperty('schema_version')) {
        this.schema_version = initObj.schema_version
      }
      else {
        this.schema_version = '';
      }
      if (initObj.hasOwnProperty('device_preset')) {
        this.device_preset = initObj.device_preset
      }
      else {
        this.device_preset = '';
      }
      if (initObj.hasOwnProperty('preset_version')) {
        this.preset_version = initObj.preset_version
      }
      else {
        this.preset_version = '';
      }
      if (initObj.hasOwnProperty('preset_depth_work_mode_version')) {
        this.preset_depth_work_mode_version = initObj.preset_depth_work_mode_version
      }
      else {
        this.preset_depth_work_mode_version = '';
      }
      if (initObj.hasOwnProperty('color_preset')) {
        this.color_preset = initObj.color_preset
      }
      else {
        this.color_preset = '';
      }
      if (initObj.hasOwnProperty('depth_precision')) {
        this.depth_precision = initObj.depth_precision
      }
      else {
        this.depth_precision = '';
      }
      if (initObj.hasOwnProperty('disparity_to_depth_mode')) {
        this.disparity_to_depth_mode = initObj.disparity_to_depth_mode
      }
      else {
        this.disparity_to_depth_mode = '';
      }
      if (initObj.hasOwnProperty('exposure_range_mode')) {
        this.exposure_range_mode = initObj.exposure_range_mode
      }
      else {
        this.exposure_range_mode = '';
      }
      if (initObj.hasOwnProperty('depth_registration')) {
        this.depth_registration = initObj.depth_registration
      }
      else {
        this.depth_registration = false;
      }
      if (initObj.hasOwnProperty('align_mode')) {
        this.align_mode = initObj.align_mode
      }
      else {
        this.align_mode = '';
      }
      if (initObj.hasOwnProperty('align_target_stream')) {
        this.align_target_stream = initObj.align_target_stream
      }
      else {
        this.align_target_stream = '';
      }
      if (initObj.hasOwnProperty('frame_aggregate_mode')) {
        this.frame_aggregate_mode = initObj.frame_aggregate_mode
      }
      else {
        this.frame_aggregate_mode = '';
      }
      if (initObj.hasOwnProperty('enable_frame_sync')) {
        this.enable_frame_sync = initObj.enable_frame_sync
      }
      else {
        this.enable_frame_sync = false;
      }
      if (initObj.hasOwnProperty('time_domain')) {
        this.time_domain = initObj.time_domain
      }
      else {
        this.time_domain = '';
      }
      if (initObj.hasOwnProperty('sync_mode')) {
        this.sync_mode = initObj.sync_mode
      }
      else {
        this.sync_mode = '';
      }
      if (initObj.hasOwnProperty('intra_camera_sync_reference')) {
        this.intra_camera_sync_reference = initObj.intra_camera_sync_reference
      }
      else {
        this.intra_camera_sync_reference = '';
      }
      if (initObj.hasOwnProperty('data_json')) {
        this.data_json = initObj.data_json
      }
      else {
        this.data_json = '';
      }
      if (initObj.hasOwnProperty('success')) {
        this.success = initObj.success
      }
      else {
        this.success = false;
      }
      if (initObj.hasOwnProperty('message')) {
        this.message = initObj.message
      }
      else {
        this.message = '';
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GetDeviceConfigResponse
    // Serialize message field [schema_version]
    bufferOffset = _serializer.string(obj.schema_version, buffer, bufferOffset);
    // Serialize message field [device_preset]
    bufferOffset = _serializer.string(obj.device_preset, buffer, bufferOffset);
    // Serialize message field [preset_version]
    bufferOffset = _serializer.string(obj.preset_version, buffer, bufferOffset);
    // Serialize message field [preset_depth_work_mode_version]
    bufferOffset = _serializer.string(obj.preset_depth_work_mode_version, buffer, bufferOffset);
    // Serialize message field [color_preset]
    bufferOffset = _serializer.string(obj.color_preset, buffer, bufferOffset);
    // Serialize message field [depth_precision]
    bufferOffset = _serializer.string(obj.depth_precision, buffer, bufferOffset);
    // Serialize message field [disparity_to_depth_mode]
    bufferOffset = _serializer.string(obj.disparity_to_depth_mode, buffer, bufferOffset);
    // Serialize message field [exposure_range_mode]
    bufferOffset = _serializer.string(obj.exposure_range_mode, buffer, bufferOffset);
    // Serialize message field [depth_registration]
    bufferOffset = _serializer.bool(obj.depth_registration, buffer, bufferOffset);
    // Serialize message field [align_mode]
    bufferOffset = _serializer.string(obj.align_mode, buffer, bufferOffset);
    // Serialize message field [align_target_stream]
    bufferOffset = _serializer.string(obj.align_target_stream, buffer, bufferOffset);
    // Serialize message field [frame_aggregate_mode]
    bufferOffset = _serializer.string(obj.frame_aggregate_mode, buffer, bufferOffset);
    // Serialize message field [enable_frame_sync]
    bufferOffset = _serializer.bool(obj.enable_frame_sync, buffer, bufferOffset);
    // Serialize message field [time_domain]
    bufferOffset = _serializer.string(obj.time_domain, buffer, bufferOffset);
    // Serialize message field [sync_mode]
    bufferOffset = _serializer.string(obj.sync_mode, buffer, bufferOffset);
    // Serialize message field [intra_camera_sync_reference]
    bufferOffset = _serializer.string(obj.intra_camera_sync_reference, buffer, bufferOffset);
    // Serialize message field [data_json]
    bufferOffset = _serializer.string(obj.data_json, buffer, bufferOffset);
    // Serialize message field [success]
    bufferOffset = _serializer.bool(obj.success, buffer, bufferOffset);
    // Serialize message field [message]
    bufferOffset = _serializer.string(obj.message, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GetDeviceConfigResponse
    let len;
    let data = new GetDeviceConfigResponse(null);
    // Deserialize message field [schema_version]
    data.schema_version = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [device_preset]
    data.device_preset = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [preset_version]
    data.preset_version = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [preset_depth_work_mode_version]
    data.preset_depth_work_mode_version = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [color_preset]
    data.color_preset = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [depth_precision]
    data.depth_precision = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [disparity_to_depth_mode]
    data.disparity_to_depth_mode = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [exposure_range_mode]
    data.exposure_range_mode = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [depth_registration]
    data.depth_registration = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [align_mode]
    data.align_mode = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [align_target_stream]
    data.align_target_stream = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [frame_aggregate_mode]
    data.frame_aggregate_mode = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [enable_frame_sync]
    data.enable_frame_sync = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [time_domain]
    data.time_domain = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [sync_mode]
    data.sync_mode = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [intra_camera_sync_reference]
    data.intra_camera_sync_reference = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [data_json]
    data.data_json = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [success]
    data.success = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [message]
    data.message = _deserializer.string(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.schema_version);
    length += _getByteLength(object.device_preset);
    length += _getByteLength(object.preset_version);
    length += _getByteLength(object.preset_depth_work_mode_version);
    length += _getByteLength(object.color_preset);
    length += _getByteLength(object.depth_precision);
    length += _getByteLength(object.disparity_to_depth_mode);
    length += _getByteLength(object.exposure_range_mode);
    length += _getByteLength(object.align_mode);
    length += _getByteLength(object.align_target_stream);
    length += _getByteLength(object.frame_aggregate_mode);
    length += _getByteLength(object.time_domain);
    length += _getByteLength(object.sync_mode);
    length += _getByteLength(object.intra_camera_sync_reference);
    length += _getByteLength(object.data_json);
    length += _getByteLength(object.message);
    return length + 67;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/GetDeviceConfigResponse';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '5df94601ebc864c7f2d9a8d4b93c986e';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
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
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GetDeviceConfigResponse(null);
    if (msg.schema_version !== undefined) {
      resolved.schema_version = msg.schema_version;
    }
    else {
      resolved.schema_version = ''
    }

    if (msg.device_preset !== undefined) {
      resolved.device_preset = msg.device_preset;
    }
    else {
      resolved.device_preset = ''
    }

    if (msg.preset_version !== undefined) {
      resolved.preset_version = msg.preset_version;
    }
    else {
      resolved.preset_version = ''
    }

    if (msg.preset_depth_work_mode_version !== undefined) {
      resolved.preset_depth_work_mode_version = msg.preset_depth_work_mode_version;
    }
    else {
      resolved.preset_depth_work_mode_version = ''
    }

    if (msg.color_preset !== undefined) {
      resolved.color_preset = msg.color_preset;
    }
    else {
      resolved.color_preset = ''
    }

    if (msg.depth_precision !== undefined) {
      resolved.depth_precision = msg.depth_precision;
    }
    else {
      resolved.depth_precision = ''
    }

    if (msg.disparity_to_depth_mode !== undefined) {
      resolved.disparity_to_depth_mode = msg.disparity_to_depth_mode;
    }
    else {
      resolved.disparity_to_depth_mode = ''
    }

    if (msg.exposure_range_mode !== undefined) {
      resolved.exposure_range_mode = msg.exposure_range_mode;
    }
    else {
      resolved.exposure_range_mode = ''
    }

    if (msg.depth_registration !== undefined) {
      resolved.depth_registration = msg.depth_registration;
    }
    else {
      resolved.depth_registration = false
    }

    if (msg.align_mode !== undefined) {
      resolved.align_mode = msg.align_mode;
    }
    else {
      resolved.align_mode = ''
    }

    if (msg.align_target_stream !== undefined) {
      resolved.align_target_stream = msg.align_target_stream;
    }
    else {
      resolved.align_target_stream = ''
    }

    if (msg.frame_aggregate_mode !== undefined) {
      resolved.frame_aggregate_mode = msg.frame_aggregate_mode;
    }
    else {
      resolved.frame_aggregate_mode = ''
    }

    if (msg.enable_frame_sync !== undefined) {
      resolved.enable_frame_sync = msg.enable_frame_sync;
    }
    else {
      resolved.enable_frame_sync = false
    }

    if (msg.time_domain !== undefined) {
      resolved.time_domain = msg.time_domain;
    }
    else {
      resolved.time_domain = ''
    }

    if (msg.sync_mode !== undefined) {
      resolved.sync_mode = msg.sync_mode;
    }
    else {
      resolved.sync_mode = ''
    }

    if (msg.intra_camera_sync_reference !== undefined) {
      resolved.intra_camera_sync_reference = msg.intra_camera_sync_reference;
    }
    else {
      resolved.intra_camera_sync_reference = ''
    }

    if (msg.data_json !== undefined) {
      resolved.data_json = msg.data_json;
    }
    else {
      resolved.data_json = ''
    }

    if (msg.success !== undefined) {
      resolved.success = msg.success;
    }
    else {
      resolved.success = false
    }

    if (msg.message !== undefined) {
      resolved.message = msg.message;
    }
    else {
      resolved.message = ''
    }

    return resolved;
    }
};

module.exports = {
  Request: GetDeviceConfigRequest,
  Response: GetDeviceConfigResponse,
  md5sum() { return '5df94601ebc864c7f2d9a8d4b93c986e'; },
  datatype() { return 'orbbec_camera/GetDeviceConfig'; }
};
