#ifndef KART_STATUS_HPP
#define KART_STATUS_HPP
#include <touchgfx/widgets/Widget.hpp>
#include "kart_display.h"
class KartStatus : public touchgfx::Widget {
public:
    KartStatus();
    void update(const KartUiSnapshot& snapshot);
    void setLabel(const char* text);
    virtual void draw(const touchgfx::Rect& area) const override;
    virtual touchgfx::Rect getSolidRect() const override {return touchgfx::Rect(0,0,getWidth(),getHeight());}
private:
    char lines[5][41];
};
#endif
