#ifndef DOC_AI_S3CAM_CONFIG_H
#define DOC_AI_S3CAM_CONFIG_H

#include <driver/gpio.h>

// DOC INÍCIO — configuração da placa ESP32-S3-CAM N16R8
// Primeiro marco: rede XiaoZhi + áudio I2S.
// A tela ST7735 e a câmera OV3660 ainda não são inicializadas por esta classe.

#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// Microfone MS3625 e amplificador MAX98357A em I2S simplex.
#define AUDIO_I2S_METHOD_SIMPLEX
#define AUDIO_I2S_MIC_GPIO_SCK  GPIO_NUM_1
#define AUDIO_I2S_MIC_GPIO_WS   GPIO_NUM_2
#define AUDIO_I2S_MIC_GPIO_DIN  GPIO_NUM_3
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_41
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_42
#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_47

#define BOOT_BUTTON_GPIO        GPIO_NUM_0
// DOC INÍCIO — sensor de toque TTP223 (saída ativa em nível alto)
#define TOUCH_BUTTON_GPIO       GPIO_NUM_48
// DOC FIM

// DOC INÍCIO — barra de LED 8x WS2812 (fio DIN).
// O DIN fica no GPIO45, que nenhum outro item do DOC usa. NÃO use o GPIO18: ele é
// a linha D5 da câmera (CAMERA_PIN_D5) e, com a câmera aberta, o sensor escreve
// nesse fio e a barra acende branca sem controle.
// GPIO45 é um pino de "strapping" (lido só no instante em que a placa liga); com
// o resistor de 330 Ω em série costuma funcionar normalmente. Se a placa deixar de
// ligar com o fio conectado, tire o fio na hora de ligar, ou use o GPIO46.
#define DOC_LED_BAR_GPIO        GPIO_NUM_45
#define DOC_LED_BAR_COUNT       8
// Brilho geral 0..255 (48 = cerca de 19 %). Aumente com cuidado: 8 LEDs brancos
// no máximo puxam cerca de 480 mA dos 5 V.
#define DOC_LED_BAR_LEVEL       48
// DOC FIM

// DOC INÍCIO — sensor de distância VL53L0X no barramento I2C compartilhado
// com o PCA9685 (SDA GPIO19 / SCL GPIO20, 100 kHz, endereço 0x29).
#define DOC_I2C_SDA_GPIO        GPIO_NUM_19
#define DOC_I2C_SCL_GPIO        GPIO_NUM_20
#define DOC_I2C_FREQUENCY_HZ    100000
#define VL53L0X_I2C_ADDRESS     0x29
// DOC FIM
#define ASR_BUTTON_GPIO         GPIO_NUM_NC
#define VOLUME_UP_BUTTON_GPIO   GPIO_NUM_NC
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_NC
#define BUILTIN_LED_GPIO        GPIO_NUM_NC

// Tela ST7735 1,77" GMT177-10: pinagem registrada, inicialização separada.
#define DISPLAY_SPI_CLK_PIN    GPIO_NUM_14
#define DISPLAY_SPI_MOSI_PIN   GPIO_NUM_21
#define DISPLAY_CS_PIN         GPIO_NUM_38
#define DISPLAY_DC_PIN         GPIO_NUM_39
#define DISPLAY_RST_PIN        GPIO_NUM_40
#define DISPLAY_BACKLIGHT_PIN  GPIO_NUM_NC
#define DISPLAY_WIDTH          128
#define DISPLAY_HEIGHT         160
#define DISPLAY_OFFSET_X       0
#define DISPLAY_OFFSET_Y       0
#define DISPLAY_MIRROR_X       false
#define DISPLAY_MIRROR_Y       false
#define DISPLAY_SWAP_XY        false
#define DISPLAY_INVERT_COLOR   false
#define DISPLAY_SPI_MODE       0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false

// Câmera OV3660: pinagem registrada, inicialização ainda não ativada.
#define CAMERA_PIN_XCLK  GPIO_NUM_15
#define CAMERA_PIN_SIOD  GPIO_NUM_4
#define CAMERA_PIN_SIOC  GPIO_NUM_5
#define CAMERA_PIN_D0    GPIO_NUM_11
#define CAMERA_PIN_D1    GPIO_NUM_9
#define CAMERA_PIN_D2    GPIO_NUM_8
#define CAMERA_PIN_D3    GPIO_NUM_10
#define CAMERA_PIN_D4    GPIO_NUM_12
#define CAMERA_PIN_D5    GPIO_NUM_18
#define CAMERA_PIN_D6    GPIO_NUM_17
#define CAMERA_PIN_D7    GPIO_NUM_16
#define CAMERA_PIN_VSYNC GPIO_NUM_6
#define CAMERA_PIN_HREF  GPIO_NUM_7
#define CAMERA_PIN_PCLK  GPIO_NUM_13
#define CAMERA_PIN_PWDN  GPIO_NUM_NC
#define CAMERA_PIN_RESET GPIO_NUM_NC
#define XCLK_FREQ_HZ     20000000

