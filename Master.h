#ifndef MASTER_H
#define MASTER_H
#include <Ticker.h>
#include <Arduino.h>
class Master {
public:
    bool Mode;
    int nodeNumber;
    int nodeStatus;
    int mensaje;
    bool Next;          // Flag para transmitir al siguiente nodo.
    bool nodeResponde; // Flag que indica si el nodo responde o no.
    
    
    Master(bool mode_master, int nodo_number);
    Master(String nodoNumero, String ZonaA_status, String ZonaB_status, String Fuente_status);
    void Iniciar();
    void Configuracion();
    void Gestion();
    void Nodo_REQUEST();
    void Master_Nodo();
    void Master_Mensaje();
    void Secuencia();
    static void timer_master_ISR(); // Declaración como estática
    int  Nodo_Proximo;
    void Master_Request();
    void Nodo_Status(String nodeNumber, String zonaA, String zonaB, String fuente);
    void Master_DB();

    // Vaiables para la base de datos del nodo
    String nodo_Number="";
    String Zone_A="";
    String Zone_B="";
    String Zone_C="";
    String Node_DB="";
    String Fuente="";
private:
    bool firstScan;
    bool LED_Azul;
    int  Nodo_Primero=1;
    int  Nodo_Ultimo=3;
    int  Nodo_Consultado;
    int  Nodo_Anterior;
    int  Nodo_Actual;
    int  Nodo_Siguiente;
};

#endif // MASTER_H