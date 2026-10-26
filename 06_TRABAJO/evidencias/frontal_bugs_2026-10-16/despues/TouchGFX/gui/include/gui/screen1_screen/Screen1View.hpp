#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP
#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <gui/common/KartDashboard.hpp>
class Screen1View : public Screen1ViewBase {
public:
    Screen1View() {}
    virtual void setupScreen() override;
    void updateDisplayData(const KartUiSnapshot& snapshot);
    virtual void modoTextClick() override {Kart_RequestNextMode();}
    virtual void onPlayPauseClick() override {Kart_ToggleRecording();}
private:
    KartDashboard dashboard;
};
#endif
