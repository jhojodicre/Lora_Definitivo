// WebServer.cpp
#include "Lora.h"
#include "Master.h"
#include "Functions.h"
#include "NodeWebServer.h"
#include <Preferences.h>

namespace {
const char* NVS_NAMESPACE_NODE = "node_config";
const char* NVS_KEY_NODE_ADDR = "node_addr";
}

//1. Constructor
LoRaWebServer::LoRaWebServer(uint16_t serverPort) : port(serverPort), isRunning(false) {
    server = new WebServer(port);
}

//2. Inicializar servidor
void LoRaWebServer::begin(Lora* node, Functions* functions, Master* master) {
    nodeRef         = node;
    functionsRef    = functions;
    masterRef       = master;

    // Cargar dirección persistida antes de arrancar el protocolo.
    cargarDireccionDesdeNVS();

    NodeData nodos[5];          // Array de hasta 5 nodos
    //Configurar WiFi
    if(masterRef->MasterMode){
        Serial.println("🌐 Modo Master - Configurando WiFi...");
        configurarWiFi(ssid_master, password_master);
    } else {
        configurarWiFi(ssid_nodo, password_nodo);
    }

    // Configurar servidor
    configurarServidor();

    // Configurar rutas
    configurarRutasServidor();

    inicializarNodos();
    server->onNotFound([this]() { handleNotFound(); });
    
    server->begin();
    isRunning = true;
    
    Serial.println("=== WEB SERVER INICIADO ===");
    Serial.printf("IP: %s:%d\n", WiFi.localIP().toString().c_str(), port);
}

bool LoRaWebServer::cargarDireccionDesdeNVS() {
    if (!masterRef) {
        return false;
    }

    Preferences preferences;
    preferences.begin(NVS_NAMESPACE_NODE, true);
    bool hasKey = preferences.isKey(NVS_KEY_NODE_ADDR);
    int storedAddress = preferences.getInt(NVS_KEY_NODE_ADDR, -1);
    preferences.end();

    if (!hasKey || storedAddress < 0 || storedAddress > 255) {
        Serial.println("📄 Dirección de nodo no encontrada en NVS, usando valor actual");
        return false;
    }

    masterRef->NodeAddress = static_cast<char>(storedAddress);
    Serial.printf("📄 Dirección de nodo cargada desde NVS: %c (%d)\n", masterRef->NodeAddress, storedAddress);
    return true;
}

bool LoRaWebServer::guardarDireccionEnNVS(char address) {
    Preferences preferences;
    preferences.begin(NVS_NAMESPACE_NODE, false);
    size_t bytesWritten = preferences.putInt(NVS_KEY_NODE_ADDR, static_cast<int>(address));
    preferences.end();

    if (bytesWritten == 0) {
        Serial.println("❌ Error al guardar dirección de nodo en NVS");
        return false;
    }

    Serial.printf("💾 Dirección de nodo guardada en NVS: %c (%d)\n", address, static_cast<int>(address));
    return true;
}

//2.1 Configurar servidor
void LoRaWebServer::configurarServidor() {
    //S-5.3 HTTP Client
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

}

//2.2 Configurar WiFi
void LoRaWebServer::configurarWiFi(const char* ssid, const char* password) {
    WiFi.mode(WIFI_STA);
    
    // Configurar IP estática según el modo (Master o Nodo)
    // if(masterRef->MasterMode) {
    //     // Configurar IP estática para Master
    //     WiFi.config(ip_master, gateway_master, subnet_master, dns1_master, dns2_master);
    //     Serial.println("📡 Configurando IP estática para Master...");
    //     Serial.printf("   IP: %s\n", ip_master.toString().c_str());
    // } else {
    //     // Configurar IP estática para Nodo
    //     WiFi.config(ip_nodo, gateway_nodo, subnet_nodo, dns1_nodo, dns2_nodo);
    //     Serial.println("📡 Configurando IP estática para Nodo...");
    //     Serial.printf("   IP: %s\n", ip_nodo.toString().c_str());
    // }
    
    WiFi.begin(ssid, password);
    
    do{
        delay(500);
        Serial.print(".");
        ++conection_try;
    }while(WiFi.status() != WL_CONNECTED && conection_try != 20);
    
    Serial.println();
    Serial.println("✅ Conectado a la red WiFi con IP estática");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
}

//2.3 FUNCIÓN: INICIALIZAR NODOS
void LoRaWebServer::inicializarNodos() {
    Serial.println("📋 Inicializando NUMERO de nodos...");
    
    for (int i = 0; i < 5; i++) {
        nodos[i].nodoId = "";
        nodos[i].comu = "0";
        nodos[i].zoneA = "0";
        nodos[i].zoneB = "0";
        nodos[i].output1 = "0";
        nodos[i].output2 = "0";
        nodos[i].fuente = "0";
        nodos[i].lastUpdate = 0;
        nodos[i].isActive = false;
    }
    
    Serial.println("✅ Array de nodos inicializado");
}

