#include "stream_def.h"

#include <utility>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavutil/error.h>
#include <libavutil/pixdesc.h>

#ifdef __cplusplus
}
#endif

#include "util/throw_error.h"

// StreamError

StreamError::StreamError(int av_err) noexcept
  : code_(av_err) {
  char str[AV_ERROR_MAX_STRING_SIZE];
  av_make_error_string(&str[0], AV_ERROR_MAX_STRING_SIZE, av_err);
  what_message_ = str;
}

StreamError::StreamError(const std::string &what_arg) noexcept
  : StreamError(STREAM_ERROR_ANY, std::move(what_arg)) {
}

StreamError::StreamError(int code, const std::string &what_arg) noexcept
  : code_(code), what_message_(std::move(what_arg)) {
}

int StreamError::code() const noexcept {
  return code_;
}

const char *StreamError::what() const noexcept {
  return what_message_.c_str();
}

// to/from string

std::string StreamMethodToString(StreamMethod method) {
  switch (method) {
    case STREAM_METHOD_NONE:    return "none";
    case STREAM_METHOD_FILE:    return "file";
    case STREAM_METHOD_NETWORK: return "network";
    case STREAM_METHOD_WEBCAM:  return "webcam";
    default: throw StreamError("StreamMethod unknown");
  }
}

StreamMethod StreamMethodFromString(const std::string &method) {
  if (method == "none")     return STREAM_METHOD_NONE;
  if (method == "file")     return STREAM_METHOD_FILE;
  if (method == "network")  return STREAM_METHOD_NETWORK;
  if (method == "webcam")   return STREAM_METHOD_WEBCAM;
  throw_error<StreamError>() << "StreamMethod unknown: " << method;
  return STREAM_METHOD_NONE;
}

std::string PixelFormatToString(AVPixelFormat pix_fmt) {
  auto desc = av_pix_fmt_desc_get(pix_fmt);
  return desc->name;
}

AVPixelFormat PixelFormatFromString(const std::string &pix_fmt) {
  return av_get_pix_fmt(pix_fmt.c_str());
}