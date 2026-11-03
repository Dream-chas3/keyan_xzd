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

class SetAwbGainRequest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.r_gain = null;
      this.b_gain = null;
      this.g_gain = null;
    }
    else {
      if (initObj.hasOwnProperty('r_gain')) {
        this.r_gain = initObj.r_gain
      }
      else {
        this.r_gain = 0;
      }
      if (initObj.hasOwnProperty('b_gain')) {
        this.b_gain = initObj.b_gain
      }
      else {
        this.b_gain = 0;
      }
      if (initObj.hasOwnProperty('g_gain')) {
        this.g_gain = initObj.g_gain
      }
      else {
        this.g_gain = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type SetAwbGainRequest
    // Serialize message field [r_gain]
    bufferOffset = _serializer.uint16(obj.r_gain, buffer, bufferOffset);
    // Serialize message field [b_gain]
    bufferOffset = _serializer.uint16(obj.b_gain, buffer, bufferOffset);
    // Serialize message field [g_gain]
    bufferOffset = _serializer.uint16(obj.g_gain, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SetAwbGainRequest
    let len;
    let data = new SetAwbGainRequest(null);
    // Deserialize message field [r_gain]
    data.r_gain = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [b_gain]
    data.b_gain = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [g_gain]
    data.g_gain = _deserializer.uint16(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 6;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/SetAwbGainRequest';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'ba49b4639eebdf2f5e1caf9b205628b8';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # Raw Q8.8 AWB channel gains.
    uint16 r_gain
    uint16 b_gain
    uint16 g_gain
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SetAwbGainRequest(null);
    if (msg.r_gain !== undefined) {
      resolved.r_gain = msg.r_gain;
    }
    else {
      resolved.r_gain = 0
    }

    if (msg.b_gain !== undefined) {
      resolved.b_gain = msg.b_gain;
    }
    else {
      resolved.b_gain = 0
    }

    if (msg.g_gain !== undefined) {
      resolved.g_gain = msg.g_gain;
    }
    else {
      resolved.g_gain = 0
    }

    return resolved;
    }
};

class SetAwbGainResponse {
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
    // Serializes a message object of type SetAwbGainResponse
    // Serialize message field [success]
    bufferOffset = _serializer.bool(obj.success, buffer, bufferOffset);
    // Serialize message field [message]
    bufferOffset = _serializer.string(obj.message, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SetAwbGainResponse
    let len;
    let data = new SetAwbGainResponse(null);
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
    return 'orbbec_camera/SetAwbGainResponse';
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
    const resolved = new SetAwbGainResponse(null);
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
  Request: SetAwbGainRequest,
  Response: SetAwbGainResponse,
  md5sum() { return '78d4a958be234c52a0c5e570d6402ff4'; },
  datatype() { return 'orbbec_camera/SetAwbGain'; }
};
