#ifndef CPU_H
#define CPU_H

#include <stdint.h>

// Pull in everything CPU-related
#include "cpu/cpu_context.h"
#include "cpu/cpu_frame.h"
#include "cpu/cpu_output.h"

/**
 * Provide the sample_cpu_data() prototype here so main.c doesn't need
 * a manual forward declaration.
 */
void sample_cpu_data(CpuContext *ctx, int32_t timestep);

#endif // CPU_H
