#pragma once

inline uint32_t GetTimestamp();
inline uint32_t GetTimestamp(uint32_t after_current_in_ms);
inline bool IsTimeout(uint32_t timestamp);
