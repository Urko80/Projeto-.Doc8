/*
 * DOC — Efeitos e tabelas da barra de LED (8x WS2812).
 * Arquivo: main/boards/doc-ai-s3cam/doc_led_effects.cc
 */
#include "doc_led_effects.h"

#include <cmath>
#include <cstring>

namespace {

// ============================================================
// DOC — 1. TABELA DE EMOÇÕES  (edite aqui as cores)
// ============================================================
// Formato: {nome da emoção, {modo, {R, G, B}, duração do ciclo em ms}}
// As cores são em escala cheia; o brilho geral é controlado por
// DOC_LED_BAR_LEVEL em config.h.
struct EmotionEntry {
    const char* name;
    DocLedStyle style;
};

constexpr DocLedStyle kNeutral = {DocLedMode::kBreathe, {30, 90, 255}, 2500};

constexpr EmotionEntry kEmotions[] = {
    // padrão
    {"neutral",     kNeutral},
    {"idle",        kNeutral},
    {"normal",      kNeutral},
    {"robot_2",     kNeutral},
    {"staticstate", kNeutral},
    // alegria: verde
    {"happy",       {DocLedMode::kBreathe, {0, 255, 40}, 1200}},
    {"laughing",    {DocLedMode::kBreathe, {0, 255, 40}, 900}},
    {"funny",       {DocLedMode::kBreathe, {0, 255, 40}, 900}},
    {"joy",         {DocLedMode::kBreathe, {0, 255, 40}, 1200}},
    {"excited",     {DocLedMode::kBreathe, {0, 255, 40}, 800}},
    {"glee",        {DocLedMode::kBreathe, {0, 255, 40}, 1000}},
    // carinho: rosa
    {"loving",      {DocLedMode::kBreathe, {255, 20, 120}, 1600}},
    {"love",        {DocLedMode::kBreathe, {255, 20, 120}, 1600}},
    {"kissy",       {DocLedMode::kBreathe, {255, 20, 120}, 1400}},
    // tristeza: azul, devagar
    {"sad",         {DocLedMode::kBreathe, {0, 40, 255}, 2800}},
    {"crying",      {DocLedMode::kBreathe, {0, 40, 255}, 2000}},
    {"cry",         {DocLedMode::kBreathe, {0, 40, 255}, 2000}},
    {"worried",     {DocLedMode::kBreathe, {0, 40, 255}, 2200}},
    // raiva: vermelho
    {"angry",       {DocLedMode::kBreathe, {255, 0, 0}, 700}},
    {"anger",       {DocLedMode::kBreathe, {255, 0, 0}, 700}},
    {"furious",     {DocLedMode::kBlink,   {255, 0, 0}, 360}},
    {"rage",        {DocLedMode::kBlink,   {255, 0, 0}, 360}},
    // surpresa: amarelo piscando
    {"surprised",   {DocLedMode::kBlink,   {255, 200, 0}, 500}},
    {"surprise",    {DocLedMode::kBlink,   {255, 200, 0}, 500}},
    {"shocked",     {DocLedMode::kBlink,   {255, 200, 0}, 380}},
    // dúvida: laranja
    {"confused",    {DocLedMode::kBreathe, {255, 90, 0}, 900}},
    {"uncertain",   {DocLedMode::kBreathe, {255, 90, 0}, 1100}},
    {"embarrassed", {DocLedMode::kBreathe, {255, 60, 40}, 1300}},
    {"warning",     {DocLedMode::kBlink,   {255, 120, 0}, 500}},
    // pensando: roxo, cometa indo e voltando
    {"thinking",    {DocLedMode::kScroll,  {150, 0, 255}, 1400}},
    // calmos e legais
    {"cool",        {DocLedMode::kSolid,   {0, 120, 255}, 1000}},
    {"confident",   {DocLedMode::kSolid,   {80, 40, 255}, 1000}},
    {"smug",        {DocLedMode::kSolid,   {80, 40, 255}, 1000}},
    {"relaxed",     {DocLedMode::kBreathe, {0, 255, 140}, 2400}},
    {"delicious",   {DocLedMode::kBreathe, {0, 255, 140}, 1600}},
    {"smile",       {DocLedMode::kBreathe, {0, 255, 140}, 1800}},
    {"winking",     {DocLedMode::kBreathe, {255, 0, 150}, 900}},
    {"wink",        {DocLedMode::kBreathe, {255, 0, 150}, 900}},
    // sono: laranja fraco, bem devagar
    {"sleepy",      {DocLedMode::kBreathe, {255, 70, 0}, 4000}},
    {"sleeping",    {DocLedMode::kBreathe, {20, 20, 160}, 5000}},
    {"sleep",       {DocLedMode::kBreathe, {20, 20, 160}, 5000}},
    // bagunça: arco-íris
    {"silly",       {DocLedMode::kRainbow, {255, 255, 255}, 1500}},
    {"crazy",       {DocLedMode::kRainbow, {255, 255, 255}, 1100}},
    {"derp",        {DocLedMode::kRainbow, {255, 255, 255}, 1500}},
    {"cheeky",      {DocLedMode::kRainbow, {255, 255, 255}, 2400}},
    {"playful",     {DocLedMode::kRainbow, {255, 255, 255}, 2400}},
    {"dizzy",       {DocLedMode::kRainbow, {255, 255, 255}, 700}},
};

// ============================================================
// DOC — 2. ESTILOS POR ESTADO DO ROBÔ  (edite aqui)
// ============================================================
constexpr DocLedStyle kOff             = {DocLedMode::kOff,     {0, 0, 0}, 1000};
constexpr DocLedStyle kStarting        = {DocLedMode::kScroll,  {0, 60, 255}, 1200};
constexpr DocLedStyle kWifiConfig      = {DocLedMode::kBlink,   {0, 60, 255}, 1000};
constexpr DocLedStyle kConnecting      = {DocLedMode::kSolid,   {0, 40, 255}, 1000};
constexpr DocLedStyle kListening       = {DocLedMode::kBreathe, {0, 255, 255}, 2200};  // ouvindo (sem voz)
constexpr DocLedStyle kListeningVoice  = {DocLedMode::kSolid,   {0, 255, 255}, 1000};  // ouvindo (com voz)
constexpr DocLedStyle kActivating      = {DocLedMode::kBlink,   {0, 255, 0}, 1000};
constexpr DocLedStyle kUpgrading       = {DocLedMode::kBlink,   {0, 255, 0}, 200};
constexpr DocLedStyle kAudioTest       = {DocLedMode::kBlink,   {255, 200, 0}, 600};
constexpr DocLedStyle kFatalError      = {DocLedMode::kBlink,   {255, 0, 0}, 400};

constexpr float kPi = 3.14159265f;

char LowerChar(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; }

const DocLedStyle* FindEmotion(const char* name) {
    if (name == nullptr || name[0] == '\0') return nullptr;
    char buf[24];
    size_t i = 0;
    for (; name[i] != '\0' && i < sizeof(buf) - 1; i++) buf[i] = LowerChar(name[i]);
    buf[i] = '\0';
    for (const auto& e : kEmotions) {
        if (std::strcmp(buf, e.name) == 0) return &e.style;
    }
    return nullptr;
}

uint8_t ToByte(float v) {
    if (v < 0.0f) return 0;
    if (v > 255.0f) return 255;
    return static_cast<uint8_t>(v + 0.5f);
}

DocRgb Scaled(DocRgb c, float factor, uint8_t level) {
    const float k = factor * static_cast<float>(level) / 255.0f;
    return {ToByte(c.r * k), ToByte(c.g * k), ToByte(c.b * k)};
}

// Matiz 0..1 -> cor cheia.
DocRgb Hue(float h) {
    h = h - std::floor(h);
    const float x = h * 6.0f;
    const int sector = static_cast<int>(x);
    const float f = x - static_cast<float>(sector);
    float r = 0, g = 0, b = 0;
    switch (sector % 6) {
        case 0: r = 1; g = f; b = 0; break;
        case 1: r = 1 - f; g = 1; b = 0; break;
        case 2: r = 0; g = 1; b = f; break;
        case 3: r = 0; g = 1 - f; b = 1; break;
        case 4: r = f; g = 0; b = 1; break;
        default: r = 1; g = 0; b = 1 - f; break;
    }
    return {ToByte(r * 255.0f), ToByte(g * 255.0f), ToByte(b * 255.0f)};
}

}  // namespace

