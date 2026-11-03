#!/usr/bin/env python3
import socket
import sys
import time
import struct

def build_angle_cmd(roll, pitch, yaw):
    """构造云台角度控制命令，返回 72 字节的命令包"""
    buf = bytearray(72)
    buf[0] = 0xA8
    buf[1] = 0xE5
    buf[2] = 0x48
    buf[3] = 0x00
    buf[4] = 0x02

    # 角度放大100倍，小端写入
    roll_scaled = roll * 100
    pitch_scaled = pitch * 100
    yaw_scaled = yaw * 100

    buf[5] = roll_scaled & 0xFF
    buf[6] = (roll_scaled >> 8) & 0xFF
    buf[7] = pitch_scaled & 0xFF
    buf[8] = (pitch_scaled >> 8) & 0xFF
    buf[9] = yaw_scaled & 0xFF
    buf[10] = (yaw_scaled >> 8) & 0xFF

    buf[11] = 0x04
    buf[69] = 0x10

    # CRC16 计算（与 gcu_control.c 中相同）
    crc = calculate_crc16(buf[:70])
    buf[70] = (crc >> 8) & 0xFF
    buf[71] = crc & 0xFF

    return bytes(buf)

def calculate_crc16(data):
    crc = 0
    crc_ta = [
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    ]
    for byte in data:
        da = (crc >> 12) & 0xF
        crc = ((crc << 4) & 0xFFFF) ^ crc_ta[da ^ (byte >> 4)]
        da = (crc >> 12) & 0xF
        crc = ((crc << 4) & 0xFFFF) ^ crc_ta[da ^ (byte & 0x0F)]
    return crc

def main():
    # 过滤掉 roslaunch 注入的参数（包含 ':=' 的项）
    args = [arg for arg in sys.argv[1:] if ':=' not in arg]

    ip = args[0] if len(args) > 0 else "192.168.144.108"
    port = int(args[1]) if len(args) > 1 else 2332
    pitch = int(args[2]) if len(args) > 2 else -90

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3)
        sock.connect((ip, port))
        cmd = build_angle_cmd(0, pitch, 0)   # roll=0, pitch=-90, yaw=0
        sock.sendall(cmd)
        # 等待响应（可选，非必要）
        time.sleep(0.1)
        print(f"Gimbal pitch set to {pitch} degrees")
    except Exception as e:
        print(f"Failed to set gimbal: {e}", file=sys.stderr)
    finally:
        sock.close()





if __name__ == "__main__":
    main()