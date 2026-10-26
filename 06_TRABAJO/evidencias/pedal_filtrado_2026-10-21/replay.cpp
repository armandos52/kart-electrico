#define main regression_main
#include "fuentes/stm32_frontal/tests/frontend_test.cpp"
#undef main
#include <fstream>
#include <sstream>
#include <vector>
int main(){
    // Solo ADC y tiempos históricos. CAN, dirección y freno son simulados.
    std::ifstream data("reposo_replay.csv");std::string line,phase,last_phase;unsigned count=0,bad=0;
    std::getline(data,line);
    while(std::getline(data,line)){
        std::stringstream row(line);std::string timestamp,adc;
        std::getline(row,phase,',');std::getline(row,timestamp,',');std::getline(row,adc);
        if(phase!=last_phase){reset();last_phase=phase;}
        tick=(uint32_t)std::stoul(timestamp);adc_accel_value=(uint16_t)std::stoul(adc);
        feedback();pins[2]=1;Kart_ControlTick();count++;
        if(ui.accel_raw || !ui.pedal_free || ui.gear!=1 || kart_u16be(sent_command))bad++;
    }
    printf("ADC historico, resto simulado: %u muestras; %u incumplimientos reposo cero/permiso D\n",count,bad);
    std::ofstream curves("curvas.csv");curves<<"adc,dev8,dev9,dev10_estable\n";
    for(unsigned adc=124;adc<=1961;adc++){
        reset();pedal_sample((uint16_t)adc);
        curves<<adc<<','<<(adc>=1861?4095:(adc-124)*4095/1737)<<','
            <<(adc<=320?0:adc>=1861?4095:(adc-320)*4095/1541)<<','<<ui.accel_raw<<'\n';
    }
    std::ofstream step("escalon_sintetico.csv");step<<"t_ms,adc_fisico,mando_dev10,pedal_libre\n";
    reset();
    for(unsigned i=0;i<14;i++){
        pedal_sample(i<3 || i>=11?200:i<7?800:1400);
        step<<i*5<<','<<ui.accel_adc<<','<<ui.accel_raw<<','<<unsigned(ui.pedal_free)<<'\n';
    }
    return count==2061 && bad==0?0:1;
}
