#include <Arduino.h>
#include <avr/interrupt.h>
#include "timer.h"

// ===== 50Hz 采样节拍标志 =====
volatile bool sampleTick = false;

// ===== 初始化 Timer1：50Hz CTC =====
// 16MHz / 64 = 250 kHz
// 50 Hz => 20 ms => 0.02 * 250k = 5000 counts => OCR1A = 4999
void timerInit50Hz() {
    cli();                 // 关全局中断

    TCCR1A = 0;            // 普通端口操作
    TCCR1B = 0;
    TCNT1  = 0;            // 清计数器

    // CTC 模式（Clear Timer on Compare Match）
    TCCR1B |= (1 << WGM12);

    // 比较值：20 ms
    OCR1A = 4999;

    // 预分频：64
    TCCR1B |= (1 << CS11) | (1 << CS10);

    // 允许比较匹配中断
    TIMSK1 |= (1 << OCIE1A);

    sei();                 // 开全局中断
}

// ===== Timer1 比较匹配 ISR =====
// 只置位标志，禁止做任何耗时/IO
ISR(TIMER1_COMPA_vect) {
    sampleTick = true;
}
