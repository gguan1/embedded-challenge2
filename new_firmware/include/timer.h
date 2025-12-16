#pragma once
#include <stdbool.h>

// ===== 50Hz 采样节拍标志 =====
// ISR 中置位，主循环清零
extern volatile bool sampleTick;

// 初始化 Timer1，使其以 50Hz 触发中断
void timerInit50Hz();
