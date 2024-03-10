#pragma once

inline uint32_t getTimestamp();
inline uint32_t getTimestamp(uint32_t after_current_in_ms);
inline uint32_t getTimePassedFrom(uint32_t timestamp);
inline bool isTimeout(uint32_t timestamp);
