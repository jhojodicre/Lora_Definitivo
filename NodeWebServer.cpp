// WebServer.cpp
#include <WebServer.h>
#include "Lora.h"
#include "Master.h"
#include "Functions.h"
#include "NodeWebServer.h"

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
<html lang='es'>
<head>
    <meta charset="UTF-8">
    <title>Barrio San Diego</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap" rel="stylesheet">
    <style>
        body {
            font-family: 'Roboto', Arial, sans-serif;
            margin: 0;
            background: linear-gradient(135deg, #4f8cff 0%, #6a11cb 100%);
            min-height: 100vh;
        }
        .container {
            max-width: 700px;
            margin: 40px auto;
            background: rgba(255,255,255,0.95);
            padding: 32px 24px;
            border-radius: 18px;
            box-shadow: 0 8px 32px rgba(76, 110, 245, 0.15);
        }
        h1 {
            color: #2d3a4a;
            text-align: center;
            font-weight: 700;
            margin-bottom: 24px;
        }
        .section {
            border: 1px solid #e0e7ef;
            padding: 20px;
            margin: 18px 0;
            border-radius: 10px;
            background: #f7faff;
        }
        .status {
            background: #e3f2fd;
            padding: 14px;
            border-radius: 7px;
            margin-bottom: 10px;
            color: #1a237e;
            font-size: 1.05em;
        }
        input[type="text"] {
            padding: 10px;
            border: 1px solid #bdbdbd;
            border-radius: 5px;
            width: 220px;
            font-size: 1em;
            margin-right: 8px;
        }
        .btn {
            padding: 10px 22px;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            font-weight: 600;
            font-size: 1em;
            margin: 4px 2px;
            transition: background 0.2s, box-shadow 0.2s;
        }
        .btn-primary {
            background: linear-gradient(90deg, #4f8cff 0%, #6a11cb 100%);
            color: #fff;
            box-shadow: 0 2px 8px rgba(76, 110, 245, 0.12);
        }
        .btn-primary:hover {
            background: linear-gradient(90deg, #6a11cb 0%, #4f8cff 100%);
        }
        .btn-config {
            background: #fff;
            color: #4f8cff;
            border: 1px solid #4f8cff;
        }
        .btn-config:hover {
            background: #4f8cff;
            color: #fff;
        }
        .config-btns {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-top: 10px;
        }
        .config-label {
            font-weight: 500;
            color: #2d3a4a;
            margin-bottom: 8px;
        }
        #result {
            margin-top: 12px;
            font-size: 1.05em;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 Sistema de Seguridad Perimetral</h1>
        <div class="section">
            <h3>📊 Estado del Sistema</h3>
            <div class="status" id="status">
                <b>Sistema:</b> Master LoRa v1.0<br>
                <b>IP:</b> )rawliteral" + WiFi.localIP().toString() + R"rawliteral(<br>
                <b>WiFi:</b> )rawliteral" + WiFi.SSID() + R"rawliteral(<br>
                <b>RSSI:</b> )rawliteral" + String(WiFi.RSSI()) + R"rawliteral( dBm
            </div>
            <button onclick="location.reload()" class="btn btn-primary">Actualizar</button>
        </div>
        <div class="section">
            <h3>🎮 Control de Comandos</h3>
            <input type="text" id="command" placeholder="Comando (ej: z1AB)" maxlength="6">
            <button onclick="sendCommand()" class="btn btn-primary">Enviar Comando</button>
            <div id="result"></div>
        </div>
        <div class="section">
            <h3>⚙️ Configuración Rápida</h3>
            <div class="config-label">Comandos predeterminados:</div>
            <div class="config-btns">
                <button class="btn btn-config" onclick="sendPreset('C11')">led 1 ON</button>
                <button class="btn btn-config" onclick="sendPreset('C10')">led 1 OFF</button>
                <button class="btn btn-config" onclick="sendPreset('C21')">led 2 ON</button>
                <button class="btn btn-config" onclick="sendPreset('C20')">led 2 OFF</button>
                <button class="btn btn-config" onclick="sendPreset('C31')">led 3 ON</button>
                <button class="btn btn-config" onclick="sendPreset('C30')">led 3 OFF</button>
            </div>
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
        function sendPreset(cmd) {
            document.getElementById('command').value = cmd;
            sendCommand();
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