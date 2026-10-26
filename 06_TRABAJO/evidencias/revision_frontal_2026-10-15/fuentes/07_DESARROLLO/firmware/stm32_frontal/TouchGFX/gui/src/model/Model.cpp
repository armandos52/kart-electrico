#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "kart_frontend.h"
Model::Model() : modelListener(0) {}
void Model::tick() {
    // Solo TouchGFX publica a la vista; copia coherente del control.
    static uint32_t last=0;
    KartUiSnapshot snapshot;Kart_CopySnapshot(&snapshot);
    if(snapshot.tick-last<100)return;
    last=snapshot.tick;
    if(modelListener)modelListener->onKartSnapshot(snapshot);
}
