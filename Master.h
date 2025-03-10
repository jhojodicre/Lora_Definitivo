#ifndef MASTER_H
#define MASTER_H

class Master {
public:
    Master(bool mode_master);
    
    void Iniciar();
    void Configuracion();
    void Gestion();
    static void timer_master_ISR();
    void Nodo_REQUEST();

private:
    bool firstScan;
    bool LED_Azul;
    bool masterMode;
    int  Nodo_Primero;
    int  Nodo_Ultimo;
    int  Nodo_Proximo;
    int  Nodo_Consultado;
    int  Nodo_Anterior;
    int  Nodo_Actual;
    int  Nodo_Siguiente;
};

#endif // MASTER_H