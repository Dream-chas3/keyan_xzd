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

class SendActionCommandRequest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.device_key = null;
      this.group_key = null;
      this.group_mask = null;
      this.broadcast_ip = null;
      this.trigger_mode = null;
      this.delay_ms = null;
      this.scheduled_time = null;
    }
    else {
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
      if (initObj.hasOwnProperty('broadcast_ip')) {
        this.broadcast_ip = initObj.broadcast_ip
      }
      else {
        this.broadcast_ip = '';
      }
      if (initObj.hasOwnProperty('trigger_mode')) {
        this.trigger_mode = initObj.trigger_mode
      }
      else {
        this.trigger_mode = 0;
      }
      if (initObj.hasOwnProperty('delay_ms')) {
        this.delay_ms = initObj.delay_ms
      }
      else {
        this.delay_ms = 0;
      }
      if (initObj.hasOwnProperty('scheduled_time')) {
        this.scheduled_time = initObj.scheduled_time
      }
      else {
        this.scheduled_time = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type SendActionCommandRequest
    // Serialize message field [device_key]
    bufferOffset = _serializer.uint32(obj.device_key, buffer, bufferOffset);
    // Serialize message field [group_key]
    bufferOffset = _serializer.uint32(obj.group_key, buffer, bufferOffset);
    // Serialize message field [group_mask]
    bufferOffset = _serializer.uint32(obj.group_mask, buffer, bufferOffset);
    // Serialize message field [broadcast_ip]
    bufferOffset = _serializer.string(obj.broadcast_ip, buffer, bufferOffset);
    // Serialize message field [trigger_mode]
    bufferOffset = _serializer.uint8(obj.trigger_mode, buffer, bufferOffset);
    // Serialize message field [delay_ms]
    bufferOffset = _serializer.uint32(obj.delay_ms, buffer, bufferOffset);
    // Serialize message field [scheduled_time]
    bufferOffset = _serializer.uint64(obj.scheduled_time, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SendActionCommandRequest
    let len;
    let data = new SendActionCommandRequest(null);
    // Deserialize message field [device_key]
    data.device_key = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [group_key]
    data.group_key = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [group_mask]
    data.group_mask = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [broadcast_ip]
    data.broadcast_ip = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [trigger_mode]
    data.trigger_mode = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [delay_ms]
    data.delay_ms = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [scheduled_time]
    data.scheduled_time = _deserializer.uint64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.broadcast_ip);
    return length + 29;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/SendActionCommandRequest';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '0df160849bdfb2980b03bf6634190c95';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint8 TRIGGER_MODE_IMMEDIATE=0
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
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SendActionCommandRequest(null);
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

    if (msg.broadcast_ip !== undefined) {
      resolved.broadcast_ip = msg.broadcast_ip;
    }
    else {
      resolved.broadcast_ip = ''
    }

    if (msg.trigger_mode !== undefined) {
      resolved.trigger_mode = msg.trigger_mode;
    }
    else {
      resolved.trigger_mode = 0
    }

    if (msg.delay_ms !== undefined) {
      resolved.delay_ms = msg.delay_ms;
    }
    else {
      resolved.delay_ms = 0
    }

    if (msg.scheduled_time !== undefined) {
      resolved.scheduled_time = msg.scheduled_time;
    }
    else {
      resolved.scheduled_time = 0
    }

    return resolved;
    }
};

// Constants for message
SendActionCommandRequest.Constants = {
  TRIGGER_MODE_IMMEDIATE: 0,
  TRIGGER_MODE_DELAYED: 1,
  TRIGGER_MODE_TIMESTAMP: 2,
}

class SendActionCommandResponse {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.success = null;
      this.encoded_scheduled_time = null;
      this.message = null;
    }
    else {
      if (initObj.hasOwnProperty('success')) {
        this.success = initObj.success
      }
      else {
        this.success = false;
      }
      if (initObj.hasOwnProperty('encoded_scheduled_time')) {
        this.encoded_scheduled_time = initObj.encoded_scheduled_time
      }
      else {
        this.encoded_scheduled_time = 0;
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
    // Serializes a message object of type SendActionCommandResponse
    // Serialize message field [success]
    bufferOffset = _serializer.bool(obj.success, buffer, bufferOffset);
    // Serialize message field [encoded_scheduled_time]
    bufferOffset = _serializer.uint64(obj.encoded_scheduled_time, buffer, bufferOffset);
    // Serialize message field [message]
    bufferOffset = _serializer.string(obj.message, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SendActionCommandResponse
    let len;
    let data = new SendActionCommandResponse(null);
    // Deserialize message field [success]
    data.success = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [encoded_scheduled_time]
    data.encoded_scheduled_time = _deserializer.uint64(buffer, bufferOffset);
    // Deserialize message field [message]
    data.message = _deserializer.string(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.message);
    return length + 13;
  }

  static datatype() {
    // Returns string type for a service object
    return 'orbbec_camera/SendActionCommandResponse';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '72dc76d59c271c39a018a38da49c6b18';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # Success means that the SDK dispatched the command; Action Command has no device acknowledgment.
    bool success
    # Exact GVCP/PTP value sent to the SDK. Zero is used for an immediate command.
    uint64 encoded_scheduled_time
    string message
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SendActionCommandResponse(null);
    if (msg.success !== undefined) {
      resolved.success = msg.success;
    }
    else {
      resolved.success = false
    }

    if (msg.encoded_scheduled_time !== undefined) {
      resolved.encoded_scheduled_time = msg.encoded_scheduled_time;
    }
    else {
      resolved.encoded_scheduled_time = 0
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
  Request: SendActionCommandRequest,
  Response: SendActionCommandResponse,
  md5sum() { return 'c725e105b4f2d4dfe7d74a3ac74cd5d7'; },
  datatype() { return 'orbbec_camera/SendActionCommand'; }
};
