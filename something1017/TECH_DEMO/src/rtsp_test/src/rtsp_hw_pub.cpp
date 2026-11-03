/**
 * RK3588 MPP 硬件解码器 + RTSP 拉流 + OpenCV 显示
 * 
 * 流程说明：
 * 1. RTSP 网络拉流 (av_read_frame)
 * 2. H264/H265 解析 (av_parser_parse2) - 组装完整帧
 * 3. MPP 硬件解码 (mpp_decoder)
 * 4. YUV->BGR 颜色转换 (OpenCV cvtColor)
 * 5. OSD 绘制 (cv::putText)
 * 6. OpenCV 显示 (cv::imshow)
 * 
 * 耗时测量点：
 * - RTSP Read: 网络读取耗时
 * - Decode: MPP 硬解耗时  
 * - Convert: YUV->BGR 颜色转换耗时
 * - Display: OpenCV 绘制+显示耗时
 */


#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

#include "rk_mpi.h"
#include "mpp_buffer.h"
#include "mpp_frame.h"
#include "mpp_packet.h"

#define mpp_log printf
#define mpp_err printf

// ============================================================
// 解码器上下文结构
// ============================================================
typedef struct DecoderContext {
    MppCtx ctx;                 // MPP 上下文
    MppApi *mpi;                // MPP API 接口
    MppPacket packet;           // MPP 数据包
    MppFrame frame;             // MPP 解码帧
    MppBufferGroup buf_mgr;     // MPP 缓冲区管理器
    RK_U32 width;               // 视频宽度
    RK_U32 height;              // 视频高度
    RK_U32 hor_stride;          // 水平步长
    RK_U32 ver_stride;          // 垂直步长
    MppCodingType coding_type;  // 编码类型 (H264/H265)
    MppFrameFormat output_format; // 输出格式 (NV12)
    int frame_count;            // 解码帧计数
    int info_changed;           // 分辨率变化标志
    int frame_width;            // 当前帧宽度
    int frame_height;           // 当前帧高度
    int packet_count;           // 数据包计数
    pthread_mutex_t mutex;      // 互斥锁
    AVCodecParserContext *parser;   // H264/H265 解析器
    AVCodecContext *parser_ctx;     // 解析器上下文
    uint8_t *extradata;         // SPS/PPS 扩展数据
    int extradata_size;         // 扩展数据大小
    int extradata_sent;         // 是否已发送扩展数据
    int decoder_initialized;    // 解码器是否已初始化
    long long last_pts;         // 上一帧 PTS
    long long first_pts;        // 第一帧 PTS
    int64_t avg_decode_time;    // 平均解码时间 (微秒)
    int decode_count;           // 解码次数
} DecoderContext;

// ============================================================
// OSD 信息结构 - 用于画面叠加显示
// ============================================================
typedef struct OSDInfo {
    float fps;                  // 当前帧率
    int frame_count;            // 总帧数
    int width;                  // 视频宽度
    int height;                 // 视频高度
    float decode_time_ms;       // 解码耗时 (毫秒)
    float convert_time_ms;      // 颜色转换耗时 (毫秒)
    float rtsp_read_time_ms;    // RTSP 读取耗时 (毫秒)
    float display_time_ms;      // 显示耗时 (毫秒)
    float total_time_ms;        // 总耗时 (毫秒)
    char rtsp_url[256];         // RTSP 地址
} OSDInfo;

static OSDInfo g_osd = {0};

// ============================================================
// 高精度时间获取函数
// ============================================================

/** 
 * 获取毫秒时间戳
 * @return 毫秒数
 */
static long long get_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/** 
 * 获取微秒时间戳 - 用于精确测量耗时
 * @return 微秒数
 */
static long long get_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

// ============================================================
// MPP 缓冲区管理
// ============================================================

/** 初始化 MPP 缓冲区管理器 */
static MPP_RET dec_buf_mgr_init(MppBufferGroup *buf_mgr) {
    MppBufferGroup group = NULL;
    MPP_RET ret = mpp_buffer_group_get_internal(&group, MPP_BUFFER_TYPE_ION);
    if (ret != MPP_OK) {
        mpp_err("mpp_buffer_group_get_internal failed\n");
        return ret;
    }
    *buf_mgr = group;
    return MPP_OK;
}

/** 释放 MPP 缓冲区管理器 */
static void dec_buf_mgr_deinit(MppBufferGroup buf_mgr) {
    if (buf_mgr) {
        mpp_buffer_group_put(buf_mgr);
    }
}

// ============================================================
// FPS 统计和性能报告
// ============================================================

