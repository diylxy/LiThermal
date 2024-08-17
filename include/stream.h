#pragma once

#include <memory>
#include <unordered_map>

#ifdef __cplusplus
extern "C" {
#endif

struct AVCodecContext;
struct AVFormatContext;
struct AVStream;
struct AVPacket;
struct AVFrame;

#include <libavcodec/codec_id.h>
#include <libavcodec/packet.h>
#include <libavutil/frame.h>

#ifdef __cplusplus
}
#endif

#include "stream_def.h"

class StreamOp {
 public:
  virtual ~StreamOp() = default;
  virtual AVFrame *GetFrame(AVPacket *packet) = 0;
};

class StreamOpContext {
 public:
  virtual ~StreamOpContext() = default;
  virtual AVCodecID GetAVCodecID() = 0;
  virtual void InitAVCodecContext(AVCodecContext *) = 0;
};

struct StreamSub {
  AVStream *stream;
  std::shared_ptr<StreamOp> op;
  std::shared_ptr<StreamSubInfo> info;
};

class Stream {
 public:
  using stream_sub_t = StreamSub;
  using stream_subs_t =
      std::unordered_map<AVMediaType, std::shared_ptr<stream_sub_t>>;

  Stream() noexcept;
  ~Stream() noexcept;

  bool IsOpen() const noexcept;
  void Open(const StreamOptions &options);

  AVPacket *GetPacket(bool unref = true);
  AVFrame *GetFrame(AVMediaType type, AVPacket *packet = nullptr,
    bool unref = false);
  void UnrefPacket();  // unref packet after get

  AVFrame *GetFrameVideo();

  void Close();

  StreamOptions GetOptions() const;
  stream_subs_t GetStreamSubs() const;
  std::shared_ptr<stream_sub_t> GetStreamSub(AVMediaType type) const;

 private:
  StreamOptions options_;
  bool is_open_;

  AVFormatContext *format_ctx_;
  AVPacket *packet_;

  stream_subs_t stream_subs_;
};