// FUNCIÓN: ACTUALIZAR NODO
bool LoRaWebServer::actualizarNodo(String nodoId, String comu, String zoneA, String zoneB, 
                                  String output1, String output2, String fuente) {
    // Buscar si el nodo ya existe
    int indiceNodo = -1;
    for (int i = 0; i < 5; i++) {
        if (nodos[i].nodoId == nodoId) {
            indiceNodo = i;
            break;
        }
    }
    
    // Si el nodo no existe, buscar un espacio libre
    if (indiceNodo == -1) {
        for (int i = 0; i < 5; i++) {
            if (nodos[i].nodoId == "" || !nodos[i].isActive) {
                indiceNodo = i;
                totalNodos++;
                break;
            }
        }
    }
    
    // Si no hay espacio disponible
    if (indiceNodo == -1) {
        Serial.println("❌ Error: No hay espacio para más nodos");
        return false;
    }
    
    // Actualizar los datos del nodo
    nodos[indiceNodo].nodoId = nodoId;
    nodos[indiceNodo].comu = comu;
    nodos[indiceNodo].zoneA = zoneA;
    nodos[indiceNodo].zoneB = zoneB;
    nodos[indiceNodo].output1 = output1;
    nodos[indiceNodo].output2 = output2;
    nodos[indiceNodo].fuente = fuente;
    nodos[indiceNodo].lastUpdate = millis();
    nodos[indiceNodo].isActive = true;
    
    Serial.println("✅ Nodo " + nodoId + " actualizado en índice " + String(indiceNodo));
    return true;
}

