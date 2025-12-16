#pragma once

#include <stdint.h>

typedef struct {
    bool  tremor;          // peak in 3–5 Hz
    bool  dyskinesia;      // peak in 5–7 Hz
    float tremorIntensity; // 0–10 scale (ratio-based)
    float dyskIntensity;   // 0–10 scale (ratio-based)
} DetectionResult;

void detectorRun(float* samples);     // samples length must be FFT_SIZE
DetectionResult detectorGetResult();
