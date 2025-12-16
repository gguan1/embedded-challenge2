#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#include "sampler.h"
#include "detector.h"

#define ADXL_ADDR 0x53

// ADXL345 registers
#define REG_FIFO_CTL   0x38
#define REG_FIFO_STAT  0x39

extern Adafruit_ADXL345_Unified accel;

static float sampleBuffer[FFT_SIZE];
static uint8_t sampleIndex = 0;

// ---- 低级寄存器访问 ----
static void adxlWrite(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ADXL_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

static uint8_t adxlRead(uint8_t reg) {
    Wire.beginTransmission(ADXL_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(ADXL_ADDR, (uint8_t)1);
    return Wire.read();
}

// ---- 初始化 FIFO ----
void samplerInit() {
    // 50 Hz 已在 main 里 setDataRate()

    // FIFO: stream mode, watermark unused
    // 0b10xxxxxx = stream
    adxlWrite(REG_FIFO_CTL, 0x80);

    sampleIndex = 0;
}

// ---- 主采样更新 ----
void samplerUpdate() {
    // FIFO 中有多少个样本
    uint8_t fifoCount = adxlRead(REG_FIFO_STAT) & 0x3F;
    if (fifoCount == 0) return;

    while (fifoCount--) {
        sensors_event_t event;
        accel.getEvent(&event);

        float mag = sqrt(
            event.acceleration.x * event.acceleration.x +
            event.acceleration.y * event.acceleration.y +
            event.acceleration.z * event.acceleration.z
        );

        sampleBuffer[sampleIndex++] = mag;

        if (sampleIndex >= FFT_SIZE) {
            detectorRun(sampleBuffer);
            sampleIndex = 0;
        }
    }
}
