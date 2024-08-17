#include "stream_video.h"

#include <cassert>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif

#include "util/log.h"
#include "util/logext.h"
#include "util/throw_error.h"

StreamVideoOp::StreamVideoOp(
    const StreamVideoOptions &options,
    const std::shared_ptr<StreamOpContext> &context)
  : options_(options), op_ctx_(context), codec_ctx_(nullptr), frame_(nullptr),
    sws_ctx_(nullptr), sws_frame_(nullptr), sws_buf_(nullptr) {
}

StreamVideoOp::~StreamVideoOp() {
  Free();
}

AVFrame *StreamVideoOp::GetFrame(AVPacket *packet) {
  assert(packet != nullptr);
  auto t = logext::TimeRecord::Create("StreamVideoOp::GetFrame");

  // decode

  if (codec_ctx_ == nullptr) {
    AVCodec *codec = nullptr;
    if (!options_.dec_name.empty()) {
      codec = avcodec_find_decoder_by_name(options_.dec_name.c_str());
      VLOG_IF(1, codec != nullptr) << "Decoder found: " << options_.dec_name;
    }
    if (codec == nullptr) {
      auto codec_id = op_ctx_->GetAVCodecID();
      codec = avcodec_find_decoder(codec_id);
      if (codec == nullptr) {
        throw_error<StreamError>() << "Decoder not found, id=" << codec_id;
      }
    }

    codec_ctx_ = avcodec_alloc_context3(codec);
    if (codec_ctx_ == nullptr)
      throw StreamError("Codec alloc context fail");

    op_ctx_->InitAVCodecContext(codec_ctx_);

    if (options_.dec_thread_count > 0) {
      codec_ctx_->thread_count = options_.dec_thread_count;
    }
    if (options_.dec_thread_type > 0) {
      codec_ctx_->thread_type = options_.dec_thread_type;
    }

    int ret = avcodec_open2(codec_ctx_, codec, nullptr);
    if (ret != 0) throw StreamError(ret);

    frame_ = av_frame_alloc();
  }

  t->Beg("avcodec_send_packet");
  int ret = avcodec_send_packet(codec_ctx_, packet);
  if (ret != 0) {
    if (ret == AVERROR(EAGAIN)) {
      return nullptr;
    } else {
      throw StreamError(ret);
    }
  }
  t->End();

  t->Beg("avcodec_receive_frame");
  ret = avcodec_receive_frame(codec_ctx_, frame_);
  if (ret != 0) {
    if (ret == AVERROR(EAGAIN)) {
      return nullptr;
    } else {
      throw StreamError(ret);
    }
  }
  t->End();

  AVFrame *result = frame_;

  // scale
  //  sws, swscale, software scale
  if (options_.sws_enable) {
      auto pix_fmt = options_.sws_dst_pix_fmt;
      int width = options_.sws_dst_width;
      int height = options_.sws_dst_height;
      int align = 1;
      int flags = options_.sws_flags;

    if (sws_ctx_ == nullptr) {
      if (pix_fmt == AV_PIX_FMT_NONE) pix_fmt = codec_ctx_->pix_fmt;
      if (width <= 0) width = codec_ctx_->width;
      if (height <= 0) height = codec_ctx_->height;
      if (flags == 0) flags = SWS_BICUBIC;

      sws_frame_ = av_frame_alloc();

      int bytes_n = av_image_get_buffer_size(pix_fmt, width, height, align);
      if (sws_buf_ != nullptr && sws_buf_size_ != bytes_n) {
        av_free(sws_buf_);
        sws_buf_ = nullptr;
      }
      if (sws_buf_ == nullptr) {
        sws_buf_ = static_cast<uint8_t *>(av_malloc(bytes_n * sizeof(uint8_t)));
        sws_buf_size_ = bytes_n;
      }
      av_image_fill_arrays(sws_frame_->data, sws_frame_->linesize, sws_buf_,
          pix_fmt, width, height, align);

      sws_frame_->width = width;
      sws_frame_->height = height;

      sws_ctx_ = sws_getContext(
          codec_ctx_->width, codec_ctx_->height, codec_ctx_->pix_fmt,
          width, height, pix_fmt, flags, nullptr, nullptr, nullptr);
      if (sws_ctx_ == nullptr) throw StreamError("Get sws context fail");
    }

    t->Beg("sws_scale");
    sws_scale(sws_ctx_, frame_->data, frame_->linesize, 0, codec_ctx_->height,
      sws_frame_->data, sws_frame_->linesize);
    t->End();

    sws_frame_->pts = frame_->pts;
    result = sws_frame_;
  }

  VLOG(2) << t->Log();
  return result;
}

void StreamVideoOp::Flush() {
  if (codec_ctx_) {
    avcodec_flush_buffers(codec_ctx_);
  }
}

void StreamVideoOp::Free() {
  if (sws_buf_ != nullptr) {
    av_free(sws_buf_);
    sws_buf_ = nullptr;
  }
  if (sws_frame_) {
    av_frame_free(&sws_frame_);
    sws_frame_ = nullptr;
  }
  if (sws_ctx_) {
    sws_freeContext(sws_ctx_);
    sws_ctx_ = nullptr;
  }
  if (frame_) {
    av_frame_free(&frame_);
    frame_ = nullptr;
  }
  if (codec_ctx_) {
    avcodec_close(codec_ctx_);
    avcodec_free_context(&codec_ctx_);
    codec_ctx_ = nullptr;
  }
}

StreamVideoOpContext::StreamVideoOpContext(AVStream *stream)
  : stream_(stream) {
}

StreamVideoOpContext::~StreamVideoOpContext() {
}

AVCodecID StreamVideoOpContext::GetAVCodecID() {
  return stream_->codecpar->codec_id;
}

void StreamVideoOpContext::InitAVCodecContext(AVCodecContext *codec_ctx) {
  int ret = avcodec_parameters_to_context(codec_ctx, stream_->codecpar);
  if (ret < 0) throw StreamError(ret);
}