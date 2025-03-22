#include "Master.h"
#include <Arduino.h>
#include <Ticker.h>

Ticker timer_master;

// Constructor de la clase Master
Master::Master(bool mode_master) {
    Mode=mode_master;
    timer_master.attach(1, Master::timer_master_ISR);

}

void Master::Iniciar() {
    // Implementación del método Iniciar
}

void Master::Configuracion() {
    // Implementación del método Configuracion
}

void Master::Gestion() {
    // Implementación del método Gestion
}
  //-FP.6 PROXIMO NODO.
void Master::Nodo_REQUEST(){
    if(Nodo_Proximo==Nodo_Ultimo){
        Nodo_Proximo=Nodo_Primero-1;
    }
    if(Nodo_Proximo<=Nodo_Ultimo) {
        ++ Nodo_Proximo;
        Nodo_Consultado= Nodo_Proximo;
    }
}
void Master::timer_master_ISR(){
    // Implementación del método timer_master_ISR


}
void Master::Master_Nodo(){
    // Implementación del método Master_Nodo
    Nodo_REQUEST();
    Master_Mensaje();
}

void Master::Master_Mensaje(){
    // Implementación del método Master_Mensaje
    mensaje=Nodo_Consultado;

}