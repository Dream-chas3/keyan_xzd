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

class GetActionConfigRequest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.selector = null;
    }
    else {
      if (initObj.hasOwnProperty('selector')) {
        this.selector = initObj.selector
      }
      else {
        this.selector = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GetActionConfigRequest
    // Serialize message field [selector]
    bufferOffset = _serializer.uint32(obj.selector, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GetActionConfigRequest
    let len;
    let data = new GetActionConfigRequest(null);
    // Deserialize message field [selector]
    data.selector = _deserializer.uint32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 4;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/GetActionConfigRequest';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '2a9789e6d5c1e80876252848f8443e40';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint32 selector
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GetActionConfigRequest(null);
    if (msg.selector !== undefined) {
      resolved.selector = msg.selector;
    }
    else {
      resolved.selector = 0
    }

    return resolved;
    }
};

class GetActionConfigResponse {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.action_signal_count = null;
      this.device_key = null;
      this.group_key = null;
      this.group_mask = null;
      this.success = null;
      this.message = null;
    }
    else {
      if (initObj.hasOwnProperty('action_signal_count')) {
        this.action_signal_count = initObj.action_signal_count
      }
      else {
        this.action_signal_count = 0;
      }
      if (initObj.hasOwnProperty('device_key')) {
        this.device_key = initObj.device_key
      }
      else {
        this.device_key = 0;
      }
      if (initObj.hasOwnProperty('group_key')) {
        this.group_key = initObj.group_key
      }
      else {
        this.group_key = 0;
      }
      if (initObj.hasOwnProperty('group_mask')) {
        this.group_mask = initObj.group_mask
      }
      else {
        this.group_mask = 0;
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
    // Serializes a message object of type GetActionConfigResponse
    // Serialize message field [action_signal_count]
    bufferOffset = _serializer.uint32(obj.action_signal_count, buffer, bufferOffset);
    // Serialize message field [device_key]
    bufferOffset = _serializer.uint32(obj.device_key, buffer, bufferOffset);
    // Serialize message field [group_key]
    bufferOffset = _serializer.uint32(obj.group_key, buffer, bufferOffset);
    // Serialize message field [group_mask]
    bufferOffset = _serializer.uint32(obj.group_mask, buffer, bufferOffset);
    // Serialize message field [success]
    bufferOffset = _serializer.bool(obj.success, buffer, bufferOffset);
    // Serialize message field [message]
    bufferOffset = _serializer.string(obj.message, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GetActionConfigResponse
    let len;
    let data = new GetActionConfigResponse(null);
    // Deserialize message field [action_signal_count]
    data.action_signal_count = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [device_key]
    data.device_key = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [group_key]
    data.group_key = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [group_mask]
    data.group_mask = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [success]
    data.success = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [message]
    data.message = _deserializer.string(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.message);
    return length + 21;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/GetActionConfigResponse';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '794be0cf88f0c26a5a9e2ce652460072';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint32 action_signal_count
    uint32 device_key
    uint32 group_key
    uint32 group_mask
    bool success
    string message
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GetActionConfigResponse(null);
    if (msg.action_signal_count !== undefined) {
      resolved.action_signal_count = msg.action_signal_count;
    }
    else {
      resolved.action_signal_count = 0
    }

    if (msg.device_key !== undefined) {
      resolved.device_key = msg.device_key;
    }
    else {
      resolved.device_key = 0
    }

    if (msg.group_key !== undefined) {
      resolved.group_key = msg.group_key;
    }
    else {
      resolved.group_key = 0
    }

    if (msg.group_mask !== undefined) {
      resolved.group_mask = msg.group_mask;
    }
    else {
      resolved.group_mask = 0
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
  Request: GetActionConfigRequest,
  Response: GetActionConfigResponse,
  md5sum() { return 'b599f0e1250bf719f4e715cab6e1e366'; },
  datatype() { return 'orbbec_camera/GetActionConfig'; }
};
