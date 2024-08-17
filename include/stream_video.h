#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct AVStream;
struct AVPacket;
struct AVFrame;
struct AVCodecContext;
struct SwsContext;

#ifdef __cplusplus
}
#endif

#include <memory>

#include "stream.h"

class StreamVideoOp : public StreamOp {
 public:
  StreamVideoOp(const StreamVideoOptions &options,
                const std::shared_ptr<StreamOpContext> &context);
  ~StreamVideoOp() override;

  AVFrame *GetFrame(AVPacket *packet) override;

  void Flush();
  void Free();

 private:
  StreamVideoOptions options_;
  std::shared_ptr<StreamOpContext> op_ctx_;

  AVCodecContext *codec_ctx_;
  AVFrame *frame_;

  SwsContext *sws_ctx_;
  AVFrame *sws_frame_;
  uint8_t *sws_buf_;
  int sws_buf_size_;
};

class StreamVideoOpContext : public StreamOpContext {
 public:
  explicit StreamVideoOpContext(AVStream *stream);
  ~StreamVideoOpContext() override;

  AVCodecID GetAVCodecID() override;
  void InitAVCodecContext(AVCodecContext *) override;

 private:
  AVStream *stream_;
};