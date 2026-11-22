/*
 * DOC — Barra de LED 8x WS2812 (driver + animação).
 * Arquivo: main/boards/doc-ai-s3cam/doc_led_bar.cc
 */
#include "doc_led_bar.h"

#include <esp_log.h>

#include <cstring>

#define TAG "DocLedBar"

namespace {
constexpr int64_t kFramePeriodUs = 40 * 1000;  // 25 quadros por segundo
}

DocLedBar::DocLedBar(gpio_num_t gpio, int count, uint8_t level)
    : gpio_(gpio), count_(count > 0 ? count : 1), level_(level) {
    frame_.assign(count_, DocRgb{0, 0, 0});
    last_.assign(count_, DocRgb{0, 0, 0});
    style_start_us_ = esp_timer_get_time();

    if (!CreateStrip()) {
        ESP_LOGE(TAG, "Falha ao criar a barra de LED no GPIO %d", static_cast<int>(gpio_));
    }

    esp_timer_create_args_t args = {};
    args.callback = [](void* arg) { static_cast<DocLedBar*>(arg)->Tick(); };
    args.arg = this;
    args.dispatch_method = ESP_TIMER_TASK;
    args.name = "doc_led_bar";
    args.skip_unhandled_events = true;
    if (esp_timer_create(&args, &timer_) == ESP_OK) {
        esp_timer_start_periodic(timer_, kFramePeriodUs);
    } else {
        ESP_LOGE(TAG, "Falha ao criar o temporizador da barra de LED");
        timer_ = nullptr;
    }
}

DocLedBar::~DocLedBar() {
    if (timer_ != nullptr) {
        esp_timer_stop(timer_);
        esp_timer_delete(timer_);
        timer_ = nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    DestroyStrip();
}

bool DocLedBar::CreateStrip() {
    led_strip_config_t strip_config = {};
    strip_config.strip_gpio_num = gpio_;
    strip_config.max_leds = count_;
    strip_config.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB;
    strip_config.led_model = LED_MODEL_WS2812;

    led_strip_rmt_config_t rmt_config = {};
    rmt_config.resolution_hz = 10 * 1000 * 1000;  // 10 MHz

    if (led_strip_new_rmt_device(&strip_config, &rmt_config, &strip_) != ESP_OK) {
        strip_ = nullptr;
        return false;
    }
    led_strip_clear(strip_);
    force_refresh_ = true;
    return true;
}

void DocLedBar::DestroyStrip() {
    if (strip_ == nullptr) {
        return;
    }
    led_strip_clear(strip_);
    led_strip_del(strip_);
    strip_ = nullptr;
    // Devolve o pino ao estado inicial para outro periférico (câmera) assumir.
    gpio_reset_pin(gpio_);
}

void DocLedBar::SetStyle(const DocLedStyle& style) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (style == style_) {
        return;
    }
    style_ = style;
    style_start_us_ = esp_timer_get_time();
}

void DocLedBar::Suspend() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (suspended_) {
        return;
    }
    suspended_ = true;
    DestroyStrip();
    ESP_LOGI(TAG, "Barra de LED suspensa (pino liberado)");
}

void DocLedBar::Resume() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!suspended_) {
        return;
    }
    suspended_ = false;
    if (!CreateStrip()) {
        ESP_LOGE(TAG, "Falha ao recriar a barra de LED");
        return;
    }
    ESP_LOGI(TAG, "Barra de LED retomada");
}

void DocLedBar::Tick() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (suspended_ || strip_ == nullptr) {
        return;
    }

    const uint32_t elapsed_ms =
        static_cast<uint32_t>((esp_timer_get_time() - style_start_us_) / 1000);
    DocLedRender(style_, elapsed_ms, count_, level_, frame_.data());

    const size_t bytes = frame_.size() * sizeof(DocRgb);
    if (!force_refresh_ && std::memcmp(frame_.data(), last_.data(), bytes) == 0) {
        return;  // nada mudou: não ocupa o RMT à toa
    }

    for (int i = 0; i < count_; i++) {
        led_strip_set_pixel(strip_, i, frame_[i].r, frame_[i].g, frame_[i].b);
    }
    led_strip_refresh(strip_);
    last_ = frame_;
    force_refresh_ = false;
}
