/*
 * =========================================
 * CÓDIGO ESP32 - SERVIDOR WEB CON MESSAGING
 * =========================================
 * 
 * Este código implementa un servidor web en ESP32 que:
 * - Se conecta a WiFi
 * - Recibe mensajes HTTP POST en formato JSON
 * - Procesa comandos con formato "N1A123"
 * - Responde con confirmaciones JSON
 * - Maneja CORS para comunicación web
 * 
 * Autor: Sistema de Monitoreo ESP32
 * Fecha: 2024
 */

#include <WiFi.h>        // Librería para conectividad WiFi
#include <WebServer.h>   // Librería para servidor HTTP
#include <ArduinoJson.h> // Librería para manejo de JSON
#include <HTTPClient.h>  // Librería para cliente HTTP (enviar datos)

// ===============================
// CONFIGURACIÓN DE RED WIFI
// ===============================
const char* ssid = "TU_WIFI_NOMBRE";         // Cambia por el nombre de tu red WiFi
const char* password = "TU_WIFI_PASSWORD";   // Cambia por la contraseña de tu WiFi

// ===============================
// CONFIGURACIÓN DEL SERVIDOR EXTERNO
// ===============================
const char* servidorExterno = "http://192.168.1.100:3000"; // Cambia por la IP/URL de tu servidor
const char* endpointExterno = "/api/nodes";                 // Endpoint del servidor externo
const int timeoutHTTP = 5000;                              // Timeout para peticiones HTTP (5 segundos)

// ===============================
// CONFIGURACIÓN DEL SERVIDOR
// ===============================
WebServer server(80);  // Crear servidor en puerto 80 (HTTP estándar)

// ===============================
// VARIABLES GLOBALES
// ===============================
String lastMessage = "";      // Último mensaje recibido
String nodeStatus = "idle";   // Estado actual del nodo
int messageCount = 0;         // Contador de mensajes recibidos

// Array para almacenar datos de los nodos (máximo 50 nodos)
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

NodeData nodos[50];          // Array de hasta 50 nodos
int totalNodos = 0;          // Contador de nodos registrados
unsigned long timeoutNodo = 30000; // Timeout de nodo (30 segundos)

// ===============================
// CONFIGURACIÓN INICIAL
// ===============================
void setup() {
  // Inicializar comunicación serie para debugging
  Serial.begin(115200);
  Serial.println("\n=== INICIANDO ESP32 SERVIDOR WEB ===");
  
  // Inicializar array de nodos
  inicializarNodos();
  
  // Configurar pines si es necesario (ejemplo)
  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, LOW);
  
  // Inicializar conexión WiFi
  inicializarWiFi();
  
  // Configurar rutas del servidor web
  configurarRutasServidor();
  
  // Iniciar el servidor
  server.begin();
  Serial.println("✅ Servidor HTTP iniciado correctamente");
  Serial.println("📡 Esperando conexiones...");
}

// ===============================
// BUCLE PRINCIPAL
// ===============================
void loop() {
  // Manejar solicitudes HTTP entrantes
  server.handleClient();
  
  // Limpiar nodos expirados cada 10 segundos
  static unsigned long ultimaLimpieza = 0;
  if (millis() - ultimaLimpieza > 10000) {
    limpiarNodosExpirados();
    ultimaLimpieza = millis();
  }
  
  // Aquí puedes agregar otras tareas del ESP32
  // Por ejemplo: leer sensores, controlar actuadores, etc.
  
  // Pequeña pausa para evitar sobrecarga del procesador
  delay(10);
}

