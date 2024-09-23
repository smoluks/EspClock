#pragma once

typedef uint64_t timestamp_uS_t;

inline timestamp_uS_t GetTimestamp();
inline timestamp_uS_t GetTimestamp(uint64_t after_current_in_us);
inline bool IsTimeout(timestamp_uS_t timestamp);
