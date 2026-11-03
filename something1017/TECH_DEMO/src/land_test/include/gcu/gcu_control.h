/**
 * @file gcu_control.h
 * @brief GCU 云台控制接口
 * 
 * 功能：
 * 1. 云台角度控制
 * 2. OSD 开关控制
 * 
 * 使用示例：
 * @code
 * #include "gcu_control.h"
 * 
 * int main() {
 *     gcu_handle_t *gcu = gcu_connect("192.168.144.108", 2332);
 *     if (gcu) {
 *         gcu_set_angle(gcu, 0, 45, 60);
 *         gcu_set_osd(gcu, 1);
 *         gcu_disconnect(gcu);
 *     }
 *     return 0;
 * }
 * @endcode
 */

#ifndef GCU_CONTROL_H
#define GCU_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GCU_BUFFER_SIZE 1024
#define GCU_DEFAULT_IP "192.168.144.108"
#define GCU_DEFAULT_PORT 2332

/**
 * @brief GCU设备句柄（不透明结构体）
 */
typedef struct gcu_handle gcu_handle_t;

/**
 * @brief 连接到GCU设备
 * @param ip GCU设备IP地址（如 "192.168.144.108"）
 * @param port 端口号（通常为2332）
 * @return 成功返回设备句柄，失败返回NULL
 * 
 * @example
 * gcu_handle_t *handle = gcu_connect("192.168.144.108", 2332);
 * if (handle) {
 *     // 使用设备
 *     gcu_disconnect(handle);
 * }
 */
gcu_handle_t* gcu_connect(const char* ip, int port);

/**
 * @brief 断开与GCU设备的连接
 * @param handle 设备句柄
 */
void gcu_disconnect(gcu_handle_t *handle);

/**
 * @brief 控制云台角度
 * @param handle 设备句柄
 * @param roll 滚转角度（度）范围：-180 ~ 180
 * @param pitch 俯仰角度（度）范围：-90 ~ 90
 * @param yaw 指向角度（度）范围：-180 ~ 180
 * @return 成功返回0，失败返回-1
 * 
 * @example
 * gcu_set_angle(handle, 0, 45, 60);   // 设置角度
 * gcu_set_angle(handle, 0, -90, 0);   // 向下90度
 */
int gcu_set_angle(gcu_handle_t *handle, int roll, int pitch, int yaw);

/**
 * @brief 设置OSD（屏幕字符显示）开关状态
 * @param handle 设备句柄
 * @param enable 1-开启，0-关闭
 * @return 成功返回0，失败返回-1
 * 
 * @example
 * gcu_set_osd(handle, 1);    // 开启OSD
 * gcu_set_osd(handle, 0);    // 关闭OSD
 */
int gcu_set_osd(gcu_handle_t *handle, int enable);

/**
 * @brief 获取最后错误信息
 * @param handle 设备句柄
 * @return 错误信息字符串
 */
const char* gcu_get_last_error(gcu_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* GCU_CONTROL_H */