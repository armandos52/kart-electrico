#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

// ===== VARIÁVEIS EXTERNAS =====
extern volatile int16_t g_velocidade_kph;
extern volatile uint8_t g_modo_kart;
extern volatile int16_t g_vel_roda_esq;
extern volatile int16_t g_vel_roda_dir;
extern volatile int16_t g_vel_roda_di_esq;
extern volatile int16_t g_vel_roda_di_dir;
extern volatile uint8_t g_modo_controlo;
extern volatile uint16_t g_bms_soc;
extern volatile int16_t g_bms_temp;

// (O histórico de IDs e debug CAN podem ficar se ainda os usares,
//  mas já não são necessários para o funcionamento atual)
extern volatile uint32_t g_can_id_history[4];

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    void updateDisplayData();
    void atualizarBMS();
    void atualizarTextoModo();

    // NOVO – função chamada quando o botão transparente do modo é premido
    void modoTextClick();

    // ===== NOVA FUNÇÃO PARA O BOTÃO PLAY/PAUSE (SD) =====
    virtual void onPlayPauseClick();
    // =====================================================

    using Screen1ViewBase::textAreaVELOCIDADE;
    using Screen1ViewBase::textAreaMODO;
    using Screen1ViewBase::textAreaVEL_RODA_TR_ESQ;
    using Screen1ViewBase::textAreaVEL_RODA_TR_DIR;
    using Screen1ViewBase::textAreaVEL_RODA_DI_ESQ;
    using Screen1ViewBase::textAreaVEL_RODA_DI_DIR;
    using Screen1ViewBase::textAreaVELOCIDADEBuffer;
    using Screen1ViewBase::textAreaMODOBuffer;
    using Screen1ViewBase::imageBrake;
    using Screen1ViewBase::textAreaVEL_RODA_TR_ESQBuffer;
    using Screen1ViewBase::textAreaVEL_RODA_TR_DIRBuffer;
    using Screen1ViewBase::textAreaVEL_RODA_DI_ESQBuffer;
    using Screen1ViewBase::textAreaVEL_RODA_DI_DIRBuffer;
};

#endif
