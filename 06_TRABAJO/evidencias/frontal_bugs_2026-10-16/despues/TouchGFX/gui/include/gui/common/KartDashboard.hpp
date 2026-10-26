#ifndef KART_DASHBOARD_WIDGET_HPP
#define KART_DASHBOARD_WIDGET_HPP
#include <touchgfx/widgets/Widget.hpp>
#include <touchgfx/events/ClickEvent.hpp>
#include <touchgfx/events/DragEvent.hpp>
#include "kart_dashboard.h"
class KartDashboard : public touchgfx::Widget {
public:
    KartDashboard();
    void update(const KartUiSnapshot& snapshot);
    virtual void draw(const touchgfx::Rect& area) const override;
    virtual touchgfx::Rect getSolidRect() const override {return touchgfx::Rect(0,48,480,224);}
    virtual void handleClickEvent(const touchgfx::ClickEvent& event) override;
    virtual void handleDragEvent(const touchgfx::DragEvent&) override;
private:
    struct PaintContext {const KartDashboard* widget;touchgfx::Rect clip;};
    static void paintRect(void* context,int x,int y,int w,int h,uint32_t rgb);
    static void paintText(void* context,int x,int y,int w,uint8_t font,uint32_t rgb,const char* text);
    KartUiSnapshot snapshot;
    KartDashboardState state;
};
#endif
