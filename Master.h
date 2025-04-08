#ifndef MASTER_H
#define MASTER_H
#include <Ticker.h>
class Master {
public:
    bool Mode;
    int nodeNumber;
    int nodeStatus;
    int mensaje;
    bool Next;          // Flag para transmitir al siguiente nodo.
    

    Master(bool mode_master);
    
    void Iniciar();
    void Configuracion();
    void Gestion();
    void Nodo_REQUEST();
    void Master_Nodo();
    void Master_Mensaje();
    void Secuencia();
    static void timer_master_ISR(); // Declaración como estática

private:
    bool firstScan;
    bool LED_Azul;
    int  Nodo_Primero=1;
    int  Nodo_Ultimo=3;
    int  Nodo_Proximo;
    int  Nodo_Consultado;
    int  Nodo_Anterior;
    int  Nodo_Actual;
    int  Nodo_Siguiente;
};

#endif // MASTER_H