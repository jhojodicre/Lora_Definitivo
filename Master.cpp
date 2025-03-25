#include "Master.h"
#include <Arduino.h>
#include <Ticker.h>

Ticker timer_master;
Master* masterInstance = nullptr;
// Constructor de la clase Master
Master::Master(bool mode_master) {
    Mode=mode_master;
    timer_master.attach(1, timer_master_ISR);
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
void Master::Nodo_REQUEST(){
    if(Nodo_Proximo==Nodo_Ultimo){
        Nodo_Proximo=Nodo_Primero-1;
    }
    if(Nodo_Proximo<=Nodo_Ultimo) {
        ++ Nodo_Proximo;
        Nodo_Consultado= Nodo_Proximo;
    }
}
void Master::Master_Nodo(){
    // Implementación del método Master_Nodo
    Nodo_REQUEST();
    Master_Mensaje();
    Secuencia();
    Next=false;
}
void Master::Master_Mensaje(){
    // Implementación del método Master_Mensaje
    mensaje=Nodo_Consultado;

}
void Master::Secuencia(){
    // Implementación del método Secuencia
    timer_master.attach(3, timer_master_ISR);
}
void Master::timer_master_ISR(){
    // Implementación del método timer_master_ISR
    if (masterInstance) {
        masterInstance->Next = true; // Acceder a la variable del objeto
    }
}	