#ifndef KART_CONDITIONS_HPP
#define KART_CONDITIONS_HPP
#include <touchgfx/widgets/Widget.hpp>
#include <touchgfx/Callback.hpp>
#include "kart_display.h"
// Página «Condiciones de marcha»: cubre el área útil bajo la franja superior (filas 48-271).
// Solo presenta el snapshot del control; no lee entradas ni envía nada. Tocarla la cierra.
class KartConditions : public touchgfx::Widget {
public:
    KartConditions();
    void setCloseCallback(touchgfx::GenericCallback<>& callback) {closeCallback=&callback;setTouchable(true);}
    void update(const KartUiSnapshot& snapshot);
    virtual void draw(const touchgfx::Rect& area) const override;
    virtual void handleClickEvent(const touchgfx::ClickEvent& event) override;
    virtual touchgfx::Rect getSolidRect() const override {return touchgfx::Rect(0,0,getWidth(),getHeight());}
private:
    char lines[KART_CONDITION_ROWS][41];
    uint8_t tones[KART_CONDITION_ROWS];
    touchgfx::GenericCallback<>* closeCallback;
};
#endif
