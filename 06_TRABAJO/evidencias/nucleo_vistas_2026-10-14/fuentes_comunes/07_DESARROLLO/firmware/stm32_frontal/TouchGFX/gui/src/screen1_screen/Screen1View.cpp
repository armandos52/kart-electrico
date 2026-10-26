#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Unicode.hpp>
void Screen1View::setupScreen() {
    Screen1ViewBase::setupScreen();
    scalableImage3.setVisible(false);bmsSocText.setVisible(false);bmsTempText.setVisible(false);
    textAreaMODO.setPosition(12,102,80,65);textAreaVELOCIDADE.setPosition(115,102,220,65);
    modoControloText.setPosition(347,102,132,65);
    flexButtonModoText.setPosition(347,102,132,65);flexButtonModoText.setBoxWithBorderPosition(0,0,132,65);
    btnPlayPause.setPosition(347,48,132,50);btnPlayPause.setBoxWithBorderPosition(0,0,132,50);
    imagePlay.setPosition(395,50,45,45);imagePause.setPosition(395,50,45,45);
    imageBrake.setPosition(275,50,60,45);
    units.setLabel("KM/H");units.setPosition(120,75,72,19);add(units);
    add(status);
    KartUiSnapshot initial={};initial.mode_received=initial.mode_applied=255;
    updateDisplayData(initial);
}
void Screen1View::updateDisplayData(const KartUiSnapshot& s) {
    using namespace touchgfx;
    if(s.feedback_fresh && (!s.status_seen || (s.status_fresh && (s.backend_flags&KART_STATUS_SPEED_VALID))))
        Unicode::snprintf(textAreaVELOCIDADEBuffer,TEXTAREAVELOCIDADE_SIZE,"%d",s.speed_kph);
    else Unicode::snprintf(textAreaVELOCIDADEBuffer,TEXTAREAVELOCIDADE_SIZE,"--");
    textAreaVELOCIDADE.invalidate();
    Unicode::snprintf(textAreaMODOBuffer,TEXTAREAMODO_SIZE,"%c",s.gear==1?'D':s.gear==2?'R':'N');textAreaMODO.invalidate();
    Unicode::fromUTF8(reinterpret_cast<const uint8_t*>(Kart_ModeName(s.mode_requested)),modoControloTextBuffer,MODOCONTROLOTEXT_SIZE);modoControloText.invalidate();
    imageBrake.setVisible(s.brake);imageBrake.invalidate();
    imagePause.setVisible(s.sd_state==KART_SD_RECORDING);imagePause.invalidate();
    imagePlay.setVisible(s.sd_state!=KART_SD_RECORDING);imagePlay.invalidate();
    status.update(s);
}
