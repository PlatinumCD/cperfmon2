#include <stdio.h>
#include <string.h>

#include "cpu/cpu_frame.h"

/**
 * parse_cpu_frame:
 *   Convert raw counters to usage percentages.
 */
CpuUsageFrame parse_cpu_frame(const char *line,
                              int8_t cpu_id,
                              int32_t timestep,
                              int32_t sampling_interval,
                              RawCpuFrame *prev_raw)
{
    unsigned long long u, n, s, i, io, ir, so, st, g, gn;
    u = n = s = i = io = ir = so = st = g = gn = 0;

    sscanf(line, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
           &u, &n, &s, &i, &io, &ir, &so, &st, &g, &gn);

    // Create current raw
    RawCpuFrame cur_raw = {
        .user = u, .nice = n, .system = s, .idle = i,
        .iowait = io, .irq = ir, .softirq = so,
        .steal = st, .guest = g, .guest_nice = gn
    };

    // Initialize usage
    CpuUsageFrame usage;
    memset(&usage, 0, sizeof(usage));
    usage.cpu_id            = cpu_id;
    usage.timestep          = timestep;
    usage.sampling_interval = sampling_interval;

    static int first_time = 1;
    if (first_time && timestep == 0) {
        *prev_raw = cur_raw;
        first_time = 0;
        return usage;
    }

    // Calculate deltas
    unsigned long long user_diff   = cur_raw.user       - prev_raw->user;
    unsigned long long nice_diff   = cur_raw.nice       - prev_raw->nice;
    unsigned long long sys_diff    = cur_raw.system     - prev_raw->system;
    unsigned long long idle_diff   = cur_raw.idle       - prev_raw->idle;
    unsigned long long iowait_diff = cur_raw.iowait     - prev_raw->iowait;
    unsigned long long irq_diff    = cur_raw.irq        - prev_raw->irq;
    unsigned long long soft_diff   = cur_raw.softirq    - prev_raw->softirq;
    unsigned long long steal_diff  = cur_raw.steal      - prev_raw->steal;
    unsigned long long guest_diff  = cur_raw.guest      - prev_raw->guest;
    unsigned long long gnice_diff  = cur_raw.guest_nice - prev_raw->guest_nice;

    unsigned long long total = user_diff + nice_diff + sys_diff + idle_diff +
                               iowait_diff + irq_diff + soft_diff + steal_diff +
                               guest_diff + gnice_diff;

    if (total > 0) {
        usage.user       = (int8_t)((user_diff   * 100) / total);
        usage.nice       = (int8_t)((nice_diff   * 100) / total);
        usage.system     = (int8_t)((sys_diff    * 100) / total);
        usage.idle       = (int8_t)((idle_diff   * 100) / total);
        usage.iowait     = (int8_t)((iowait_diff * 100) / total);
        usage.irq        = (int8_t)((irq_diff    * 100) / total);
        usage.softirq    = (int8_t)((soft_diff   * 100) / total);
        usage.steal      = (int8_t)((steal_diff  * 100) / total);
        usage.guest      = (int8_t)((guest_diff  * 100) / total);
        usage.guest_nice = (int8_t)((gnice_diff  * 100) / total);
    }

    // Update previous raw
    *prev_raw = cur_raw;
    return usage;
}
