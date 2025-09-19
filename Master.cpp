#include "Master.h"
#include <Arduino.h>
#include <Ticker.h>

// Temporizadores para la gestión del protocolo
Ticker timer_master;        // Temporizador principal para consulta de nodos
Ticker timer_No_Response;   // Temporizador para timeout de respuesta de nodos

// Puntero global al objeto Master para uso en funciones estáticas
Master* masterInstance = nullptr;

/**
 * @brief Constructor principal de la clase Master
 * @param mode_master True si es Master, False si es Nodo
 * @param nodo_number Número de nodos si es Master, ID propio si es Nodo
 */
Master::Master(bool mode_master, int nodo_number) {
    // Inicialización de variables principales
    Mode = mode_master;
    nodeNumber = nodo_number;
    
    // Configuración de la secuencia de nodos
    if (Mode) {
        // En modo Master, nodeNumber indica la cantidad de nodos
        Nodo_Ultimo = nodeNumber;
        Nodo_Primero = 1;
        Nodo_Proximo = 0;
        Nodo_Consultado = 0; // Inicia en 0 para que el primer nodo sea el 1
        
        // Inicialización del arreglo de estados de nodos
        for (int i = 0; i < 10; i++) {
            estadosNodos[i].responde = false;
            estadosNodos[i].ultimoEstado = 0;
            estadosNodos[i].ultimaRespuesta = 0;
            estadosNodos[i].intentos = 0;
        }
    }
    
    // Establece referencia global para funciones estáticas
    masterInstance = this;
    
    // Inicialización de flags
    Next = false;
    nodeResponde = false;
    firstScan = true;
}

/**
 * @brief Constructor para almacenar estado de un nodo
 */
Master::Master(String nodoNumero, String ZonaA_status, String ZonaB_status, String Fuente_in_status) {
    // Almacenamiento de estados del nodo
    nodo_Number = nodoNumero;
    Zone_A = ZonaA_status;
    Zone_B = ZonaB_status;
    Fuente = Fuente_in_status;
    
    // Prepara el string concatenado para la base de datos
    Node_DB = nodo_Number + Zone_A + Zone_B + Fuente;
}

/**
 * @brief Inicializa el protocolo y los temporizadores
 */
void Master::Iniciar() {
    // En modo Master, inicia el temporizador de consulta periódica
    if (Mode) {
        Serial.println("Iniciando protocolo Master");
        timer_master.attach(5, timer_master_ISR); // Llama a la función de temporizador cada 5 segundos
        
        // Imprime información de configuración
        Serial.print("Total de nodos configurados: ");
        Serial.println(Nodo_Ultimo);
        Serial.println("Esperando primer ciclo de consulta...");
    } else {
        Serial.print("Iniciando modo Nodo ID: ");
        Serial.println(nodeNumber);
    }
}
/**
 * @brief Configura los parámetros del protocolo
 */
void Master::Configuracion() {
    // Esta función podría permitir cambiar parámetros en tiempo de ejecución
    // Por ejemplo: intervalos de consulta, timeouts, etc.
    
    // Por ahora, solo imprime la configuración actual
    if (Mode) {
        Serial.println("Configuración actual del protocolo Master:");
        Serial.print("- Nodos en la red: ");
        Serial.println(Nodo_Ultimo);
        Serial.print("- Nodo inicial: ");
        Serial.println(Nodo_Primero);
        Serial.print("- Tiempo entre consultas: ");
        Serial.println("5 segundos");
    }
}

/**
 * @brief Gestiona el ciclo principal del protocolo
 */
void Master::Gestion() {
    // Esta función podría llamarse desde el loop() principal
    // para manejar lógica adicional no basada en interrupciones
    
    // Por ejemplo, verificar nodos que no han respondido en mucho tiempo
    if (Mode) {
        unsigned long tiempoActual = millis();
        
        // Revisar todos los nodos registrados
        for (int i = 1; i <= Nodo_Ultimo; i++) {
            // Si un nodo no ha respondido en más de 30 segundos (y antes respondía)
            if (estadosNodos[i].responde && 
                (tiempoActual - estadosNodos[i].ultimaRespuesta > 30000)) {
                
                // Marcar como que no responde
                estadosNodos[i].responde = false;
                estadosNodos[i].ultimoEstado = 0;
                
                // Informar del cambio
                Serial.print("Nodo ");
                Serial.print(i);
                Serial.println(" ha dejado de responder");
            }
        }
    }
}

