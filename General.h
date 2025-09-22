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

    // int origenNodo = rx_remitente - '0'; // Convertir char a int restando el código ASCII de '0'

    //10. Miscelanius#include <HTTPClient.h>
  //https://resource.heltec.cn/download/package_heltec_esp32_index.json
  // mongodb+srv://jhojodicre:l7emAppTNpcVUTsc@cluster0.wa5aztt.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0
  // The password for jhojodicre is included in the connection string for your first time setup. 
  // This password will not be available again after exiting this connect flow.
  // jhojodicre
  // password: l7emAppTNpcVUTsc
  // ip ip (186.52.249.162).
  // ramita agregada a la rama principal.