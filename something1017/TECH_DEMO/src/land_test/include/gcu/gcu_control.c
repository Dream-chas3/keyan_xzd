/**
 * @file gcu_control.c
 * @brief GCU 云台控制接口实现
 */

#include "gcu_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>           
#include <stdarg.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <poll.h>

// ============ 设备句柄结构体 ============
struct gcu_handle {
    int socket_fd;
    char ip[16];
    int port;
    char error_msg[256];
};

// ============ 预定义OSD命令数组 ============
// OSD开启命令 (73字节)
static unsigned char osd_on[73] = {
    0xA8, 0xE5, 0x49, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x01, 0xB8, 0x60
};

// OSD关闭命令 (73字节)
static unsigned char osd_off[73] = {
    0xA8, 0xE5, 0x49, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x00, 0xA8, 0x41
};

// ============ CRC计算函数 ============
/**
 * @brief 计算CRC16校验码
 * @param ptr 数据指针
 * @param len 数据长度
 * @return CRC16校验值
 */
static uint16_t calculate_crc16(uint8_t *ptr, uint8_t len) {
    uint16_t crc;
    uint8_t da;
    uint16_t crc_ta[16] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    };
    crc = 0;
    while (len-- != 0) {
        da = crc >> 12;
        crc <<= 4;
        crc ^= crc_ta[da ^ (*ptr >> 4)];
        da = crc >> 12;
        crc <<= 4;
        crc ^= crc_ta[da ^ (*ptr & 0x0F)];
        ptr++;
    }
    return crc;
}

// ============ 命令构造函数 ============

/**
 * @brief 构建云台角度控制命令
 * @param buf 输出缓冲区（至少72字节）
 * @param roll 滚转角度（度，范围：-180 ~ 180）
 * @param pitch 俯仰角度（度，范围：-90 ~ 90）
 * @param yaw 指向角度（度，范围：-180 ~ 180）
 */
static void build_angle_cmd(unsigned char *buf, int roll, int pitch, int yaw) {
    memset(buf, 0, 72);
    buf[0] = 0xA8;      // 协议头
    buf[1] = 0xE5;      // 协议头
    buf[2] = 0x48;      // 消息类型（角度控制）
    buf[3] = 0x00;
    buf[4] = 0x02;

    // 角度值放大100倍，采用小端格式写入
    int roll_scaled = roll * 100;
    int pitch_scaled = pitch * 100;
    int yaw_scaled = yaw * 100;
    
    buf[5] = roll_scaled & 0xFF;
    buf[6] = (roll_scaled >> 8) & 0xFF;
    buf[7] = pitch_scaled & 0xFF;
    buf[8] = (pitch_scaled >> 8) & 0xFF;
    buf[9] = yaw_scaled & 0xFF;
    buf[10] = (yaw_scaled >> 8) & 0xFF;

    buf[11] = 0x04;     // 控制模式标识
    buf[69] = 0x10;     // 角度控制指令

    // 计算并写入CRC校验
    uint16_t crc = calculate_crc16(buf, 70);
    buf[70] = (crc >> 8) & 0xFF;
    buf[71] = crc & 0xFF;
}

// ============ 内部辅助函数 ============

/**
 * @brief 设置错误信息
 */
static void set_error(gcu_handle_t *handle, const char *fmt, ...) {
    if (handle) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(handle->error_msg, sizeof(handle->error_msg), fmt, args);
        va_end(args);
    }
}

/**
 * @brief 发送命令到GCU并接收响应
 * @param handle 设备句柄
 * @param cmd 命令数据
 * @param cmd_len 命令长度
 * @param response 响应缓冲区（可为NULL）
 * @param response_len 响应缓冲区大小
 * @return 成功返回接收到的响应字节数，失败返回-1
 */
static int send_command(gcu_handle_t *handle, unsigned char *cmd, int cmd_len, 
                        unsigned char *response, int response_len) {
    if (!handle || handle->socket_fd < 0 || !cmd || cmd_len <= 0) {
        if (handle) set_error(handle, "无效参数");
        return -1;
    }
    
    // 发送命令
    int sent = send(handle->socket_fd, cmd, cmd_len, MSG_NOSIGNAL);
    if (sent != cmd_len) {
        set_error(handle, "发送命令失败: sent=%d, expected=%d", sent, cmd_len);
        return -1;
    }
    
    // 如果不要求接收响应，直接返回成功
    if (!response || response_len <= 0) return 0;
    
    // 接收响应（带超时）
    fd_set fds;
    struct timeval tv;
    
    FD_ZERO(&fds);
    FD_SET(handle->socket_fd, &fds);
    tv.tv_sec = 2;  // 2秒超时
    tv.tv_usec = 0;
    
    if (select(handle->socket_fd + 1, &fds, NULL, NULL, &tv) > 0) {
        int recv_len = recv(handle->socket_fd, response, response_len, 0);
        if (recv_len < 0) {
            set_error(handle, "接收响应失败: %s", strerror(errno));
            return -1;
        }
        return recv_len;
    }
    
    set_error(handle, "接收响应超时");
    return -1;  // 超时
}