/**
 * @brief Determina el siguiente nodo a consultar en secuencia cíclica
 */
void Master::Nodo_REQUEST() {
    // Si llegamos al último nodo, volvemos al primero
    if (Nodo_Proximo == Nodo_Ultimo) {
        Nodo_Proximo = Nodo_Primero - 1;
    }
    
    // Avanzamos al siguiente nodo
    if (Nodo_Proximo <= Nodo_Ultimo) {
        ++Nodo_Proximo;
        Nodo_Consultado = Nodo_Proximo;
        
        // Registrar que estamos consultando este nodo
        estadosNodos[Nodo_Consultado].intentos++;
        
        // Información de depuración
        Serial.print("Consultando nodo: ");
        Serial.println(Nodo_Consultado);
    }
}

/**
 * @brief Prepara la consulta al siguiente nodo
 */
void Master::Master_Nodo() {
    // Primero determinamos cuál es el siguiente nodo a consultar
    Nodo_REQUEST();
    
    // Verificar si hay algún nodo en alerta que deba tener prioridad
    for (int i = 1; i <= Nodo_Ultimo; i++) {
        if (NodoEnAlerta(i)) {
            // Si hay un nodo en alerta, lo consultamos con prioridad
            Nodo_Consultado = i;
            Serial.print("Prioridad: Nodo en alerta ");
            Serial.println(i);
            break;
        }
    }
    
    // Preparamos el mensaje para el nodo seleccionado
    Master_Mensaje();
    
    // Registramos el intento de comunicación
    Serial.print("Master consulta a nodo: ");
    Serial.println(Nodo_Consultado);
}

/**
 * @brief Prepara el mensaje para el nodo consultado
 */
void Master::Master_Mensaje() {
    // El mensaje básico es simplemente el ID del nodo consultado
    mensaje = Nodo_Consultado;
    
    // Aquí podrías implementar lógica adicional para mensajes especiales
    // Por ejemplo: comandos específicos para cada nodo según su estado
}

/**
 * @brief Maneja la secuencia de consulta a nodos
 */
void Master::Secuencia() {
    // Esta función puede implementar lógicas más avanzadas para la secuencia
    // Por ejemplo: saltar nodos que no responden después de varios intentos
    
    // Por ahora solo hace logging
    if (firstScan) {
        Serial.println("Iniciando primer ciclo de consulta a nodos");
        firstScan = false;
    }
    
    // Podríamos implementar lógica para optimizar la secuencia:
    // - Saltar temporalmente nodos que no responden después de N intentos
    // - Consultar con mayor frecuencia nodos en estado crítico
    // - Alternar entre consultas rápidas y consultas completas
}
/**
 * @brief ISR para el temporizador de consulta periódica
 * Esta función se llama automáticamente por el temporizador
 */
void Master::timer_master_ISR() {
    // Como es una función estática, usamos el puntero global para acceder a la instancia
    if (masterInstance) {
        // Activamos la bandera para indicar que es momento de consultar al siguiente nodo
        masterInstance->Next = true;
        
        // También podríamos ejecutar lógica adicional aquí, pero es mejor mantener
        // las ISRs lo más cortas posible
    }
}

/**
 * @brief Procesa una petición del Master a un Nodo
 */
void Master::Master_Request() {
    // Esta función implementa la lógica para manejar peticiones especiales
    // a nodos específicos (no solo la consulta regular)
    
    // Por ejemplo, podría enviar comandos como:
    // - Solicitar estado detallado
    // - Activar/desactivar funciones específicas
    // - Actualizar configuración
    
    Serial.println("Procesando petición especial");
    
    // Aquí se podría implementar una cola de peticiones especiales
    // para enviar cuando llegue el turno de cada nodo
}

/**
 * @brief Actualiza el estado de un nodo
 */
