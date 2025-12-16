#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_TSC2007.h>

#include "ui.h"

// ===== FeatherWing TFT（固定插接，不手写管脚）=====
#define TFT_CS   9
#define TFT_DC  10
#define TFT_RST -1

static Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
static Adafruit_TSC2007 ts;

// ===== 内部状态 =====
static bool firstDraw = true;   // ⭐ 保留，但语义修正
static DetectionResult lastResult = {false, false, 0.0f, 0.0f};

// ===== 内部：画静态布局 =====
static void drawStaticLayout() {
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);

    tft.setCursor(20, 10);
    tft.print(F("Parkinson Monitor"));

    tft.drawLine(0, 40, 320, 40, ILI9341_WHITE);

    tft.setCursor(20, 60);
    tft.print(F("Tremor:"));

    tft.setCursor(20, 110);
    tft.print(F("Dyskinesia:"));

    tft.setCursor(20, 170);
    tft.print(F("Balance:"));
}

// ===== 内部：画 YES / NO 行 =====
static void drawYesNo(int x, int y, bool yes, uint16_t color) {
    tft.fillRect(x, y, 120, 24, ILI9341_BLACK);
    tft.setCursor(x, y);
    tft.setTextSize(2);
    tft.setTextColor(color);

    if (yes) {
        tft.print(F("YES"));
    } else {
        tft.print(F("NO"));
    }
}

// ===== 内部：画比值条（0–10）=====
static void drawRatioBar(float tremorVal, float dyskVal) {
    // 清空区域
    tft.fillRect(20, 190, 280, 30, ILI9341_BLACK);

    float sum = tremorVal + dyskVal;
    if (sum <= 0.0f) return;

    int barWidth = 200;
    int tremorW = (int)(barWidth * (tremorVal / sum));
    int dyskW   = barWidth - tremorW;

    // Tremor（左）
    tft.fillRect(20, 200, tremorW, 12, ILI9341_RED);
    // Dyskinesia（右）
    tft.fillRect(20 + tremorW, 200, dyskW, 12, ILI9341_ORANGE);

    tft.drawRect(20, 200, barWidth, 12, ILI9341_WHITE);
}

// ===== 内部：清空比值区域（两个都 NO）=====
static void clearRatioArea() {
    tft.fillRect(20, 190, 280, 40, ILI9341_BLACK);
}

// ===== 对外接口 =====
void uiInit() {
    tft.begin();
    tft.setRotation(3);

    Wire.begin();
    ts.begin();

    drawStaticLayout();

    // ❌ 之前这里把 firstDraw 设成 false 是 bug
    // ✅ 现在保持 firstDraw = true，让第一次 uiUpdate 一定画
}

void uiUpdate(const DetectionResult& r) {
    // === 去重逻辑：但永远允许第一次画 ===
    if (!firstDraw) {
        bool same =
            (r.tremor == lastResult.tremor) &&
            (r.dyskinesia == lastResult.dyskinesia) &&
            (fabs(r.tremorIntensity - lastResult.tremorIntensity) < 0.01f) &&
            (fabs(r.dyskIntensity   - lastResult.dyskIntensity)   < 0.01f);

        if (same) {
            return;
        }
    }

    // ===== YES / NO 显示 =====
    drawYesNo(160, 60, r.tremor,
              r.tremor ? ILI9341_RED : ILI9341_DARKGREY);

    drawYesNo(160, 110, r.dyskinesia,
              r.dyskinesia ? ILI9341_ORANGE : ILI9341_DARKGREY);

    // ===== 比值显示规则（你要求的）=====
    if (r.tremor || r.dyskinesia) {
        drawRatioBar(r.tremorIntensity, r.dyskIntensity);
    } else {
        // 两个都是 NO → 不显示比值
        clearRatioArea();
    }

    // ===== 状态更新 =====
    lastResult = r;
    firstDraw = false;   // ⭐ 第一次画完后，才允许去重
}

void uiPollTouch() {
    // 当前版本 UI 只读，不处理触摸
}
