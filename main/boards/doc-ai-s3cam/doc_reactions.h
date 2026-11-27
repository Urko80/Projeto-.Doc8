/*
 * DOC — Reações do robô: LED + olhos conforme estado e emoção.
 * Arquivo: main/boards/doc-ai-s3cam/doc_reactions.h
 *
 * O firmware já chama Led::OnStateChanged() a cada mudança de estado
 * (ouvindo, falando...) e quando a voz é detectada. Esta classe aproveita
 * esse gancho para, ao mesmo tempo:
 *   - ajustar a barra de LED (cor/animação por estado e por emoção);
 *   - ajustar os olhos: ouvindo = olha para cima; falando = boca mexe.
 * As emoções chegam por OnEmotion(), chamado pelo display da placa.
 */
#pragma once

#include <mutex>
#include <string>

#include "device_state.h"
#include "doc_led_bar.h"
#include "led/led.h"

class DocReactions : public Led {
public:
    explicit DocReactions(DocLedBar* bar);

    // Led: chamado pelo Application a cada mudança de estado / detecção de voz.
    void OnStateChanged() override;

    // Chamado pelo display quando o servidor envia uma emoção ("happy", "angry"...).
    void OnEmotion(const char* emotion);

private:
    void Refresh();  // com o mutex_ já travado

    DocLedBar* bar_;
    std::mutex mutex_;
    DeviceState last_state_ = kDeviceStateUnknown;
    bool thinking_ = false;
    std::string emotion_ = "neutral";
};
