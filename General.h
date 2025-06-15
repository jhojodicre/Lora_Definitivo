#ifndef GENERAL_H
#define GENERAL_H

class General {
public:
    General(bool ready);
    bool Iniciar();
    void Configuracion();
    void Gestion();
    void Led_Monitor(int repeticiones);
    void Led_1(int status);
    void Led_2(int status);
    void Led_3(int status);
    void Welcome();
    void Led_Status(int status);
    void Dale(int repeticiones);
    int _LED_Azul=35;
    int LED_1=37;
    int LED_2=46;
    int LED_3=45;
private:
    bool firstScan;
};

#endif // GENERAL_H