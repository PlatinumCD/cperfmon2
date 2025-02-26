#include <time.h>
#include <unistd.h>

#include "cpu.h"
#include "sampling/adaptive.h"

int main(void) {


    CpuContext ctx;
    create_cpu_context(&ctx);

    AdaptiveParams adaptive_params;
    create_adaptive_params(&adaptive_params);

    open_proc_stat(&ctx);

    // Simple time-based sampling loop
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    float elapsed = 0.0f;
    int32_t timestep = 0;

    while (elapsed < ctx.runDuration) {
        sample_cpu_data(&ctx, timestep);

        for (int i = 0; i < ctx.num_cpus+1; i++) {
            ctx.output->write_frame(ctx.output, &ctx.curr_usage[i]);
        }

        usleep((useconds_t)(ctx.sampleInterval * 1e6));

        timestep++;
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (float)((now.tv_sec - start.tv_sec)
                  + (now.tv_nsec - start.tv_nsec)/1e9);

        
        update_adaptive_params(&ctx, &adaptive_params);
    }

    return 0;
}
