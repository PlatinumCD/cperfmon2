#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "cpu/cpu_context.h"
#include "cpu/cpu_output.h"

void create_cpu_context(CpuContext *ctx) {
    // Clear
    memset(ctx, 0, sizeof(*ctx));

    // Defaults
    ctx->sampleInterval = 1.0f;
    ctx->runDuration    = 30.0f;

    // If "PERFMON_ONLY_COLLECT_AGGREGATE" => skip per-core
    if (getenv("PERFMON_ONLY_COLLECT_AGGREGATE")) {
        ctx->num_cpus = 0;
    } else {
        cpu_set_t set;
        CPU_ZERO(&set);
        if (sched_getaffinity(0, sizeof(set), &set) == 0) {
            ctx->num_cpus = (int8_t)CPU_COUNT(&set);
        }
    }

    // Allocate arrays: index 0 => aggregate (cpu_id = -1), rest => per-core
    int total = (ctx->num_cpus > 0) ? (ctx->num_cpus + 1) : 1;
    ctx->prev_raw   = calloc(total, sizeof(RawCpuFrame));  // <-- Add this line
    ctx->prev_usage = calloc(total, sizeof(CpuUsageFrame));
    ctx->curr_usage = calloc(total, sizeof(CpuUsageFrame));


    int enable_all = 0;
    const char *env_enable_all = getenv("PERFMON_ENABLE_ALL");
    if (env_enable_all && strcmp(env_enable_all, "0") != 0) {
        enable_all = 1;
    }

    // Decide output type using an environment variable (example)
    if (getenv("PERFMON_PUBLISH_DATA")) {
//        ctx.output = create_plugin_output(/*unused in init*/ 0);
    } else {
        ctx->output = create_stdout_output(enable_all);
    }
    ctx->output->init(ctx->output);


    // Override sampleInterval if set
    const char *env = getenv("PERFMON_SAMPLE_INTERVAL");
    if (env) {
        float val = strtof(env, NULL);
        if (val > 0.0f) {
            ctx->sampleInterval = val;
        }
    }
    // Override runDuration if set
    env = getenv("PERFMON_DURATION");
    if (env) {
        float val = strtof(env, NULL);
        if (val > 0.0f) {
            ctx->runDuration = val;
        }
    }

    ctx->proc_stat_fd = -1;
}

void destroy_cpu_context(CpuContext *ctx) {
    // Cleanup
    ctx->output->flush(ctx->output);
    ctx->output->close(ctx->output);

    close_proc_stat(ctx);

    free(ctx->prev_raw);
    free(ctx->prev_usage);
    free(ctx->curr_usage);
}

void open_proc_stat(CpuContext *ctx) {
    ctx->proc_stat_fd = open("/proc/stat", O_RDONLY);
    if (ctx->proc_stat_fd < 0) {
        perror("open /proc/stat");
        exit(EXIT_FAILURE);
    }
}

void close_proc_stat(CpuContext *ctx) {
    if (ctx->proc_stat_fd >= 0) {
        close(ctx->proc_stat_fd);
        ctx->proc_stat_fd = -1;
    }
}
