#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Color.hpp>
void Screen1View::setupScreen() {
    Screen1ViewBase::setupScreen();
    // Retira tambien los controles transparentes antiguos; solo navega el dashboard.
    container.removeAll();__background.setPosition(0,0,480,272);
    __background.setColor(touchgfx::Color::getColorFromRGB(0,0,0));
    add(__background);add(dashboard);
}
void Screen1View::updateDisplayData(const KartUiSnapshot& snapshot) {dashboard.update(snapshot);}