// DOC INÍCIO — I²C confirmado para o módulo PCA9685
// Fiação testada: SDA -> GPIO19; SCL -> GPIO20; endereço PCA9685 -> 0x40.
// ALERTA: GPIO19/GPIO20 também são USB D-/D+ no ESP32-S3. Com o PCA9685
// conectado nesses pinos, não usar a interface USB nativa simultaneamente.
// Para gravação/debug, usar o conversor USB-UART externo, conforme o projeto.
#define PCA9685_SDA_GPIO GPIO_NUM_19
#define PCA9685_SCL_GPIO GPIO_NUM_20
#define PCA9685_I2C_ADDRESS 0x40
#define PCA9685_I2C_FREQUENCY_HZ 100000
// DOC FIM

// DOC INÍCIO — canais e calibração de teste das rodas
// CH0: SG90 perna esquerda; CH3: SG90 perna direita; faixa inicial 1250–1750 us.
// CH1: MG90S roda esquerda; CH2: MG90S roda direita; neutro testado em 1705 us.
// CW=1665 us e CCW=1800 us foram confirmados nos testes físicos desta montagem.
// Os valores são específicos destes motores; recalibrar se forem substituídos.
// Ao validar mudanças, manter as rodas suspensas e parar diante de esforço ou ruído anormal.
#define DOC_LEG_SERVO_MIN_US 1250
#define DOC_LEG_SERVO_MAX_US 1750
#define DOC_WHEEL_LEFT_CW_US 1665
#define DOC_WHEEL_LEFT_STOP_US 1705
#define DOC_WHEEL_LEFT_CCW_US 1800
#define DOC_WHEEL_RIGHT_CW_US 1665
#define DOC_WHEEL_RIGHT_STOP_US 1705
#define DOC_WHEEL_RIGHT_CCW_US 1800
// DOC FIM

// DOC INÍCIO — movimento (valores INICIAIS; o painel web ajusta e salva em NVS)
// Os servos das rodas estão montados em espelho: "frente" exige sentidos opostos.
// Se o robô andar para trás, inverta o sentido pelo painel (aba Calibração).
#define DOC_WHEEL_LEFT_FORWARD_IS_CW   true
#define DOC_WHEEL_RIGHT_FORWARD_IS_CW  false
#define DOC_MOTION_SPEED_PCT           100   // 10..100 (% do deslocamento calibrado)
// Modo Pernas: inclinação alternada (SG90 CH0/CH3) + leve impulso nas rodas.
#define DOC_LEG_CENTER_LEFT_US         1500
#define DOC_LEG_CENTER_RIGHT_US        1500
#define DOC_LEG_LEFT_INVERTED          false
#define DOC_LEG_RIGHT_INVERTED         true  // servos espelhados: inclinação igual = pulsos opostos
#define DOC_LEG_TILT_AMPLITUDE_US      100   // começa pequeno (~9 graus); aumente aos poucos
#define DOC_STEP_HOLD_MS               450   // tempo em cada lado
#define DOC_STEP_PUSH_US               70    // deslocamento da roda em relação ao neutro
#define DOC_STEP_PUSH_MS               200   // duração do impulso
#define DOC_OBSTACLE_STOP_MM           0     // 0 = desligado; VL53L0X para as rodas se < valor
// Postura: EM PÉ (Modo Pernas) usa o "centro" das pernas; SENTADO (Modo Rodas) usa os pulsos abaixo.
// Ao trocar de modo, as pernas giram devagar até a outra postura (as rodas esperam).
// Valores iniciais CONSERVADORES (+/-250 us do centro): aumente pela aba Calibração, de 50 em 50,
// até os pés dobrarem e as rodas tocarem o chão (cerca de 90 graus).
#define DOC_LEG_SIT_LEFT_US            1750
#define DOC_LEG_SIT_RIGHT_US           1250
#define DOC_POSTURE_MS                 1500  // duração da transição em pé <-> sentado
// Limite absoluto aceito para as pernas nas posturas e na marcha (SG90: curso nominal 500..2500).
#define DOC_LEG_ABS_MIN_US             600
#define DOC_LEG_ABS_MAX_US             2400
// DOC FIM

#endif // DOC_AI_S3CAM_CONFIG_H
