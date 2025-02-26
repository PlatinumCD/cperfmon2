#ifndef CPU_CONTEXT_H
#define CPU_CONTEXT_H

#include <stdint.h>
#include "cpu_frame.h"
#include "cpu_output.h"

/**
 * CpuContext holds all state for CPU monitoring.
 */
typedef struct {
    float  sampleInterval;
    float  runDuration;
    int8_t num_cpus;
    int    proc_stat_fd;

    RawCpuFrame   *prev_raw;
    CpuUsageFrame *prev_usage;
    CpuUsageFrame *curr_usage;

    CpuOutput     *output;
} CpuContext;

// Environment setup and teardown
void create_cpu_context(CpuContext *ctx);
void destroy_cpu_context(CpuContext *ctx);

// /proc/stat management
void open_proc_stat(CpuContext *ctx);
void close_proc_stat(CpuContext *ctx);

#endif // CPU_CONTEXT_H
