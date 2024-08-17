#pragma once

#include <chrono>
#include <thread>

class Rate {
 public:
  using clock = std::chrono::system_clock;

  explicit Rate(std::int32_t frequency);
  ~Rate();

  void Sleep();

  void Reset();

  clock::duration CycleTime();

  clock::duration ExpectedCycleTime();

 private:
  clock::time_point time_beg_;
  clock::duration expected_cycle_time_, actual_cycle_time_;
};

inline
Rate::Rate(std::int32_t frequency)
  : time_beg_(clock::now()),
    expected_cycle_time_(clock::period::den / clock::period::num / frequency),
    actual_cycle_time_(0) {
}

inline
Rate::~Rate() {
}

inline
void Rate::Sleep() {
  auto expected_end = time_beg_ + expected_cycle_time_;

  auto actual_end = clock::now();

  // detect backward jumps in time
  if (actual_end < time_beg_) {
    expected_end = actual_end + expected_cycle_time_;
  }

  // calculate the time we'll sleep for
  auto sleep_time = expected_end - actual_end;

  // set the actual amount of time the loop took in case the user wants to know
  actual_cycle_time_ = actual_end - time_beg_;

  // make sure to reset our start time
  time_beg_ = expected_end;

  // if we've taken too much time we won't sleep
  if (sleep_time <= clock::duration(0)) {
    // if we've jumped forward in time, or the loop has taken more than a full
    // extra cycle, reset our cycle
    if (actual_end > expected_end + expected_cycle_time_) {
      time_beg_ = actual_end;
    }
    return;
  }

  std::this_thread::sleep_for(sleep_time);
}

inline
void Rate::Reset() {
  time_beg_ = clock::now();
}

inline
Rate::clock::duration Rate::CycleTime() {
  return actual_cycle_time_;
}

inline
Rate::clock::duration Rate::ExpectedCycleTime() {
  return expected_cycle_time_;
}