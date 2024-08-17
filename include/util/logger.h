#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <utility>

// log config

#define UTIL_LOGGER         Logger
#define UTIL_LOGGER_STREAM  std::cout

#define UTIL_LOG_PREFIX     LoggerConfig::Instance().log_prefix
#define UTIL_LOG_MINLEVEL   LoggerConfig::Instance().log_minlevel
#define UTIL_LOG_V          LoggerConfig::Instance().log_v

#ifndef UTIL_LOG_TAG
# define UTIL_LOG_TAG "native"
#endif

// log macros

#define LOG(severity) \
    UTIL_LOGGER(__FILE__, __LINE__, UTIL_LOG_TAG, UTIL_LOGGER::severity).stream()
#define LOG_IF(severity, condition) \
    !(condition) ? void(0) : LoggerVoidify() & LOG(severity)

// VLOG macros always log at the INFO log level
#define VLOG(n)       LOG_IF(INFO, UTIL_LOGGER::INFO >= UTIL_LOG_MINLEVEL && n <= UTIL_LOG_V)
#define VLOG_IS_ON(n) (n <= UTIL_LOG_V)
#define VLOG_IF(n, condition) !(condition) ? void(0) : VLOG(n)

// Logger

class LoggerConfig {
 private:
  LoggerConfig() = default;

 public:
  static LoggerConfig &Instance() {
    static LoggerConfig instance;
    return instance;
  }

  bool log_prefix = true;
  int log_minlevel = 0/*Logger::INFO*/;
  int log_v = 0;

 private:
  LoggerConfig(const LoggerConfig &) = delete;
  LoggerConfig &operator=(const LoggerConfig &) = delete;
  LoggerConfig(LoggerConfig &&) = delete;
  LoggerConfig &operator=(LoggerConfig &&) = delete;
};

class Logger {
 public:
  enum Severity {
    INFO      = 0,
    WARNING   = 1,
    ERROR     = 2,
    FATAL     = 3,
  };

  Logger(const char *file, int line, const char *tag, int severity,
      std::ostream &os = UTIL_LOGGER_STREAM)
    : file_(file), line_(line), tag_(tag), severity_(severity), ostream_(os) {
    if (severity_ < UTIL_LOG_MINLEVEL) return;
    if (!tag_.empty()) stream_ << tag_ << " ";
    if (UTIL_LOG_PREFIX) {
      StripBasename(std::string(file), &filename_only_);
      stream_ << SeverityLabel() << "/" << filename_only_ << ":" << line_ << " ";
    }
  }

  ~Logger() noexcept(false) {
    if (severity_ < UTIL_LOG_MINLEVEL) return;
    stream_ << std::endl;
    ostream_ << stream_.str();
    if (severity_ == FATAL) {
      abort();
    }
  }

  std::stringstream &stream() { return stream_; }

  template<class T>
  Logger &operator<<(const T &val) {
    stream_ << val;
    return *this;
  }

  template<class T>
  Logger &operator<<(T &&val) {
    stream_ << std::move(val);
    return *this;
  }

 private:
  void StripBasename(const std::string &full_path, std::string *filename) {
    auto pos = full_path.rfind("/");
    if (pos != std::string::npos) {
      *filename = full_path.substr(pos + 1, std::string::npos);
    } else {
      *filename = full_path;
    }
  }

  char SeverityLabel() {
    switch (severity_) {
      case INFO:      return 'I';
      case WARNING:   return 'W';
      case ERROR:     return 'E';
      case FATAL:     return 'F';
      default:        return 'V';
    }
  }

  std::stringstream stream_;

  std::string file_;
  std::string filename_only_;
  int line_;
  std::string tag_;
  int severity_;

  std::ostream &ostream_;
};

class LoggerVoidify {
 public:
  LoggerVoidify() = default;
  void operator&(const std::ostream &/*s*/) {}
};