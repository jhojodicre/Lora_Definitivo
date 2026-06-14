// WebServer.h
#ifndef NODEWEBSERVER_H
#define NODEWEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>  // ✅ Solo librería nativa ESP32
#include <ArduinoJson.h>
#include <HTTPClient.h>


class LoRaWebServer {
private:
    WebServer* server;
    uint16_t port;
    bool isRunning;

    HTTPClient http;
    const int timeoutHTTP = 5000;  // Timeout de 5 segundos

    // WiFi Replace the next variables with your SSID/Password combination
    const char* ssid_master        =  "ANTEL_AE52"; 
    const char* password_master    =  "NCsj7gjX";   
    // const char* ssid_master        =  "Telecentro-3a30"; 
    // const char* password_master    =  "GHU54P2DYG9Q";   
    const char* ssid_nodo        =  "secure";
    const char* password_nodo    =  "12345678";


    int         conection_try=0;
    
    // IP Estática para Master
    IPAddress ip_master        = IPAddress(192, 168, 1, 100);
    IPAddress gateway_master   = IPAddress(192, 168, 1, 1);
    IPAddress subnet_master    = IPAddress(255, 255, 255, 0);
    IPAddress dns1_master      = IPAddress(8, 8, 8, 8);
    IPAddress dns2_master      = IPAddress(8, 8, 4, 4);
    
    // IP Estática para Nodo
    IPAddress ip_nodo         = IPAddress(192, 168, 1, 101);
    IPAddress gateway_nodo    = IPAddress(192, 168, 1, 1);
    IPAddress subnet_nodo     = IPAddress(255, 255, 255, 0);
    IPAddress dns1_nodo       = IPAddress(8, 8, 8, 8);
    IPAddress dns2_nodo       = IPAddress(8, 8, 4, 4);

    // Referencias a objetos externos
    class Lora* nodeRef;
    class Master* masterRef;
    class Functions* functionsRef;

public:
    // Propiedades.
    String  lastMessage     = "";
    String  nodeStatus      = "idle";       // Estado actual del nodo
    int     messageCount    = 0;            // Contador de mensajes recibidos
    StaticJsonDocument<200> doc;
    bool enviadoExterno;
    bool actualizado;
    // Server configuration
        // API Replace the next variable with your API endpoint
        const char* apiEndpoint  = "http://192.168.1.100:3000/api/nodes";
        const char* apiHeartBeat = "http://192.168.1.100:3000/api/master/heartbeat";
        const char* serverName   = "http://192.168.1.100:3000/api/nodes"; // URL de tu API de Interfaz WEB
        int         httpResponseCode;
        String respuesta;

    
    int totalNodos = 0;          // Contador de nodos registrados
    unsigned long timeoutNodo = 30000; // Timeout de nodo (30 segundos)
    // Array para almacenar datos de los nodos (máximo 5 nodos)
        struct NodeData {
        String nodoId;
        String comu;
        String zoneA;
        String zoneB;
        String output1;
        String output2;
        String fuente;
        unsigned long lastUpdate;  // Timestamp de última actualización
        bool isActive;            // Si el nodo está activo
        };
    NodeData nodos[5];           // Array de nodos
    // Extraer datos del nodo
        String nodoId   = "0";
        String comu     = "0";
        String zoneA    = "0";
        String zoneB    = "0";
        String output1  = "0";
        String output2  = "0";
        String fuente   = "0";
    // Constructor
    LoRaWebServer(uint16_t serverPort = 80);
    
    // Métodos públicos
    void begin(class Lora* node, class Functions* functions, class Master* master);
    void handle();
    void stop();


    //Configuration
    void configurarServidor();
    void configurarWiFi(const char* ssid, const char* password);

    // Error json
    void enviarErrorJSON(String mensaje);
    // Envio Json Exito
    void enviarExitoJSON(String mensaje);
    
    // Configuración de rutas
    void configurarRutasServidor();

    // Procesar Mensaje
    bool procesarMensaje(String nodeId, String mensaje);   // POST /api/send

    // Handlers para rutas
    void handleRoot();
    void handleAPI();
    void handleNodeControl();
    void handleSetAddress();
    void handleGetStatus();
    void manejarMensajeRecibido();   // POST /api/send
    void manejarPruebaSistema();     // GET /api/test
    void manejarEstadoNodo();       // GET /api/status
    void manejarPreflightCORS();     // OPTIONS /api/send
    void manejarHolaMundo();         // GET /hola-mundo
    void handleNotFound();
    void configurarHeadersCORS();    // Configurar headers CORS
    void manejarPingTest();          // POST /api/test
    void manejarDatoNodoIndividual(); // POST /api/node
    void manejarDatosNodos();        // GET /api/nodes
    void manejarMensajeRecibidoJSON(); // POST /api/send con JSON
    void manejarPruebaServidorExterno(); // GET /api/test-external
    void inicializarNodos();
    bool actualizarNodo(String nodoId, String comu, String zoneA, String zoneB, 
                        String output1, String output2, String fuente);
    void manejarForzarZonas();       // POST /api/force-zones
    bool enviarDatosAlServidorExterno(String JsonString);
    // Persistencia NVS para dirección del nodo
    bool cargarDireccionDesdeNVS();
    bool guardarDireccionEnNVS(char address);
    
    // ✅ NUEVOS MÉTODOS PARA SISTEMA DE ESTADO
    void handleGetRadioStatus();           // GET /api/status/radio
    void handleGetSystemStatus();          // GET /api/status/system  
    void handleGetNodeStatus();            // GET /api/status/node
    void handleGetMasterStatus();          // GET /api/status/master
    void handleGetCommunicationStatus();   // GET /api/status/communication
    void handleGetAllStatus();             // GET /api/status/all
    void handleStatusQuery();              // GET /api/status?type=radio|system|node|master|all

};

#endif