// WebServer.h
#ifndef WEBSERVER_H
#define WEBSERVER_H

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
    // Constructor
    LoRaWebServer(uint16_t serverPort = 80);
    
    // Métodos públicos
    void begin(class Lora* node, class Master* master, class Functions* functions);
    void handle();
    void stop();
    
    // Handlers para rutas
    void handleRoot();
    void handleAPI();
    void handleNodeControl();
    void handleNotFound();
};

#endif