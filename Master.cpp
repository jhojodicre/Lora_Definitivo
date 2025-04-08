#include "Master.h"
#include <Arduino.h>
#include <Ticker.h>

Ticker timer_master;
Master* masterInstance = nullptr; // Puntero global al objeto Master
// Constructor de la clase Master
Master::Master(bool mode_master) {
    Mode=mode_master;
    masterInstance = this; // Asignar la instancia actual al puntero global
       
}
void Master::Iniciar() {
    // Implementación del método Iniciar
    timer_master.attach(5, timer_master_ISR); // Llama a la función de temporizador cada 5 segundos
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
}
void Master::timer_master_ISR() {
    // Implementación del método estático timer_master_ISR
    // Nota: No puedes acceder a variables no estáticas directamente
    // Usa variables estáticas o globales si es necesario
    if (masterInstance) {
        masterInstance->Next = true; // Acceder a la variable de instancia a través del puntero global
    }

}
