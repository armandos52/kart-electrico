#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP
#include "kart_frontend.h"
class Model;
class ModelListener {
public:
    ModelListener():model(0) {}
    virtual ~ModelListener() {}
    void bind(Model* m) {model=m;}
    virtual void onKartSnapshot(const KartUiSnapshot&) {}
protected:
    Model* model;
};
#endif
