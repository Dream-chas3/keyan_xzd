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

class SetActionConfigRequest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.device_key = null;
      this.selector = null;
      this.group_key = null;
      this.group_mask = null;
    }
    else {
      if (initObj.hasOwnProperty('device_key')) {
        this.device_key = initObj.device_key
      }
      else {
        this.device_key = 0;
      }
      if (initObj.hasOwnProperty('selector')) {
        this.selector = initObj.selector
      }
      else {
        this.selector = 0;
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
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type SetActionConfigRequest
    // Serialize message field [device_key]
    bufferOffset = _serializer.uint32(obj.device_key, buffer, bufferOffset);
    // Serialize message field [selector]
    bufferOffset = _serializer.uint32(obj.selector, buffer, bufferOffset);
    // Serialize message field [group_key]
    bufferOffset = _serializer.uint32(obj.group_key, buffer, bufferOffset);
    // Serialize message field [group_mask]
    bufferOffset = _serializer.uint32(obj.group_mask, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SetActionConfigRequest
    let len;
    let data = new SetActionConfigRequest(null);
    // Deserialize message field [device_key]
    data.device_key = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [selector]
    data.selector = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [group_key]
    data.group_key = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [group_mask]
    data.group_mask = _deserializer.uint32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 16;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/SetActionConfigRequest';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'f7a9516a3bd11f406cc36294de614360';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint32 device_key
    uint32 selector
    uint32 group_key
    uint32 group_mask
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SetActionConfigRequest(null);
    if (msg.device_key !== undefined) {
      resolved.device_key = msg.device_key;
    }
    else {
      resolved.device_key = 0
    }

    if (msg.selector !== undefined) {
      resolved.selector = msg.selector;
    }
    else {
      resolved.selector = 0
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

    return resolved;
    }
};

class SetActionConfigResponse {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.success = null;
      this.message = null;
    }
    else {
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
    // Serializes a message object of type SetActionConfigResponse
    // Serialize message field [success]
    bufferOffset = _serializer.bool(obj.success, buffer, bufferOffset);
    // Serialize message field [message]
    bufferOffset = _serializer.string(obj.message, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SetActionConfigResponse
    let len;
    let data = new SetActionConfigResponse(null);
    // Deserialize message field [success]
    data.success = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [message]
    data.message = _deserializer.string(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.message);
    return length + 5;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/SetActionConfigResponse';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '937c9679a518e3a18d831e57125ea522';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool success
    string message
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SetActionConfigResponse(null);
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
  Request: SetActionConfigRequest,
  Response: SetActionConfigResponse,
  md5sum() { return '6df9d533c9f8d312e2eb4484d8d2f98b'; },
  datatype() { return 'orbbec_camera/SetActionConfig'; }
};
