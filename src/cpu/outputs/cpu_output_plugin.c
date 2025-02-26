#include <stdio.h>
#include <stdlib.h>
#include "waggle/plugin.h"
#include "waggle/config.h"
#include "waggle/timeutil.h"
#include "cpu/cpu_output.h"

typedef struct {
    int enable_all;
    Plugin *p;
    void (*write_impl)(CpuOutput *self, const CpuUsageFrame *frame);
} PluginOutputData;

static void plugin_write_frame_all(CpuOutput *self, const CpuUsageFrame *frame) {
    PluginOutputData *d = (PluginOutputData *)self->data;
    if (!d->p) return;

    const char *fields[] = {
        "user", "nice", "system", "idle", "iowait",
        "irq", "softirq", "steal", "guest", "guest_nice"
    };
    int8_t values[] = {
        frame->user, frame->nice, frame->system, frame->idle, frame->iowait,
        frame->irq, frame->softirq, frame->steal, frame->guest, frame->guest_nice
    };

    uint64_t ts = waggle_get_timestamp_ns();
    char meta[32];
    snprintf(meta, sizeof(meta), "{\"si\": \"%" PRIu64 "\"}", frame->sampling_interval);

    for (int i = 0; i < 10; i++) {
        char topic[64];
        if (frame->cpu_id < 0)
            snprintf(topic, sizeof(topic), "perf.cpu.%s", fields[i]);
        else
            snprintf(topic, sizeof(topic), "perf.cpu%d.%s", frame->cpu_id, fields[i]);
        plugin_publish(d->p, "all", topic, values[i], ts, meta);
    }
}

static void plugin_write_frame_default(CpuOutput *self, const CpuUsageFrame *frame) {
    PluginOutputData *d = (PluginOutputData *)self->data;
    if (!d->p) return;

    const char *fields[] = {"user", "system", "idle"};
    int8_t values[] = {frame->user, frame->system, frame->idle};

    uint64_t ts = waggle_get_timestamp_ns();
    char meta[32];
    snprintf(meta, sizeof(meta), "{\"si\": \"%" PRIu64 "\"}", frame->sampling_interval);

    for (int i = 0; i < 3; i++) {
        char topic[64];
        if (frame->cpu_id < 0)
            snprintf(topic, sizeof(topic), "perf.cpu.%s", fields[i]);
        else
            snprintf(topic, sizeof(topic), "perf.cpu%d.%s", frame->cpu_id, fields[i]);
        plugin_publish(d->p, "all", topic, values[i], ts, meta);
    }
}

static void plugin_init(CpuOutput *self) {
    PluginOutputData *d = (PluginOutputData *)self->data;

    const char *user = getenv("WAGGLE_PLUGIN_USERNAME") ? getenv("WAGGLE_PLUGIN_USERNAME") : "plugin";
    const char *pass = getenv("WAGGLE_PLUGIN_PASSWORD") ? getenv("WAGGLE_PLUGIN_PASSWORD") : "plugin";
    const char *host = getenv("WAGGLE_PLUGIN_HOST") ? getenv("WAGGLE_PLUGIN_HOST") : "rabbitmq";
    int port = getenv("WAGGLE_PLUGIN_PORT") ? atoi(getenv("WAGGLE_PLUGIN_PORT")) : 5672;
    const char *app = getenv("WAGGLE_APP_ID") ? getenv("WAGGLE_APP_ID") : "";
    
    PluginConfig *cfg = plugin_config_new(user, pass, host, port, app);
    if (!cfg) {
        fprintf(stderr, "Failed to create PluginConfig.\n");
        exit(EXIT_FAILURE);
    }
    d->p = plugin_new(cfg);
    if (!d->p) {
        fprintf(stderr, "Failed to create Plugin.\n");
        exit(EXIT_FAILURE);
    }

    if (d->enable_all) {
        d->write_impl = plugin_write_frame_all;
    } else {
        d->write_impl = plugin_write_frame_default;
    }
    self->write_frame = d->write_impl;
}

static void plugin_flush(CpuOutput *self) {
    (void)self;
}

static void plugin_close(CpuOutput *self) {
    PluginOutputData *d = (PluginOutputData *)self->data;
    if (d->p) {
        plugin_free(d->p);
        d->p = NULL;
    }
    free(d);
    self->data = NULL;
}

CpuOutput *create_plugin_output(int enable_all) {
    CpuOutput *out = malloc(sizeof(CpuOutput));
    PluginOutputData *data = malloc(sizeof(PluginOutputData));

    data->enable_all = enable_all;
    data->p = NULL;
    data->write_impl = NULL;

    out->init        = plugin_init;
    out->write_frame = NULL;  // Set in plugin_init.
    out->flush       = plugin_flush;
    out->close       = plugin_close;
    out->data        = data;
    return out;
}
