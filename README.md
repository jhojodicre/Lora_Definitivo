# 🚀 Sistema de Seguridad Perimetral LoRa - Servidor Web

## 📋 Descripción General

Este proyecto implementa un servidor web integrado en un ESP32 para controlar un sistema de seguridad perimetral basado en comunicación LoRa. El servidor permite la configuración, monitoreo y control remoto de nodos LoRa a través de una interfaz web moderna y APIs REST.

## 🏗️ Arquitectura del Sistema

### Componentes Principales

1. **ESP32** - Microcontrolador principal
2. **Módulo LoRa** - Comunicación de largo alcance
3. **WiFi** - Conexión a red local
4. **Servidor Web** - Interfaz de control y APIs

### Clases del Sistema

- `LoRaWebServer` - Manejo del servidor web y rutas
- `Lora` - Control del módulo LoRa y comunicaciones
- `Master` - Lógica del nodo maestro
- `Functions` - Ejecución de comandos y funciones

## 🌐 Servidor Web - Estructura y Funcionamiento

### Ubicación del Código

**Archivos principales:**
- `NodeWebServer.cpp` - Implementación del servidor web
- `NodeWebServer.h` - Declaraciones de la clase

### Inicialización del Servidor

```cpp
// En Lora_Definitivo.ino
LoRaWebServer webServer(80);  // Puerto 80

void setup() {
    // ... otras configuraciones ...
    webServer.begin(&Node, &Master, &Correr);
}

void loop() {
    webServer.handle();  // Procesar peticiones HTTP
}
```

## 🛣️ Sistema de Rutas (Endpoints)

### Configuración de Rutas

Las rutas se definen centralizadamente en la función `configurarRutasServidor()`:

```cpp
void LoRaWebServer::configurarRutasServidor() {
    Serial.println("🛣️  Configurando rutas del servidor...");
    
    // Definición de todas las rutas
    server->on("/", HTTP_GET, [this]() { handleRoot(); });
    server->on("/api/send", HTTP_POST, [this]() { manejarMensajeRecibido(); });
    // ... más rutas ...
}
```

### 📍 Mapa Completo de Endpoints

| Método | Endpoint | Función Handler | Descripción |
|--------|----------|----------------|-------------|
| GET       | `/`         | `handleRoot()` | Página principal HTML |
| GET       | `/api`      | `handleAPI()` | Información básica del sistema |
| GET       | `/api/status` | `handleGetStatus()` | Estado completo del nodo |
| GET       | `/api/test` | `manejarPruebaSistema()` | Test del sistema |
| POST      | `/api/send` | `manejarMensajeRecibido()` | Enviar mensajes al sistema |
| POST      | `/control` | `handleNodeControl()` | Control de comandos |
| POST      | `/set-address` | `handleSetAddress()` | Cambiar dirección del nodo |
| GET       | `/hola-mundo` | `manejarHolaMundo()` | Saludo de prueba del sistema |
| OPTIONS   | `/api/send` | `manejarPreflightCORS()` | Preflight CORS |

---

## 📱 Endpoints Detallados

### 1. **GET /** - Página Principal
**Función:** `handleRoot()`  
**Propósito:** Servir la interfaz web principal

**Características:**
- Interfaz HTML moderna con CSS responsivo
- Control en tiempo real del sistema
- Botones para comandos predefinidos
- Visualización del estado del nodo

**Respuesta:** HTML completo con JavaScript integrado

---

### 2. **GET /api** - Información Básica
**Función:** `handleAPI()`  
**Propósito:** Obtener información básica del sistema

**Respuesta JSON:**
```json
{
    "system": "LoRa Communication System",
    "version": "1.0",
    "uptime": 123456,
    "wifi": {
        "ssid": "ANTEL_0322",
        "ip": "192.168.1.100",
        "rssi": -45
    },
    "node": {
        "address": "A",
        "mode": "MASTER"
    }
}
```

---

### 3. **GET /api/status** - Estado Completo
**Función:** `handleGetStatus()`  
**Propósito:** Obtener estado detallado del sistema

**Respuesta JSON:**
```json
{
    "timestamp": 123456,
    "system": {
        "name": "LoRa Security System",
        "version": "1.0",
        "uptime_ms": 123456
    },
    "wifi": {
        "ssid": "ANTEL_0322",
        "ip": "192.168.1.100",
        "rssi": -45,
        "status": "connected"
    },
    "node": {
        "address_char": "A",
        "address_ascii": 65,
        "mode": "MASTER"
    },
    "master": {
        "node_count": 1,
        "next_node": 2
    }
}
```

---

### 4. **GET /api/test** - Test del Sistema
**Función:** `manejarPruebaSistema()`  
**Propósito:** Verificar el estado de todos los componentes

**Respuesta JSON:**
```json
{
    "test_status": "OK",
    "system_name": "LoRa Security System",
    "uptime_seconds": 3600,
    "free_heap": 234567,
    "chip_id": "ESP32-D0WD-V3",
    "components": {
        "wifi": true,
        "node": true,
        "master": true,
        "functions": true
    },
    "node_info": {
        "address": "A",
        "mode": "MASTER"
    }
}
```

---

### 5. **POST /api/send** - Enviar Mensajes
**Función:** `manejarMensajeRecibido()`  
**Propósito:** Procesar mensajes y comandos

**Parámetros:**
- `message` (string): Comando a ejecutar

**Ejemplo de uso:**
```javascript
fetch('/api/send', {
    method: 'POST',
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body: 'message=C110'
})
```

