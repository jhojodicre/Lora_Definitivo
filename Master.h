#ifndef MASTER_H
#define MASTER_H

class Master {
public:
    bool Mode;
    int nodeNumber;
    int nodeStatus;
    int mensaje;

    Master(bool mode_master);
    
    void Iniciar();
    void Configuracion();
    void Gestion();
    static void timer_master_ISR();
    void Nodo_REQUEST();
    void Master_Nodo();
    void Master_Mensaje();

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