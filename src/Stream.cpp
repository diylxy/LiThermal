#include "stream.h"

#include <utility>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>

#ifdef __cplusplus
}
#endif

#include "util/throw_error.h"
#include "stream_video.h"

// Stream

Stream::Stream() noexcept
  : is_open_(false), format_ctx_(nullptr), packet_(nullptr) {
}

Stream::~Stream() noexcept {
  Close();
}

bool Stream::IsOpen() const noexcept {
  return is_open_;
}

void Stream::Open(const StreamOptions &options) {
  if (options.method == STREAM_METHOD_NONE || options.input_url.empty()) {
    throw StreamError("Options invalid, method and input_url must set");
  }
  options_ = options;

  // init

  if (options.method == STREAM_METHOD_FILE) {
    // ignore
  } else if (options.method == STREAM_METHOD_NETWORK) {
    avformat_network_init();
  } else if (options.method == STREAM_METHOD_WEBCAM) {
    avdevice_register_all();
  } else {
    throw StreamError("Options method invalid");
  }

  // options

  format_ctx_ = avformat_alloc_context();
  AVInputFormat *input_fmt = nullptr;
  AVDictionary *input_opt = nullptr;

  if (options.method == STREAM_METHOD_WEBCAM) {
    input_fmt = av_find_input_format(options.input_format.c_str());
    if (input_fmt == nullptr) {
      throw_error<StreamError>() << "Options input_format not found: "
        << options.input_format;
    }

    if (options.width > 0 && options.height > 0) {
      std::ostringstream os;
      os << options.width << "x" << options.height;
      av_dict_set(&input_opt, "video_size", os.str().c_str(), 0);
    }
    if (options.framerate > 0) {
      av_dict_set_int(&input_opt, "framerate", options.framerate, 0);
    }
    if (options.pixel_format != AV_PIX_FMT_NONE) {
      auto desc = av_pix_fmt_desc_get(options.pixel_format);
      av_dict_set(&input_opt, "pixel_format", desc->name, 0);
    }
  }

  if (options.rtbufsize > 0) {
    av_dict_set_int(&input_opt, "rtbufsize", options.rtbufsize, 0);
  } else if (options.method == STREAM_METHOD_WEBCAM) {
    if (options.width > 0 && options.height > 0) {
      int64_t rtbufsize = options.width * options.height;
      rtbufsize *= (options.framerate > 0) ? options.framerate : 15;
      rtbufsize *= 2;
      av_dict_set_int(&input_opt, "rtbufsize", rtbufsize, 0);
    }
  }
  if (options.max_delay > 0) {
    av_dict_set_int(&input_opt, "max_delay", options.max_delay, 0);
  }

  if (!options.rtsp_transport.empty()) {
    if (options.rtsp_transport == "udp" || options.rtsp_transport == "tcp") {
      av_dict_set(&input_opt, "rtsp_transport", options.rtsp_transport.c_str(),
                  0);
    } else {
      throw_error<StreamError>() << "Options rtsp_transport invalid: "
        << options.rtsp_transport;
    }
  }
  if (options.stimeout > 0) {
    av_dict_set_int(&input_opt, "stimeout", options.stimeout, 0);
  }
  // detecting a timeout in ffmpeg
  //  https://stackoverflow.com/a/10666409
  // format_ctx_->interrupt_callback.callback = interrupt_cb;
  // format_ctx_->interrupt_callback.opaque = format_ctx_;

  // open

  int ret = avformat_open_input(&format_ctx_, options.input_url.c_str(),
    input_fmt, (input_opt == nullptr) ? nullptr : &input_opt);
  if (ret != 0) throw StreamError(ret);

  ret = avformat_find_stream_info(format_ctx_, nullptr);
  if (ret < 0) throw StreamError(ret);

  if (options_.dump_format)
    av_dump_format(format_ctx_, 0, options.input_url.c_str(), 0);

  // stream

  for (unsigned int i = 0; i < format_ctx_->nb_streams; i++) {
    auto codec_type = format_ctx_->streams[i]->codecpar->codec_type;
    if (stream_subs_.find(codec_type) != stream_subs_.end()) {
      // stream of the type already set, only keep the first one
      continue;
    }
    if (codec_type == AVMEDIA_TYPE_VIDEO) {
      auto info = std::make_shared<StreamSubInfo>();
      avcodec_parameters_copy(info->codecpar,
          format_ctx_->streams[i]->codecpar);
      stream_subs_[codec_type] = std::shared_ptr<StreamSub>(new StreamSub{
        format_ctx_->streams[i],
        std::make_shared<StreamVideoOp>(
            options.video,
            std::make_shared<StreamVideoOpContext>(format_ctx_->streams[i])),
        std::move(info),
      });
    } else {
      // stream of the type need support later, such as AVMEDIA_TYPE_AUDIO
      continue;
    }
  }

  is_open_ = true;
}

AVPacket *Stream::GetPacket(bool unref) {
  if (packet_ == nullptr) {
    packet_ = av_packet_alloc();
  }
  int ret = av_read_frame(format_ctx_, packet_);
  if (ret < 0) throw StreamError(ret);

  if (unref) av_packet_unref(packet_);
  return packet_;
}

AVFrame *Stream::GetFrame(AVMediaType type, AVPacket *packet, bool unref) {
  if (packet == nullptr) {
    packet = GetPacket(false);
  }
  auto sub = GetStreamSub(type);
  AVFrame *frame = nullptr;
  if (packet->stream_index == sub->stream->index) {
    frame = sub->op->GetFrame(packet);
  }
  if (unref) av_packet_unref(packet_);
  return frame;
}

void Stream::UnrefPacket() {
  if (packet_ != nullptr) {
    av_packet_unref(packet_);
  }
}

AVFrame *Stream::GetFrameVideo() {
  return GetFrame(AVMEDIA_TYPE_VIDEO, nullptr, true);
}

void Stream::Close() {
  stream_subs_.clear();
  if (packet_) {
    av_packet_free(&packet_);
    packet_ = nullptr;
  }
  if (format_ctx_) {
    avformat_close_input(&format_ctx_);
    format_ctx_ = nullptr;
  }
  is_open_ = false;
}


StreamOptions Stream::GetOptions() const {
  return options_;
}

Stream::stream_subs_t Stream::GetStreamSubs() const {
  return stream_subs_;
}

std::shared_ptr<Stream::stream_sub_t>
Stream::GetStreamSub(AVMediaType type) const {
  try {
    return stream_subs_.at(type);
  } catch (const std::out_of_range &e) {
    throw StreamError(e.what());
  }
}