**Respuesta JSON:**
```json
{
    "success": true,
    "message": "Mensaje procesado correctamente",
    "received_message": "C110",
    "timestamp": 123456
}
```

---

### 6. **POST /control** - Control de Comandos
**Función:** `handleNodeControl()`  
**Propósito:** Ejecutar comandos específicos del nodo

**Parámetros:**
- `command` (string): Comando a ejecutar (mínimo 2 caracteres)

**Validaciones:**
- Longitud mínima: 2 caracteres
- Auto-completado a 6 caracteres con "0"

**Respuesta:** Texto plano con confirmación

---

### 7. **POST /set-address** - Cambiar Dirección
**Función:** `handleSetAddress()`  
**Propósito:** Modificar la dirección del nodo

**Parámetros:**
- `address` (char): Nueva dirección (A-Z, 1-9)

**Validaciones:**
- Solo un carácter
- Rango permitido: A-Z o 1-9

**Respuesta JSON:**
```json
{
    "success": true,
    "new_address": "B",
    "ascii_value": 66
}
```

---

### 8. **GET /hola-mundo** - Saludo del Sistema
**Función:** `manejarHolaMundo()`  
**Propósito:** Endpoint de prueba que saluda y muestra información básica del sistema

**Características:**
- No requiere parámetros
- Respuesta amigable en JSON
- Información del estado actual
- Ideal para testing y verificación

**Ejemplo de uso:**
```javascript
fetch('/hola-mundo')
.then(response => response.json())
.then(data => console.log(data));
```

**Respuesta JSON:**
```json
{
    "message": "¡Hola Mundo desde el ESP32!",
    "sistema": "LoRa Security System",
    "timestamp": 123456,
    "uptime_segundos": 3600,
    "nodo_actual": "A",
    "modo": "MASTER",
    "status": "🚀 Sistema funcionando correctamente"
}
```

---

### 9. **OPTIONS /api/send** - CORS Preflight
**Función:** `manejarPreflightCORS()`  
**Propósito:** Manejar peticiones preflight para CORS

**Headers configurados:**
- `Access-Control-Allow-Origin: *`
- `Access-Control-Allow-Methods: POST, GET, OPTIONS`
- `Access-Control-Allow-Headers: Content-Type`

---

## 🔄 Flujo de Interacción

### 1. **Inicialización**
```
ESP32 Setup → WiFi Connect → Web Server Start → Routes Config
```

### 2. **Petición HTTP**
```
Cliente HTTP → ESP32 WebServer → Route Handler → Response
```

### 3. **Procesamiento de Comandos**
```
Web Interface → POST /control → Functions Class → LoRa Transmission
```

### 4. **Actualización de Estado**
```
LoRa Reception → Node Update → GET /api/status → Updated Response
```

## 🌍 Acceso al Sistema

### URLs de Acceso
- **Interfaz Principal:** `http://[IP_ESP32]/`
- **API Básica:** `http://[IP_ESP32]/api`
- **Estado Completo:** `http://[IP_ESP32]/api/status`
- **Test Sistema:** `http://[IP_ESP32]/api/test`
- **Hola Mundo:** `http://[IP_ESP32]/hola-mundo`

### Ejemplo de IP
Si el ESP32 obtiene la IP `192.168.1.100`:
- Interfaz web: `http://192.168.1.100/`
- API status: `http://192.168.1.100/api/status`
- Hola mundo: `http://192.168.1.100/hola-mundo`

## 🛠️ Desarrollo y Personalización

### Agregar Nuevos Endpoints

1. **Definir la ruta en `configurarRutasServidor()`:**
```cpp
server->on("/nuevo-endpoint", HTTP_GET, [this]() {
    manejarNuevoEndpoint();
});
```

2. **Implementar el handler:**
```cpp
void LoRaWebServer::manejarNuevoEndpoint() {
    // Lógica del endpoint
    server->send(200, "application/json", "{}");
}
```

3. **Declarar en el header `NodeWebServer.h`:**
```cpp
void manejarNuevoEndpoint();
```

### Modificar la Interfaz Web

La interfaz HTML se encuentra en la función `handleRoot()` usando raw string literals:
```cpp
String html = R"rawliteral(
<!DOCTYPE html>
<html>
<!-- HTML aquí -->
</html>
)rawliteral";
```

## 🔐 Seguridad y CORS

### Configuración CORS
- Permite peticiones desde cualquier origen (`*`)
- Métodos permitidos: GET, POST, OPTIONS
- Headers permitidos: Content-Type

### Validaciones Implementadas
- Validación de parámetros requeridos
- Validación de longitud de comandos
- Validación de caracteres permitidos
- Verificación de disponibilidad de componentes

## 📊 Monitoreo y Debug

### Logging Serial
Todos los endpoints generan logs informativos:
```
🛣️  Configurando rutas del servidor...
✅ Rutas configuradas correctamente
📨 Mensaje recibido via API: C110
🧪 Test del sistema ejecutado via API
```

### Estados de Respuesta HTTP
- `200` - Éxito
- `400` - Error de parámetros
- `404` - Endpoint no encontrado
- `500` - Error interno del servidor

## 📚 Tecnologías Utilizadas

- **ESP32 Arduino Core** - Framework base
- **WebServer Library** - Servidor HTTP
- **ArduinoJson** - Manejo de JSON
- **WiFi Library** - Conectividad de red
- **HTML5/CSS3/JavaScript** - Interfaz web

---

*Desarrollado para el Sistema de Seguridad Perimetral - Barrio San Diego*