// Manejar cliente
void LoRaWebServer::handle() {
    if (isRunning) {
        server->handleClient();
    }
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
    
    if (masterRef) {
        doc["node"]["address"] = String(masterRef->NodeAddress);
        doc["node"]["mode"] = masterRef->MasterMode ? "MASTER" : "SLAVE";
    }
    
    String response;
    serializeJson(doc, response);
    server->send(200, "application/json", response);
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
    // CORS preflight para /api/send
    server->on("/api/send", HTTP_OPTIONS, [this]() {
        manejarPreflightCORS();
    });    
    // RUTA 3: Endpoint de prueba (GET /api/test)
    server->on("/api/test", HTTP_GET, [this]() {
        manejarPruebaSistema();
    });
    
    // RUTA 4: Estado del nodo (GET /api/status)
    server->on("/api/status", HTTP_GET, [this]() {
        handleGetStatus(); // se puede usar manejarEstadoNodo() si se desea
        // manejarEstadoNodo();
    });
    // RUTA 4.1: Estado específico del master (GET /api/status/master)
    server->on("/api/status/master", HTTP_GET, [this]() {
        handleGetMasterStatus();
    });
      // RUTA 4.2: Recibir datos de un nodo individual (POST /api/node)
    server->on("/api/node", HTTP_POST, [this]() {
        manejarDatoNodoIndividual();
    });

    // RUTA 5: Control de comandos (POST /control)
    server->on("/control", HTTP_POST, [this]() {
        handleNodeControl();
    });
    // RUTA 5.1 Endpoint simple de prueba post
    server->on("/api/ping", HTTP_POST, [this]() {
        manejarPingTest();
    });
    // CORS preflight para /api/ping
    server->on("/api/ping", HTTP_OPTIONS, [this]() {
        manejarPreflightCORS();
    });
    // RUTA 5.2: Forzar zonas (POST /api/force-zones)
    server->on("/api/force-zones", HTTP_POST, [this]() {
        manejarForzarZonas();
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
    // RUTA 8.1: CORS para endpoint /api/node
    server->on("/api/node", HTTP_OPTIONS, [this]() {
        manejarPreflightCORS();
    });
    // RUTA 8.2: CORS para endpoint /api/force-zones
    server->on("/api/force-zones", HTTP_OPTIONS, [this]() {
        manejarPreflightCORS();
    });
    
    // RUTA 9: Hola Mundo (GET /hola-mundo)
    server->on("/hola-mundo", HTTP_GET, [this]() {
        manejarHolaMundo();
    });
    
    

    Serial.println("✅ Rutas configuradas correctamente");
}

// ✅ NUEVO: Manejar mensajes recibidos via POST /api/send
void LoRaWebServer::manejarMensajeRecibido() {
    // Configurar headers CORS
    configurarHeadersCORS();
    // Verificar que se recibió contenido JSON
    if (!server->hasArg("plain")) {
        Serial.println("❌ Error: No se recibió contenido JSON");
        enviarErrorJSON("No se recibió contenido JSON");
        return;
    }

    // Obtener el cuerpo de la solicitud
    String cuerpoJSON = server->arg("plain");
    // Serial.println("📝 JSON recibido: " + cuerpoJSON);

    // Parsear el JSON recibido
    DynamicJsonDocument documento(1024);
    DeserializationError error = deserializeJson(documento, cuerpoJSON);
  
    // Verificar que el JSON es válido
    if (error) {
        Serial.println("❌ Error al parsear JSON: " + String(error.c_str()));
        enviarErrorJSON("JSON inválido");
        return;
    }
    
    // Extraer datos del JSON
    String nodeId = documento["nodeId"] | "";
    String mensaje = documento["message"] | "";
    
    Serial.println("🎯 Node ID: " + nodeId);
    Serial.println("💬 Mensaje: " + mensaje);
    
    // Procesar el mensaje
    bool exito = procesarMensaje(nodeId, mensaje);
    
    // Enviar respuesta apropiada
    if (exito) {
        enviarExitoJSON("Mensaje procesado correctamente");
        messageCount++;
        lastMessage = mensaje;
    } else {
        enviarErrorJSON("Error al procesar el mensaje");
    }

}

// Procesar Mensaje
bool LoRaWebServer::procesarMensaje(String nodeId, String mensaje) {
    // ✅ VALIDACIÓN ROBUSTA: Verificar todas las referencias
        if (!nodeRef) {
            Serial.println("❌ Error: nodeRef es null");
            return false;
        }
        if (!functionsRef) {
            Serial.println("❌ Error: functionsRef es null");
            return false;
        }
    
    // ✅ VALIDACIÓN: Verificar que los objetos tienen memoria válida
        if (nodeId.length() == 0) {
            Serial.println("❌ Error: nodeId vacío");
            return false;
        }
        
        if (mensaje.length() == 0) {
            Serial.println("❌ Error: mensaje vacío");
            return false;
        }
        
        // Validar nodo
        // if (nodeId != String(masterRef->NodeAddress)) {
        //     Serial.printf("⚠️ Advertencia: Nodo ID no coincide: %s != %c\n", nodeId.c_str(), masterRef->NodeAddress);
        //     // No retornar false, procesar de todas formas
        // }
    
    // Procesar mensaje de forma segura
    try {
        // ✅ SEGURIDAD: Usar functionsRef en lugar de acceso directo
        String comando = mensaje;
        if (comando.length() < 6) {
            // Rellenar comando a 6 caracteres
            while (comando.length() < 6) {
                comando += "0";
            }
        }
        masterRef->Master_ExecuteFromServer(mensaje);
        
        Serial.printf("📲📡 Mensaje procesado: %s\n", mensaje.c_str());
    } catch (...) {
        Serial.println("❌ Error: Excepción al procesar mensaje");
        return false;
    }
    
    return true; // ✅ CORREGIDO: Agregar return true
}

// MANEJADOR: ESTADO DEL NODO
void LoRaWebServer::manejarEstadoNodo() {
  Serial.println("📊 Solicitud GET /api/status - Estado del nodo");
  
  configurarHeadersCORS();
  
  // Crear respuesta JSON con estado completo
  DynamicJsonDocument respuesta(512);
  respuesta["nodeId"] = "1";  // Cambiar por el ID real del nodo
  respuesta["status"] = nodeStatus;
  respuesta["lastMessage"] = lastMessage;
  respuesta["messageCount"] = messageCount;
  respuesta["uptime"] = millis();
  respuesta["wifiRSSI"] = WiFi.RSSI();
  respuesta["freeHeap"] = ESP.getFreeHeap();
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
  server->send(200, "application/json", jsonString);
}

// MANEJADOR: DATO DE NODO INDIVIDUAL
void LoRaWebServer::manejarDatoNodoIndividual() {
    Serial.println("\n📦 RECIBIENDO DATOS DE NODO INDIVIDUAL - POST /api/node");
    configurarHeadersCORS();
    // Verificar que se recibió contenido JSON
    if (!server->hasArg("plain")) {
        Serial.println("❌ Error: No se recibió contenido JSON");
        enviarErrorJSON("No se recibió contenido JSON");
        return;
    }
    // Obtener el cuerpo de la solicitud
    String cuerpoJSON = server->arg("plain");
    Serial.println("📝 JSON de nodo recibido: " + cuerpoJSON);
    // Parsear el JSON recibido
    StaticJsonDocument<512> documento;
    DeserializationError error = deserializeJson(documento, cuerpoJSON);
    // Verificar que el JSON es válido
    if (error) {
        Serial.println("❌ Error al parsear JSON: " + String(error.c_str()));
        enviarErrorJSON("JSON inválido");
        return;
    }
    // Extraer datos del nodo
    nodoId   = documento["nodoId"] | "";
    comu     = documento["comu"] | "0";
    zoneA    = documento["zoneA"] | "0";
    zoneB    = documento["zoneB"] | "0";
    output1  = documento["output1"] | "0";
    output2  = documento["output2"] | "0";
    fuente   = documento["fuente"] | "0";
    // Validar que el nodoId no esté vacío
    if (nodoId == "") {
        Serial.println("❌ Error: nodoId requerido");
        enviarErrorJSON("nodoId es requerido");
        return;
    }
    Serial.println("🎯 Actualizando datos del Nodo: " + nodoId);
    Serial.println("   Comu: " + comu + " | ZoneA: " + zoneA + " | ZoneB: " + zoneB);
    Serial.println("   Out1: " + output1 + " | Out2: " + output2 + " | Fuente: " + fuente);
    // Actualizar o crear el nodo en el array LOCAL
    actualizado = actualizarNodo(nodoId, comu, zoneA, zoneB, output1, output2, fuente);
    // ENVIAR DATOS AL SERVIDOR EXTERNO
    // enviadoExterno = enviarDatosAlServidorExterno(nodoId, comu, zoneA, zoneB, output1, output2, fuente);
    // Evaluar resultado de ambas operaciones
    if (actualizado && enviadoExterno) {
        enviarExitoJSON("Datos del nodo " + nodoId + " actualizados localmente y enviados al servidor");
        Serial.println("✅ Nodo " + nodoId + " procesado completamente (local + externo)");
    } else if (actualizado && !enviadoExterno) {
        enviarExitoJSON("Datos del nodo " + nodoId + " actualizados localmente (error al enviar al servidor externo)");
        Serial.println("⚠️  Nodo " + nodoId + " actualizado solo localmente");
    } else if (!actualizado && enviadoExterno) {
        enviarErrorJSON("Error local del nodo " + nodoId + " pero enviado al servidor externo");
        Serial.println("⚠️  Nodo " + nodoId + " enviado externamente pero error local");
    } else {
        enviarErrorJSON("Error completo al procesar el nodo " + nodoId);
        Serial.println("❌ Error completo procesando nodo " + nodoId);
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
    response["components"]["functions"] = (functionsRef != nullptr);
    
    if (masterRef) {
        response["node_info"]["address"] = String(masterRef->NodeAddress);
        response["node_info"]["mode"] = masterRef->MasterMode ? "MASTER" : "SLAVE";
    }
    
    String jsonResponse;
    serializeJson(response, jsonResponse);
    server->send(200, "application/json", jsonResponse);
    
    Serial.println("🧪 Test del sistema ejecutado via API");
}

// ✅ NUEVO: Manejar preflight CORS
void LoRaWebServer::manejarPreflightCORS() {
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
    response["nodo_actual"] = masterRef ? String(masterRef->NodeAddress) : "N/A";
    response["modo"] = masterRef ? (masterRef->MasterMode ? "MASTER" : "SLAVE") : "UNKNOWN";
    response["status"] = "🚀 Sistema funcionando correctamente";
    
    String jsonResponse;
    serializeJson(response, jsonResponse);
    server->send(200, "application/json", jsonResponse);
    
    Serial.println("👋 Endpoint Hola Mundo ejecutado");
}
// Manjear Ping Test
void LoRaWebServer::manejarPingTest() {
    server->sendHeader("Access-Control-Allow-Origin", "*");
    configurarHeadersCORS();
    StaticJsonDocument<200> response;
    response["message"] = "Ping Test OK";
    response["timestamp"] = millis();
    response["uptime_seconds"] = millis() / 1000;
    
    String jsonResponse;
    serializeJson(response, jsonResponse);
    server->send(200, "application/json", jsonResponse);
    
    Serial.println("🔍 Ping Test ejecutado");
}
void LoRaWebServer::handleGetMasterStatus() {

      // Configurar conexión HTTP
  http.begin(apiHeartBeat);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-Master/1.0");
  http.setTimeout(timeoutHTTP);
// 🐞🐞 Print Debug
//   Serial.println("📦 JSON enviando " + JsonString);


    StaticJsonDocument<320> response;
    response["success"] = true;
    response["source"] = "ESP32-Master";
    response["timestamp"] = millis()/1000;

    String jsonResponse;
    serializeJson(response, jsonResponse);


    // Realizar petición POST
    httpResponseCode = http.POST(jsonResponse);

  // Procesar respuesta
    if (httpResponseCode > 0) {
        respuesta = http.getString();
        Serial.print("📥 Código respuesta: " + String(httpResponseCode));
        // Serial.println("📄 Respuesta servidor: " + respuesta);
        
        if (httpResponseCode == 200 || httpResponseCode == 201) {
            Serial.println("🌐 RX : ✅" );
            Serial.println();
            Serial.println();
            http.end();
        } else {
            Serial.println("⚠️  Servidor respondió con código: " + String(httpResponseCode));
            http.end();
        }
    } else {
        Serial.println("❌ Error en petición HTTP: " + String(httpResponseCode));
        Serial.println("   Error: " + String(http.errorToString(httpResponseCode)));
        http.end();
    }


    Serial.println("✅ GET /api/status/master ejecutado");
}
// ✅ NUEVO: Manejar forzar zonas
void LoRaWebServer::manejarForzarZonas() {
    Serial.println("\n🔧 FORZANDO ZONAS - POST /api/force-zones");
    configurarHeadersCORS();
    
    // Verificar que se recibió contenido JSON
    if (!server->hasArg("plain")) {
        Serial.println("❌ Error: No se recibió contenido JSON");
        enviarErrorJSON("No se recibió contenido JSON");
        return;
    }

    // Obtener el cuerpo de la solicitud
    String cuerpoJSON = server->arg("plain");
    Serial.println("📝 JSON de forzar zonas recibido: " + cuerpoJSON);

    // Parsear el JSON recibido
    StaticJsonDocument<512> documento;
    DeserializationError error = deserializeJson(documento, cuerpoJSON);

    // Verificar que el JSON es válido
    if (error) {
        Serial.println("❌ Error al parsear JSON: " + String(error.c_str()));
        enviarErrorJSON("JSON inválido");
        return;
    }

    // Verificar que nodeRef existe
    if (!nodeRef) {
        Serial.println("❌ Error: nodeRef no disponible");
        enviarErrorJSON("Node reference no disponible");
        return;
    }

    // Variables para trackear qué se actualizó
    bool actualizado = false;
    String mensajeRespuesta = "Zonas actualizadas: ";

    // Procesar Zone_A_Force
    if (documento.containsKey("zone_a_force")) {
        bool valor = documento["zone_a_force"] | false;
        nodeRef->Zone_A_Force = valor;
        nodeRef->Zone_A_Forzar = true;  // Activar el forzado
        Serial.println("🔧 Zone_A_Force = " + String(valor ? "true" : "false"));
        mensajeRespuesta += "Zone_A(" + String(valor ? "1" : "0") + ") ";
        actualizado = true;
    }

    // Procesar Zone_B_Force
    if (documento.containsKey("zone_b_force")) {
        bool valor = documento["zone_b_force"] | false;
        nodeRef->Zone_B_Force = valor;
        nodeRef->Zone_B_Forzar = true;  // Activar el forzado
        Serial.println("🔧 Zone_B_Force = " + String(valor ? "true" : "false"));
        mensajeRespuesta += "Zone_B(" + String(valor ? "1" : "0") + ") ";
        actualizado = true;
    }

    // Procesar Fuente_in_Force
    if (documento.containsKey("fuente_force")) {
        bool valor = documento["fuente_force"] | false;
        nodeRef->Fuente_in_Force = valor;
        nodeRef->Fuente_in_Forzar = true;  // Activar el forzado
        Serial.println("🔧 Fuente_in_Force = " + String(valor ? "true" : "false"));
        mensajeRespuesta += "Fuente(" + String(valor ? "1" : "0") + ") ";
        actualizado = true;
    }

    // Procesar desactivación de forzado
    if (documento.containsKey("disable_force")) {
        bool deshabilitar = documento["disable_force"] | false;
        if (deshabilitar) {
            nodeRef->Zone_A_Forzar = false;
            nodeRef->Zone_B_Forzar = false;
            nodeRef->Fuente_in_Forzar = false;
            Serial.println("🔓 Forzado deshabilitado para todas las zonas");
            mensajeRespuesta = "Forzado deshabilitado para todas las zonas";
            actualizado = true;
        }
    }

    // Enviar respuesta
    if (actualizado) {
        // Crear respuesta detallada
        StaticJsonDocument<400> respuesta;
        respuesta["success"] = true;
        respuesta["message"] = mensajeRespuesta;
        respuesta["timestamp"] = millis();
        
        // Estado actual de las zonas forzadas
        respuesta["current_state"]["zone_a_force"] = nodeRef->Zone_A_Force;
        respuesta["current_state"]["zone_a_forzar"] = nodeRef->Zone_A_Forzar;
        respuesta["current_state"]["zone_b_force"] = nodeRef->Zone_B_Force;
        respuesta["current_state"]["zone_b_forzar"] = nodeRef->Zone_B_Forzar;
        respuesta["current_state"]["fuente_force"] = nodeRef->Fuente_in_Force;
        respuesta["current_state"]["fuente_forzar"] = nodeRef->Fuente_in_Forzar;
        
        String jsonRespuesta;
        serializeJson(respuesta, jsonRespuesta);
        server->send(200, "application/json", jsonRespuesta);
        
        Serial.println("✅ " + mensajeRespuesta);
    } else {
        Serial.println("❌ No se especificaron zonas válidas para forzar");
        enviarErrorJSON("No se especificaron zonas válidas para forzar");
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

// Configuracion de headers CORS
    // Esta función se llama para configurar los headers CORS en las respuestas del servidor
void LoRaWebServer::configurarHeadersCORS() {
    server->sendHeader("Access-Control-Allow-Origin", "*");
    server->sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server->sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    server->sendHeader("Access-Control-Max-Age", "86400");
    server->send(200, "text/plain", "");
}

// Enviar respuesta de error
void LoRaWebServer::enviarErrorJSON(String mensaje) {
    // ✅ OPTIMIZACIÓN: Usar StaticJsonDocument para evitar fragmentación de memoria
    StaticJsonDocument<256> respuesta;
    respuesta["success"] = false;
    respuesta["error"] = mensaje;
    respuesta["timestamp"] = millis();

    String jsonString;
    size_t bytesWritten = serializeJson(respuesta, jsonString);
    
    if (bytesWritten == 0) {
        Serial.println("❌ Error: No se pudo serializar JSON de error");
        server->send(500, "text/plain", "Error interno del servidor");
        return;
    }

    configurarHeadersCORS();
    server->send(400, "application/json", jsonString);
    Serial.println("❌ Error enviado: " + mensaje);
}

// Enviar respuesta de éxito
void LoRaWebServer::enviarExitoJSON(String mensaje) {
    // ✅ OPTIMIZACIÓN: Usar StaticJsonDocument para evitar fragmentación de memoria
    StaticJsonDocument<256> respuesta;
    respuesta["success"] = true;
    respuesta["message"] = mensaje;
    respuesta["timestamp"] = millis();
    
    String jsonString;
    size_t bytesWritten = serializeJson(respuesta, jsonString);
    
    if (bytesWritten == 0) {
        Serial.println("❌ Error: No se pudo serializar JSON de éxito");
        server->send(500, "text/plain", "Error interno del servidor");
        return;
    }
    
    configurarHeadersCORS();
    server->send(200, "application/json", jsonString);
    Serial.println("✅ Éxito enviado: " + mensaje);
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
    if (masterRef) {
        masterRef->NodeAddress = newChar;
        Serial.printf("Nueva dirección del nodo: %c (%d)\n", newChar, (int)newChar);

        if (!guardarDireccionEnNVS(newChar)) {
            server->send(500, "application/json", "{\"error\":\"No se pudo guardar en NVS\"}");
            return;
        }
        
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
    
    if (masterRef) {
        doc["node"]["address_char"] = String(masterRef->NodeAddress);
        doc["node"]["address_ascii"] = (int)masterRef->NodeAddress;
        doc["node"]["mode"] = masterRef ? (masterRef->MasterMode ? "MASTER" : "SLAVE") : "UNKNOWN";
        
        // ✅ AÑADIR: Estado de forzado de zonas
        doc["node"]["zone_a_force"] = nodeRef->Zone_A_Force;
        doc["node"]["zone_a_forzar"] = nodeRef->Zone_A_Forzar;
        doc["node"]["zone_b_force"] = nodeRef->Zone_B_Force;
        doc["node"]["zone_b_forzar"] = nodeRef->Zone_B_Forzar;
        doc["node"]["fuente_force"] = nodeRef->Fuente_in_Force;
        doc["node"]["fuente_forzar"] = nodeRef->Fuente_in_Forzar;
    }

    if (nodeRef && nodeRef) {
    doc["master"]["node_count"] = String(masterRef->NodeAddress); // Corregido: char a String
        doc["master"]["next_node"] = nodeRef->nodo_a_Consultar;
    }
    
    String response;
    serializeJson(doc, response);
    server->send(200, "application/json", response);
}

bool LoRaWebServer::enviarDatosAlServidorExterno(String JsonString) {

  Serial.print("\n🌐 TX SERVER ...");
  
  // Verificar conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Error: WiFi no conectado");
    return false;
  }
// 🐞🐞 Print Debug
  // Construir URL completa
//   Serial.println("🎯 URL destino: " + String(apiEndpoint));

  // Configurar conexión HTTP
  http.begin(apiEndpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-Master/1.0");
  http.setTimeout(timeoutHTTP);

// 🐞🐞 Print Debug
//   Serial.println("📦 JSON enviando " + JsonString);

  // Realizar petición POST
  httpResponseCode = http.POST(JsonString);

  // Procesar respuesta
  if (httpResponseCode > 0) {
    respuesta = http.getString();
    Serial.print("📥 Código respuesta: " + String(httpResponseCode));
    // Serial.println("📄 Respuesta servidor: " + respuesta);
    
    if (httpResponseCode == 200 || httpResponseCode == 201) {
    Serial.println("🌐 RX : ✅" );
    Serial.println();
    Serial.println();
      http.end();
      return true;
    } else {
      Serial.println("⚠️  Servidor respondió con código: " + String(httpResponseCode));
      http.end();
      return false;
    }
  } else {
    Serial.println("❌ Error en petición HTTP: " + String(httpResponseCode));
    Serial.println("   Error: " + String(http.errorToString(httpResponseCode)));
    http.end();
    return false;
  }
}


// Página principal
void LoRaWebServer::handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang='es'>
<head>
    <meta charset="UTF-8">
    <title>Seguridad Perimetral</title>
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
                <b>Nodo:</b> <span id="nodeChar">)rawliteral" + (masterRef ? String(masterRef->NodeAddress) : String("-")) + R"rawliteral(</span><br>
                <b>ASCII decimal:</b> <span id="nodeAscii">)rawliteral" + (masterRef ? String((int)masterRef->NodeAddress) : String("-")) + R"rawliteral(</span><br>
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
        <div class="section">
            <h3>🔧 Control de Forzado de Zonas</h3>
            <div class="config-label">Forzar valores de sensores:</div>
            <div style="margin-bottom: 10px;">
                <label style="display: inline-block; width: 120px; font-weight: 500;">Zona A:</label>
                <input type="checkbox" id="forceZoneA" style="margin-right: 5px;">
                <select id="zoneAValue" style="padding: 5px; border-radius: 4px; border: 1px solid #bdbdbd;">
                    <option value="false">Desactivada (0)</option>
                    <option value="true">Activada (1)</option>
                </select>
            </div>
            <div style="margin-bottom: 10px;">
                <label style="display: inline-block; width: 120px; font-weight: 500;">Zona B:</label>
                <input type="checkbox" id="forceZoneB" style="margin-right: 5px;">
                <select id="zoneBValue" style="padding: 5px; border-radius: 4px; border: 1px solid #bdbdbd;">
                    <option value="false">Desactivada (0)</option>
                    <option value="true">Activada (1)</option>
                </select>
            </div>
            <div style="margin-bottom: 15px;">
                <label style="display: inline-block; width: 120px; font-weight: 500;">Fuente:</label>
                <input type="checkbox" id="forceFuente" style="margin-right: 5px;">
                <select id="fuenteValue" style="padding: 5px; border-radius: 4px; border: 1px solid #bdbdbd;">
                    <option value="false">Desactivada (0)</option>
                    <option value="true">Activada (1)</option>
                </select>
            </div>
            <div class="config-btns">
                <button onclick="applyForceZones()" class="btn btn-primary">Aplicar Forzado</button>
                <button onclick="disableAllForcing()" class="btn btn-config">Deshabilitar Todo</button>
                <button onclick="getForceStatus()" class="btn btn-config">Ver Estado</button>
            </div>
            <div id="forceResult"></div>
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
        
        // ✅ NUEVAS FUNCIONES: Control de forzado de zonas
        function applyForceZones() {
            var payload = {};
            var hasChanges = false;
            
            // Verificar Zone A
            if (document.getElementById('forceZoneA').checked) {
                payload.zone_a_force = document.getElementById('zoneAValue').value === 'true';
                hasChanges = true;
            }
            
            // Verificar Zone B
            if (document.getElementById('forceZoneB').checked) {
                payload.zone_b_force = document.getElementById('zoneBValue').value === 'true';
                hasChanges = true;
            }
            
            // Verificar Fuente
            if (document.getElementById('forceFuente').checked) {
                payload.fuente_force = document.getElementById('fuenteValue').value === 'true';
                hasChanges = true;
            }
            
            if (!hasChanges) {
                alert('Selecciona al menos una zona para forzar');
                return;
            }
            
            fetch('/api/force-zones', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(payload)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    document.getElementById('forceResult').innerHTML = 
                        '<p style="color: green;">✅ ' + data.message + '</p>';
                    console.log('Estado actual:', data.current_state);
                } else {
                    document.getElementById('forceResult').innerHTML = 
                        '<p style="color: red;">❌ Error: ' + data.error + '</p>';
                }
            })
            .catch(error => {
                document.getElementById('forceResult').innerHTML = 
                    '<p style="color: red;">❌ Error de conexión: ' + error + '</p>';
            });
        }
        
        function disableAllForcing() {
            var payload = {
                disable_force: true
            };
            
            fetch('/api/force-zones', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(payload)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    document.getElementById('forceResult').innerHTML = 
                        '<p style="color: blue;">🔓 ' + data.message + '</p>';
                    // Desmarcar todos los checkboxes
                    document.getElementById('forceZoneA').checked = false;
                    document.getElementById('forceZoneB').checked = false;
                    document.getElementById('forceFuente').checked = false;
                } else {
                    document.getElementById('forceResult').innerHTML = 
                        '<p style="color: red;">❌ Error: ' + data.error + '</p>';
                }
            })
            .catch(error => {
                document.getElementById('forceResult').innerHTML = 
                    '<p style="color: red;">❌ Error de conexión: ' + error + '</p>';
            });
        }
        
        function getForceStatus() {
            fetch('/api/status')
            .then(response => response.json())
            .then(data => {
                var statusText = '📊 Estado de forzado:<br>';
                if (data.node) {
                    statusText += '• Zona A: ' + (data.node.zone_a_forzar ? 'FORZADA (' + data.node.zone_a_force + ')' : 'NORMAL') + '<br>';
                    statusText += '• Zona B: ' + (data.node.zone_b_forzar ? 'FORZADA (' + data.node.zone_b_force + ')' : 'NORMAL') + '<br>';
                    statusText += '• Fuente: ' + (data.node.fuente_forzar ? 'FORZADA (' + data.node.fuente_force + ')' : 'NORMAL');
                } else {
                    statusText += 'No se pudo obtener el estado';
                }
                document.getElementById('forceResult').innerHTML = 
                    '<p style="color: blue;">' + statusText + '</p>';
            })
            .catch(error => {
                document.getElementById('forceResult').innerHTML = 
                    '<p style="color: red;">❌ Error al obtener estado: ' + error + '</p>';
            });
        }
    </script>
</body>
</html>
)rawliteral";
    server->send(200, "text/html", html);
}

