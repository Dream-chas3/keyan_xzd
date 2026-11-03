// Auto-generated. Do not edit!

// (in-package orbbec_camera.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let StreamStatus = require('./StreamStatus.js');
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class DeviceStatus {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.device_online = null;
      this.connection_type = null;
      this.customer_calibration_ready = null;
      this.calibration_from_factory = null;
      this.calibration_from_launch_param = null;
      this.streams = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('device_online')) {
        this.device_online = initObj.device_online
      }
      else {
        this.device_online = false;
      }
      if (initObj.hasOwnProperty('connection_type')) {
        this.connection_type = initObj.connection_type
      }
      else {
        this.connection_type = '';
      }
      if (initObj.hasOwnProperty('customer_calibration_ready')) {
        this.customer_calibration_ready = initObj.customer_calibration_ready
      }
      else {
        this.customer_calibration_ready = false;
      }
      if (initObj.hasOwnProperty('calibration_from_factory')) {
        this.calibration_from_factory = initObj.calibration_from_factory
      }
      else {
        this.calibration_from_factory = false;
      }
      if (initObj.hasOwnProperty('calibration_from_launch_param')) {
        this.calibration_from_launch_param = initObj.calibration_from_launch_param
      }
      else {
        this.calibration_from_launch_param = false;
      }
      if (initObj.hasOwnProperty('streams')) {
        this.streams = initObj.streams
      }
      else {
        this.streams = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DeviceStatus
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [device_online]
    bufferOffset = _serializer.bool(obj.device_online, buffer, bufferOffset);
    // Serialize message field [connection_type]
    bufferOffset = _serializer.string(obj.connection_type, buffer, bufferOffset);
    // Serialize message field [customer_calibration_ready]
    bufferOffset = _serializer.bool(obj.customer_calibration_ready, buffer, bufferOffset);
    // Serialize message field [calibration_from_factory]
    bufferOffset = _serializer.bool(obj.calibration_from_factory, buffer, bufferOffset);
    // Serialize message field [calibration_from_launch_param]
    bufferOffset = _serializer.bool(obj.calibration_from_launch_param, buffer, bufferOffset);
    // Serialize message field [streams]
    // Serialize the length for message field [streams]
    bufferOffset = _serializer.uint32(obj.streams.length, buffer, bufferOffset);
    obj.streams.forEach((val) => {
      bufferOffset = StreamStatus.serialize(val, buffer, bufferOffset);
    });
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DeviceStatus
    let len;
    let data = new DeviceStatus(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [device_online]
    data.device_online = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [connection_type]
    data.connection_type = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [customer_calibration_ready]
    data.customer_calibration_ready = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [calibration_from_factory]
    data.calibration_from_factory = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [calibration_from_launch_param]
    data.calibration_from_launch_param = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [streams]
    // Deserialize array length for message field [streams]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.streams = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.streams[i] = StreamStatus.deserialize(buffer, bufferOffset)
    }
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    length += _getByteLength(object.connection_type);
    object.streams.forEach((val) => {
      length += StreamStatus.getMessageSize(val);
    });
    return length + 12;
  }

  static datatype() {
    // Returns string type for a message object
    return 'orbbec_camera/DeviceStatus';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'c5afe24864ffc8299459142b55ca2268';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    std_msgs/Header header
    bool device_online
    string connection_type
    bool customer_calibration_ready
    bool calibration_from_factory
    bool calibration_from_launch_param
    orbbec_camera/StreamStatus[] streams
    
    ================================================================================
    MSG: std_msgs/Header
    # Standard metadata for higher-level stamped data types.
    # This is generally used to communicate timestamped data 
    # in a particular coordinate frame.
    # 
    # sequence ID: consecutively increasing ID 
    uint32 seq
    #Two-integer timestamp that is expressed as:
    # * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
    # * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
    # time-handling sugar is provided by the client library
    time stamp
    #Frame this data is associated with
    string frame_id
    
    ================================================================================
    MSG: orbbec_camera/StreamStatus
    string topic_name
    bool has_subscribers
    float64 publish_rate_hz
    float64 delay_ms_avg
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new DeviceStatus(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.device_online !== undefined) {
      resolved.device_online = msg.device_online;
    }
    else {
      resolved.device_online = false
    }

    if (msg.connection_type !== undefined) {
      resolved.connection_type = msg.connection_type;
    }
    else {
      resolved.connection_type = ''
    }

    if (msg.customer_calibration_ready !== undefined) {
      resolved.customer_calibration_ready = msg.customer_calibration_ready;
    }
    else {
      resolved.customer_calibration_ready = false
    }

    if (msg.calibration_from_factory !== undefined) {
      resolved.calibration_from_factory = msg.calibration_from_factory;
    }
    else {
      resolved.calibration_from_factory = false
    }

    if (msg.calibration_from_launch_param !== undefined) {
      resolved.calibration_from_launch_param = msg.calibration_from_launch_param;
    }
    else {
      resolved.calibration_from_launch_param = false
    }

    if (msg.streams !== undefined) {
      resolved.streams = new Array(msg.streams.length);
      for (let i = 0; i < resolved.streams.length; ++i) {
        resolved.streams[i] = StreamStatus.Resolve(msg.streams[i]);
      }
    }
    else {
      resolved.streams = []
    }

    return resolved;
    }
};

module.exports = DeviceStatus;
