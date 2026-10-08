#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <cstdint>

// ===== DECLARAR VARIÁVEIS EXTERNAS DO main.c =====
// Usar "extern" dentro do bloco extern "C" para apenas DECLARAR, não DEFINIR
extern "C" {
    extern volatile uint8_t g_can_data_buffer[8];   // ✅ extern = apenas declaração
    extern volatile uint8_t g_can_new_data_flag;    // ✅ extern = apenas declaração
    extern volatile uint32_t g_can_received_id;     // ✅ extern = apenas declaração
}
// Contador para debug
static int tick_count = 0;
static int data_count = 0;

Model::Model() : modelListener(0)
{
}

void Model::tick()
{
	  tick_count++;

	    // A cada 100 ticks, mostra no LCD que o Model está vivo
	    // (se tiver acesso à BSP, senão ignore esta parte)

	    if (g_can_new_data_flag == 1)
	    {
	        data_count++;
	        g_can_new_data_flag = 0;

	        if (modelListener != 0)
	        {
	            modelListener->notifyCANDataChanged(
	                const_cast<uint8_t*>(g_can_data_buffer),
	                g_can_received_id
	            );
	        }
	    }
}
