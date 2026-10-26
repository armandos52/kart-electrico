#ifndef SCREEN1PRESENTER_HPP
#define SCREEN1PRESENTER_HPP
#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
class Screen1View;
class Screen1Presenter : public touchgfx::Presenter, public ModelListener {
public:
    explicit Screen1Presenter(Screen1View& v);
    virtual void activate() {}
    virtual void deactivate() {}
    virtual void onKartSnapshot(const KartUiSnapshot& snapshot) override;
private:
    Screen1View& view;
};
#endif
