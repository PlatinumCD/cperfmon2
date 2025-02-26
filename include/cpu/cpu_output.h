#ifndef CPU_OUTPUT_H
#define CPU_OUTPUT_H

#include "cpu_frame.h"

typedef struct CpuOutput {
    void (*init)(struct CpuOutput *self);
    void (*write_frame)(struct CpuOutput *self, const CpuUsageFrame *frame);
    void (*flush)(struct CpuOutput *self);
    void (*close)(struct CpuOutput *self);
    void *data;
} CpuOutput;

// STDOUT output
CpuOutput *create_stdout_output(int enable_all);

// (Optional) plugin output
CpuOutput *create_plugin_output(int enable_all);

#endif // CPU_OUTPUT_H
