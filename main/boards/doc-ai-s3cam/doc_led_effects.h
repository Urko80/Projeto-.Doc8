/*
 * DOC — Efeitos e tabelas da barra de LED (8x WS2812).
 * Arquivo: main/boards/doc-ai-s3cam/doc_led_effects.h
 *
 * Esta parte NÃO usa ESP-IDF: só calcula cores a partir do tempo. Assim a
 * tabela de emoções pode ser testada no computador e editada num único lugar.
 */
#pragma once

#include <cstdint>

#include "device_state.h"

struct DocRgb {
    uint8_t r, g, b;
};

enum class DocLedMode : uint8_t {
    kOff,      // apagado
    kSolid,    // cor fixa
    kBreathe,  // respira (acende e apaga suave)
    kBlink,    // pisca
    kScroll,   // um "cometa" vai e volta
    kRainbow,  // arco-íris girando
};

struct DocLedStyle {
    DocLedMode mode;
    DocRgb color;         // cor em escala cheia (0..255); o brilho geral é aplicado depois
    uint16_t period_ms;   // duração de um ciclo da animação
};

inline bool operator==(const DocLedStyle& a, const DocLedStyle& b) {
    return a.mode == b.mode && a.color.r == b.color.r && a.color.g == b.color.g &&
           a.color.b == b.color.b && a.period_ms == b.period_ms;
}
inline bool operator!=(const DocLedStyle& a, const DocLedStyle& b) { return !(a == b); }

// Estilo da barra para uma emoção enviada pelo servidor (ex.: "angry", "happy").
// Nome desconhecido usa o estilo de "neutral".
DocLedStyle DocLedStyleForEmotion(const char* emotion);

// Estilo da barra para o estado atual do robô.
//  - voice_detected: o microfone está ouvindo voz (só importa em "ouvindo");
//  - thinking: o servidor mandou a emoção "thinking" e ainda não começou a falar;
//  - emotion: última emoção recebida (usada quando o robô está falando).
DocLedStyle DocLedStyleForState(DeviceState state, bool voice_detected, bool thinking,
                                const char* emotion);

// Calcula as cores dos `count` LEDs no instante `elapsed_ms` desde que o estilo começou.
// `level` (0..255) é o brilho geral: 255 = máximo, 48 = cerca de 19 %.
void DocLedRender(const DocLedStyle& style, uint32_t elapsed_ms, int count, uint8_t level,
                  DocRgb* out);
