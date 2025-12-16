#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#include "sampler.h"
#include "detector.h"
#include "ui.h"

// ================= 全局对象 =================
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// ================= Arduino setup =================
void setup() {
    Serial.begin(9600);
    while (!Serial) {
    ; // wait for USB CDC
}


    Wire.begin();

    // ---- 初始化加速度计 ----
    if (!accel.begin()) {
        Serial.println("ADXL345 not detected");
        while (1);
    }

    accel.setRange(ADXL345_RANGE_2_G);
    accel.setDataRate(ADXL345_DATARATE_50_HZ);

    
uiInit();

samplerInit();

}

// ================= Arduino loop =================
void loop() {
    samplerUpdate();                    // 中断驱动采样
    uiUpdate(detectorGetResult());       // UI 只读 detector 结果
}
