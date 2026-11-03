// Auto-generated. Do not edit!

// (in-package orbbec_camera.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let DepthFilterParam = require('./DepthFilterParam.js');

//-----------------------------------------------------------

class DepthFilterState {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.filter_name = null;
      this.enabled = null;
      this.params = null;
    }
    else {
      if (initObj.hasOwnProperty('filter_name')) {
        this.filter_name = initObj.filter_name
      }
      else {
        this.filter_name = '';
      }
      if (initObj.hasOwnProperty('enabled')) {
        this.enabled = initObj.enabled
      }
      else {
        this.enabled = false;
      }
      if (initObj.hasOwnProperty('params')) {
        this.params = initObj.params
      }
      else {
        this.params = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DepthFilterState
    // Serialize message field [filter_name]
    bufferOffset = _serializer.string(obj.filter_name, buffer, bufferOffset);
    // Serialize message field [enabled]
    bufferOffset = _serializer.bool(obj.enabled, buffer, bufferOffset);
    // Serialize message field [params]
    // Serialize the length for message field [params]
    bufferOffset = _serializer.uint32(obj.params.length, buffer, bufferOffset);
    obj.params.forEach((val) => {
      bufferOffset = DepthFilterParam.serialize(val, buffer, bufferOffset);
    });
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DepthFilterState
    let len;
    let data = new DepthFilterState(null);
    // Deserialize message field [filter_name]
    data.filter_name = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [enabled]
    data.enabled = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [params]
    // Deserialize array length for message field [params]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.params = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.params[i] = DepthFilterParam.deserialize(buffer, bufferOffset)
    }
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.filter_name);
    object.params.forEach((val) => {
      length += DepthFilterParam.getMessageSize(val);
    });
    return length + 9;
  }

  static datatype() {
    // Returns string type for a message object
    return 'orbbec_camera/DepthFilterState';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'a4c5e50af4025d2a64d42b41c1cb6b04';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    string filter_name
    bool enabled
    DepthFilterParam[] params
    
    ================================================================================
    MSG: orbbec_camera/DepthFilterParam
    string name
    string value
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new DepthFilterState(null);
    if (msg.filter_name !== undefined) {
      resolved.filter_name = msg.filter_name;
    }
    else {
      resolved.filter_name = ''
    }

    if (msg.enabled !== undefined) {
      resolved.enabled = msg.enabled;
    }
    else {
      resolved.enabled = false
    }

    if (msg.params !== undefined) {
      resolved.params = new Array(msg.params.length);
      for (let i = 0; i < resolved.params.length; ++i) {
        resolved.params[i] = DepthFilterParam.Resolve(msg.params[i]);
      }
    }
    else {
      resolved.params = []
    }

    return resolved;
    }
};

module.exports = DepthFilterState;
