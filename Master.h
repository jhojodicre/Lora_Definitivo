#ifndef MASTER_H
#define MASTER_H
#include <Ticker.h>
#include <Arduino.h>
#include <ArduinoJson.h>

// Forward declaration para evitar dependencia circular
class Lora;

/**
 * @brief Clase Master: Implementa las reglas del protocolo de comunicación Lora
 * 
 * Esta clase maneja:
 * 1. La secuencia de consulta a los nodos
 * 2. El estado de respuesta de cada nodo
 * 3. El procesamiento de alertas desde los nodos
 * 4. Las peticiones especiales del servidor a nodos específicos
 */
class Master {
public:
    // ----- TIPOS DE MENSAJE DEL PROTOCOLO -----
    static constexpr char MSG_INFO = 'i';
    static constexpr char MSG_EMERGENCY = 'E';
    static constexpr char MSG_MASTER_SPECIAL = 'M';
    static constexpr char MSG_NODE_REPLY = 'P';
    static constexpr char MSG_ACK = 'O';
    static constexpr char MSG_ALERT = 'A';
    static constexpr char MSG_FUNCTION_SPECIAL = 'F';
    static constexpr char MSG_RESET = 'R';
    static constexpr char MSG_POLL = '.';

    // ----- FLAGS Y ESTADOS DEL PROTOCOLO -----
    bool MasterMode=false;                          // true = Modo Master, false = Modo Nodo
    bool NodeMode=false;                            // true = Modo Nodo, false = Modo Master
    int  nodeNumber;                    // Número de nodo o cantidad total de nodos
    int  nodeStatus;           // Estado actual del nodo (0=No responde, 1=Normal, 2=Alerta)
    bool Next = false;                 // Flag para indicar que es momento de transmitir al siguiente nodo
    bool NextSurvey = false;           // Flag para indicar que es momento de transmitir al siguiente nodo en la encuesta
    bool nodeResponde = false;         // Flag que indica si el nodo responde o no
    bool nodeNoResponde = false;       // Flag que indica que el nodo no respondió
    bool nodeAlerta = false;           // Flag que indica si el nodo está en alerta
    bool F_Calibration_EN = false;      // Flag que indica si la calibración está habilitada
    bool F_Calibration_Complete = false; // Flag que indica si la calibración ha sido completada
    String Lora_Rxdata;     // Datos recibidos por Lora
    bool F_Calibration=false;
    int  timeout_NoResponse = 3000; // Tiempo de espera para considerar que un nodo no responde (ms)
    int  timeout_master = 4000; // Tiempo entre consultas a nodos (ms)
    String message_type=String(MSG_INFO); // Tipo de mensaje recibido. "i"=información, "E"=Emergencia, "M"=Mensaje especial del Master al Nodo.
    // Se enviara un mensaje al iniciar el nodo, o cuando el master lo solicite, despues de un reset o cuando el nodo detecte un evento en las zonas. El mensaje se procesara para actualizar la base de datos del Master y se enviara al servidor/DB.

    Master(bool mode_master, int nodo_number, char NodeAddress);
    

    void Iniciar(class Lora* Node, class Functions* Correr);
    
    void ResetNodeAlertState();
    void ScheduleNodeAlertSync(uint16_t delayMs = NODE_ALERT_SYNC_DELAY_MS);
    void HandleNodeNoResponse(); 

    void Preguntar();
    void Node_Protocol();
    void Calibration_Protocol();
    
    
    static void timer_master_ISR();   /* *@brief ISR para el temporizador de consulta periódica   */
    void MasterDecodificar(String mensaje_rx_lora);
    void Master_Request();    /*** @brief Procesa una petición del Master a un Nodo*/
    void Master_Print_RX(); /*** @brief Imprime el mensaje recibido por el Master*/
    void Nodo_Status(String nodeNumber, String zonaA, String zonaB, String fuente);/*** @brief Actualiza la base de datos del Master con información de nodos*/
    void Master_DB();
    void Nodo_REQUEST();
    void Master_Protocol();
    void Master_Counter();
    void Master_Nodo();  /*** @brief Prepara el mensaje para el nodo consultado*/
    void MasterMessage();
    void Master_Status_Address();
    void SerializeObjectToJson();
    void Master_Calibration_Init();
    void Master_Calibration_End();
    void Master_ExecuteFromServer(String mensajeServer);
    bool EncolarMensajeServidor(const String& mensajeLora);
    bool ObtenerSiguienteMensajeServidor(String& mensajeLora);
    void ProcesarColaServidor();
    bool EnviarTramaCentral(const String& mensajeTx, char nodoEsperado, bool esServidor);
    void LiberarCanalTx(const String& motivo);
    void RevisarTimeoutTx();
    int  msg_enviado=0;
    int   MasterCounter=0;
    String counterStr="" ;

