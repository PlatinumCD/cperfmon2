#ifndef CPU_FRAME_H
#define CPU_FRAME_H

#include <stdint.h>

/**
 * Raw counters from /proc/stat
 */
typedef struct _RawCpuFrame {
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long guest_nice;
} RawCpuFrame;

/**
 * Computed usage (percent).
 */
typedef struct _CpuUsageFrame {
    int8_t  cpu_id;
    int32_t timestep;
    uint64_t sampling_interval;
    int8_t  user;
    int8_t  nice;
    int8_t  system;
    int8_t  idle;
    int8_t  iowait;
    int8_t  irq;
    int8_t  softirq;
    int8_t  steal;
    int8_t  guest;
    int8_t  guest_nice;
} CpuUsageFrame;

/**
 * parse_cpu_frame:
 *   Reads a line of /proc/stat, calculates usage percentages from
 *   previous raw counters, updates the previous counters, and returns
 *   a CpuUsageFrame.
 */
CpuUsageFrame parse_cpu_frame(const char *line,
                              int8_t cpu_id,
                              int32_t timestep,
                              uint64_t sampling_interval,
                              RawCpuFrame *prev_raw);

#endif // CPU_FRAME_H
