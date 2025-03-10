#ifndef GENERAL_H
#define GENERAL_H

class General {
public:
    General(bool ready);
    bool Iniciar();
    void Configuracion();
    void Gestion();
    void Led_Monitor(int repeticiones);
    void Welcome();
private:
    bool firstScan;
    int _LED_Azul=35;
};

#endif // GENERAL_H