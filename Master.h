#ifndef MASTER_H
#define MASTER_H
#include <Ticker.h>
#include <Arduino.h>

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
    bool Mode;                 // true = Modo Master, false = Modo Nodo
    int  nodeNumber;           // Número de nodo o cantidad total de nodos
    int  nodeStatus;           // Estado actual del nodo (0=No responde, 1=Normal, 2=Alerta)
    int  mensaje;              // Mensaje actual para enviar
    bool Next = false;                 // Flag para indicar que es momento de transmitir al siguiente nodo
    bool nodeResponde = false;         // Flag que indica si el nodo responde o no
    bool nodeNoResponde = false;       // Flag que indica que el nodo no respondió
    bool nodeAlerta = false;           // Flag que indica si el nodo está en alerta
    String Lora_Rxdata;     // Datos recibidos por Lora
    // ----- CONSTRUCTORES -----
    /**
     * @brief Constructor para modo Master
     * @param mode_master true=Modo Master, false=Modo Nodo
     * @param nodo_number Cantidad total de nodos en la red si es Master, número de nodo si es Nodo
     */
    Master(bool mode_master, int nodo_number);
    
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
    void Iniciar();
    
    /**
     * @brief Configura parámetros del protocolo
     */
    void Configuracion();
    
    /**
     * @brief Gestiona el ciclo principal del protocolo
     */
    void Gestion();
    
    // ----- MÉTODOS DE GESTIÓN DE SECUENCIA DE NODOS -----
    /**
     * @brief Determina el siguiente nodo a consultar
     */
    void Nodo_REQUEST();
    
    /**
     * @brief Prepara la consulta al siguiente nodo
     */
    void Master_Nodo();
    
    /**
     * @brief Prepara el mensaje para el nodo consultado
     */
    void Master_Mensaje();
    
    /**
     * @brief Maneja la secuencia de consulta a nodos
     */
    void Secuencia();
    
    // ----- MÉTODOS DE TEMPORIZADOR Y PETICIONES -----
    /**
     * @brief ISR para el temporizador de consulta periódica
     */
    static void timer_master_ISR();
    
    /**
     * @brief Procesa una petición del Master a un Nodo
     */
    void Master_Request();
    
    // ----- MÉTODOS DE GESTIÓN DE DATOS DE NODOS -----
    /**
     * @brief Actualiza el estado de un nodo
     * @param nodeNumber Número de nodo
     * @param zonaA Estado de Zona A
     * @param zonaB Estado de Zona B
     * @param fuente Estado de la Fuente
     */
    void Nodo_Status(String nodeNumber, String zonaA, String zonaB, String fuente);
    
    /**
     * @brief Actualiza la base de datos del Master con información de nodos
     */
    void Master_DB();
    
    /**
     * @brief Procesa un mensaje recibido y determina acciones
     * @param origen ID del nodo origen
     * @param mensaje Contenido del mensaje
     * @return true si el mensaje requiere acción especial
     */
    void ProcesarMensaje(String mensaje_rx_lora);
    
    /**
     * @brief Verifica si un nodo está en estado de alerta
     * @param nodoID ID del nodo a verificar
     * @return true si el nodo está en alerta
     */
    bool NodoEnAlerta(int nodoID);
    
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
    String nodo_Number="";      // Número de nodo en formato String
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
    

    String rx_remitente      = "";
    String rx_destinatario    = "";
    String rx_mensaje         = "";
    String rx_funct_mode      = "";
    String rx_funct_num       = "";
    String rx_funct_parameter1 = "";
    String rx_funct_parameter2 = "";
    String rx_funct_parameter3 = "";
    String rx_funct_parameter4 = "";
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
};

#endif // MASTER_H