    // Pasos para Pasar una instancia de Clase
    // Declarar un puntero a la clase
    // Asignar el puntero en el método Iniciar
    // Usar el puntero para llamar métodos o acceder a variables
    // Ejemplo:
    class Functions* correrRef; // Puntero a la clase Functions para ejecutar funciones
    class Lora* nodeRef; // Puntero a la clase Lora para interacción con radio

    static constexpr uint16_t NODE_ALERT_SYNC_DELAY_MS = 2000;
    static constexpr uint8_t MAX_NODE_RETRIES = 2;

    
    


    void Node_Decodificar();
    void Node_Counter();
    void Node_Print_RX();
    void Node_Alerta();
    void Node_Message();
    void Node_Recibir();
    bool NodoEnAlerta(int nodoID);
    void NodeStatusUpdate();
    void Node_Flag_Print();

    bool F_Node_Excecute=false;
    bool F_Responder=true; // se activara para que el nodo envie un mensjae al iniciar el el protocolo.
    bool F_NodeAlertaActiva=false;
    bool F_NodeAlertaPendienteTx=false;
    bool F_NodeRxSincronizado=false;
    bool F_NodeAlertSyncScheduled=false;
    bool F_ForzarTxEmergencia=false;
    int  nodeCounter=0;
    



    String GenerarPeticionEspecial(int nodoID, String comando);

    void DebugEstadoBanderas();
    // ----- VARIABLES PARA LA BASE DE DATOS DE NODOS -----
    String Zone_A="";           // Estado de la Zona A (0=Normal, 1=Alerta)
    String Zone_B="";           // Estado de la Zona B (0=Normal, 1=Alerta)
    String Zone_C="";           // Estado adicional (si se requiere)
    String Node_DB="";          // Concatenación de estados para transmisión
    String Fuente="";           // Estado de la fuente de alimentación

    // ----- VARIABLES INTERNAS DE CONTROL -----
    bool firstScan;             // Flag para primera ejecución
    bool LED_Azul;              // Control de LED indicador
    int  Nodo_Primero=1;        // ID del primer nodo de la red
    int  Nodo_Proximo;          // Número del próximo nodo a consultar
    int  Nodo_Ultimo=3;         // ID del último nodo de la red
    int  Nodo_Consultado;       // ID del nodo actualmente consultado

    int  Nodo_Anterior;         // ID del nodo previamente consultado
    int  Nodo_Actual;           // ID del nodo actual en proceso
    int  Nodo_Siguiente;        // ID del siguiente nodo a consultar
    bool F_NodeAcknoledged=true; // Flag para indicar que se ha recibido un ACK del maestro después de una alerta

    String rx_remitente        = " ";
    String rx_destinatario     = " ";
    String rx_mensaje          = " ";
    String rx_funct_mode       = " ";
    String rx_funct_num        = " ";
    String rx_funct_parameter1 = " ";
    String rx_funct_parameter2 = " ";
    String rx_funct_parameter3 = " ";
    String rx_funct_parameter4 = " ";
 
    String tx_remitente        = " ";
    String tx_destinatario     = " ";
    String tx_mensaje          = " ";
    String tx_funct_mode       = " ";
    String tx_funct_num        = " ";
    String tx_funct_parameter1 = " ";
    String tx_funct_parameter2 = " ";
    String tx_funct_parameter3 = " ";
    String tx_funct_parameter4 = " ";

