/*
 * DOC — Barra de LED 8x WS2812 (driver + animação).
 * Arquivo: main/boards/doc-ai-s3cam/doc_led_bar.h
 *
 * Um temporizador atualiza a barra a ~25 quadros por segundo a partir do
 * estilo atual (ver doc_led_effects.h). A barra pode ser "suspensa": isso
 * libera o RMT e o GPIO, necessário quando o pino é compartilhado com a
 * câmera (GPIO18 = D5 da câmera OV3660).
 */
#pragma once

#include <driver/gpio.h>
#include <esp_timer.h>
#include <led_strip.h>

#include <cstdint>
#include <mutex>
#include <vector>

#include "doc_led_effects.h"

class DocLedBar {
public:
    // level: brilho geral 0..255 (255 = máximo). count: quantidade de LEDs.
    DocLedBar(gpio_num_t gpio, int count, uint8_t level);
    ~DocLedBar();

    DocLedBar(const DocLedBar&) = delete;
    DocLedBar& operator=(const DocLedBar&) = delete;

    // Troca o estilo. Se for igual ao atual, a animação continua de onde estava.
    void SetStyle(const DocLedStyle& style);

    // Apaga, libera o RMT e o pino (a câmera pode usá-lo). Idempotente.
    void Suspend();
    // Recria a barra depois que a câmera soltou o pino. Idempotente.
    void Resume();

private:
    bool CreateStrip();
    void DestroyStrip();
    void Tick();

    gpio_num_t gpio_;
    int count_;
    uint8_t level_;

    std::mutex mutex_;
    led_strip_handle_t strip_ = nullptr;
    esp_timer_handle_t timer_ = nullptr;

    DocLedStyle style_{DocLedMode::kOff, {0, 0, 0}, 1000};
    int64_t style_start_us_ = 0;
    bool suspended_ = false;
    bool force_refresh_ = true;
    std::vector<DocRgb> frame_;
    std::vector<DocRgb> last_;
};
