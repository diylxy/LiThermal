#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <utility>

#include "log.h"

namespace logext {

using clock = std::chrono::system_clock;

/*
auto t = logext::TimeRecord::Create("task");
t->Beg("step a");
t->End();
VLOG(2) << t->Log();
*/
class TimeRecord {
 public:
  TimeRecord() = default;
  virtual ~TimeRecord() = default;

  static std::shared_ptr<TimeRecord> Create(std::string label);

  virtual void Reset(std::string) {}
  virtual void Beg(std::string) {}
  virtual void End() {}
  virtual std::string Log() { return ""; }
};

class TimeRecordImpl : public TimeRecord {
 public:
  struct record_t {
    std::string label;
    clock::time_point time_beg;
    clock::time_point time_end;
  };

  explicit TimeRecordImpl(std::string label) {
    Reset(std::move(label));
  }
  ~TimeRecordImpl() override {
  }

  void Reset(std::string label) override {
    is_beg_ = false;
    record_ = {
        std::move(label),
        clock::now(),
        clock::time_point{}};
    record_slices_.clear();
  }

  void Beg(std::string label) override {
    if (is_beg_) End();
    is_beg_ = true;
    record_slices_.push_back({
        std::move(label),
        clock::now(),
        clock::time_point{}});
  }

  void End() override {
    if (!is_beg_) return;
    is_beg_ = false;
    record_slices_.back().time_end = clock::now();
  }

  std::string Log() override {
    if (is_beg_) End();
    using namespace std::chrono;  // NOLINT
    record_.time_end = clock::now();
    auto &&d = duration_cast<milliseconds>(
        record_.time_end - record_.time_beg).count();
    std::stringstream ss;
    ss << record_.label << " cost " << d << " ms";
    if (!record_slices_.empty()) {
      ss << ", slices=[";
      for (std::size_t i = 0, n = record_slices_.size()-1; i <= n; i++) {
        auto &&r = record_slices_[i];
        auto &&d = duration_cast<milliseconds>(r.time_end - r.time_beg).count();
        ss << r.label << " " << d;
        if (i != n) ss << ", ";
      }
      ss << "] ms";
    }
    return ss.str();
  }

 private:
  bool is_beg_;
  record_t record_;
  std::vector<record_t> record_slices_;
};

inline
std::shared_ptr<TimeRecord> TimeRecord::Create(std::string label) {
  if (VLOG_IS_ON(2)) {
    return std::make_shared<TimeRecordImpl>(std::move(label));
  } else {
    static auto &&t = std::make_shared<TimeRecord>();
    return t;
  }
}

/*
auto t = logext::TimeStat::Create(60*2);
t->Beg();
t->End();
VLOG(2) << t->Log();
*/
class TimeStat : public std::enable_shared_from_this<TimeStat> {
 public:
  using duration = std::chrono::microseconds;

  explicit TimeStat(int period_secs) : period_secs_(period_secs) {}
  virtual ~TimeStat() = default;

  static std::shared_ptr<TimeStat> Create(int period_secs);

  virtual std::shared_ptr<TimeStat> Beg() { return shared_from_this(); }
  virtual std::shared_ptr<TimeStat> End() { return shared_from_this(); }
  virtual duration Elapsed() { return duration{}; }
  virtual duration Average() { return duration{}; }
  virtual std::string Log(const std::string &label = "") { return label; }

 protected:
  int period_secs_;
};

class TimeStatImpl : public TimeStat {
 public:
  explicit TimeStatImpl(int period_secs)
    : TimeStat(period_secs), time_beg_(false) {}
  ~TimeStatImpl() override {};

  std::shared_ptr<TimeStat> Beg() override {
    using namespace std::chrono;  // NOLINT
    auto now = clock::now();
    for (auto it = times_.begin(); it != times_.end(); it++) {
      if (duration_cast<seconds>(std::get<1>(*it) - now).count() >
          period_secs_) {
        it = times_.erase(it);
      } else {
        break;
      }
    }
    if (time_beg_ && !times_.empty()) {
      // update time beg if Beg() twice
      auto &&t = times_.back();
      std::get<0>(t) = clock::now();
      return shared_from_this();
    }
    time_beg_ = true;
    times_.push_back(std::make_tuple(std::move(now), clock::time_point{}));
    return shared_from_this();
  }

  std::shared_ptr<TimeStat> End() override {
    if (!time_beg_) LOG(FATAL) << "Time is end, call Beg() first";
    time_beg_ = false;
    auto &&t = times_.back();
    std::get<1>(t) = clock::now();
    return shared_from_this();
  }

  duration Elapsed() override {
    if (time_beg_) LOG(FATAL) << "Time is beg, call End() first";
    if (times_.empty()) LOG(FATAL) << "Times is empty, call Beg/End() first";
    auto &&t = times_.back();
    return std::chrono::duration_cast<duration>(std::get<1>(t)-std::get<0>(t));
  }

  duration Average() override {
    std::int64_t sum = 0;
    if (times_.empty()) return duration{};
    for (auto &&t : times_) {
      sum += std::chrono::duration_cast<duration>(
          std::get<1>(t)-std::get<0>(t)).count();
    }
    return duration{sum / times_.size()};
  }

  std::string Log(const std::string &label) override {
    std::stringstream ss;
    ss << label << " cost " << Elapsed().count() * 0.001f << " ms, avg "
      << Average().count() * 0.001f << " ms";
    return ss.str();
  }

 private:
  bool time_beg_;
  std::vector<std::tuple<clock::time_point, clock::time_point>> times_;
};

inline
std::shared_ptr<TimeStat> TimeStat::Create(int period_secs) {
  if (VLOG_IS_ON(2)) {
    return std::make_shared<TimeStatImpl>(period_secs);
  } else {
    return std::make_shared<TimeStat>(period_secs);
  }
}

/*
class TimeGuard {
 public:
  explicit TimeGuard(std::string label)
    : label_{std::move(label)}, time_start_{clock::now()} {
  }
  ~TimeGuard() {
    using namespace std::chrono;  // NOLINT
    auto &&d = duration_cast<microseconds>(clock::now() - time_start_).count();
    VLOG(2) << label_ << " cost " << d * 0.001 << " ms";
  }

  static std::unique_ptr<TimeGuard> Create(std::string label);

 private:
  std::string label_;
  clock::time_point time_start_;
};
*/

}  // namespace logext