#ifndef LORA_H
#define LORA_H
#include <Arduino.h>
class Lora {
public:
    Lora(uint nodeNumber);
    void   Setup();
    void   Lora_Setup();
    void   Lora_TX();
    void   Lora_RX();
    static void rx();

private:
    int     Zona_A_in=38;
    int     Zona_B_in=39;

    int     PB_ZA_in=40;
    int     PB_ZB_in=41;
    int     PB_ZAB_in=42;
    
    int     Rele_1_out=16;
    int     Rele_2_out=17;


    bool    Zone_A;
    bool    Zone_B;
    bool    Zone_C;
    String  rxdata;
    long        mensaje = 0;
    uint64_t    last_tx = 0;
    uint64_t    tx_time;
    uint64_t    minimum_pause;

};

#endif // NODO_H