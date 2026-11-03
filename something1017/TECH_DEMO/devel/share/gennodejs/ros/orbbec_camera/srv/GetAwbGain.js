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

class GetAwbGainRequest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
    }
    else {
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GetAwbGainRequest
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GetAwbGainRequest
    let len;
    let data = new GetAwbGainRequest(null);
    return data;
  }

  static getMessageSize(object) {
    return 0;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/GetAwbGainRequest';
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
    const resolved = new GetAwbGainRequest(null);
    return resolved;
    }
};

class GetAwbGainResponse {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.r_gain = null;
      this.b_gain = null;
      this.g_gain = null;
      this.success = null;
      this.message = null;
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
    // Serializes a message object of type GetAwbGainResponse
    // Serialize message field [r_gain]
    bufferOffset = _serializer.uint16(obj.r_gain, buffer, bufferOffset);
    // Serialize message field [b_gain]
    bufferOffset = _serializer.uint16(obj.b_gain, buffer, bufferOffset);
    // Serialize message field [g_gain]
    bufferOffset = _serializer.uint16(obj.g_gain, buffer, bufferOffset);
    // Serialize message field [success]
    bufferOffset = _serializer.bool(obj.success, buffer, bufferOffset);
    // Serialize message field [message]
    bufferOffset = _serializer.string(obj.message, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GetAwbGainResponse
    let len;
    let data = new GetAwbGainResponse(null);
    // Deserialize message field [r_gain]
    data.r_gain = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [b_gain]
    data.b_gain = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [g_gain]
    data.g_gain = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [success]
    data.success = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [message]
    data.message = _deserializer.string(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.message);
    return length + 11;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/GetAwbGainResponse';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '0942635903184795f22b4ac62ff74930';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # Raw Q8.8 AWB channel gains.
    uint16 r_gain
    uint16 b_gain
    uint16 g_gain
    bool success
    string message
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GetAwbGainResponse(null);
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
  Request: GetAwbGainRequest,
  Response: GetAwbGainResponse,
  md5sum() { return '0942635903184795f22b4ac62ff74930'; },
  datatype() { return 'orbbec_camera/GetAwbGain'; }
};