/**
 * 更新 FPS 并打印性能报告
 * 每秒执行一次，计算过去一秒内的平均帧率
 */
static void update_fps(DecoderContext *dec) {
    static long long last_time = 0;     // 上次打印时间
    static int frame_count = 0;         // 一秒内帧数计数
    
    frame_count++;
    long long now = get_ms();
    
    // 每隔 1000ms (1秒) 打印一次
    if (now - last_time >= 1000) {
        // 计算帧率: 帧数 / 时间差(秒)
        float fps = frame_count * 1000.0f / (now - last_time);
        
        // 更新 OSD 数据
        g_osd.fps = fps;
        g_osd.frame_count = dec->frame_count;
        g_osd.width = dec->width;
        g_osd.height = dec->height;
        
        // 打印性能报告到控制台
        printf("\n========== PERF REPORT ==========\n");
        printf("FPS: %.1f | Frames: %d | Resolution: %dx%d\n", 
               fps, dec->frame_count, dec->width, dec->height);
        printf("----------------------------------\n");
        printf("RTSP Read:    %7.3f ms (网络拉流耗时)\n", g_osd.rtsp_read_time_ms);
        printf("Decode:       %7.3f ms (MPP硬解码耗时)\n", g_osd.decode_time_ms);
        printf("Convert:      %7.3f ms (YUV->BGR颜色转换)\n", g_osd.convert_time_ms);
        printf("Display:      %7.3f ms (OpenCV显示耗时)\n", g_osd.display_time_ms);
        printf("----------------------------------\n");
        printf("TOTAL:        %7.3f ms (每帧总耗时)\n", g_osd.total_time_ms);
        printf("Target FPS:   %7.1f fps (理论最大帧率)\n", 1000.0 / g_osd.total_time_ms);
        printf("==================================\n");
        
        // 重置计数器
        frame_count = 0;
        last_time = now;
    }
}

// ============================================================
// 解码器初始化
// ============================================================

/** 发送 SPS/PPS 扩展数据到解码器 */
static int send_extradata_to_decoder(DecoderContext *dec) {
    if (!dec->extradata || dec->extradata_sent) {
        return 0;
    }
    
    MPP_RET ret;
    ret = mpp_packet_init(&dec->packet, dec->extradata, dec->extradata_size);
    if (ret != MPP_OK) {
        mpp_err("Failed to init extradata packet\n");
        return -1;
    }
    
    ret = dec->mpi->decode_put_packet(dec->ctx, dec->packet);
    if (ret != MPP_OK) {
        mpp_err("Failed to send extradata to decoder: %d\n", ret);
        return -1;
    }
    
    dec->extradata_sent = 1;
    return 0;
}

