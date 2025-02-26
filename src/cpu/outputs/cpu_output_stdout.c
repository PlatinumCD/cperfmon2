#include <stdio.h>
#include <stdlib.h>
#include "cpu/cpu_output.h"

typedef struct {
    int enable_all;
    void (*write_impl)(CpuOutput *self, const CpuUsageFrame *frame);
} StdoutData;

// Separate write functions
static void stdout_write_frame_all(CpuOutput *self, const CpuUsageFrame *frame) {
    (void)self;
    printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
        frame->cpu_id, frame->timestep, frame->sampling_interval,
        frame->user, frame->nice, frame->system, frame->idle,
        frame->iowait, frame->irq, frame->softirq,
        frame->steal, frame->guest, frame->guest_nice
    );
}

static void stdout_write_frame_default(CpuOutput *self, const CpuUsageFrame *frame) {
    (void)self;
    printf("%d,%d,%d,%d,%d,%d\n",
        frame->cpu_id, frame->timestep, frame->sampling_interval,
        frame->user, frame->system, frame->idle
    );
}

// Normal callbacks
static void stdout_init(CpuOutput *self) {
    StdoutData *d = (StdoutData *)self->data;

    if (d->enable_all) {
        printf("cpu_id,timestep,sampling_interval,user,nice,system,idle,iowait,irq,softirq,steal,guest,guest_nice\n");
        d->write_impl = stdout_write_frame_all;
    } else {
        printf("cpu_id,timestep,sampling_interval,user,system,idle\n");
        d->write_impl = stdout_write_frame_default;
    }
    fflush(stdout);

    // Assign function pointer
    self->write_frame = d->write_impl;
}

static void stdout_flush(CpuOutput *self) {
    (void)self;
    fflush(stdout);
}

static void stdout_close(CpuOutput *self) {
    free(self->data);
    self->data = NULL;
}

CpuOutput *create_stdout_output(int enable_all) {
    CpuOutput *out = malloc(sizeof(CpuOutput));
    StdoutData *data = malloc(sizeof(StdoutData));

    data->enable_all = enable_all;
    data->write_impl = NULL;

    out->init        = stdout_init;
    out->write_frame = NULL; // set in stdout_init
    out->flush       = stdout_flush;
    out->close       = stdout_close;
    out->data        = data;
    return out;
}