//6. HTTP Send
    //   void sendJsonToMongoDB() {
    //     if (WiFi.status() == WL_CONNECTED) {
    //       jsonString = Node.jsonString; // Obtener la cadena JSON del objeto
    //       httpResponseCode = http.POST(jsonString);
    //       if (httpResponseCode > 0) {
    //         String response = http.getString();
    //         Serial.println(httpResponseCode);
    //         Serial.println(response);
    //       } else {
    //         Serial.println("Error en la solicitud HTTP");
    //       }
    //       Serial.println(jsonString);
    //       http.end(); // Finaliza la conexión HTTP
    //     }
    //     else {
    //       Serial.println("WiFi not connected");
    //     }
    //   }
    //   void http_Post() {
    //     jsonString = Node.jsonString; // Obtener la cadena JSON del objeto
    //     httpResponseCode = http.POST(jsonString); // Realizar petición POST
    //     Serial.println("📦 JSON enviando: " + jsonString);
    //     // Procesar respuesta
    //     if (httpResponseCode > 0) {
    //       String respuesta = http.getString();
    //       Serial.println("📥 Código respuesta: " + String(httpResponseCode));
    //       Serial.println("📄 Respuesta servidor: " + respuesta);
    //       if (httpResponseCode == 200 || httpResponseCode == 201) {
    //         Serial.println("✅ Datos enviados exitosamente al servidor externo");
    //         http.end();
    //         // return true;
    //       } else {
    //         Serial.println("⚠️  Servidor respondió con código: " + String(httpResponseCode));
    //         http.end();
    //         // return false;
    //       }
    //     } else {
    //       Serial.println("❌ Error en petición HTTP: " + String(httpResponseCode));
    //       Serial.println("   Error: " + http.errorToString(httpResponseCode));
    //       http.end();
    //       // return false;
    //     }
    //       http.end();
    //   }
    //   void DeserializeJson(){
    //     // jsonString = "{\"comm\":1,\"node\":3,\"zoneA\":100,\"zoneB\":100,\"output1\":0,\"output2\":1}"; // Ejemplo de cadena JSON
    //     DeserializationError error = deserializeJson(doc, jsonString);
    //     if (error) {
    //       Serial.print(F("deserializeJson() failed: "));
    //       Serial.println(error.f_str());
    //       return;
    //     }
    //     // nombre = doc["nombre"].as<String>();
    //     nombre = doc["temperature"];
    //     valueJson = doc["humidity"];   //.as<int>();
    //     Serial.print("MCU: ");
    //     Serial.println(nombre);
    //     Serial.print("Valor: ");
    //     Serial.println(valueJson);
    //   }
    //   void registerNode() {
    //     String payload = "{\"nodeId\":\"ESP32_001\",\"location\":\"Entrada Principal\",\"type\":\"motion_sensor\"}";
    //     int httpResponseCode = http.POST(payload);
    //     if (httpResponseCode > 0) {
    //       String response = http.getString();
    //       Serial.println("Node registered: " + response);
    //     }
    //     http.end();
    //   }
