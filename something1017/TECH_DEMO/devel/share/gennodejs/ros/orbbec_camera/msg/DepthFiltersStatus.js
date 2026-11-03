// Auto-generated. Do not edit!

// (in-package orbbec_camera.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let DepthFilterState = require('./DepthFilterState.js');
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class DepthFiltersStatus {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.filters = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('filters')) {
        this.filters = initObj.filters
      }
      else {
        this.filters = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DepthFiltersStatus
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [filters]
    // Serialize the length for message field [filters]
    bufferOffset = _serializer.uint32(obj.filters.length, buffer, bufferOffset);
    obj.filters.forEach((val) => {
      bufferOffset = DepthFilterState.serialize(val, buffer, bufferOffset);
    });
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DepthFiltersStatus
    let len;
    let data = new DepthFiltersStatus(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [filters]
    // Deserialize array length for message field [filters]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.filters = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.filters[i] = DepthFilterState.deserialize(buffer, bufferOffset)
    }
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    object.filters.forEach((val) => {
      length += DepthFilterState.getMessageSize(val);
    });
    return length + 4;
  }

  static datatype() {
    // Returns string type for a message object
    return 'orbbec_camera/DepthFiltersStatus';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '7e84f3ab9e3323f2de9c2ef408d5df13';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    std_msgs/Header header
    DepthFilterState[] filters
    
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
    MSG: orbbec_camera/DepthFilterState
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
    const resolved = new DepthFiltersStatus(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.filters !== undefined) {
      resolved.filters = new Array(msg.filters.length);
      for (let i = 0; i < resolved.filters.length; ++i) {
        resolved.filters[i] = DepthFilterState.Resolve(msg.filters[i]);
      }
    }
    else {
      resolved.filters = []
    }

    return resolved;
    }
};

module.exports = DepthFiltersStatus;
