#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "cpu/cpu_context.h"
#include "cpu/cpu_frame.h"

void sample_cpu_data(CpuContext *ctx, int32_t timestep) {
    if (ctx->proc_stat_fd < 0) {
        return;
    }

    // Rewind /proc/stat
    lseek(ctx->proc_stat_fd, 0, SEEK_SET);

    char buf[8192];
    ssize_t n = read(ctx->proc_stat_fd, buf, sizeof(buf) - 1);
    if (n <= 0) {
        fprintf(stderr, "Error reading /proc/stat: %s\n", strerror(errno));
        return;
    }
    buf[n] = '\0';

    int cpu_line_idx = 0;
    char *saveptr = NULL;
    char *line = strtok_r(buf, "\n", &saveptr);

    while (line) {
        if (strncmp(line, "cpu", 3) == 0) {
            // The first CPU line => aggregate => cpu_id=-1 (index 0)
            int8_t cpu_id = (int8_t)(cpu_line_idx - 1);

            // Move old usage to prev_usage
            ctx->prev_usage[cpu_line_idx] = ctx->curr_usage[cpu_line_idx];

            // Parse new usage from raw counters
            CpuUsageFrame usage = parse_cpu_frame(
                line,
                cpu_id,
                timestep,
                (int32_t)(ctx->sampleInterval * 1e6),
                &ctx->prev_raw[cpu_line_idx]
            );
            ctx->curr_usage[cpu_line_idx] = usage;

            cpu_line_idx++;

            if (cpu_line_idx > ctx->num_cpus) {
                break;
            }
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }

}
