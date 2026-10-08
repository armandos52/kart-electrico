#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <touchgfx/hal/Types.hpp>
#include <cstdint>

class Model;  // Forward declaration

class ModelListener
{
public:
    ModelListener() : model(0) {}
    
    virtual ~ModelListener() {}

    void bind(Model* m)
    {
        model = m;
    }

    // Método chamado quando chegam novos dados CAN
    virtual void notifyCANDataChanged(uint8_t* data, uint32_t id) {}

protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
