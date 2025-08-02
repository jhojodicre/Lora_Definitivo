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

// ===============================
// CONFIGURACIÓN DE RED WIFI
// ===============================
const char* ssid = "TU_WIFI_NOMBRE";         // Cambia por el nombre de tu red WiFi
const char* password = "TU_WIFI_PASSWORD";   // Cambia por la contraseña de tu WiFi

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

// ===============================
// CONFIGURACIÓN INICIAL
// ===============================
void setup() {
  // Inicializar comunicación serie para debugging
  Serial.begin(115200);
  Serial.println("\n=== INICIANDO ESP32 SERVIDOR WEB ===");
  
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
  
  // Aquí puedes agregar otras tareas del ESP32
  // Por ejemplo: leer sensores, controlar actuadores, etc.
  
  // Pequeña pausa para evitar sobrecarga del procesador
  delay(10);
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
  
  // RUTA 5: Manejo de preflight CORS (OPTIONS)
  server.on("/api/send", HTTP_OPTIONS, []() {
    manejarPreflightCORS();
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

// ===============================
// FUNCIÓN: PROCESAR MENSAJE
// ===============================
bool procesarMensaje(String nodeId, String mensaje) {
  Serial.println("\n🔄 PROCESANDO MENSAJE...");
  
  // Verificar formato básico del mensaje (debe ser N + nodeId + función + números)
  String prefijoEsperado = "N" + nodeId;
  
  if (!mensaje.startsWith(prefijoEsperado)) {
    Serial.println("❌ El mensaje no corresponde a este nodo");
    Serial.println("   Esperado: " + prefijoEsperado + "...");
    Serial.println("   Recibido: " + mensaje);
    return false;
  }
  
  // Verificar longitud del mensaje (debe ser exactamente 6 caracteres)
  if (mensaje.length() != 6) {
    Serial.println("❌ Longitud de mensaje incorrecta: " + String(mensaje.length()));
    Serial.println("   Debe ser exactamente 6 caracteres");
    return false;
  }
  
  // Extraer componentes del mensaje
  // Formato: N[nodeId][función][número][param1][param2]
  // Ejemplo: N1A123 = Nodo 1, Función A, Número 1, Parámetros 2,3
  
  char funcion = mensaje.charAt(2);    // Tercer carácter (A, B, C, D, E)
  char numero = mensaje.charAt(3);     // Cuarto carácter (0-9)
  char param1 = mensaje.charAt(4);     // Quinto carácter (0-9)
  char param2 = mensaje.charAt(5);     // Sexto carácter (0-9)
  
  Serial.println("📋 DESGLOSE DEL MENSAJE:");
  Serial.println("   Nodo: " + nodeId);
  Serial.println("   Función: " + String(funcion));
  Serial.println("   Número: " + String(numero));
  Serial.println("   Parámetro 1: " + String(param1));
  Serial.println("   Parámetro 2: " + String(param2));
  
  // Ejecutar acción según la función
  bool resultado = ejecutarFuncion(funcion, numero, param1, param2);
  
  if (resultado) {
    Serial.println("✅ Mensaje procesado exitosamente");
    nodeStatus = "active";
  } else {
    Serial.println("❌ Error al ejecutar la función");
    nodeStatus = "error";
  }
  
  return resultado;
}

// ===============================
// FUNCIÓN: EJECUTAR FUNCIÓN
// ===============================
bool ejecutarFuncion(char funcion, char numero, char param1, char param2) {
  Serial.println("⚙️  EJECUTANDO FUNCIÓN: " + String(funcion));
  
  // Convertir caracteres a números para facilitar el procesamiento
  int num = numero - '0';    // Convertir char a int
  int p1 = param1 - '0';     // Convertir char a int
  int p2 = param2 - '0';     // Convertir char a int
  
  switch (funcion) {
    case 'A':
      return ejecutarFuncionA(num, p1, p2);
      
    case 'B':
      return ejecutarFuncionB(num, p1, p2);
      
    case 'C':
      return ejecutarFuncionC(num, p1, p2);
      
    case 'D':
      return ejecutarFuncionD(num, p1, p2);
      
    case 'E':
      return ejecutarFuncionE(num, p1, p2);
      
    default:
      Serial.println("❌ Función desconocida: " + String(funcion));
      return false;
  }
}

// ===============================
// FUNCIÓN A: CONTROL ZONA A
// ===============================
bool ejecutarFuncionA(int numero, int param1, int param2) {
  Serial.println("🔵 FUNCIÓN A - Control Zona A");
  Serial.println("   Número: " + String(numero));
  Serial.println("   Parámetros: " + String(param1) + ", " + String(param2));
  
  // AQUÍ IMPLEMENTA TU LÓGICA ESPECÍFICA PARA LA ZONA A
  // Ejemplos:
  // - Controlar relés
  // - Activar motores
  // - Configurar sensores
  
  // Ejemplo básico: controlar LED según parámetros
  /*
  if (param1 == 1) {
    digitalWrite(LED_BUILTIN, HIGH);  // Encender LED
    Serial.println("   LED encendido");
  } else {
    digitalWrite(LED_BUILTIN, LOW);   // Apagar LED
    Serial.println("   LED apagado");
  }
  */
  
  Serial.println("✅ Función A ejecutada correctamente");
  return true;  // Retornar true si la operación fue exitosa
}

// ===============================
// FUNCIÓN B: CONTROL ZONA B
// ===============================
bool ejecutarFuncionB(int numero, int param1, int param2) {
  Serial.println("🟢 FUNCIÓN B - Control Zona B");
  Serial.println("   Número: " + String(numero));
  Serial.println("   Parámetros: " + String(param1) + ", " + String(param2));
  
  // AQUÍ IMPLEMENTA TU LÓGICA ESPECÍFICA PARA LA ZONA B
  
  Serial.println("✅ Función B ejecutada correctamente");
  return true;
}

// ===============================
// FUNCIÓN C: CONTROL SALIDAS
// ===============================
bool ejecutarFuncionC(int numero, int param1, int param2) {
  Serial.println("🟡 FUNCIÓN C - Control Salidas Digitales");
  Serial.println("   Número: " + String(numero));
  Serial.println("   Parámetros: " + String(param1) + ", " + String(param2));
  
  // AQUÍ IMPLEMENTA TU LÓGICA PARA CONTROL DE SALIDAS
  // Ejemplo: controlar múltiples salidas digitales
  
  Serial.println("✅ Función C ejecutada correctamente");
  return true;
}

// ===============================
// FUNCIÓN D: CONFIGURACIÓN
// ===============================
bool ejecutarFuncionD(int numero, int param1, int param2) {
  Serial.println("🟠 FUNCIÓN D - Configuración del Sistema");
  Serial.println("   Número: " + String(numero));
  Serial.println("   Parámetros: " + String(param1) + ", " + String(param2));
  
  // AQUÍ IMPLEMENTA LÓGICA DE CONFIGURACIÓN
  // Ejemplos:
  // - Cambiar configuraciones
  // - Guardar parámetros en EEPROM
  // - Reiniciar configuraciones
  
  if (numero == 0 && param1 == 0 && param2 == 0) {
    Serial.println("   Reset de configuración solicitado");
    // Implementar reset aquí
  }
  
  Serial.println("✅ Función D ejecutada correctamente");
  return true;
}

// ===============================
// FUNCIÓN E: RESET SISTEMA
// ===============================
bool ejecutarFuncionE(int numero, int param1, int param2) {
  Serial.println("🔴 FUNCIÓN E - Reset del Sistema");
  Serial.println("   Número: " + String(numero));
  Serial.println("   Parámetros: " + String(param1) + ", " + String(param2));
  
  // AQUÍ IMPLEMENTA LÓGICA DE RESET
  
  if (numero == 9 && param1 == 9 && param2 == 9) {
    Serial.println("   ⚠️  RESET COMPLETO SOLICITADO");
    // Implementar reset completo aquí
    // ESP.restart();  // Descomentar para reiniciar el ESP32
  }
  
  Serial.println("✅ Función E ejecutada correctamente");
  return true;
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
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
  server.sendHeader("Access-Control-Max-Age", "86400");
}

// ===============================
// FUNCIÓN: ENVIAR ERROR JSON
// ===============================
void enviarErrorJSON(String mensaje) {
  DynamicJsonDocument respuesta(256);
  respuesta["success"] = false;
  respuesta["error"] = mensaje;
  respuesta["timestamp"] = millis();
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
  server.send(400, "application/json", jsonString);
  Serial.println("❌ Error enviado: " + mensaje);
}

// ===============================
// FUNCIÓN: ENVIAR ÉXITO JSON
// ===============================
void enviarExitoJSON(String mensaje) {
  DynamicJsonDocument respuesta(256);
  respuesta["success"] = true;
  respuesta["message"] = mensaje;
  respuesta["timestamp"] = millis();
  
  String jsonString;
  serializeJson(respuesta, jsonString);
  
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
 * 
 * 2. CONFIGURAR CREDENCIALES WIFI:
 *    - Cambiar "TU_WIFI_NOMBRE" por el nombre de tu red
 *    - Cambiar "TU_WIFI_PASSWORD" por tu contraseña
 * 
 * 3. CARGAR CÓDIGO AL ESP32:
 *    - Seleccionar la placa ESP32 en Arduino IDE
 *    - Compilar y subir el código
 * 
 * 4. VERIFICAR FUNCIONAMIENTO:
 *    - Abrir Monitor Serie (115200 baudios)
 *    - Anotar la dirección IP que aparece
 *    - Actualizar la IP en tu interfaz web
 * 
 * 5. PROBAR CONEXIÓN:
 *    - Acceder a http://IP_DEL_ESP32 en el navegador
 *    - Usar NetworkDiagnostics en la interfaz web
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


        String    nodeJS    = "nodoId";
        String    commJS    = "comu";
        String    zoneAJS   = "zoneA";
        String    zoneBJS   = "zoneB";
        String    output1JS = "output1";
        String    output2JS = "output2";
        String    fuenteJS  = "fuente";