DocLedStyle DocLedStyleForEmotion(const char* emotion) {
    const DocLedStyle* found = FindEmotion(emotion);
    return found != nullptr ? *found : kNeutral;
}

DocLedStyle DocLedStyleForState(DeviceState state, bool voice_detected, bool thinking,
                                const char* emotion) {
    switch (state) {
        case kDeviceStateStarting:
            return kStarting;
        case kDeviceStateWifiConfiguring:
            return kWifiConfig;
        case kDeviceStateConnecting:
            return kConnecting;
        case kDeviceStateActivating:
            return kActivating;
        case kDeviceStateUpgrading:
            return kUpgrading;
        case kDeviceStateAudioTesting:
            return kAudioTest;
        case kDeviceStateFatalError:
            return kFatalError;
        case kDeviceStateListening:
            // O servidor manda "thinking" depois que você termina de falar e antes da resposta.
            if (thinking) return DocLedStyleForEmotion("thinking");
            return voice_detected ? kListeningVoice : kListening;
        case kDeviceStateSpeaking:
        case kDeviceStateNotifying:
            // Falando: a cor vem da emoção da resposta.
            return DocLedStyleForEmotion(thinking ? "neutral" : emotion);
        case kDeviceStateIdle:
        case kDeviceStateUnknown:
        default:
            return kOff;
    }
}

