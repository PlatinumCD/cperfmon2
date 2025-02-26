#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sampling/adaptive.h>

#define PI 3.14159265358979323846f

void create_adaptive_params(AdaptiveParams *params) {
    params->Tmin = 0.2f;
    params->Tmax = 1.20f;
    params->gamma = 0.15f;   // scales the PEWMA output to get the sampling interval
    params->alpha = 0.7f;
    params->beta = 0.5f;
    params->decay = 0.6f;
    params->sigma_hat = 0.0f;   // running mean of diff (PEWMA state)
    params->delta_hat = 0.0f;   // running mean of diff squared
}

static int first_time = 1;

void update_adaptive_params(CpuContext *ctx, AdaptiveParams *params) {
    float prev_system_load = (100.0f - (float)ctx->prev_usage->idle);
    float curr_system_load = (100.0f - (float)ctx->curr_usage->idle);

    // Compute the absolute difference between current and previous value.
    float diff = fabsf(curr_system_load - prev_system_load);
    printf("Load diff: %f\n", diff);

    // First-time initialization: set prev_x and default sample interval.
    if (first_time) {
        first_time = 0;
        ctx->sampleInterval = params->Tmin;
        params->sigma_hat = diff;
        params->delta_hat = diff * diff;
        printf("First time initialization: sampleInterval = %f, sigma_hat = %f, delta_hat = %f\n\n\n",
               ctx->sampleInterval, params->sigma_hat, params->delta_hat);
        return;
    }

    float epsilon = 1e-6f;
    float var = fmaxf(params->delta_hat - (params->sigma_hat * params->sigma_hat), epsilon);
    float std = sqrtf(var);

    // Compute Gaussian probability density.
    float inv_sqrt_2pi = 1.0f / sqrtf(2.0f * PI);
    float P = inv_sqrt_2pi;
    if (std > 0.0f) {
        float z = (diff - params->sigma_hat) / std;
        P = inv_sqrt_2pi * expf(-0.5f * z * z);
    }

    // Adjust the update rate.
    float a_tilde = params->alpha * (1.0f - params->beta * P);
    float new_sigma = a_tilde * params->sigma_hat + (1.0f - a_tilde) * diff;
    float new_delta = a_tilde * params->delta_hat + (1.0f - a_tilde) * (diff * diff);

    float new_var = fmaxf(new_delta - (new_sigma * new_sigma), epsilon);
    float new_std = sqrtf(new_var);
    float confidence = 1.0f - fabsf(new_std - std) / (std + epsilon);

    // Update the PEWMA state.
    params->sigma_hat = new_sigma;
    params->delta_hat = new_delta;

    // Compute a new sampling interval using sigma_hat scaled by gamma,
    // and clamp it between Tmin and Tmax.
    // Compute a new interval based on the PEWMA output.
    float new_interval = params->sigma_hat * params->gamma;

    // Let the sample interval recover toward Tmax, then adjust by the new_interval.
    ctx->sampleInterval += (params->Tmax - ctx->sampleInterval) * params->decay;
    ctx->sampleInterval -= new_interval;

    // Clamp the sample interval between Tmin and Tmax.
    if (ctx->sampleInterval < params->Tmin)
        ctx->sampleInterval = params->Tmin;
    if (ctx->sampleInterval > params->Tmax)
        ctx->sampleInterval = params->Tmax;

    printf("Updated sigma_hat: %f, delta_hat: %f, sampleInterval: %f, confidence: %f\n\n\n",
           params->sigma_hat, params->delta_hat, ctx->sampleInterval, confidence);
}
