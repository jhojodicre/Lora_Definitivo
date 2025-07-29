// WebServer.cpp
#include <WebServer.h>
#include "Lora.h"
#include "Master.h"
#include "Functions.h"

// Constructor
LoRaWebServer::LoRaWebServer(uint16_t serverPort) : port(serverPort), isRunning(false) {
    server = new WebServer(port);
}

// Inicializar servidor
void LoRaWebServer::begin(Lora* node, Master* master, Functions* functions) {
    nodeRef = node;
    masterRef = master;
    functionsRef = functions;
    
    // Configurar rutas
    server->on("/", [this]() { handleRoot(); });
    server->on("/api", [this]() { handleAPI(); });
    server->on("/control", HTTP_POST, [this]() { handleNodeControl(); });
    server->onNotFound([this]() { handleNotFound(); });
    
    server->begin();
    isRunning = true;
    
    Serial.println("=== WEB SERVER INICIADO ===");
    Serial.printf("IP: %s:%d\n", WiFi.localIP().toString().c_str(), port);
}

// Manejar cliente
void LoRaWebServer::handle() {
    if (isRunning) {
        server->handleClient();
    }
}

// Página principal
void LoRaWebServer::handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>LoRa Control</title>
    <style>
        body { font-family: Arial; margin: 20px; background: #f5f5f5; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        h1 { text-align: center; color: #333; }
        .section { border: 1px solid #ddd; padding: 15px; margin: 10px 0; border-radius: 5px; }
        input { padding: 8px; margin: 5px; border: 1px solid #ccc; border-radius: 3px; }
        button { padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; background: #007bff; color: white; }
        .status { background: #e8f5e8; padding: 10px; border-radius: 5px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 LoRa System Control</h1>
        
        <div class="section">
            <h3>📊 Estado del Sistema</h3>
            <div class="status" id="status">
                Sistema: LoRa v1.0<br>
                IP: )rawliteral" + WiFi.localIP().toString() + R"rawliteral(<br>
                WiFi: )rawliteral" + WiFi.SSID() + R"rawliteral(<br>
                RSSI: )rawliteral" + String(WiFi.RSSI()) + R"rawliteral( dBm
            </div>
            <button onclick="location.reload()">Actualizar</button>
        </div>
        
        <div class="section">
            <h3>🎮 Control de Comandos</h3>
            <input type="text" id="command" placeholder="Comando (ej: z1AB)" maxlength="6"><br>
            <button onclick="sendCommand()">Enviar Comando</button>
            <div id="result"></div>
        </div>
    </div>

    <script>
        function sendCommand() {
            var cmd = document.getElementById('command').value;
            if(cmd.length < 2) {
                alert('Comando debe tener al menos 2 caracteres');
                return;
            }
            
            fetch('/control', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'command=' + cmd
            })
            .then(response => response.text())
            .then(data => {
                document.getElementById('result').innerHTML = '<p>Respuesta: ' + data + '</p>';
            })
            .catch(error => {
                document.getElementById('result').innerHTML = '<p>Error: ' + error + '</p>';
            });
        }
    </script>
</body>
</html>
)rawliteral";
    
    server->send(200, "text/html", html);
}

// API info
void LoRaWebServer::handleAPI() {
    StaticJsonDocument<300> doc;
    doc["system"] = "LoRa Communication System";
    doc["version"] = "1.0";
    doc["uptime"] = millis();
    doc["wifi"]["ssid"] = WiFi.SSID();
    doc["wifi"]["ip"] = WiFi.localIP().toString();
    doc["wifi"]["rssi"] = WiFi.RSSI();
    
    if (nodeRef) {
        doc["node"]["address"] = String(nodeRef->local_Address);
        doc["node"]["mode"] = masterRef->Mode ? "MASTER" : "SLAVE";
    }
    
    String response;
    serializeJson(doc, response);
    server->send(200, "application/json", response);
}

// Control de nodo
void LoRaWebServer::handleNodeControl() {
    if (!server->hasArg("command")) {
        server->send(400, "text/plain", "Error: Comando requerido");
        return;
    }
    
    String command = server->arg("command");
    
    // Validar comando
    if (command.length() < 2) {
        server->send(400, "text/plain", "Error: Comando muy corto");
        return;
    }
    
    // Rellenar comando a 6 caracteres
    while (command.length() < 6) {
        command += "0";
    }
    
    // Ejecutar comando
    if (functionsRef) {
        functionsRef->Functions_Request(command);
        functionsRef->Functions_Run();
        
        Serial.printf("WebServer ejecutó: %s\n", command.c_str());
        server->send(200, "text/plain", "Comando ejecutado: " + command);
    } else {
        server->send(500, "text/plain", "Error: Functions no disponible");
    }
}

// 404 Not Found
void LoRaWebServer::handleNotFound() {
    server->send(404, "text/plain", "Página no encontrada");
}

// Detener servidor
void LoRaWebServer::stop() {
    if (isRunning) {
        server->stop();
        isRunning = false;
        Serial.println("Web Server detenido");
    }
}