/** 初始化解码器 */
static int init_decoder(DecoderContext *dec, MppCodingType type, RK_U32 width, RK_U32 height,
                        uint8_t *extradata, int extradata_size) {
    MPP_RET ret;
    
    dec->coding_type = type;
    dec->width = width;
    dec->height = height;
    dec->hor_stride = 0;
    dec->ver_stride = 0;
    dec->frame_count = 0;
    dec->info_changed = 0;
    dec->frame_width = 0;
    dec->frame_height = 0;
    dec->packet_count = 0;
    dec->output_format = MPP_FMT_YUV420SP;  // NV12 格式
    dec->extradata_sent = 0;
    dec->decoder_initialized = 0;
    dec->last_pts = 0;
    dec->first_pts = 0;
    dec->avg_decode_time = 0;
    dec->decode_count = 0;
    
    // 保存 SPS/PPS 扩展数据
    if (extradata && extradata_size > 0) {
        dec->extradata = (uint8_t*)malloc(extradata_size);
        memcpy(dec->extradata, extradata, extradata_size);
        dec->extradata_size = extradata_size;
    } else {
        dec->extradata = NULL;
        dec->extradata_size = 0;
    }
    
    // 创建 MPP 上下文
    ret = mpp_create(&dec->ctx, &dec->mpi);
    if (ret != MPP_OK) {
        mpp_err("mpp_create failed ret %d\n", ret);
        return -1;
    }
    
    // 初始化解码器
    ret = mpp_init(dec->ctx, MPP_CTX_DEC, dec->coding_type);
    if (ret != MPP_OK) {
        mpp_err("mpp_init failed ret %d\n", ret);
        return -1;
    }
    
    // 设置输出格式为 NV12
    ret = dec->mpi->control(dec->ctx, MPP_DEC_SET_OUTPUT_FORMAT, &dec->output_format);
    
    // 初始化缓冲区管理器
    ret = dec_buf_mgr_init(&dec->buf_mgr);
    if (ret != MPP_OK) {
        return -1;
    }
    
    // 设置外部缓冲区组
    if (dec->buf_mgr) {
        dec->mpi->control(dec->ctx, MPP_DEC_SET_EXT_BUF_GROUP, dec->buf_mgr);
    }
    
    // 创建 MPP 数据包
    ret = mpp_packet_init(&dec->packet, NULL, 0);
    if (ret != MPP_OK) {
        return -1;
    }
    
    // 初始化 H264/H265 解析器 (用于组装完整帧)
    dec->parser = av_parser_init(AV_CODEC_ID_H264);
    dec->parser_ctx = avcodec_alloc_context3(NULL);
    if (dec->parser_ctx && extradata && extradata_size > 0) {
        dec->parser_ctx->extradata = (uint8_t*)av_mallocz(extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        memcpy(dec->parser_ctx->extradata, extradata, extradata_size);
        dec->parser_ctx->extradata_size = extradata_size;
    }
    
    pthread_mutex_init(&dec->mutex, NULL);
    dec->decoder_initialized = 1;
    
    return 0;
}

// ============================================================
// 解码核心函数
// ============================================================

/**
 * 处理分辨率变化
 * 当视频分辨率改变时调用，重新配置解码器
 */
static int handle_info_change(DecoderContext *dec, MppFrame frame) {
    MPP_RET ret;
    RK_U32 width = mpp_frame_get_width(frame);
    RK_U32 height = mpp_frame_get_height(frame);
    RK_U32 hor_stride = mpp_frame_get_hor_stride(frame);
    RK_U32 ver_stride = mpp_frame_get_ver_stride(frame);
    
    // 更新分辨率
    dec->width = width;
    dec->height = height;
    dec->hor_stride = hor_stride;
    dec->ver_stride = ver_stride;
    
    // 重新配置解码器
    ret = dec->mpi->control(dec->ctx, MPP_DEC_SET_OUTPUT_FORMAT, &dec->output_format);
    
    // 重新配置缓冲区
    if (dec->buf_mgr) {
        size_t buf_size = hor_stride * ver_stride * 3 / 2;
        mpp_buffer_group_limit_config(dec->buf_mgr, buf_size * 24, 24);
        dec->mpi->control(dec->ctx, MPP_DEC_SET_EXT_BUF_GROUP, dec->buf_mgr);
    }
    
    // 通知解码器分辨率变化已处理
    ret = dec->mpi->control(dec->ctx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
    dec->info_changed = 0;
    
    return 0;
}

/**
 * 解码一帧数据
 * 
 * 流程：
 * 1. 使用 H264 解析器将 RTP 包组装成完整帧
 * 2. 将完整帧发送给 MPP 解码器
 * 3. 从 MPP 解码器获取解码后的帧
 * 
 * @param dec 解码器上下文
 * @param data 输入数据 (RTP 包)
 * @param len 数据长度
 * @param frame_data 输出: 解码后帧数据指针
 * @param frame_width 输出: 帧宽度
 * @param frame_height 输出: 帧高度
 * @return 1=成功解码一帧, 0=无帧输出, -1=错误
 */
static int decode_frame(DecoderContext *dec, uint8_t *data, size_t len, 
                        void **frame_data, int *frame_width, int *frame_height) {
    MPP_RET ret;
    uint8_t *parse_data = data;
    int parse_len = len;
    int decoded = 0;
    
    if (!dec->decoder_initialized) return 0;
    
    // ============================================================
    // 步骤1: H264/H265 解析 - 将 RTP 包组装成完整帧
    // ============================================================
    if (dec->parser) {
        while (parse_len > 0) {
            uint8_t *out_data = NULL;
            int out_size = 0;
            
            // 解析器会缓存不完整的包，当有完整帧时输出
            int consumed = av_parser_parse2(dec->parser, dec->parser_ctx, 
                                            &out_data, &out_size,
                                            parse_data, parse_len,
                                            AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (consumed < 0) break;
            
            parse_data += consumed;
            parse_len -= consumed;
            
            // 获得一个完整的帧
            if (out_size > 0) {
                if (dec->packet) mpp_packet_deinit(&dec->packet);
                ret = mpp_packet_init(&dec->packet, out_data, out_size);
                if (ret == MPP_OK) {
                    // 发送完整帧到 MPP 解码器
                    dec->mpi->decode_put_packet(dec->ctx, dec->packet);
                    decoded = 1;
                }
            }
        }
    }
    
    // ============================================================
    // 步骤2: MPP 硬件解码
    // ============================================================
    if (decoded) {
        long long start_time = get_us();  // 开始计时
        
        // 释放上一帧
        if (dec->frame) {
            mpp_frame_deinit(&dec->frame);
            dec->frame = NULL;
        }
        
        // 从解码器获取解码后的帧
        ret = dec->mpi->decode_get_frame(dec->ctx, &dec->frame);
        if (ret != MPP_OK) return 0;
        
        if (dec->frame) {
            // 检查分辨率是否变化
            if (mpp_frame_get_info_change(dec->frame)) {
                handle_info_change(dec, dec->frame);
                return 0;
            }
            
            // 获取解码后的帧数据 (NV12 格式)
            MppBuffer buffer = mpp_frame_get_buffer(dec->frame);
            if (buffer) {
                *frame_data = mpp_buffer_get_ptr(buffer);
                *frame_width = mpp_frame_get_width(dec->frame);
                *frame_height = mpp_frame_get_height(dec->frame);
                
                // 计算解码耗时
                long long end_time = get_us();
                long long decode_us = end_time - start_time;
                
                // 更新平均解码时间
                dec->avg_decode_time = (dec->avg_decode_time * dec->decode_count + decode_us) / (dec->decode_count + 1);
                dec->decode_count++;
                g_osd.decode_time_ms = dec->avg_decode_time / 1000.0f;
                
                dec->frame_count++;
                update_fps(dec);  // 更新 FPS 统计
                return 1;  // 成功解码一帧
            }
        }
    }
    
    return 0;  // 没有解码出完整帧
}

// ============================================================
// OSD 绘制函数
// ============================================================

/**
 * 在画面上绘制 OSD 信息
 * 在图像顶部绘制半透明背景和文字
 */
static void draw_osd(cv::Mat &frame, OSDInfo *osd) {
    if (frame.empty()) return;
    
    int frame_h = frame.rows;
    int frame_w = frame.cols;
    
    // 创建透明背景层
    cv::Mat overlay;
    frame.copyTo(overlay);
    
    // 绘制顶部黑色半透明条
    cv::rectangle(overlay, cv::Point(0, 0), cv::Point(frame_w, 70), cv::Scalar(0, 0, 0), -1);
    cv::addWeighted(overlay, 0.5, frame, 0.5, 0, frame);
    
    char text[256];
    int font_face = cv::FONT_HERSHEY_SIMPLEX;
    double font_scale = 0.55;
    cv::Scalar color(0, 255, 0);  // 绿色文字
    int thickness = 1;
    
    // 第一行: FPS, 分辨率, 帧数
    snprintf(text, sizeof(text), "FPS:%.1f  %dx%d  Frames:%d", 
             osd->fps, osd->width, osd->height, osd->frame_count);
    cv::putText(frame, text, cv::Point(10, 20), font_face, font_scale, color, thickness);
    
    // 第二行: 各环节耗时
    snprintf(text, sizeof(text), "RTSP:%.2fms  Decode:%.2fms  Convert:%.2fms  Display:%.2fms", 
             osd->rtsp_read_time_ms, osd->decode_time_ms, 
             osd->convert_time_ms, osd->display_time_ms);
    cv::putText(frame, text, cv::Point(10, 40), font_face, font_scale, color, thickness);
    
    // 第三行: 总耗时和理论帧率
    snprintf(text, sizeof(text), "Total:%.2fms  Target:%.1ffps", 
             osd->total_time_ms, 1000.0 / osd->total_time_ms);
    cv::putText(frame, text, cv::Point(10, 60), font_face, font_scale, color, thickness);
}

// ============================================================
// 颜色转换和显示函数
// ============================================================

/**
 * 将 NV12 格式转换为 BGR 并显示
 * 
 * 流程：
 * 1. 从 MPP 帧中获取 NV12 数据
 * 2. 使用 OpenCV cvtColor 转换为 BGR 格式
 * 3. 裁剪到实际分辨率
 * 
 * @param frame MPP 解码帧
 * @param dec 解码器上下文
 * @param display_mat 输出: BGR 格式的 OpenCV Mat
 * @param convert_time 输出: 颜色转换耗时 (微秒)
 * @param display_time 输出: 显示耗时 (微秒)
 * @return 0=成功, -1=失败
 */
static int convert_and_display(MppFrame frame, DecoderContext *dec, cv::Mat &display_mat,
                                long long &convert_time, long long &display_time) {
    if (!frame) return -1;
    
    MppBuffer buffer = mpp_frame_get_buffer(frame);
    if (!buffer) return -1;
    
    void *data = mpp_buffer_get_ptr(buffer);
    if (!data) return -1;
    
    RK_U32 width = dec->width;
    RK_U32 height = dec->height;
    RK_U32 hor_stride = dec->hor_stride ? dec->hor_stride : width;
    RK_U32 ver_stride = dec->ver_stride ? dec->ver_stride : height;
    
    // ============================================================
    // 步骤3: YUV (NV12) -> BGR 颜色转换 (耗时测量点)
    // ============================================================
    long long convert_start = get_us();
    
    // 定义一个静态的 cv::Mat 对象，用于缓存转换后的 BGR 图像
    // static 关键字让这个变量在函数多次调用之间保持存在，避免每帧都重新分配内存
    static cv::Mat bgr_cache;
    // 检查缓存是否为空，或者尺寸是否与当前帧不匹配
    // 如果是首次运行，或者视频分辨率发生了变化，就需要重新分配内存
    if (bgr_cache.empty() || bgr_cache.cols != (int)width || bgr_cache.rows != (int)height) {
        bgr_cache.create(height, width, CV_8UC3);
    }
    
    // ============================================================
    // 以下是将 NV12 数据包装成 OpenCV Mat 对象
    // ============================================================

    // ver_stride: 垂直步长（通常等于图像高度）
    // hor_stride:  水平步长（通常等于图像宽度，但可能有填充）
    // CV_8UC1:     8位无符号整型，1通道（灰度/单通道）
    // data:        MPP 解码器输出的原始 NV12 数据指针
    cv::Mat nv12(ver_stride * 3 / 2, hor_stride, CV_8UC1, data);

    // ============================================================
    // 执行颜色空间转换
    // ============================================================

    // cv::cvtColor: OpenCV 的颜色转换函数
    // 参数1 (nv12):       源图像 (NV12 格式)
    // 参数2 (bgr_cache):  目标图像 (BGR 格式)
    // 参数3:              转换代码，表示从 NV12 转换到 BGR
    cv::cvtColor(nv12, bgr_cache, cv::COLOR_YUV2BGR_NV12);
    
    convert_time = get_us() - convert_start;
    
    if (bgr_cache.empty()) return -1;
    
    // 裁剪到实际分辨率 (去除 stride 填充)
    if (bgr_cache.cols > (int)width || bgr_cache.rows > (int)height) {
        cv::Rect roi(0, 0, width, height);
        display_mat = bgr_cache(roi);
    } else {
        display_mat = bgr_cache;
    }
    
    return 0;
}

// ============================================================
// 解码器销毁
// ============================================================
static void destroy_decoder(DecoderContext *dec) {
    if (!dec) return;
    
    pthread_mutex_lock(&dec->mutex);
    
    if (dec->extradata) {
        free(dec->extradata);
        dec->extradata = NULL;
    }
    
    if (dec->parser) {
        av_parser_close(dec->parser);
        dec->parser = NULL;
    }
    
    if (dec->parser_ctx) {
        if (dec->parser_ctx->extradata) av_free(dec->parser_ctx->extradata);
        avcodec_free_context(&dec->parser_ctx);
    }
    
    if (dec->packet) {
        mpp_packet_deinit(&dec->packet);
        dec->packet = NULL;
    }
    
    if (dec->frame) {
        mpp_frame_deinit(&dec->frame);
        dec->frame = NULL;
    }
    
    if (dec->buf_mgr) {
        dec_buf_mgr_deinit(dec->buf_mgr);
        dec->buf_mgr = NULL;
    }
    
    if (dec->ctx) {
        mpp_destroy(dec->ctx);
        dec->ctx = NULL;
    }
    
    dec->decoder_initialized = 0;
    pthread_mutex_unlock(&dec->mutex);
    pthread_mutex_destroy(&dec->mutex);
}

// ============================================================
// RTSP 显示主函数
// ============================================================

/**
 * RTSP 拉流并显示主函数
 * 
 * 完整流程：
 * 1. 初始化 FFmpeg 网络
 * 2. 打开 RTSP 流，设置低延迟选项
 * 3. 查找视频流信息
 * 4. 初始化解码器
 * 5. 循环处理:
 *    a. av_read_frame - 从网络读取数据包
 *    b. decode_frame - 解析 + MPP 硬解
 *    c. convert_and_display - 颜色转换 + OpenCV 显示
 *    d. 更新性能统计
 */
static int rtsp_display(const char *url, int display_width, int display_height) {
    AVFormatContext *fmt_ctx = NULL;
    AVPacket *packet = NULL;
    DecoderContext dec;
    cv::Mat frame_mat;
    int video_stream_index = -1;
    int ret;
    char errbuf[128];
    MppCodingType coding_type = MPP_VIDEO_CodingUnused;
    AVCodecParameters *codec_params = NULL;
    int frame_count_display = 0;
    uint8_t *extradata = NULL;
    int extradata_size = 0;
    AVDictionary *opts = NULL;
    AVDictionary *stream_opts = NULL;
    
    memset(&dec, 0, sizeof(dec));
    memset(&g_osd, 0, sizeof(g_osd));
    snprintf(g_osd.rtsp_url, sizeof(g_osd.rtsp_url), "%s", url);
    
    // ============================================================
    // 步骤0: 初始化 FFmpeg 网络库
    // ============================================================
    avformat_network_init();
    
    // 分配格式上下文
    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx) {
        mpp_err("Failed to allocate format context\n");
        goto cleanup;
    }
    
    // ============================================================
    // 步骤1: 打开 RTSP 流，设置低延迟选项
    // ============================================================
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);      // TCP 传输 (UDP 丢包率高)
    av_dict_set(&opts, "max_delay", "50000", 0);         // 最大延迟 50ms
    av_dict_set(&opts, "buffer_size", "512000", 0);      // 缓冲区大小
    av_dict_set(&opts, "stimeout", "5000000", 0);        // 5秒超时
    av_dict_set(&opts, "fflags", "nobuffer", 0);         // 禁用缓冲
    av_dict_set(&opts, "flags", "low_delay", 0);         // 低延迟模式
    
    ret = avformat_open_input(&fmt_ctx, url, NULL, &opts);
    if (ret < 0) {
        av_strerror(ret, errbuf, sizeof(errbuf));
        mpp_err("Failed to open RTSP stream: %s\n", errbuf);
        goto cleanup;
    }
    
    // 查找流信息
    av_dict_set(&stream_opts, "flags", "low_delay", 0);
    ret = avformat_find_stream_info(fmt_ctx, &stream_opts);
    if (ret < 0) {
        mpp_err("Failed to find stream info\n");
        goto cleanup;
    }
    
    // 查找视频流
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    
    if (video_stream_index == -1) {
        mpp_err("No video stream found\n");
        goto cleanup;
    }
    
    codec_params = fmt_ctx->streams[video_stream_index]->codecpar;
    printf("\nVideo: %s | %dx%d\n", 
            avcodec_get_name(codec_params->codec_id),
            codec_params->width, codec_params->height);
    
    // 确定编码类型
    switch (codec_params->codec_id) {
        case AV_CODEC_ID_H264:
            coding_type = MPP_VIDEO_CodingAVC;
            break;
        case AV_CODEC_ID_H265:
            coding_type = MPP_VIDEO_CodingHEVC;
            break;
        default:
            mpp_err("Unsupported codec\n");
            goto cleanup;
    }
    
    // 获取 SPS/PPS 扩展数据
    if (fmt_ctx->streams[video_stream_index]->codecpar->extradata_size > 0) {
        extradata = fmt_ctx->streams[video_stream_index]->codecpar->extradata;
        extradata_size = fmt_ctx->streams[video_stream_index]->codecpar->extradata_size;
    }
    
    // ============================================================
    // 步骤2: 初始化解码器
    // ============================================================
    ret = init_decoder(&dec, coding_type, codec_params->width, codec_params->height, 
                       extradata, extradata_size);
    if (ret) {
        mpp_err("Failed to init decoder\n");
        goto cleanup;
    }
    
    packet = av_packet_alloc();
    if (!packet) {
        mpp_err("Failed to alloc packet\n");
        goto cleanup;
    }
    
    // 创建 OpenCV 显示窗口
    cv::namedWindow("RTSP", cv::WINDOW_NORMAL);
    cv::resizeWindow("RTSP", codec_params->width, codec_params->height);
    
    printf("\nStarting decoding...\n");
    printf("Display resolution: %dx%d (original)\n", codec_params->width, codec_params->height);
    printf("Controls: q=quit, s=screenshot\n\n");
    
    // 发送 SPS/PPS 到解码器
    send_extradata_to_decoder(&dec);
    
    // ============================================================
    // 主循环: 解码 + 显示
    // ============================================================
    while (1) {
        // ============================================================
        // 步骤A: RTSP 网络读取 (耗时测量点 - RTSP Read)
        // ============================================================
        long long rtsp_start = get_us();
        ret = av_read_frame(fmt_ctx, packet);
        long long rtsp_time = get_us() - rtsp_start;
        
        if (ret < 0) {
            if (ret == AVERROR_EOF) break;
            usleep(10000);
            continue;
        }
        
        if (packet->stream_index == video_stream_index) {
            void *frame_data = NULL;
            int frame_width = 0, frame_height = 0;
            
            // ============================================================
            // 步骤B: H264 解析 + MPP 硬件解码 (耗时测量点在 decode_frame 内部)
            // ============================================================
            long long decode_start = get_us();
            ret = decode_frame(&dec, packet->data, packet->size, 
                               &frame_data, &frame_width, &frame_height);
            long long decode_time = get_us() - decode_start;
            
            // ============================================================
            // 步骤C: 成功解码一帧后，进行颜色转换和显示
            // ============================================================
            if (ret == 1 && frame_data && dec.frame) {
                long long convert_time = 0, display_time = 0;
                
                // 颜色转换 (YUV -> BGR)
                ret = convert_and_display(dec.frame, &dec, frame_mat, convert_time, display_time);
                
                if (ret == 0 && !frame_mat.empty()) {
                    // 更新 OSD 数据
                    g_osd.rtsp_read_time_ms = rtsp_time / 1000.0;
                    g_osd.convert_time_ms = convert_time / 1000.0;
                    
                    // 绘制 OSD
                    draw_osd(frame_mat, &g_osd);
                    
                    // ============================================================
                    // 步骤D: OpenCV 显示 (耗时测量点 - Display)
                    // ============================================================
                    long long imshow_start = get_us();
                    cv::imshow("RTSP", frame_mat);
                    display_time = get_us() - imshow_start;
                    
                    g_osd.display_time_ms = display_time / 1000.0;
                    g_osd.total_time_ms = (rtsp_time + decode_time + convert_time + display_time) / 1000.0;
                    
                    frame_count_display++;
                    
                    // 每100帧打印一次详细数据
                    if (frame_count_display % 100 == 0) {
                        printf("\nFrame %d detail:\n", frame_count_display);
                        printf("   RTSP read:  %6.2f ms (网络拉流)\n", rtsp_time / 1000.0);
                        printf("   Decode:     %6.2f ms (MPP硬解码)\n", decode_time / 1000.0);
                        printf("   Convert:    %6.2f ms (YUV->BGR转换)\n", convert_time / 1000.0);
                        printf("   Display:    %6.2f ms (OpenCV显示)\n", display_time / 1000.0);
                        printf("   Total:      %6.2f ms\n", g_osd.total_time_ms);
                    }
                }
                
                // 释放 MPP 帧
                if (dec.frame) {
                    mpp_frame_deinit(&dec.frame);
                    dec.frame = NULL;
                }
            }
        }
        
        av_packet_unref(packet);
        
        // 处理键盘输入
        char key = cv::waitKey(1);
        if (key == 'q' || key == 27) {
            printf("\nUser requested exit\n");
            break;
        } else if (key == 's') {
            static int shot_num = 0;
            char filename[64];
            snprintf(filename, sizeof(filename), "screenshot_%d.jpg", shot_num++);
            cv::imwrite(filename, frame_mat);
            printf("Saved screenshot: %s\n", filename);
        }
    }
    
cleanup:
    if (opts) av_dict_free(&opts);
    if (stream_opts) av_dict_free(&stream_opts);
    if (packet) av_packet_free(&packet);
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    
    destroy_decoder(&dec);
    cv::destroyAllWindows();
    avformat_network_deinit();
    
    return 0;
}

// ============================================================
// 主函数
// ============================================================
int main(int argc, char **argv) {
    // -------- ROS 初始化（新增） --------
    ros::init(argc, argv, "rtsp_hw_pub");
    ros::NodeHandle nh;
    ros::Publisher image_pub = nh.advertise<sensor_msgs::Image>("/rtsp_cam/image_raw", 1);
    ros::Rate loop_rate(30);
    // -----------------------------------

    const char *rtsp_url = "rtsp://192.168.144.108";  // 默认地址
    int display_width = 0, display_height = 0;

    printf("\n========================================\n");
    printf("  RK3588 MPP Hardware Decoder - ROS Publisher\n");
    printf("========================================\n\n");

    if (argc >= 2) rtsp_url = argv[1];
    if (argc >= 3) display_width = atoi(argv[2]);
    if (argc >= 4) display_height = atoi(argv[3]);

    // ---------- 原 rtsp_display 函数的内联实现（只改显示部分） ----------
    AVFormatContext *fmt_ctx = NULL;
    AVPacket *packet = NULL;
    DecoderContext dec;
    cv::Mat frame_mat;
    int video_stream_index = -1;
    int ret;
    char errbuf[128];
    MppCodingType coding_type = MPP_VIDEO_CodingUnused;
    AVCodecParameters *codec_params = NULL;
    uint8_t *extradata = NULL;
    int extradata_size = 0;
    AVDictionary *opts = NULL, *stream_opts = NULL;

    memset(&dec, 0, sizeof(dec));
    memset(&g_osd, 0, sizeof(g_osd));
    snprintf(g_osd.rtsp_url, sizeof(g_osd.rtsp_url), "%s", rtsp_url);

    avformat_network_init();
    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx) {
        mpp_err("Failed to allocate format context\n");
        goto cleanup;
    }

    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "max_delay", "50000", 0);
    av_dict_set(&opts, "buffer_size", "512000", 0);
    av_dict_set(&opts, "stimeout", "5000000", 0);
    av_dict_set(&opts, "fflags", "nobuffer", 0);
    av_dict_set(&opts, "flags", "low_delay", 0);

    ret = avformat_open_input(&fmt_ctx, rtsp_url, NULL, &opts);
    if (ret < 0) {
        av_strerror(ret, errbuf, sizeof(errbuf));
        mpp_err("Failed to open RTSP stream: %s\n", errbuf);
        goto cleanup;
    }

    av_dict_set(&stream_opts, "flags", "low_delay", 0);
    ret = avformat_find_stream_info(fmt_ctx, &stream_opts);
    if (ret < 0) { mpp_err("Failed to find stream info\n"); goto cleanup; }

    for (unsigned i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) { mpp_err("No video stream\n"); goto cleanup; }

    codec_params = fmt_ctx->streams[video_stream_index]->codecpar;
    printf("Video: %s | %dx%d\n", avcodec_get_name(codec_params->codec_id),
           codec_params->width, codec_params->height);

    switch (codec_params->codec_id) {
        case AV_CODEC_ID_H264: coding_type = MPP_VIDEO_CodingAVC; break;
        case AV_CODEC_ID_H265: coding_type = MPP_VIDEO_CodingHEVC; break;
        default: mpp_err("Unsupported codec\n"); goto cleanup;
    }

    extradata = fmt_ctx->streams[video_stream_index]->codecpar->extradata;
    extradata_size = fmt_ctx->streams[video_stream_index]->codecpar->extradata_size;

    if (init_decoder(&dec, coding_type, codec_params->width, codec_params->height,
                     extradata, extradata_size)) {
        mpp_err("Decoder init failed\n");
        goto cleanup;
    }

    packet = av_packet_alloc();
    if (!packet) { mpp_err("Failed to alloc packet\n"); goto cleanup; }

    send_extradata_to_decoder(&dec);

    ROS_INFO("rtsp_hw_pub running ...");

    // 主循环：解码 -> 转换 -> OSD -> 发布
    while (ros::ok()) {   // 原为 while(1)
        long long rtsp_start = get_us();
        ret = av_read_frame(fmt_ctx, packet);
        long long rtsp_time = get_us() - rtsp_start;

        if (ret < 0) {
            if (ret == AVERROR_EOF) break;
            usleep(10000);
            continue;
        }

        if (packet->stream_index == video_stream_index) {
            void *frame_data = NULL;
            int frame_width = 0, frame_height = 0;

            long long decode_start = get_us();
            ret = decode_frame(&dec, packet->data, packet->size,
                               &frame_data, &frame_width, &frame_height);
            long long decode_time = get_us() - decode_start;

            if (ret == 1 && frame_data && dec.frame) {
                long long convert_time = 0, display_time = 0;

                ret = convert_and_display(dec.frame, &dec, frame_mat, convert_time, display_time);
                if (ret == 0 && !frame_mat.empty()) {
                    g_osd.rtsp_read_time_ms = rtsp_time / 1000.0;
                    g_osd.convert_time_ms = convert_time / 1000.0;

                    // 绘制 OSD 信息（如不需要，可注释本行）
                    //draw_osd(frame_mat, &g_osd);

                    // ********** 原：cv::imshow("RTSP", frame_mat); **********
                    // ********** 改为 ROS 发布 **********
                    sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", frame_mat).toImageMsg();
                    image_pub.publish(msg);
                    // ************************************

                    display_time = 0;
                    g_osd.display_time_ms = 0.0;
                    g_osd.total_time_ms = (rtsp_time + decode_time + convert_time) / 1000.0;
                }

                if (dec.frame) {
                    mpp_frame_deinit(&dec.frame);
                    dec.frame = NULL;
                }
            }
        }
        av_packet_unref(packet);

        // 原键盘交互全部删除
        ros::spinOnce();
        loop_rate.sleep();
    }

cleanup:
    if (opts) av_dict_free(&opts);
    if (stream_opts) av_dict_free(&stream_opts);
    if (packet) av_packet_free(&packet);
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    destroy_decoder(&dec);
    avformat_network_deinit();
    return 0;
}