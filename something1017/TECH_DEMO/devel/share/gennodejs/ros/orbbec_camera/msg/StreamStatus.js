// Auto-generated. Do not edit!

// (in-package orbbec_camera.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class StreamStatus {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.topic_name = null;
      this.has_subscribers = null;
      this.publish_rate_hz = null;
      this.delay_ms_avg = null;
    }
    else {
      if (initObj.hasOwnProperty('topic_name')) {
        this.topic_name = initObj.topic_name
      }
      else {
        this.topic_name = '';
      }
      if (initObj.hasOwnProperty('has_subscribers')) {
        this.has_subscribers = initObj.has_subscribers
      }
      else {
        this.has_subscribers = false;
      }
      if (initObj.hasOwnProperty('publish_rate_hz')) {
        this.publish_rate_hz = initObj.publish_rate_hz
      }
      else {
        this.publish_rate_hz = 0.0;
      }
      if (initObj.hasOwnProperty('delay_ms_avg')) {
        this.delay_ms_avg = initObj.delay_ms_avg
      }
      else {
        this.delay_ms_avg = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type StreamStatus
    // Serialize message field [topic_name]
    bufferOffset = _serializer.string(obj.topic_name, buffer, bufferOffset);
    // Serialize message field [has_subscribers]
    bufferOffset = _serializer.bool(obj.has_subscribers, buffer, bufferOffset);
    // Serialize message field [publish_rate_hz]
    bufferOffset = _serializer.float64(obj.publish_rate_hz, buffer, bufferOffset);
    // Serialize message field [delay_ms_avg]
    bufferOffset = _serializer.float64(obj.delay_ms_avg, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type StreamStatus
    let len;
    let data = new StreamStatus(null);
    // Deserialize message field [topic_name]
    data.topic_name = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [has_subscribers]
    data.has_subscribers = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [publish_rate_hz]
    data.publish_rate_hz = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [delay_ms_avg]
    data.delay_ms_avg = _deserializer.float64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.topic_name);
    return length + 21;
  }

  static datatype() {
    // Returns string type for a message object
    return 'orbbec_camera/StreamStatus';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '921f032866beb1e18363e1b44b10b8c1';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
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
    const resolved = new StreamStatus(null);
    if (msg.topic_name !== undefined) {
      resolved.topic_name = msg.topic_name;
    }
    else {
      resolved.topic_name = ''
    }

    if (msg.has_subscribers !== undefined) {
      resolved.has_subscribers = msg.has_subscribers;
    }
    else {
      resolved.has_subscribers = false
    }

    if (msg.publish_rate_hz !== undefined) {
      resolved.publish_rate_hz = msg.publish_rate_hz;
    }
    else {
      resolved.publish_rate_hz = 0.0
    }

    if (msg.delay_ms_avg !== undefined) {
      resolved.delay_ms_avg = msg.delay_ms_avg;
    }
    else {
      resolved.delay_ms_avg = 0.0
    }

    return resolved;
    }
};

module.exports = StreamStatus;
