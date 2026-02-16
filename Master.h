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
    int  timeout_NoResponse = 4000; // Tiempo de espera para considerar que un nodo no responde (ms)
    String message_type=""; // Tipo de mensaje recibido

    // ----- CONSTRUCTORES -----
    /**
     * @brief Constructor para modo Master
     * @param mode_master true=Modo Master, false=Modo Nodo
     * @param nodo_number Cantidad total de nodos en la red si es Master, número de nodo si es Nodo
     */
    Master(bool mode_master, int nodo_number, char NodeAddress);
    
    /**
     * @brief Constructor para almacenar estado de un nodo
     * @param nodoNumero Número de nodo
     * @param ZonaA_status Estado de la Zona A
     * @param ZonaB_status Estado de la Zona B
     * @param Fuente_status Estado de la Fuente
     */
    Master(String nodoNumero, String ZonaA_status, String ZonaB_status, String Fuente_status);
    
    // ----- MÉTODOS DE INICIALIZACIÓN Y CONFIGURACIÓN -----
    /**
     * @brief Inicializa los temporizadores y configuraciones del protocolo
     */
    void Iniciar(class Lora* Node, class Functions* Correr);

    void Preguntar();
    void Node_Protocol();
    void Calibration_Protocol();
    
    /**
     * @brief Configura parámetros del protocolo
     */
    void Configuracion();
    void Master_Protocol();
    void Master_Counter();

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
    /**
     * @brief Gestiona el ciclo principal del protocolo
     */
    void Gestion();
    void SerializeObjectToJson();
    /**
     * @brief Inicializa el protocolo de calibración Master
     */
    void Master_Calibration_Init();
    void Master_Calibration_End();

    // ----- MÉTODOS DE GESTIÓN DE SECUENCIA DE NODOS -----
    /**
     * @brief Determina el siguiente nodo a consultar
     */
    void Nodo_REQUEST();
    
    /**
     * @brief Prepara la consulta al siguiente nodo
     */
    void Master_Nodo();  /*** @brief Prepara el mensaje para el nodo consultado*/
    void MasterMessage();
    void Master_Status_Address();

    void Master_ExecuteFromServer(String mensajeServer);
    
    void Secuencia();/*** @brief Maneja la secuencia de consulta a nodos*/
    
    // ----- MÉTODOS DE TEMPORIZADOR Y PETICIONES -----
 
    static void timer_master_ISR();   /* *@brief ISR para el temporizador de consulta periódica   */
    

    void Master_Request();    /*** @brief Procesa una petición del Master a un Nodo*/
    
    // ----- MÉTODOS DE GESTIÓN DE DATOS DE NODOS -----
    /**
     * @brief Actualiza el estado de un nodo
     * @param nodeNumber Número de nodo
     * @param zonaA Estado de Zona A
     * @param zonaB Estado de Zona B
     * @param fuente Estado de la Fuente
     */
    void Nodo_Status(String nodeNumber, String zonaA, String zonaB, String fuente);/*** @brief Actualiza la base de datos del Master con información de nodos*/
    void Master_DB();
    void Node_Decodificar();
    void Node_Counter();
    void Node_Print_RX();
    /**
     * @brief Imprime el estado actual de todos los nodos
     */
    bool F_Node_Excecute=false;
    bool F_Responder=false;
    void Node_Message();
    
    int nodeCounter=0;
    /**
     * @brief Procesa un mensaje recibido y determina acciones
     * @param origen ID del nodo origen
     * @param mensaje Contenido del mensaje
     * @return true si el mensaje requiere acción especial
     */
    void MasterDecodificar(String mensaje_rx_lora);
    
    /**
     * @brief Verifica si un nodo está en estado de alerta
     * @param nodoID ID del nodo a verificar
     * @return true si el nodo está en alerta
     */
    bool NodoEnAlerta(int nodoID);
    void NodeStatusUpdate();
    /**
     * @brief Genera mensaje para petición especial a un nodo
     * @param nodoID ID del nodo destinatario
     * @param comando Comando a enviar
     * @return Mensaje codificado para envío
     */
    String GenerarPeticionEspecial(int nodoID, String comando);

    void Master_DecodificarMensaje(String mensaje);
    
    /**
     * @brief Método de debug para mostrar el estado de las banderas
     */
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
private:
    // ----- ESTRUCTURAS PARA GESTIÓN DE NODOS -----
    struct EstadoNodo {
        bool responde;          // Si el nodo está respondiendo
        int  ultimoEstado;      // Último estado conocido (0=No responde, 1=Normal, 2=Alerta)
        long ultimaRespuesta;   // Timestamp de la última respuesta recibida
        int  intentos;          // Contador de intentos de comunicación
    };
    
    // Arreglo para almacenar el estado de cada nodo (índice = nodoID)
    EstadoNodo estadosNodos[10]; // Soporte hasta 10 nodos


    //🌐🌐** Variables para envair al servidor🌐🌐 */
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