void Master::Nodo_Status(String nodeNumber_st, String zonaA_st, String zonaB_st, String fuente_st) {
    // Guardamos los datos recibidos del nodo
    nodo_Number = nodeNumber_st;
    Zone_A = zonaA_st;
    Zone_B = zonaB_st;
    Fuente = fuente_st;
    
    // Creamos la cadena concatenada para almacenamiento/transmisión
    Node_DB = nodo_Number + Zone_A + Zone_B + Fuente;
    
    // Convertimos el número de nodo a entero para actualizar su estado
    int nodoID = nodo_Number.toInt();
    if (nodoID > 0 && nodoID <= Nodo_Ultimo) {
        // Actualizar el estado del nodo en el arreglo de estados
        estadosNodos[nodoID].responde = true;
        estadosNodos[nodoID].ultimaRespuesta = millis();
        
        // Determinar si el nodo está en alerta (si Zona A o B están en "1")
        if (zonaA_st == "1" || zonaB_st == "1") {
            estadosNodos[nodoID].ultimoEstado = 2; // Alerta
            Serial.print("¡ALERTA! Nodo ");
            Serial.print(nodoID);
            Serial.println(" reporta activación de zona");
        } else {
            estadosNodos[nodoID].ultimoEstado = 1; // Normal
        }
        
        // Reset del contador de intentos
        estadosNodos[nodoID].intentos = 0;
    }
    
    // Mostramos la información recibida
    Serial.print("Nodo: ");
    Serial.print(nodo_Number);
    Serial.print(" | Zona A: ");
    Serial.print(Zone_A);
    Serial.print(" | Zona B: ");
    Serial.print(Zone_B);
    Serial.print(" | Fuente: ");
    Serial.println(Fuente);
}

/**
 * @brief Actualiza la base de datos del Master con información de nodos
 */
void Master::Master_DB() {
    // Esta función podría implementar la lógica para almacenar
    // los datos de todos los nodos de forma persistente
    
    // Por ejemplo:
    // - Guardar en memoria flash/EEPROM
    // - Enviar a un servidor central
    // - Actualizar una base de datos local
    
    // Por ahora solo muestra un resumen del estado de los nodos
    Serial.println("Estado actual de nodos:");
    for (int i = 1; i <= Nodo_Ultimo; i++) {
        Serial.print("Nodo ");
        Serial.print(i);
        Serial.print(": ");
        
        if (estadosNodos[i].responde) {
            if (estadosNodos[i].ultimoEstado == 2) {
                Serial.println("EN ALERTA");
            } else {
                Serial.println("Normal");
            }
        } else {
            Serial.print("Sin respuesta (");
            Serial.print(estadosNodos[i].intentos);
            Serial.println(" intentos)");
        }
    }
}

/**
 * @brief Procesa un mensaje recibido y determina acciones
 */
bool Master::ProcesarMensaje(int origen, String mensaje) {
    // Esta función analiza mensajes recibidos y determina acciones especiales
    
    // Registrar que el nodo ha respondido
    if (origen > 0 && origen <= Nodo_Ultimo) {
        estadosNodos[origen].responde = true;
        estadosNodos[origen].ultimaRespuesta = millis();
        estadosNodos[origen].intentos = 0;
    }
    
    // Buscar comandos especiales en el mensaje
    if (mensaje.indexOf("ALERTA") >= 0) {
        // Mensaje de alerta - requiere atención especial
        Serial.print("¡ALERTA RECIBIDA DE NODO ");
        Serial.print(origen);
        Serial.println("!");
        
        if (origen > 0 && origen <= Nodo_Ultimo) {
            estadosNodos[origen].ultimoEstado = 2; // Estado de alerta
        }
        
        return true; // Mensaje requiere acción especial
    } 
    else if (mensaje.indexOf("RESET") >= 0) {
        // Comando de reinicio
        Serial.print("Comando de RESET recibido de nodo ");
        Serial.println(origen);
        return true;
    }
    
    // Mensaje normal sin acciones especiales
    return false;
}

/**
 * @brief Verifica si un nodo está en estado de alerta
 */
bool Master::NodoEnAlerta(int nodoID) {
    if (nodoID > 0 && nodoID <= Nodo_Ultimo) {
        return (estadosNodos[nodoID].responde && 
                estadosNodos[nodoID].ultimoEstado == 2);
    }
    return false;
}

/**
 * @brief Genera mensaje para petición especial a un nodo
 */
String Master::GenerarPeticionEspecial(int nodoID, String comando) {
    // Formato básico de mensaje: ID:COMANDO
    String mensaje = String(nodoID) + ":" + comando;
    
    Serial.print("Generando petición especial: ");
    Serial.println(mensaje);
    
    return mensaje;
}