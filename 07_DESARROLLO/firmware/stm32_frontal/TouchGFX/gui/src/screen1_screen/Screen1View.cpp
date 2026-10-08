#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Unicode.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <cstring>
#include <cstdio>
#include "cmsis_os.h"

// ===== Variáveis globais (partilhadas com main.c) =====
extern volatile int16_t g_velocidade_kph;
extern volatile uint8_t g_modo_kart;
extern volatile int16_t g_vel_roda_esq;
extern volatile int16_t g_vel_roda_dir;
extern volatile int16_t g_vel_roda_di_esq;
extern volatile int16_t g_vel_roda_di_dir;
extern volatile uint8_t g_modo_controlo;
extern volatile uint8_t g_brake_pressed;
extern volatile uint16_t g_bms_soc;
extern volatile int16_t g_bms_temp;
extern volatile uint16_t g_steering_raw;

// ===== NOVA VARIÁVEL PARA O CONTROLO DO SD =====
extern volatile uint8_t g_sd_record_enable;

// ===== PROTÓTIPO DA FUNÇÃO Close_SD (vinda do main.c) =====
extern "C" void Close_SD(void);

// ===== Instância global da View =====
extern "C" {
    Screen1View* screen1View = NULL;
}

// ===== Protótipo (para simulador) =====
extern "C" void atualizarTela1(void);

// ===== Ponte C/C++ para main.c =====
extern "C" void atualizarTela1(void)
{
    if (screen1View == nullptr) return;
    touchgfx::HAL::getInstance()->lockFrameBuffer();
    screen1View->updateDisplayData();
    touchgfx::HAL::getInstance()->unlockFrameBuffer();
}