// ===============================
// FUNCIÓN: INICIALIZAR NODOS
// ===============================
void inicializarNodos() {
  Serial.println("📋 Inicializando array de nodos...");
  
  
  for (int i = 0; i < 50; i++) {
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

// ===============================
// FUNCIÓN: INICIALIZAR WIFI
// ===============================
void inicializarWiFi() {
  Serial.print("🔗 Conectando a WiFi: ");
  Serial.println(ssid);
  
  // Iniciar conexión WiFi
  WiFi.begin(ssid, password);
  
  // Esperar hasta conectar (con timeout implícito)
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 50) {
    delay(500);
    Serial.print(".");
    intentos++;
  }
  
  // Verificar si se conectó exitosamente
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado!");
    Serial.print("📍 Dirección IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("🌐 Accede desde: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Error: No se pudo conectar a WiFi");
    Serial.println("🔧 Verifica las credenciales y reinicia el ESP32");
  }
}

// ===============================
// FUNCIÓN: CONFIGURAR RUTAS DEL SERVIDOR
// ===============================
void configurarRutasServidor() {
  Serial.println("🛣️  Configurando rutas del servidor...");
  
  // RUTA 1: Página principal (GET /)
  server.on("/", HTTP_GET, []() {
    manejarPaginaPrincipal();
  });
  
  // RUTA 2: Endpoint principal para recibir mensajes (POST /api/send)
  server.on("/api/send", HTTP_POST, []() {
    manejarMensajeRecibido();
  });
  
  // RUTA 3: Endpoint de prueba (GET /api/test)
  server.on("/api/test", HTTP_GET, []() {
    manejarPruebaSistema();
  });
  
  // RUTA 4: Estado del nodo (GET /api/status)
  server.on("/api/status", HTTP_GET, []() {
    manejarEstadoNodo();
  });
  
  // RUTA 4.1: Datos de todos los nodos (GET /api/nodes)
  server.on("/api/nodes", HTTP_GET, []() {
    manejarDatosNodos();
  });
  
  // RUTA 4.2: Recibir datos de un nodo individual (POST /api/node)
  server.on("/api/node", HTTP_POST, []() {
    manejarDatoNodoIndividual();
  });
  
  // RUTA 5: Manejo de preflight CORS (OPTIONS)
  server.on("/api/send", HTTP_OPTIONS, []() {
    manejarPreflightCORS();
  });
  
  // RUTA 5.1: CORS para endpoint /api/node
  server.on("/api/node", HTTP_OPTIONS, []() {
    manejarPreflightCORS();
  });
  
  // RUTA 5.2: Endpoint simple de prueba POST
  server.on("/api/ping", HTTP_POST, []() {
    manejarPingTest();
  });
  
  // RUTA 5.3: Probar conexión con servidor externo
  server.on("/api/test-external", HTTP_GET, []() {
    manejarPruebaServidorExterno();
  });
  
  // RUTA 6: Página 404 para rutas no encontradas
  server.onNotFound([]() {
    manejarPaginaNoEncontrada();
  });
  
  Serial.println("✅ Rutas configuradas correctamente");
}

// ===============================
// MANEJADOR: PÁGINA PRINCIPAL
// ===============================
void manejarPaginaPrincipal() {
  Serial.println("📄 Solicitud GET / - Página principal");
  
  // HTML simple para mostrar información del ESP32
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>ESP32 Node Monitor</title>";
  html += "<meta charset='UTF-8'>";
  html += "</head><body>";
  html += "<h1>🔧 ESP32 Node Monitor</h1>";
  html += "<p><strong>Estado:</strong> " + nodeStatus + "</p>";
  html += "<p><strong>IP:</strong> " + WiFi.localIP().toString() + "</p>";
  html += "<p><strong>Último mensaje:</strong> " + lastMessage + "</p>";
  html += "<p><strong>Mensajes recibidos:</strong> " + String(messageCount) + "</p>";
  html += "<hr><p><em>Servidor funcionando correctamente ✅</em></p>";
  html += "</body></html>";
  
  // Enviar respuesta HTML
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", html);
}

// ===============================
// MANEJADOR: MENSAJE RECIBIDO (PRINCIPAL)
// ===============================
void manejarMensajeRecibido() {
  Serial.println("\n📨 MENSAJE RECIBIDO - POST /api/send");
  Serial.println("🔗 Cliente conectado desde: " + server.client().remoteIP().toString());
  
  // Configurar headers CORS para permitir solicitudes desde navegadores
  configurarHeadersCORS();
  
  // Verificar que se recibió contenido JSON
  if (!server.hasArg("plain")) {
    Serial.println("❌ Error: No se recibió contenido JSON");
    enviarErrorJSON("No se recibió contenido JSON");
    return;
  }
  
  // Obtener el cuerpo de la solicitud
  String cuerpoJSON = server.arg("plain");
  Serial.println("📝 JSON recibido: " + cuerpoJSON);
  
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
  // bool exito = procesarMensaje(nodeId, mensaje);
  
  // Enviar respuesta apropiada
  if (exito) {
    enviarExitoJSON("Mensaje procesado correctamente");
    messageCount++;
    lastMessage = mensaje;
    Serial.println("✅ Respuesta de éxito enviada al cliente");
  } else {
    enviarErrorJSON("Error al procesar el mensaje");
    Serial.println("❌ Respuesta de error enviada al cliente");
  }
}

// ===============================
// MANEJADOR: PRUEBA DEL SISTEMA
// ===============================
void manejarPruebaSistema() {
  Serial.println("🧪 Solicitud GET /api/test - Prueba del sistema");
  
  configurarHeadersCORS();
  
  // Crear respuesta JSON de prueba
  DynamicJsonDocument respuesta(512);
  respuesta["status"] = "ok";
  respuesta["message"] = "ESP32 funcionando correctamente";
  respuesta["ip"] = WiFi.localIP().toString();
  respuesta["uptime"] = millis();
  respuesta["freeHeap"] = ESP.getFreeHeap();
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
  server.send(200, "application/json", jsonString);
  Serial.println("✅ Respuesta de prueba enviada");
}

// ===============================
// MANEJADOR: ESTADO DEL NODO
// ===============================
void manejarEstadoNodo() {
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
  
  server.send(200, "application/json", jsonString);
}

// ===============================
// MANEJADOR: DATO DE NODO INDIVIDUAL
// ===============================
void manejarDatoNodoIndividual() {
  Serial.println("\n📦 RECIBIENDO DATOS DE NODO INDIVIDUAL - POST /api/node");
  
  configurarHeadersCORS();
  
  // Verificar que se recibió contenido JSON
  if (!server.hasArg("plain")) {
    Serial.println("❌ Error: No se recibió contenido JSON");
    enviarErrorJSON("No se recibió contenido JSON");
    return;
  }
  
  // Obtener el cuerpo de la solicitud
  String cuerpoJSON = server.arg("plain");
  Serial.println("📝 JSON de nodo recibido: " + cuerpoJSON);
  
  // Parsear el JSON recibido
  DynamicJsonDocument documento(512);
  DeserializationError error = deserializeJson(documento, cuerpoJSON);
  
  // Verificar que el JSON es válido
  if (error) {
    Serial.println("❌ Error al parsear JSON: " + String(error.c_str()));
    enviarErrorJSON("JSON inválido");
    return;
  }
  
  // Extraer datos del nodo
  String nodoId = documento["nodoId"] | "";
  String comu = documento["comu"] | "0";
  String zoneA = documento["zoneA"] | "0";
  String zoneB = documento["zoneB"] | "0";
  String output1 = documento["output1"] | "0";
  String output2 = documento["output2"] | "0";
  String fuente = documento["fuente"] | "0";
  
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
  bool actualizado = actualizarNodo(nodoId, comu, zoneA, zoneB, output1, output2, fuente);
  
  // ENVIAR DATOS AL SERVIDOR EXTERNO
  bool enviadoExterno = enviarDatosAlServidorExterno(nodoId, comu, zoneA, zoneB, output1, output2, fuente);
  
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

// ===============================
// FUNCIÓN: ACTUALIZAR NODO
// ===============================
bool actualizarNodo(String nodoId, String comu, String zoneA, String zoneB, 
                    String output1, String output2, String fuente) {
  
  // Buscar si el nodo ya existe
  int indice = -1;
  for (int i = 0; i < 50; i++) {
    if (nodos[i].nodoId == nodoId && nodos[i].isActive) {
      indice = i;
      break;
    }
  }
  
  // Si no existe, buscar un slot libre
  if (indice == -1) {
    for (int i = 0; i < 50; i++) {
      if (!nodos[i].isActive) {
        indice = i;
        totalNodos++;
        Serial.println("🆕 Nuevo nodo registrado en posición " + String(i));
        break;
      }
    }
  }
  
  // Si no hay espacio disponible
  if (indice == -1) {
    Serial.println("❌ No hay espacio para más nodos (máximo 50)");
    return false;
  }
  
  // Actualizar datos del nodo
  nodos[indice].nodoId = nodoId;
  nodos[indice].comu = comu;
  nodos[indice].zoneA = zoneA;
  nodos[indice].zoneB = zoneB;
  nodos[indice].output1 = output1;
  nodos[indice].output2 = output2;
  nodos[indice].fuente = fuente;
  nodos[indice].lastUpdate = millis();
  nodos[indice].isActive = true;
  
  return true;
}

// ===============================
// FUNCIÓN: LIMPIAR NODOS EXPIRADOS
// ===============================
void limpiarNodosExpirados() {
  unsigned long tiempoActual = millis();
  int nodosLimpiados = 0;
  
  for (int i = 0; i < 50; i++) {
    if (nodos[i].isActive && (tiempoActual - nodos[i].lastUpdate) > timeoutNodo) {
      Serial.println("⏰ Nodo " + nodos[i].nodoId + " expirado, removiendo...");
      nodos[i].isActive = false;
      nodos[i].nodoId = "";
      nodosLimpiados++;
      totalNodos--;
    }
  }
  
  if (nodosLimpiados > 0) {
    Serial.println("🧹 " + String(nodosLimpiados) + " nodos expirados removidos");
  }
}

// ===============================
// FUNCIÓN: ENVIAR DATOS AL SERVIDOR EXTERNO
// ===============================
bool enviarDatosAlServidorExterno(String nodoId, String comu, String zoneA, String zoneB, 
                                  String output1, String output2, String fuente) {
  
  Serial.println("\n🌐 ENVIANDO DATOS AL SERVIDOR EXTERNO...");
  
  // Verificar conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Error: WiFi no conectado");
    return false;
  }
  
  // Construir URL completa
  String urlCompleta = String(servidorExterno) + String(endpointExterno);
  Serial.println("🎯 URL destino: " + urlCompleta);
  
  // Configurar conexión HTTP
  http.begin(urlCompleta);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-Master/1.0");
  http.setTimeout(timeoutHTTP);
  
  // Crear JSON con los datos del nodo
  DynamicJsonDocument documento(512);
  documento["nodoId"] = nodoId;
  documento["comu"] = comu;
  documento["zoneA"] = zoneA;
  documento["zoneB"] = zoneB;
  documento["output1"] = output1;
  documento["output2"] = output2;
  documento["fuente"] = fuente;
  documento["timestamp"] = millis();
  documento["source"] = "ESP32-Master";
  
  // Convertir JSON a String
  String jsonString;
  serializeJson(documento, jsonString);
  
  Serial.println("📦 JSON enviando: " + jsonString);
  
  // Realizar petición POST
  int codigoRespuesta = http.POST(jsonString);
  
  // Procesar respuesta
  if (codigoRespuesta > 0) {
    String respuesta = http.getString();
    Serial.println("📥 Código respuesta: " + String(codigoRespuesta));
    Serial.println("📄 Respuesta servidor: " + respuesta);
    
    if (codigoRespuesta == 200 || codigoRespuesta == 201) {
      Serial.println("✅ Datos enviados exitosamente al servidor externo");
      http.end();
      return true;
    } else {
      Serial.println("⚠️  Servidor respondió con código: " + String(codigoRespuesta));
      http.end();
      return false;
    }
  } else {
    Serial.println("❌ Error en petición HTTP: " + String(codigoRespuesta));
    Serial.println("   Error: " + http.errorToString(codigoRespuesta));
    http.end();
    return false;
  }
}
// ===============================
// MANEJADOR: DATOS DE NODOS (MODIFICADO)
// ===============================
void manejarDatosNodos() {
  Serial.println("📊 Solicitud GET /api/nodes - Enviando datos almacenados");
  
  configurarHeadersCORS();
  
  // Limpiar nodos expirados antes de enviar datos
  limpiarNodosExpirados();
  
  // Crear array JSON con datos almacenados
  DynamicJsonDocument respuesta(4096);  // Buffer más grande
  JsonArray nodosArray = respuesta.createNestedArray();
  
  int nodosEnviados = 0;
  
  // Recorrer array de nodos y enviar solo los activos
  for (int i = 0; i < 50; i++) {
    if (nodos[i].isActive) {
      JsonObject nodo = nodosArray.createNestedObject();
      nodo["nodoId"] = nodos[i].nodoId;
      nodo["comu"] = nodos[i].comu;
      nodo["zoneA"] = nodos[i].zoneA;
      nodo["zoneB"] = nodos[i].zoneB;
      nodo["output1"] = nodos[i].output1;
      nodo["output2"] = nodos[i].output2;
      nodo["fuente"] = nodos[i].fuente;
      
      // Información adicional para debugging
      unsigned long tiempoTranscurrido = (millis() - nodos[i].lastUpdate) / 1000;
      Serial.println("   Nodo " + nodos[i].nodoId + 
                     " - Última actualización: " + String(tiempoTranscurrido) + "s");
      
      nodosEnviados++;
    }
  }
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
  Serial.println("📤 Enviando " + String(nodosEnviados) + " nodos activos");
  Serial.println("📊 Total nodos en memoria: " + String(totalNodos));
  
  server.send(200, "application/json", jsonString);
  Serial.println("✅ Datos de nodos enviados correctamente");
}

// ===============================
// MANEJADOR: PING TEST
// ===============================
void manejarPingTest() {
  Serial.println("🏓 Solicitud POST /api/ping - Ping test");
  
  configurarHeadersCORS();
  
  DynamicJsonDocument respuesta(256);
  respuesta["success"] = true;
  respuesta["message"] = "Pong! ESP32 respondiendo correctamente";
  respuesta["timestamp"] = millis();
  respuesta["ip"] = WiFi.localIP().toString();
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
  Serial.println("📤 Enviando pong: " + jsonString);
  server.send(200, "application/json", jsonString);
  Serial.println("✅ Ping test completado");
}

// ===============================
// MANEJADOR: PRUEBA SERVIDOR EXTERNO
// ===============================
void manejarPruebaServidorExterno() {
  Serial.println("🌐 Solicitud GET /api/test-external - Probando servidor externo");
  
  configurarHeadersCORS();
  
  // Probar conexión con datos de prueba
  bool conexionExitosa = enviarDatosAlServidorExterno("TEST", "1", "0", "1", "0", "1", "1");
  
  DynamicJsonDocument respuesta(512);
  respuesta["success"] = conexionExitosa;
  respuesta["serverUrl"] = String(servidorExterno) + String(endpointExterno);
  respuesta["timestamp"] = millis();
  
  if (conexionExitosa) {
    respuesta["message"] = "Conexión exitosa con servidor externo";
    respuesta["status"] = "connected";
  } else {
    respuesta["message"] = "Error al conectar con servidor externo";
    respuesta["status"] = "disconnected";
    respuesta["troubleshooting"] = "Verificar URL, red y que el servidor esté funcionando";
  }
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
  server.send(conexionExitosa ? 200 : 500, "application/json", jsonString);
  Serial.println(conexionExitosa ? "✅ Prueba servidor externo OK" : "❌ Prueba servidor externo FALLO");
}

// ===============================
// MANEJADOR: PREFLIGHT CORS
// ===============================
void manejarPreflightCORS() {
  Serial.println("🔄 Solicitud OPTIONS - Preflight CORS");
  
  configurarHeadersCORS();
  server.send(200, "text/plain", "");
}

// ===============================
// MANEJADOR: PÁGINA NO ENCONTRADA
// ===============================
void manejarPaginaNoEncontrada() {
  Serial.println("❓ Ruta no encontrada: " + server.uri());
  
  configurarHeadersCORS();
  
  DynamicJsonDocument respuesta(256);
  respuesta["error"] = "Ruta no encontrada";
  respuesta["path"] = server.uri();
  respuesta["method"] = server.method();
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
  server.send(404, "application/json", jsonString);
}

// ===============================
// FUNCIÓN: CONFIGURAR HEADERS CORS
// ===============================
void configurarHeadersCORS() {
  Serial.println("🌐 Configurando headers CORS...");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
  server.sendHeader("Access-Control-Max-Age", "86400");
  server.sendHeader("Connection", "close");
  Serial.println("✅ Headers CORS configurados");
}

// ===============================
// FUNCIÓN: ENVIAR ERROR JSON
// ===============================
void enviarErrorJSON(String mensaje) {
  Serial.println("📤 Preparando respuesta de error...");
  
  configurarHeadersCORS();  // Asegurar CORS en la respuesta
  
  DynamicJsonDocument respuesta(256);
  respuesta["success"] = false;
  respuesta["error"] = mensaje;
  respuesta["timestamp"] = millis();
  respuesta["nodeId"] = "1";  // ID del nodo que responde
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
  Serial.println("📤 Enviando JSON de error: " + jsonString);
  server.send(400, "application/json", jsonString);
  Serial.println("❌ Error enviado: " + mensaje);
}

// ===============================
// FUNCIÓN: ENVIAR ÉXITO JSON
// ===============================
void enviarExitoJSON(String mensaje) {
  Serial.println("📤 Preparando respuesta de éxito...");
  
  configurarHeadersCORS();  // Asegurar CORS en la respuesta
  
  DynamicJsonDocument respuesta(256);
  respuesta["success"] = true;
  respuesta["message"] = mensaje;
  respuesta["timestamp"] = millis();
  respuesta["nodeId"] = "1";  // ID del nodo que responde
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
  Serial.println("📤 Enviando JSON de éxito: " + jsonString);
  server.send(200, "application/json", jsonString);
  Serial.println("✅ Éxito enviado: " + mensaje);
}

// ===============================
// FIN DEL CÓDIGO
// ===============================

/*
 * ==========================================
 * INSTRUCCIONES DE INSTALACIÓN Y USO
 * ==========================================
 * 
 * 1. INSTALAR LIBRERÍAS EN ARDUINO IDE:
 *    - WiFi.h (incluida en ESP32)
 *    - WebServer.h (incluida en ESP32)
 *    - ArduinoJson.h (buscar en Library Manager)
 *    - HTTPClient.h (incluida en ESP32)
 * 
 * 2. CONFIGURAR CREDENCIALES WIFI:
 *    - Cambiar "TU_WIFI_NOMBRE" por el nombre de tu red
 *    - Cambiar "TU_WIFI_PASSWORD" por tu contraseña
 * 
 * 3. CONFIGURAR SERVIDOR EXTERNO:
 *    - Cambiar "http://192.168.1.100:3000" por la URL de tu servidor
 *    - Cambiar "/api/nodes" por el endpoint correcto
 *    - Ajustar timeout si es necesario
 * 
 * 4. CARGAR CÓDIGO AL ESP32:
 *    - Seleccionar la placa ESP32 en Arduino IDE
 *    - Compilar y subir el código
 * 
 * 5. VERIFICAR FUNCIONAMIENTO:
 *    - Abrir Monitor Serie (115200 baudios)
 *    - Anotar la dirección IP que aparece
 *    - Probar endpoint /api/test-external para verificar conexión
 * 
 * 6. PROBAR CONEXIÓN:
 *    - Acceder a http://IP_DEL_ESP32 en el navegador
 *    - Usar /api/test-external para probar servidor externo
 *    - Usar NetworkDiagnostics en la interfaz web
 * 
 * ==========================================
 * NUEVOS ENDPOINTS DISPONIBLES
 * ==========================================
 * 
 * POST /api/node - Recibir datos de nodo (ahora envía también al servidor externo)
 * GET /api/test-external - Probar conexión con servidor externo
 * 
 * ==========================================
 * ARQUITECTURA DE COMUNICACIÓN
 * ==========================================
 * 
 * [Nodos LoRa] → [ESP32 Maestro] → [Servidor Externo]
 *                      ↓               ↓
 *                 Almacena local    node-monitor-web
 *                      ↓
 *                [Interfaz React] (backup/local)
 * 
 * ==========================================
 * FORMATO DE MENSAJES SOPORTADOS
 * ==========================================
 * 
 * Estructura: N[nodeId][función][número][param1][param2]
 * 
 * Ejemplos:
 * - N1A123: Nodo 1, Función A, Número 1, Parámetros 2,3
 * - N2B456: Nodo 2, Función B, Número 4, Parámetros 5,6
 * - N1D000: Nodo 1, Reset configuración
 * - N1E999: Nodo 1, Reset completo
 * 
 * ==========================================
 */
