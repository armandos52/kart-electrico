#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <cstdio>
#include <cstring>

Screen1Presenter::Screen1Presenter(Screen1View& v)
    : view(v)
{
}

void Screen1Presenter::activate()
{
}

void Screen1Presenter::deactivate()
{
}

void Screen1Presenter::notifyCANDataChanged(uint8_t* data, uint32_t id)
{
    char buffer[100];

    // Formatar apenas com caracteres seguros
    snprintf(buffer, sizeof(buffer), "ID:%03X D:%02X%02X%02X%02X",
              id, data[0], data[1], data[2], data[3]);

    view.updateDisplayData();
}

