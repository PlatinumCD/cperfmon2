#ifndef ADAPTIVE_H
#define ADAPTIVE_H

#include <stdint.h>

#include "cpu/cpu_context.h"

typedef struct {
    float delta_hat;        // The previous predicted change (PEWMA estimate).
    float sigma_hat;        // The previous predicted standard deviation.
    float new_delta_hat;
    float new_sigma_hat;
    float decay;
    float Tmin;            // Minimum allowed interval.
    float Tmax;            // Maximum allowed interval.
    float gamma;            // Imprecision threshold (between 0 and 1).
    float alpha;            // Base smoothing factor for PEWMA.
    float beta;             // Weight factor used to adjust the smoothing based on probability.
   
    float deque[14];
    int deque_idx;
} AdaptiveParams;

void create_adaptive_params(AdaptiveParams *params);

void update_adaptive_params(CpuContext *ctx, AdaptiveParams *params);

#endif
