// WebServer.cpp
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
    configurarRutasServidor();
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

// ✅ NUEVA FUNCIÓN: Configurar todas las rutas del servidor
void LoRaWebServer::configurarRutasServidor() {
    Serial.println("🛣️  Configurando rutas del servidor...");
    
    // RUTA 1: Página principal (GET /)
    server->on("/", HTTP_GET, [this]() {
        handleRoot();
    });
    
    // RUTA 2: Endpoint principal para recibir mensajes (POST /api/send)
    server->on("/api/send", HTTP_POST, [this]() {
        manejarMensajeRecibido();
    });
    
    // RUTA 3: Endpoint de prueba (GET /api/test)
    server->on("/api/test", HTTP_GET, [this]() {
        manejarPruebaSistema();
    });
    
    // RUTA 4: Estado del nodo (GET /api/status)
    server->on("/api/status", HTTP_GET, [this]() {
        handleGetStatus();
    });
    
    // RUTA 5: Control de comandos (POST /control)
    server->on("/control", HTTP_POST, [this]() {
        handleNodeControl();
    });
    
    // RUTA 6: Cambiar dirección (POST /set-address)
    server->on("/set-address", HTTP_POST, [this]() {
        handleSetAddress();
    });
    
    // RUTA 7: API información básica (GET /api)
    server->on("/api", HTTP_GET, [this]() {
        handleAPI();
    });
    
    // RUTA 8: Manejo de preflight CORS (OPTIONS)
    server->on("/api/send", HTTP_OPTIONS, [this]() {
        manejarPreflightCORS();
    });
    
    // RUTA 9: Hola Mundo (GET /hola-mundo)
    server->on("/hola-mundo", HTTP_GET, [this]() {
        manejarHolaMundo();
    });
    
    Serial.println("✅ Rutas configuradas correctamente");
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
            <h3>🔢 Número de Nodo Asignado</h3>
            <div class="status">
                <b>Nodo:</b> <span id="nodeChar">)rawliteral" + (nodeRef ? String(nodeRef->local_Address) : String("-")) + R"rawliteral(</span><br>
                <b>ASCII decimal:</b> <span id="nodeAscii">)rawliteral" + (nodeRef ? String((int)nodeRef->local_Address) : String("-")) + R"rawliteral(</span><br>
            </div>
            <input type="text" id="newAddress" placeholder="Nueva dirección (A-Z, 1-9)" maxlength="1">
            <button onclick="changeAddress()" class="btn btn-primary">Cambiar Dirección</button>
            <button onclick="refreshStatus()" class="btn btn-config">Actualizar Estado</button>
            <div id="addressResult"></div>
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
        
        // ✅ NUEVA FUNCIÓN: Cambiar dirección del nodo
        function changeAddress() {
            var newAddr = document.getElementById('newAddress').value;
            if(newAddr.length !== 1) {
                alert('La dirección debe ser un solo carácter (A-Z o 1-9)');
                return;
            }
            
            fetch('/set-address', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'address=' + newAddr
            })
            .then(response => response.json())
            .then(data => {
                if(data.success) {
                    document.getElementById('nodeChar').textContent = data.new_address;
                    document.getElementById('nodeAscii').textContent = data.ascii_value;
                    document.getElementById('addressResult').innerHTML = 
                        '<p style="color: green;">✅ Dirección cambiada a: ' + data.new_address + ' (ASCII: ' + data.ascii_value + ')</p>';
                } else {
                    document.getElementById('addressResult').innerHTML = 
                        '<p style="color: red;">❌ Error: ' + data.error + '</p>';
                }
            })
            .catch(error => {
                document.getElementById('addressResult').innerHTML = 
                    '<p style="color: red;">❌ Error de conexión: ' + error + '</p>';
            });
        }
        
        // ✅ NUEVA FUNCIÓN: Actualizar estado desde el servidor
        function refreshStatus() {
            fetch('/get-status')
            .then(response => response.json())
            .then(data => {
                if(data.node) {
                    document.getElementById('nodeChar').textContent = data.node.address_char;
                    document.getElementById('nodeAscii').textContent = data.node.address_ascii;
                }
                document.getElementById('addressResult').innerHTML = 
                    '<p style="color: blue;">🔄 Estado actualizado - Uptime: ' + Math.floor(data.system.uptime_ms / 1000) + 's</p>';
            })
            .catch(error => {
                document.getElementById('addressResult').innerHTML = 
                    '<p style="color: red;">❌ Error al actualizar: ' + error + '</p>';
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

// ✅ NUEVO: Cambiar dirección del nodo
void LoRaWebServer::handleSetAddress() {
    if (!server->hasArg("address")) {
        server->send(400, "application/json", "{\"error\":\"Dirección requerida\"}");
        return;
    }
    
    String newAddress = server->arg("address");
    
    // Validar que sea un solo carácter
    if (newAddress.length() != 1) {
        server->send(400, "application/json", "{\"error\":\"Dirección debe ser un solo carácter\"}");
        return;
    }
    
    char newChar = newAddress.charAt(0);
    
    // Validar rango (por ejemplo, solo A-Z, 1-9)
    if (!((newChar >= 'A' && newChar <= 'Z') || (newChar >= '1' && newChar <= '9'))) {
        server->send(400, "application/json", "{\"error\":\"Dirección debe ser A-Z o 1-9\"}");
        return;
    }
    
    // Cambiar la dirección
    if (nodeRef) {
        nodeRef->local_Address = newChar;
        Serial.printf("Nueva dirección del nodo: %c (%d)\n", newChar, (int)newChar);
        
        String response = "{\"success\":true,\"new_address\":\"" + String(newChar) + 
                         "\",\"ascii_value\":" + String((int)newChar) + "}";
        server->send(200, "application/json", response);
    } else {
        server->send(500, "application/json", "{\"error\":\"Nodo no disponible\"}");
    }
}

// ✅ NUEVO: Obtener estado completo
void LoRaWebServer::handleGetStatus() {
    StaticJsonDocument<400> doc;
    
    doc["timestamp"] = millis();
    doc["system"]["name"] = "LoRa Security System";
    doc["system"]["version"] = "1.0";
    doc["system"]["uptime_ms"] = millis();
    
    doc["wifi"]["ssid"] = WiFi.SSID();
    doc["wifi"]["ip"] = WiFi.localIP().toString();
    doc["wifi"]["rssi"] = WiFi.RSSI();
    doc["wifi"]["status"] = (WiFi.status() == WL_CONNECTED) ? "connected" : "disconnected";
    
    if (nodeRef) {
        doc["node"]["address_char"] = String(nodeRef->local_Address);
        doc["node"]["address_ascii"] = (int)nodeRef->local_Address;
        doc["node"]["mode"] = masterRef ? (masterRef->Mode ? "MASTER" : "SLAVE") : "UNKNOWN";
    }
    
    if (masterRef && masterRef->Mode) {
        doc["master"]["node_count"] = masterRef->nodeNumber;
        doc["master"]["next_node"] = masterRef->Nodo_Proximo;
    }
    
    String response;
    serializeJson(doc, response);
    server->send(200, "application/json", response);
}

// ✅ NUEVO: Manejar mensajes recibidos via POST /api/send
void LoRaWebServer::manejarMensajeRecibido() {
    // Configurar headers CORS
    server->sendHeader("Access-Control-Allow-Origin", "*");
    server->sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server->sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    if (!server->hasArg("message")) {
        server->send(400, "application/json", "{\"error\":\"Mensaje requerido\"}");
        return;
    }
    
    String mensaje = server->arg("message");
    
    // Validar mensaje
    if (mensaje.length() < 1) {
        server->send(400, "application/json", "{\"error\":\"Mensaje no puede estar vacío\"}");
        return;
    }
    
    // Procesar mensaje con el objeto Functions
    if (functionsRef) {
        functionsRef->Functions_Request(mensaje);
        functionsRef->Functions_Run();
        
        Serial.printf("📨 Mensaje recibido via API: %s\n", mensaje.c_str());
        
        StaticJsonDocument<200> response;
        response["success"] = true;
        response["message"] = "Mensaje procesado correctamente";
        response["received_message"] = mensaje;
        response["timestamp"] = millis();
        
        String jsonResponse;
        serializeJson(response, jsonResponse);
        server->send(200, "application/json", jsonResponse);
    } else {
        server->send(500, "application/json", "{\"error\":\"Sistema de funciones no disponible\"}");
    }
}

// ✅ NUEVO: Endpoint de prueba del sistema
void LoRaWebServer::manejarPruebaSistema() {
    server->sendHeader("Access-Control-Allow-Origin", "*");
    
    StaticJsonDocument<300> response;
    response["test_status"] = "OK";
    response["system_name"] = "LoRa Security System";
    response["timestamp"] = millis();
    response["uptime_seconds"] = millis() / 1000;
    response["free_heap"] = ESP.getFreeHeap();
    response["chip_id"] = ESP.getChipModel();
    
    // Test de componentes
    response["components"]["wifi"] = (WiFi.status() == WL_CONNECTED);
    response["components"]["node"] = (nodeRef != nullptr);
    response["components"]["master"] = (masterRef != nullptr);
    response["components"]["functions"] = (functionsRef != nullptr);
    
    if (nodeRef) {
        response["node_info"]["address"] = String(nodeRef->local_Address);
        response["node_info"]["mode"] = masterRef ? (masterRef->Mode ? "MASTER" : "SLAVE") : "UNKNOWN";
    }
    
    String jsonResponse;
    serializeJson(response, jsonResponse);
    server->send(200, "application/json", jsonResponse);
    
    Serial.println("🧪 Test del sistema ejecutado via API");
}

// ✅ NUEVO: Manejar preflight CORS
void LoRaWebServer::manejarPreflightCORS() {
    server->sendHeader("Access-Control-Allow-Origin", "*");
    server->sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server->sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server->send(200, "text/plain", "");
    configurarHeadersCORS();
    Serial.println("🌐 Preflight CORS manejado");
}

// ✅ NUEVO: Endpoint Hola Mundo
void LoRaWebServer::manejarHolaMundo() {
    server->sendHeader("Access-Control-Allow-Origin", "*");
    
    StaticJsonDocument<200> response;
    response["message"] = "¡Hola Mundo desde el ESP32!";
    response["sistema"] = "LoRa Security System";
    response["timestamp"] = millis();
    response["uptime_segundos"] = millis() / 1000;
    response["nodo_actual"] = nodeRef ? String(nodeRef->local_Address) : "N/A";
    response["modo"] = masterRef ? (masterRef->Mode ? "MASTER" : "SLAVE") : "UNKNOWN";
    response["status"] = "🚀 Sistema funcionando correctamente";
    
    String jsonResponse;
    serializeJson(response, jsonResponse);
    server->send(200, "application/json", jsonResponse);
    
    Serial.println("👋 Endpoint Hola Mundo ejecutado");
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
void LoRaWebServer::configurarHeadersCORS() {
    server->sendHeader("Access-Control-Allow-Origin", "*");
    server->sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server->sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    server->sendHeader("Access-Control-Max-Age", "86400");
}
