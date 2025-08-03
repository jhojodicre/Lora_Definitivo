// WebServer.h
#ifndef NODEWEBSERVER_H
#define NODEWEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>  // ✅ Solo librería nativa ESP32
#include <ArduinoJson.h>

class LoRaWebServer {
private:
    WebServer* server;
    uint16_t port;
    bool isRunning;
    
    // Referencias a objetos externos
    class Lora* nodeRef;
    class Master* masterRef;
    class Functions* functionsRef;
    
public:
    // Propiedades.
    String lastMessage = "";
    String nodeStatus = "idle";   // Estado actual del nodo
    int messageCount = 0;         // Contador de mensajes recibidos
    // Constructor
    LoRaWebServer(uint16_t serverPort = 80);
    
    // Métodos públicos
    void begin(class Lora* node, class Master* master, class Functions* functions);
    void handle();
    void stop();

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
    void manejarPreflightCORS();     // OPTIONS /api/send
    void manejarHolaMundo();         // GET /hola-mundo
    void handleNotFound();
    void configurarHeadersCORS();    // Configurar headers CORS
    void manejarPingTest();          // POST /api/test
};

#endif