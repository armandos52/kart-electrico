#include <gui/screen1_screen/Screen1View.hpp>
Screen1Presenter::Screen1Presenter(Screen1View& v):view(v) {}
void Screen1Presenter::onKartSnapshot(const KartUiSnapshot& snapshot) {view.updateDisplayData(snapshot);}
