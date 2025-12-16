#include <Arduino.h>
#include <math.h>
#include <ArduinoFFT.h>

#include "detector.h"
#include "sampler.h"   // provides FFT_SIZE + SAMPLE_RATE_HZ

static float vReal[FFT_SIZE];
static float vImag[FFT_SIZE];

static ArduinoFFT<float> FFT(vReal, vImag, FFT_SIZE, SAMPLE_RATE_HZ);

static DetectionResult lastResult = {false, false, 0.0f, 0.0f};

static void findPeakFull(float &f_peak, float &A_peak) {
    A_peak = 0.0f;
    f_peak = 0.0f;

    const int bins = FFT_SIZE / 2;

    // Skip DC bin i=0 (mean/gravity), search positive frequencies
    for (int i = 1; i < bins; i++) {
        const float a = vReal[i];
        if (a > A_peak) {
            A_peak = a;
            f_peak = (i * (float)SAMPLE_RATE_HZ) / (float)FFT_SIZE;
        }
    }
}

static void computeBandEnergy(float &E35, float &E57) {
    E35 = 0.0f;
    E57 = 0.0f;

    const int bins = FFT_SIZE / 2;

    for (int i = 1; i < bins; i++) {
        const float freq = (i * (float)SAMPLE_RATE_HZ) / (float)FFT_SIZE;
        const float e = vReal[i] * vReal[i];

        if (freq >= 3.0f && freq < 5.0f) {
            E35 += e;
        } else if (freq >= 5.0f && freq <= 7.0f) {
            E57 += e;
        }
        // no early break; bins are small anyway
    }
}

void detectorRun(float* samples) {
    // Copy samples directly (expects FFT_SIZE samples)
    for (int i = 0; i < FFT_SIZE; i++) {
        vReal[i] = samples[i];
        vImag[i] = 0.0f;
    }

    // (Optional) Windowing — leave OFF for now (transparent debugging)
    // FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);

    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude();

    float f_peak = 0.0f;
    float A_peak = 0.0f;
    findPeakFull(f_peak, A_peak);

    // YES/NO strictly by rubric
    lastResult.tremor     = (f_peak >= 3.0f && f_peak < 5.0f);
    lastResult.dyskinesia = (f_peak >= 5.0f && f_peak <= 7.0f);

    // Ratio-based intensity (3–5 vs 5–7) on a 0–10 scale
    float E35 = 0.0f, E57 = 0.0f;
    computeBandEnergy(E35, E57);

    const float denom = E35 + E57;
    if (denom > 0.0f) {
        lastResult.tremorIntensity = (E35 / denom) * 10.0f;
        lastResult.dyskIntensity   = (E57 / denom) * 10.0f;
    } else {
        lastResult.tremorIntensity = 0.0f;
        lastResult.dyskIntensity   = 0.0f;
    }

    // Debug (you can comment this out later to save flash)
    Serial.print("f_peak=");
    Serial.print(f_peak, 2);
    Serial.print(" A_peak=");
    Serial.println(A_peak, 2);
}

DetectionResult detectorGetResult() {
    return lastResult;
}