void DocLedRender(const DocLedStyle& style, uint32_t elapsed_ms, int count, uint8_t level,
                  DocRgb* out) {
    if (out == nullptr || count <= 0) return;
    const uint32_t period = style.period_ms == 0 ? 1000u : style.period_ms;
    const float phase = static_cast<float>(elapsed_ms % period) / static_cast<float>(period);

    for (int i = 0; i < count; i++) out[i] = {0, 0, 0};

    switch (style.mode) {
        case DocLedMode::kOff:
            break;

        case DocLedMode::kSolid:
            for (int i = 0; i < count; i++) out[i] = Scaled(style.color, 1.0f, level);
            break;

        case DocLedMode::kBreathe: {
            // Nunca apaga por completo (12 % no mínimo), para não parecer que desligou.
            const float f = 0.12f + 0.88f * (0.5f - 0.5f * std::cos(2.0f * kPi * phase));
            for (int i = 0; i < count; i++) out[i] = Scaled(style.color, f, level);
            break;
        }

        case DocLedMode::kBlink: {
            const float f = phase < 0.5f ? 1.0f : 0.0f;
            for (int i = 0; i < count; i++) out[i] = Scaled(style.color, f, level);
            break;
        }

        case DocLedMode::kScroll: {
            // Cometa que vai de uma ponta à outra e volta.
            const float span = static_cast<float>(count > 1 ? count - 1 : 1);
            float pos = phase * 2.0f * span;
            if (pos > span) pos = 2.0f * span - pos;
            for (int i = 0; i < count; i++) {
                float d = std::fabs(static_cast<float>(i) - pos);
                float f = 1.0f - d / 1.8f;
                if (f < 0.0f) f = 0.0f;
                out[i] = Scaled(style.color, f * f, level);
            }
            break;
        }

        case DocLedMode::kRainbow:
            for (int i = 0; i < count; i++) {
                const DocRgb c = Hue(phase + static_cast<float>(i) / static_cast<float>(count));
                out[i] = Scaled(c, 1.0f, level);
            }
            break;
    }
}
