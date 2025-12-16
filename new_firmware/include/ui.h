#pragma once
#include "detector.h"

// 初始化 TFT 和 touch
void uiInit();

// 根据最新检测结果更新显示
// 只在 detectorRun() 之后调用
void uiUpdate(const DetectionResult& result);

// 轮询触摸（可选）
// 不得影响采样
void uiPollTouch();