Screen1View::Screen1View()
{
    screen1View = this;
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    for(int i = 0; i < 10; i++) {
        modoControloTextBuffer[i] = 0;
    }
    modoControloTextBuffer[0] = 'D';
    modoControloTextBuffer[1] = 'I';
    modoControloTextBuffer[2] = 'F';
    modoControloTextBuffer[3] = '\0';
    modoControloText.invalidate();

    memset(textAreaVELOCIDADEBuffer, 0, TEXTAREAVELOCIDADE_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVELOCIDADEBuffer, TEXTAREAVELOCIDADE_SIZE, "0");
    textAreaVELOCIDADE.setWildcard(textAreaVELOCIDADEBuffer);
    textAreaVELOCIDADE.invalidate();

    memset(textAreaMODOBuffer, 0, TEXTAREAMODO_SIZE * sizeof(Unicode::UnicodeChar));
    textAreaMODOBuffer[0] = 'N';
    textAreaMODOBuffer[1] = '\0';
    textAreaMODO.setWildcard(textAreaMODOBuffer);
    textAreaMODO.invalidate();

    memset(textAreaVEL_RODA_TR_ESQBuffer, 0, TEXTAREAVEL_RODA_TR_ESQ_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVEL_RODA_TR_ESQBuffer, TEXTAREAVEL_RODA_TR_ESQ_SIZE, "0");
    textAreaVEL_RODA_TR_ESQ.setWildcard(textAreaVEL_RODA_TR_ESQBuffer);
    textAreaVEL_RODA_TR_ESQ.invalidate();

    memset(textAreaVEL_RODA_TR_DIRBuffer, 0, TEXTAREAVEL_RODA_TR_DIR_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVEL_RODA_TR_DIRBuffer, TEXTAREAVEL_RODA_TR_DIR_SIZE, "0");
    textAreaVEL_RODA_TR_DIR.setWildcard(textAreaVEL_RODA_TR_DIRBuffer);
    textAreaVEL_RODA_TR_DIR.invalidate();

    memset(textAreaVEL_RODA_DI_ESQBuffer, 0, TEXTAREAVEL_RODA_DI_ESQ_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVEL_RODA_DI_ESQBuffer, TEXTAREAVEL_RODA_DI_ESQ_SIZE, "0");
    textAreaVEL_RODA_DI_ESQ.setWildcard(textAreaVEL_RODA_DI_ESQBuffer);
    textAreaVEL_RODA_DI_ESQ.invalidate();

    memset(textAreaVEL_RODA_DI_DIRBuffer, 0, TEXTAREAVEL_RODA_DI_DIR_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVEL_RODA_DI_DIRBuffer, TEXTAREAVEL_RODA_DI_DIR_SIZE, "0");
    textAreaVEL_RODA_DI_DIR.setWildcard(textAreaVEL_RODA_DI_DIRBuffer);
    textAreaVEL_RODA_DI_DIR.invalidate();

    memset(bmsSocTextBuffer, 0, BMSSOCTEXT_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(bmsSocTextBuffer, BMSSOCTEXT_SIZE, "--");
    bmsSocText.setWildcard(bmsSocTextBuffer);
    bmsSocText.invalidate();

    memset(bmsTempTextBuffer, 0, BMSTEMPTEXT_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(bmsTempTextBuffer, BMSTEMPTEXT_SIZE, "--");
    bmsTempText.setWildcard(bmsTempTextBuffer);
    bmsTempText.invalidate();

    // ===== NOVO: Inicialização das imagens SD =====
    imagePlay.setVisible(true);
    imagePause.setVisible(false);
    imagePlay.invalidate();
    imagePause.invalidate();

    atualizarTextoModo();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent()
{
    updateDisplayData();
}

void Screen1View::onPlayPauseClick()
{
    g_sd_record_enable = !g_sd_record_enable;
    imagePlay.setVisible(!g_sd_record_enable);
    imagePause.setVisible(g_sd_record_enable);
    imagePlay.invalidate();
    imagePause.invalidate();

    touchgfx::HAL::getInstance()->lockFrameBuffer();
    touchgfx::HAL::getInstance()->flushFrameBuffer();
    touchgfx::HAL::getInstance()->unlockFrameBuffer();

    if (g_sd_record_enable == 0) {
        Close_SD();
    }
}

void Screen1View::modoTextClick()
{
    if (g_velocidade_kph != 0 || g_modo_kart != 2) {
        return;
    }

    g_modo_controlo++;
    if (g_modo_controlo > 2) {
        g_modo_controlo = 0;
    }
    atualizarTextoModo();
}

void Screen1View::atualizarTextoModo()
{
    const char* texto = "DIF";

    switch(g_modo_controlo) {
        case 0: texto = "DIF"; break;
        case 1: texto = "ESC"; break;
        case 2: texto = "BLQ"; break;
    }

    for(int i = 0; i < 10; i++) {
        modoControloTextBuffer[i] = 0;
    }

    int i = 0;
    while(texto[i] != '\0' && i < 9) {
        modoControloTextBuffer[i] = texto[i];
        i++;
    }
    modoControloTextBuffer[i] = '\0';

    modoControloText.invalidate();
}

void Screen1View::updateDisplayData()
{
    uint32_t now = osKernelGetTickCount();
    static uint32_t last_update = 0;

    if (now - last_update < 100) {
        return;
    }
    last_update = now;

    // Velocidade principal (km/h)
    memset(textAreaVELOCIDADEBuffer, 0, TEXTAREAVELOCIDADE_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVELOCIDADEBuffer, TEXTAREAVELOCIDADE_SIZE, "%d", g_velocidade_kph);
    textAreaVELOCIDADE.setWildcard(textAreaVELOCIDADEBuffer);
    textAreaVELOCIDADE.invalidate();

    // Modo (D/R/N)
    memset(textAreaMODOBuffer, 0, TEXTAREAMODO_SIZE * sizeof(Unicode::UnicodeChar));
    if (g_modo_kart == 0) textAreaMODOBuffer[0] = 'D';
    else if (g_modo_kart == 1) textAreaMODOBuffer[0] = 'R';
    else textAreaMODOBuffer[0] = 'N';
    textAreaMODOBuffer[1] = '\0';
    textAreaMODO.setWildcard(textAreaMODOBuffer);
    textAreaMODO.invalidate();

    // Roda traseira esquerda
    memset(textAreaVEL_RODA_TR_ESQBuffer, 0, TEXTAREAVEL_RODA_TR_ESQ_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVEL_RODA_TR_ESQBuffer, TEXTAREAVEL_RODA_TR_ESQ_SIZE, "%d", g_vel_roda_esq);
    textAreaVEL_RODA_TR_ESQ.setWildcard(textAreaVEL_RODA_TR_ESQBuffer);
    textAreaVEL_RODA_TR_ESQ.invalidate();

    // Roda traseira direita
    memset(textAreaVEL_RODA_TR_DIRBuffer, 0, TEXTAREAVEL_RODA_TR_DIR_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVEL_RODA_TR_DIRBuffer, TEXTAREAVEL_RODA_TR_DIR_SIZE, "%d", g_vel_roda_dir);
    textAreaVEL_RODA_TR_DIR.setWildcard(textAreaVEL_RODA_TR_DIRBuffer);
    textAreaVEL_RODA_TR_DIR.invalidate();

    // Roda dianteira esquerda (VELOCIDADE)
    memset(textAreaVEL_RODA_DI_ESQBuffer, 0, TEXTAREAVEL_RODA_DI_ESQ_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVEL_RODA_DI_ESQBuffer, TEXTAREAVEL_RODA_DI_ESQ_SIZE, "%d", g_vel_roda_di_esq);
    textAreaVEL_RODA_DI_ESQ.setWildcard(textAreaVEL_RODA_DI_ESQBuffer);
    textAreaVEL_RODA_DI_ESQ.invalidate();

    // Roda dianteira direita
    memset(textAreaVEL_RODA_DI_DIRBuffer, 0, TEXTAREAVEL_RODA_DI_DIR_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(textAreaVEL_RODA_DI_DIRBuffer, TEXTAREAVEL_RODA_DI_DIR_SIZE, "%d", g_vel_roda_di_dir);
    textAreaVEL_RODA_DI_DIR.setWildcard(textAreaVEL_RODA_DI_DIRBuffer);
    textAreaVEL_RODA_DI_DIR.invalidate();

    // Travão
    imageBrake.setVisible(g_brake_pressed == 1);
    imageBrake.invalidate();

    // BMS
    atualizarBMS();
}

void Screen1View::atualizarBMS()
{
    memset(bmsSocTextBuffer, 0, BMSSOCTEXT_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(bmsSocTextBuffer, BMSSOCTEXT_SIZE, "%d%%", g_bms_soc);
    bmsSocText.setWildcard(bmsSocTextBuffer);
    bmsSocText.invalidate();

    memset(bmsTempTextBuffer, 0, BMSTEMPTEXT_SIZE * sizeof(Unicode::UnicodeChar));
    Unicode::snprintf(bmsTempTextBuffer, BMSTEMPTEXT_SIZE, "%d", g_bms_temp);
    bmsTempText.setWildcard(bmsTempTextBuffer);
    bmsTempText.invalidate();
}
