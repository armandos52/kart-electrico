#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP
#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <gui/common/KartStatus.hpp>
#include <gui/common/KartConditions.hpp>
#include <touchgfx/Callback.hpp>
class Screen1View : public Screen1ViewBase {
public:
    Screen1View() : conditionsOpen(false),openConditionsCallback(this,&Screen1View::openConditions),
        closeConditionsCallback(this,&Screen1View::closeConditions) {}
    virtual void setupScreen() override;
    void updateDisplayData(const KartUiSnapshot& snapshot);
    virtual void modoTextClick() override {Kart_RequestNextMode();}
    virtual void onPlayPauseClick() override {Kart_ToggleRecording();}
private:
    // Tocar el bloque de estado abre «Condiciones de marcha»; tocar la página la cierra.
    void openConditions();
    void closeConditions();
    KartStatus status;
    KartStatus units;
    KartConditions conditions;
    KartUiSnapshot latest;
    bool conditionsOpen;
    touchgfx::Callback<Screen1View> openConditionsCallback;
    touchgfx::Callback<Screen1View> closeConditionsCallback;
};
#endif
