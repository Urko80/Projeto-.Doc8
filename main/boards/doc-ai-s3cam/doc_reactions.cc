/*
 * DOC — Reações do robô: LED + olhos conforme estado e emoção.
 * Arquivo: main/boards/doc-ai-s3cam/doc_reactions.cc
 */
#include "doc_reactions.h"

#include <cstring>

#include "application.h"
#include "display/custom_eyes.h"

namespace {

std::string Lower(const char* s) {
    std::string out;
    for (; s != nullptr && *s != '\0'; ++s) {
        out.push_back((*s >= 'A' && *s <= 'Z') ? static_cast<char>(*s - 'A' + 'a') : *s);
    }
    return out;
}

}  // namespace

DocReactions::DocReactions(DocLedBar* bar) : bar_(bar) {}

void DocReactions::OnStateChanged() {
    std::lock_guard<std::mutex> lock(mutex_);

    auto& app = Application::GetInstance();
    const DeviceState state = app.GetDeviceState();

    if (state != last_state_) {
        // Entrou em outro estado: "pensando" só vale dentro do mesmo estado em que chegou.
        last_state_ = state;
        thinking_ = false;
    } else if (state == kDeviceStateListening && app.IsVoiceDetected()) {
        // Você voltou a falar: não está mais pensando.
        thinking_ = false;
    }
    Refresh();
}

void DocReactions::OnEmotion(const char* emotion) {
    if (emotion == nullptr || emotion[0] == '\0') {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);

    const std::string name = Lower(emotion);
    thinking_ = (name == "thinking");
    if (!thinking_) {
        emotion_ = name;
    }
    Refresh();
}

void DocReactions::Refresh() {
    auto& app = Application::GetInstance();
    const DeviceState state = app.GetDeviceState();
    const bool voice = app.IsVoiceDetected();

    // ---- LED ----
    if (bar_ != nullptr) {
        bar_->SetStyle(DocLedStyleForState(state, voice, thinking_, emotion_.c_str()));
    }

    // ---- Olhos ----
    // A expressão "thinking" já controla o olhar; não atrapalhar.
    if (thinking_) {
        return;
    }
    switch (state) {
        case kDeviceStateListening:
            // Ouvindo: olha para cima, atento. Quando ouve voz, volta ao olhar livre.
            CustomEyes_SetTalking(false);
            CustomEyes_SetGaze(!voice, 0.0f, -4.0f);
            break;
        case kDeviceStateSpeaking:
        case kDeviceStateNotifying:
            // Falando: a boca abre e fecha; a expressão continua a da emoção.
            CustomEyes_SetGaze(false, 0.0f, 0.0f);
            CustomEyes_SetTalking(true);
            break;
        default:
            CustomEyes_SetGaze(false, 0.0f, 0.0f);
            CustomEyes_SetTalking(false);
            break;
    }
}
