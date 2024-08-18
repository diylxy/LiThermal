#include <stdio.h>
#include "videoCodec.h"
AVFormatContext *input_ctx = NULL;
AVFormatContext *output_ctx = NULL;
AVCodecContext *decoder_ctx = NULL;
AVStream *in_stream = NULL;
AVStream *out_stream = NULL;
struct SwsContext *sws_ctx = NULL;
AVPacket *packet = NULL;
AVFrame *frame = NULL;
int video_stream_index = -1;
bool packet_dumping = false;

int openInputStream(const char *input_url)
{
    if (avformat_open_input(&input_ctx, input_url, NULL, NULL) < 0)
    {
        fprintf(stderr, "Could not open input stream.\n");
        return -1;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(input_ctx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream information.\n");
        return -1;
    }

    // Find the video stream
    for (int i = 0; i < input_ctx->nb_streams; i++)
    {
        if (input_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1)
    {
        fprintf(stderr, "Could not find a video stream.\n");
        return -1;
    }

    in_stream = input_ctx->streams[video_stream_index];
    return 0;
}

int openInputDecoder()
{
    AVCodec *decoder = avcodec_find_decoder(in_stream->codecpar->codec_id);

    if (!decoder)
    {
        fprintf(stderr, "Failed to find MJPEG codec.\n");
        return -1;
    }

    decoder_ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(decoder_ctx, in_stream->codecpar);

    if (avcodec_open2(decoder_ctx, decoder, NULL) < 0)
    {
        fprintf(stderr, "Failed to open decoder.\n");
        return -1;
    }
    return 0;
}

int openMisc()
{
    // 帧缓冲区
    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate frame.\n");
        return -1;
    }
    packet = av_packet_alloc();
    sws_ctx = sws_getContext(decoder_ctx->width, decoder_ctx->height, decoder_ctx->pix_fmt,
                             decoder_ctx->width, decoder_ctx->height, AV_PIX_FMT_BGRA,
                             SWS_POINT, NULL, NULL, NULL);
}

bool codec_openStream(const char *url)
{
    if (openInputStream(url) < 0)
        return false;
    if (openInputDecoder() < 0)
        return false;
    if (openMisc() < 0)
        return false;
    return true;
}

void codec_closeEverything()
{
    if (input_ctx != NULL)
        avformat_close_input(&input_ctx);
    else if (output_ctx != NULL)
    {
        if (!(output_ctx->oformat->flags & AVFMT_NOFILE))
        {
            av_write_frame(output_ctx, packet);
            av_write_trailer(output_ctx);
            avio_closep(&output_ctx->pb);
        }
        avformat_free_context(output_ctx);
        packet_dumping = false;
    }
    if (decoder_ctx != NULL)
        avcodec_free_context(&decoder_ctx);
    if (frame != NULL)
        av_frame_free(&frame);
    if (sws_ctx != NULL)
    {
        sws_freeContext(sws_ctx);
        sws_ctx = NULL;
    }
}

int openOutputFile(const char *output_file)
{
    if (in_stream == NULL || decoder_ctx == NULL)
    {
        fprintf(stderr, "No input stream.\n");
        return -1;
    }
    avformat_alloc_output_context2(&output_ctx, NULL, NULL, output_file);
    if (!output_ctx)
    {
        fprintf(stderr, "Could not create output context.\n");
        return -1;
    }
    out_stream = avformat_new_stream(output_ctx, NULL);
    if (!out_stream)
    {
        fprintf(stderr, "Failed to allocate output stream.\n");
        return -1;
    }
    out_stream->time_base = in_stream->time_base; // 设置输出流时基

    // avcodec_parameters_from_context(out_stream->codecpar, encoder_ctx); // 如需重编码用这个
    avcodec_parameters_from_context(out_stream->codecpar, decoder_ctx); // 直接写入数据包

    ///////////////////////////////////////////////////////////////////////////////////////打开目标文件
    if (!(output_ctx->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&output_ctx->pb, output_file, AVIO_FLAG_WRITE) < 0)
        {
            fprintf(stderr, "Could not open output file.\n");
            return -1;
        }
    }
    ////写入输出头信息
    if (avformat_write_header(output_ctx, NULL) < 0)
    {
        fprintf(stderr, "Error occurred when writing header.\n");
        return -1;
    }

    return 0;
}

void codec_enablePacketDumping(bool en, const char *dump_target)
{
    if (packet_dumping != en)
    {
        packet_dumping = en;
        if (packet_dumping == true) // 打开
        {
            openOutputFile(dump_target);
        }
        else
        {
            if (output_ctx != NULL)
            {
                if (!(output_ctx->oformat->flags & AVFMT_NOFILE))
                {
                    av_write_frame(output_ctx, packet);
                    avio_closep(&output_ctx->pb);
                    av_write_trailer(output_ctx);
                }
                avformat_free_context(output_ctx);
            }
        }
    }
}

AVFrame *codec_getFrame()
{
    int ret;
    while (av_read_frame(input_ctx, packet) >= 0)
    {
        if (packet->stream_index == video_stream_index)
        {
            // 在这里转储mjpeg packet
            if (packet_dumping == true)
            {
                if (output_ctx != NULL)
                {
                    av_write_frame(output_ctx, packet);
                }
            }
            // packet输入解码器
            ret = avcodec_send_packet(decoder_ctx, packet);
            if (ret < 0)
            {
                fprintf(stderr, "Error sending packet for decoding.\n");
                break;
            }
            while (ret >= 0)
            {
                // 读取解码器输出
                ret = avcodec_receive_frame(decoder_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0)
                {
                    fprintf(stderr, "Error during decoding.\n");
                    return NULL;
                }

                AVFrame *scaled_frame = av_frame_alloc();
                scaled_frame->format = AV_PIX_FMT_BGRA;
                scaled_frame->width = decoder_ctx->width;
                scaled_frame->height = decoder_ctx->height;
                av_frame_get_buffer(scaled_frame, 0);
                // 色彩空间转换
                sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0, frame->height,
                          scaled_frame->data, scaled_frame->linesize);

                scaled_frame->pts = frame->pts;
                // scaled_frame：转换后的帧
                // 不考虑出现需解码多个帧的情况，直接返回
                av_packet_unref(packet);
                return scaled_frame;
            }
        }
        av_packet_unref(packet);
    }
    printf("got nothing\n");
    return NULL;
}
