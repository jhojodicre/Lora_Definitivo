#ifndef LORA_H
#define LORA_H
#include <Arduino.h>
#include <Ticker.h>
#include <ArduinoJson.h>
class Lora {
public:
    Lora(char nodeNumber);
    void   Setup();
    void   Lora_Setup(class Functions* correr);
    void   Lora_TX();
    void   Lora_RX();
    static void rx();
    void   Lora_IO_Zones();
    void   Lora_Nodo_Frame();
    void   Lora_Nodo_Decodificar();
    void   Lora_Node_Print(String z_executed);
    void   Lora_Master_Frame();
    void   Lora_Master_Decodificar();
    void   Lora_Dummy_Simulate();
    static  void   Lora_timerNodo_Answer();
    void   Lora_Master_DB();
    void   SerializeObjectToJson();
    void   Lora_WebMessage(String mensaje);
    void   Lora_IO_Zones_Force();
    void   Lora_IO_Zone_A_ACK();
    void   Lora_IO_Zone_B_ACK();
    static void   Lora_time_ZoneA_reach();
    static void   Lora_time_ZoneB_reach();
    void   Lora_Timer_Enable(int answerTime);
    void   Lora_Event_Disable();
    void   Lora_Node_Protocol();
    void   Lora_Node_Print_RX();

    bool    F_Responder=false;
    bool    F_Recibido=false;
    bool    F_Nodo_Excecute=false;
    bool    F_Master_Excecute=false;
    bool    F_Master_Update=false;
    bool    F_function_Special=false;
    bool    F_Event_Enable=false;
    bool    timer_ZA_En=true;
    bool    timer_ZB_En=true;
    bool    Timer_Nodo_Answer_F=false;      // flag que indica que el timer de responder esta activo.
    bool    F_No_Responder=false;
    // byte    Master_Address=0xFF; // Direccion del maestro.
        String  Master_Address="X"; // Direccion del maestro.
        char    ascii_representation[9];
        String  rxdata;
        String  txdata;
        String  mensaje;
        byte    nodo_local;
        char    nodo_status;        // Estado del nodo en este byte esta el estado de las entradas si esta en error o falla
        char    local_Address='1';  // Direccion del nodo local.
        char    nodo_consultado;  // Direccion del nodo consultado.
        char    nodo_Number;
        String  nodo_a_Consultar=" ";   // Direccion del nodo a consultar.
        String  nodo_DB=" ";
    //Variables para la recepcion de mensaje.
        char    rx_remitente;           // Nodo que envia el mensaje.
        char    rx_destinatario;        // Nodo que recibe el mensaje.
        String  rx_mensaje;             // Mensaje recibido.
        String  rx_funct_mode;          // Tipo de funcion a ejecutar.
        String  rx_funct_num;           // Numero de funcion a ejecutar.
        String  rx_funct_parameter1;    // Parametro 1 de la Funcion.
        String  rx_funct_parameter2;    // Parametro 2 de la Funcion.
        String  rx_funct_parameter3;    // Parametro 3 de la Funcion.
        String  rx_funct_parameter4;    // Parametro 4 de la Funcion.

        String  rx_mensaje_DB;          // Mensaje recibido.
        String  rx_ST_ZA_DB;            // Estado de la Zona A.
        String  rx_ST_ZB_DB;            // Estado de la Zona B.
        String  rx_ST_FT_DB;            // Estado de la Fuente.

        String  tx_remitente;           // Nodo que envia el mensaje.
        String  tx_destinatario;        // Nodo que recibe el mensaje.
        String  tx_mensaje;             // Mensaje recibido.
        String  tx_funct_mode;          // Tipo de funcion a ejecutar.
        String  tx_funct_num;           // Numero de funcion a ejecutar.
        String  tx_funct_parameter1;    // Parametro 1 de la Funcion.
        String  tx_funct_parameter2;    // Parametro 2 de la Funcion.

        String    jsonString;

        String    tx_nodo_lora_1;
        String    tx_nodo_lora_2;
        String    tx_nodo_lora_3;
        String    tx_nodo_lora_4;
        String    tx_nodo_lora_5;
        String    tx_nodo_lora_6;
        String    tx_nodo_lora_7;
        String    tx_nodo_lora_8;


        String    rx_master_lora_1;
        String    rx_master_lora_2;
        String    rx_master_lora_3;
        String    rx_master_lora_4;
        String    rx_master_lora_5;
        String    rx_master_lora_6;
        String    rx_master_lora_7;
        String    rx_master_lora_8;

    // Forzados
        bool    Zone_A_Force;
        bool    Zone_B_Force;
        bool    Fuente_in_Force;

        bool    Zone_A_Forzar;
        bool    Zone_B_Forzar;
        bool    Fuente_in_Forzar;

    //mensaje para enviar

        String  Node_Num_str;
        String  Node_Status_str;
        String  Zone_A_str;
        String  Zone_B_str;
        String  Rele_1_out_str;
        String  Rele_2_out_str;
        String  Fuente_in_str;
        String  Tipo_de_Mensaje;
        
        String  Zone_A_F_str;
        String  Zone_B_F_str;

        String  Zone_A_ACK_str;
        String  Zone_B_ACK_str;

    // Instancias de Clases:
        class Functions* correrRef;
private:
    // Entradas Fisicas
        int     Zona_A_in=39;
        int     Zona_B_in=38;
        int     Zona_C_in=40; // Pulsador C. PRG.

        int     PB_ZA_in=40;
        int     PB_ZB_in=41;
        int     PB_ZC_in=42;

        int     Rele_1_out=5;            // 16;
        int     Rele_2_out=6;            // 17;

        int     Fuente_in=43; // 3.3V

        
    // Estadon del Nodo
        bool    Node_Status;

    // Entradas Auxiliares
        bool    Zone_A;
        bool    Zone_B;
        bool    Zone_C;
        // Zonas Reonocidas
        bool    Zone_A_ACK;
        bool    Zone_B_ACK;
        bool    Zone_AB_ACK;

        bool    Fuente_in_ST;

        bool    Rele_1_out_ST;
        bool    Rele_2_out_ST;

    // Estados de Zonas
        bool    Zone_A_ST;
        bool    Zone_B_ST;

    // Zonas en Error
        bool    Zone_A_ERR;
        bool    Zone_B_ERR;

        int     Zonas;
        int     Zonas_Fallan;


    // long        mensaje = 0;
        uint64_t    last_tx = 0;
        uint64_t    tx_time;
        uint64_t    minimum_pause;
    // Json Vaibles
        String    nodeJS    = "nodoId";
        String    commJS    = "comu";
        String    zoneAJS   = "zoneA";
        String    zoneBJS   = "zoneB";
        String    output1JS = "output1";
        String    output2JS = "output2";
        String    fuenteJS  = "fuente";
        StaticJsonDocument<300> doc;

};

#endif // NODO_H