    String tx_node_lora_1 = " ";
    String tx_node_lora_2 = " ";
    String tx_node_lora_3 = " ";
    String tx_node_lora_4 = " ";
    String tx_node_lora_5 = " ";
    String tx_node_lora_6 = " ";
    String tx_node_lora_7 = " ";
    String tx_node_lora_8 = " ";

    bool    F_No_Responder=false;
    bool    F_Node_Atiende=false;
    bool    F_MasterMode=false;
    bool    F_NodeMode=false;
    bool    F_MasterCalibration=false;
    bool    F_ServerUpdate=false;
    bool    F_NodeStatusUpdate=false;
    bool    F_ServerQueuePending=false;
    bool    F_Server_Master=false;


    String jsonString;



    // byte    Master_Address=0xFF; // Direccion del maestro.
        String  Master_Address="X"; // Direccion del maestro.
        char    NodeAddress='1';      // Direccion del nodo local.
        char    ascii_representation[9];
        String  rxdata;
        String  txdata;
        String  mensaje;
        byte    nodo_local;
        char    nodo_status;            // Estado del nodo en este byte esta el estado de las entradas si esta en error o falla
        char    nodo_consultado;        // Direccion del nodo consultado.
        String  nodo_Number;
        String  nodo_a_Consultar=" ";   // Direccion del nodo a consultar.
        String  nodo_DB=" ";
        int     Num_Nodos=1;            // Numero de nodos en el sistema.
        String  Node_to_Calibrate=" ";  // Nodo que se esta calibrando.
        String  Device_King = "0";      // Tipo de dispositivo: N=Nodo normal, M=Master especial (si aplica)
        String  Device_Number = "0";    // Numero de dispositivo para identificar diferentes tipos de nodos.
        String  message_From_Server = " "; // Mensaje recibido del servidor para ser procesado por el Master.
private:
    static const int SERVER_QUEUE_SIZE = 10;
    static const int SERVER_BURST_MAX = 2;
    String serverMessageQueue[SERVER_QUEUE_SIZE];
    int serverQueueHead = 0;
    int serverQueueTail = 0;
    int serverQueueCount = 0;

    bool txEnCurso = false;
    bool txEsServidor = false;
    char txNodoEsperado = ' ';
    unsigned long txStartMs = 0;
    int serverBurstCount = 0;
    bool retryNoResponsePending = false;
    int nodoRetryPendiente = 0;
    String Nodo_Esperado = " ";

    // ----- ESTRUCTURAS PARA GESTIÓN DE NODOS -----
    struct EstadoNodo {
        bool responde;          // Si el nodo está respondiendo
        int  ultimoEstado;      // Último estado conocido (0=No responde, 1=Normal, 2=Alerta)
        long ultimaRespuesta;   // Timestamp de la última respuesta recibida
        int  intentos;          // Contador de intentos de comunicación
    };
    
    // Arreglo para almacenar el estado de cada nodo (índice = nodoID)
    EstadoNodo estadosNodos[10]; // Soporte hasta 10 nodos


    //🌐🌐** Variables para enviar al servidor🌐🌐 */
    StaticJsonDocument<300> doc;
    String    Node_Status_str   = " ";                              // Comunicacion ok
    String    Node_Num_str      = " "; // Numero de Nodo consultado
    String    rx_master_lora_1  = " "; // Direccion del nodo que responde
    String    rx_master_lora_2  = " "; // Direccion del maestro
    String    rx_master_lora_3  = " "; // Estado de la zona A
    String    rx_master_lora_4  = " "; // Estado de the zona B
    String    rx_master_lora_5  = " "; // Estado de the salida 1
    String    rx_master_lora_6  = " "; // Estado de the salida 2
    String    rx_master_lora_7  = " "; // Estado de the fuente
    String    rx_master_lora_8  = " "; // Tipo de mensaje, si es de emergencia

    String    nodeJS    = "nodoId";
    String    commJS    = "comu";
    String    zoneAJS   = "zoneA";
    String    zoneBJS   = "zoneB";
    String    output1JS = "output1";
    String    output2JS = "output2";
    String    fuenteJS  = "fuente";


};

#endif // MASTER_H