// ============ 公共API实现 ============

gcu_handle_t* gcu_connect(const char* ip, int port) {
    if (!ip || port <= 0) {
        fprintf(stderr, "gcu_connect: 无效参数\n");
        return NULL;
    }
    
    gcu_handle_t *handle = (gcu_handle_t*)malloc(sizeof(gcu_handle_t));
    if (!handle) {
        perror("malloc failed");
        return NULL;
    }
    
    memset(handle, 0, sizeof(gcu_handle_t));
    strncpy(handle->ip, ip, sizeof(handle->ip) - 1);
    handle->ip[sizeof(handle->ip) - 1] = '\0';
    handle->port = port;
    handle->socket_fd = -1;
    
    // 创建socket
    handle->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (handle->socket_fd < 0) {
        set_error(handle, "创建socket失败: %s", strerror(errno));
        free(handle);
        return NULL;
    }
    
    // 配置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    
    // Bound connect and I/O latency. This is called only after explicit enable,
    // on the service worker, never from the 30 Hz control timer.
    int flags = fcntl(handle->socket_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(handle->socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(handle->socket_fd); free(handle); return NULL;
    }
    int connected = connect(handle->socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (connected < 0 && errno == EINPROGRESS) {
        struct pollfd pfd = {handle->socket_fd, POLLOUT, 0};
        int error = 0; socklen_t length = sizeof(error);
        connected = poll(&pfd, 1, 500);
        if (connected > 0 && getsockopt(handle->socket_fd, SOL_SOCKET, SO_ERROR, &error, &length) == 0 && error == 0)
            connected = 0;
        else connected = -1;
    }
    if (connected < 0 || fcntl(handle->socket_fd, F_SETFL, flags) < 0) {
        set_error(handle, "连接失败: %s:%d - %s", ip, port, strerror(errno));
        close(handle->socket_fd);
        free(handle);
        return NULL;
    }
    struct timeval timeout = {0, 500000};
    setsockopt(handle->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(handle->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    return handle;
}

void gcu_disconnect(gcu_handle_t *handle) {
    if (handle) {
        if (handle->socket_fd >= 0) {
            close(handle->socket_fd);
        }
        free(handle);
    }
}

int gcu_set_angle(gcu_handle_t *handle, int roll, int pitch, int yaw) {
    if (!handle) return -1;
    
    // 参数范围检查
    if (roll < -60 || roll > 60) {
        set_error(handle, "滚转角度超出范围: %d (应为 -60~60)", roll);
        return -1;
    }
    if (pitch < -90 || pitch > 90) {
        set_error(handle, "俯仰角度超出范围: %d (应为 -90~90)", pitch);
        return -1;
    }
    if (yaw < -170 || yaw > 170) {
        set_error(handle, "指向角度超出范围: %d (应为 -170~170)", yaw);
        return -1;
    }
    
    unsigned char cmd[72];
    build_angle_cmd(cmd, roll, pitch, yaw);
    
    unsigned char response[GCU_BUFFER_SIZE];
    int recv_len = send_command(handle, cmd, 72, response, GCU_BUFFER_SIZE);
    
    if (recv_len <= 0) return -1;
    
    // 可选：验证响应CRC
    if (recv_len >= 2) {
        uint16_t crc_recv = (response[recv_len-2] << 8) | response[recv_len-1];
        uint16_t crc_calc = calculate_crc16(response, recv_len - 2);
        if (crc_recv != crc_calc) {
            set_error(handle, "响应CRC校验失败");
            return -1;
        }
    }
    
    return 0;
}

int gcu_set_osd(gcu_handle_t *handle, int enable) {
    if (!handle) return -1;
    
    unsigned char *cmd = enable ? osd_on : osd_off;
    int cmd_len = sizeof(osd_on);
    
    unsigned char response[GCU_BUFFER_SIZE];
    int recv_len = send_command(handle, cmd, cmd_len, response, GCU_BUFFER_SIZE);
    
    return (recv_len >= 0) ? 0 : -1;
}

const char* gcu_get_last_error(gcu_handle_t *handle) {
    if (!handle) return "无效的句柄";
    return handle->error_msg;
}
