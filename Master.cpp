#include "Master.h"
#include "Lora.h"
#include "Functions.h"
#include <Arduino.h>
#include <Ticker.h>

// Temporizadores para la gestión del protocolo
Ticker timer_master;        // Temporizador principal para consulta de nodos
Ticker timer_No_Response;   // Temporizador para timeout de respuesta de nodos
Ticker timer_Survey;       // Temporizador para la encuesta de nodos

// Puntero global al objeto Master para uso en funciones estáticas
Master* masterInstance = nullptr;

Master::Master(bool mode_master, int nodo_number, char localAddress) {
    /**
     * @brief Constructor principal de la clase Master
     * @param mode_master True si es Master, False si es Nodo
     * @param nodo_number Número de nodos si es Master, ID propio si es Nodo
     */
    // Inicialización de variables principales
    MasterMode = mode_master;
    NodeMode   = !mode_master;
    nodeNumber = nodo_number;
    NodeAddress = localAddress;
    // Configuración de la secuencia de nodos
    if (MasterMode) {
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
    nodeNoResponde = false;  // Inicializar correctamente para evitar valores aleatorios
    firstScan = true;
    timeout_NoResponse=3000;
}
Master::Master(String nodoNumero, String ZonaA_status, String ZonaB_status, String Fuente_in_status) {
    /**
     * @brief Constructor para almacenar estado de un nodo
     */
    // Almacenamiento de estados del nodo
    nodo_Number = nodoNumero.charAt(0);
    Zone_A = ZonaA_status;
    Zone_B = ZonaB_status;
    Fuente = Fuente_in_status;
    
    // Prepara el string concatenado para la base de datos
    Node_DB = nodo_Number + Zone_A + Zone_B + Fuente;
}

void Master::Iniciar(Lora* Node, Functions* Correr) {
    /**
     * @brief Inicializa el protocolo y los temporizadores
     */
    // En modo Master, inicia el temporizador de consulta periódica

    nodeRef = Node;
    correrRef = Correr;
    if (MasterMode) {
    Serial.println("Iniciando protocolo Master");
    // IMPORTANTE: attach usa segundos; para 5 segundos, usar attach(5.0) o attach_ms(5000)
    timer_master.attach_ms(5000, timer_master_ISR); // Llama a la función de temporizador cada 5 segundos
        
        // Imprime información de configuración
        Serial.print("Total de nodos configurados: ");
        Serial.println(Nodo_Ultimo);
        Serial.println("Esperando primer ciclo de consulta...");
    } else {
        Serial.print("Iniciando modo Nodo ID: ");
        Serial.println(nodeNumber);
    }
}
void Master::Configuracion() {
    /**
     * @brief Configura los parámetros del protocolo
     */
    // Esta función podría permitir cambiar parámetros en tiempo de ejecución
    // Por ejemplo: intervalos de consulta, timeouts, etc.
    

}

void Master::Gestion() {
    /**
     * @brief Gestiona el ciclo principal del protocolo
     */
    // Esta función podría llamarse desde el loop() principal
    // para manejar lógica adicional no basada en interrupciones
    

}
void Master::Secuencia() {
    /**
     * @brief Maneja la secuencia de consulta a nodos
     */
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
void Master::Nodo_Status(String nodeNumber_st, String zonaA_st, String zonaB_st, String fuente_st) {
    /**
     * @brief Actualiza el estado de un nodo
     */
    // Guardamos los datos recibidos del nodo
    nodo_Number = nodeNumber_st.charAt(0);
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
void Master::Master_DB() {
    /**
     * @brief Actualiza la base de datos del Master con información de nodos
     */
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
void Master::Master_Request() {
    // Esta función implementa la lógica para manejar peticiones especiales
    /**
     * @brief Procesa una petición del Master a un Nodo
     */
    // a nodos específicos (no solo la consulta regular)
    
    // Por ejemplo, podría enviar comandos como:
    // - Solicitar estado detallado
    // - Activar/desactivar funciones específicas
    // - Actualizar configuración
    
    Serial.println("Procesando petición especial");
    
    // Aquí se podría implementar una cola de peticiones especiales
    // para enviar cuando llegue el turno de cada nodo
}
bool Master::NodoEnAlerta(int nodoID) {
    /**
     * @brief Verifica si un nodo está en estado de alerta
     */
    if (nodoID > 0 && nodoID <= Nodo_Ultimo) {
        return (estadosNodos[nodoID].responde && 
                estadosNodos[nodoID].ultimoEstado == 2);
    }
    return false;
}
void Master::Master_Calibration_Init() {
    F_Calibration_EN = true;
    F_Calibration_Complete = false;

    
    timer_master.detach();              // Detener el temporizador principal del Master
    timer_No_Response.detach();         // Detener el temporizador de no respuesta
    Serial.println("Iniciando protocolo de calibración Master");
    // Configurar temporizador para encuesta de nodos cada 5 segundos
    timer_Survey.attach_ms(5000, [this]() {
        this->NextSurvey = true; // Activar bandera para consultar siguiente nodo
    });
}
void Master::Master_Calibration_End() {
    F_Calibration_EN = false;
    F_Calibration_Complete = true;
    timer_Survey.detach(); // Detener el temporizador de encuesta
    Serial.println("Finalizando protocolo de calibración Master");
    // Reiniciar el temporizador principal del Master
    timer_master.attach_ms(1000, timer_master_ISR);
}
String Master::GenerarPeticionEspecial(int nodoID, String comando) {
    /**
     * @brief Genera mensaje para petición especial a un nodo
     */
    // Formato básico de mensaje: ID:COMANDO
    String mensaje = String(nodoID) + ":" + comando;
    
    Serial.print("Generando petición especial: ");
    Serial.println(mensaje);
    
    return mensaje;
}
void Master::Master_DecodificarMensaje(String mensaje) {
    Serial.print("Mensaje recibido: ");
    Serial.println(mensaje);
}
void Master::DebugEstadoBanderas() {
    Serial.print("=== DEBUG BANDERAS === Nodo consultado: ");
    Serial.print(Nodo_Consultado);
    Serial.print(" | nodeResponde: ");
    Serial.print(nodeResponde ? "TRUE" : "FALSE");
    Serial.print(" | nodeNoResponde: ");
    Serial.print(nodeNoResponde ? "TRUE" : "FALSE");
    Serial.print(" | nodeAlerta: ");
    Serial.println(nodeAlerta ? "TRUE" : "FALSE");
}




//**💢💢 Programa Principal 💢💢******************/
void Master::Calibration_Protocol() {
    /**
     * @brief Protocolo para modo de calibración
     */
    // Aquí implementa la lógica del protocolo de calibración
    // Por ejemplo: ajustar potencia de transmisión, medir RSSI, etc.
    
    Serial.println("🔧 Ejecutando protocolo de calibración");
    
    // Ejemplo de implementación básica:
    // - Enviar paquetes de prueba
    // - Medir calidad de señal
    // - Ajustar parámetros de radio
}


void Master::Node_Print_RX(){
Serial.print("📩 Mensaje recibido - Remitente: ");
Serial.print(rx_remitente);
Serial.print(" | Destinatario: ");
Serial.print(rx_destinatario);
Serial.print(" | Modo: ");
Serial.print(rx_funct_mode);
Serial.print(" | Num: ");
Serial.print(rx_funct_num);
Serial.print(" | Param1: ");
Serial.print(rx_funct_parameter1);
Serial.print(" | Param2: ");
Serial.println(rx_funct_parameter2);
 }
void Master::Node_Message(){
  // 0. Function Llamada desde Lora_Nodo_Decodificar.
  // 1. Preparamos paquete para enviar
    //Estados de Entradas.
    // bitWrite(nodo_local,0, );
    // bitWrite(nodo_local,0, Zone_A_ST);
    // bitWrite(nodo_local,1, Zone_B_ERR);
    // bitWrite(nodo_local,2, Zone_B_ST);
    // bitWrite(nodo_local,3, Zone_A_ERR);
    // bitWrite(nodo_local,4, false);
    // bitWrite(nodo_local,5, false);
    // bitWrite(nodo_local,6, true);
    // bitWrite(nodo_local,7, false);
    // nodo_status=char(nodo_local);

    tx_node_lora_1          =String(NodeAddress);         // Direccion del nodo local.
    tx_node_lora_2          =String(Master_Address);        // Direccion del maestro.
    tx_node_lora_3          =nodeRef->Zone_A_str;              // Estado de la zona A      
    tx_node_lora_4          =nodeRef->Zone_B_str;              // Estado de la zona B
    tx_node_lora_5          =nodeRef->Rele_1_out_str;          // Estado de la Salida 1
    tx_node_lora_6          =nodeRef->Rele_2_out_str;          // Estado de la Salida 2
    tx_node_lora_7          =nodeRef->Fuente_in_str;           // Estado de la Fuente
    tx_node_lora_8          =message_type;                  // Tipo de mensaje


    if(message_type=="F"){     
        tx_node_lora_5 = counterStr.substring(0, 1); // primer dígito
        tx_node_lora_6 = counterStr.substring(1, 2); // segundo dígito
        tx_node_lora_7 = counterStr.substring(2, 3); // tercer dígito
        tx_node_lora_8 = counterStr.substring(3, 4); // cuarto dígito
    }
    // 2. Armamos el paquete a enviar.
        mensaje = String(  tx_node_lora_1 + tx_node_lora_2 + tx_node_lora_3 + tx_node_lora_4 + tx_node_lora_5 + tx_node_lora_6 + tx_node_lora_7 + tx_node_lora_8);
}   
void Master::Node_Counter(){
    ++nodeCounter;
 }
void Master::Node_Decodificar(){
  // 1. Leemos el mensaje recibido.
    rx_master_lora_1 = mensaje.substring(0, 1); // Direccion del nodo que responde.
    rx_master_lora_2 = mensaje.substring(1, 2); // Direccion del maestro.
    rx_master_lora_3 = mensaje.substring(2, 3); // Estado de la zona A.
    rx_master_lora_4 = mensaje.substring(3, 4); // Estado de la zona B.
    rx_master_lora_5 = mensaje.substring(4, 5); // Estado de la salida 1.
    rx_master_lora_6 = mensaje.substring(5, 6); // Estado de la salida 2.
    rx_master_lora_7 = mensaje.substring(6, 7); // Estado de la fuente.
    rx_master_lora_8 = mensaje.substring(7, 8); // Tipo de mensaje, si es de emergencia.

    // 2. Asignamos los valores a las variables correspondientes.
    rx_destinatario      =rx_master_lora_2;          // Direccion del nodo que responde.
    rx_remitente         =rx_master_lora_1;          // Direccion del maestro.
    rx_funct_mode        =rx_master_lora_8;          // Modo de funcion.
    rx_funct_num         =rx_master_lora_5;          // Numero de funcion.
    rx_funct_parameter1  =rx_master_lora_6;          // Parametro 1.
    rx_funct_parameter2  =rx_master_lora_7;          // Parametro 2.

    Node_Print_RX(); // Imprimimos el mensaje recibido.
    if(rx_destinatario.charAt(0)==NodeAddress){
      Serial.println("Nodo_Atiende");
      if(rx_funct_mode=="E"){
        Serial.println("Peticion escuchada");
        F_Node_Excecute=true;  //Flag Desactivado en L-4.3
      }
      if(rx_funct_mode=="M"){
      }
      if(rx_funct_mode=="A"){
        // 3. Contador de mensajes enviados.
        String counterStr = String(nodeCounter, DEC);
        while (counterStr.length() < 4) counterStr = "0" + counterStr; // Asegura 4 dígitos
        Node_Counter();
      }

      F_Responder=true;
      F_Node_Atiende=true;
      // Protocol.nodeResponde=F_Node_Atiende;
    }
    nodeRef->F_Recibido=false;               // Flag activado desde Lora_Nodo_Decodificar Se resetea la bandera de recepcion.
  }
// 🛂🛂NODO PROTOCOL🛂🛂
void Master::Node_Protocol() {
    /**
     * @brief Protocolo para cuando el dispositivo está en modo Nodo
     */
    // Aquí implementa la lógica del protocolo del Nodo
    // Por ejemplo: responder a consultas del Master, reportar estado, etc.
    
    //-P.1 LORA RX
    nodeRef->Lora_RX();
    mensaje = nodeRef->rxdata;      // Se lee el mensaje recibido.
    //-P.2 Node IO.
    nodeRef->Lora_IO_Zones(); // Se actualizan los estados de las zonas.
    // nodeRef->Lora_IO_Dummy_Simulate(); // Se simulan las señales de entrada.
     //-P.3 Nodo Evento en Zonas
    if(nodeRef->F_IO_Event_Enable && nodeRef->msg_enviar){
      Serial.println("event");
      while(msg_enviado<2){
        Node_Message();  // Antes de enviar el mensaje se prepara la trama del nodo.
        nodeRef->Lora_TX(mensaje);
        delay(100);
        ++ msg_enviado;
      }
      nodeRef->msg_enviar=false;
      msg_enviado=0;
      nodeRef->F_IO_Event_Enable = false;
    }
      //-P.4 Nodo RX.
    if(nodeRef->F_Recibido){
      Node_Decodificar();        // Se recibe el mensaje.
      Serial.println("Mensaje Decodificado");
    }
      //-P.5 Nodo Ejecuta Funciones.
    if(F_Node_Excecute){
      // Validación de datos antes de ejecutar funciones
      String command = rx_funct_mode + rx_funct_num + rx_funct_parameter1 + rx_funct_parameter2;
      Serial.print("Ejecutando comando: ");
      Serial.println(command);
      
      // Verificar que el comando tenga la longitud mínima esperada
      if(command.length() >= 4 && rx_funct_mode != "" && rx_funct_num != ""){
        try {
          correrRef->Functions_Request(command);
          correrRef->Functions_Run();
          Serial.println("Comando ejecutado correctamente");
        } catch (...) {
          Serial.println("Error al ejecutar comando - evitando reinicio");
        }
      } else {
        Serial.println("Comando inválido - ignorando para evitar reinicio");
        Serial.print("rx_funct_mode: ");
        Serial.println(rx_funct_mode);
        Serial.print("rx_funct_num: ");
        Serial.println(rx_funct_num);
      }
      F_Responder = true;
      F_Node_Excecute=false;
    }
    // Ejemplo de implementación básica:
    // - Escuchar mensajes del Master
    // - Responder con estado actual
    // - Reportar alertas si es necesario
      //-P.6 Nodo TX.
    if(F_Responder){
      Node_Message();    // Antes de enviar el mensaje se prepara la trama del nodo.
      nodeRef->Lora_TX(mensaje);            // Se envia el mensaje.
      F_Responder=false;
    }
}


void Master::timer_master_ISR() {
    /**
     * @brief ISR para el temporizador de consulta periódica
     * Esta función se llama automáticamente por el temporizador
     */
    // Como es una función estática, usamos el puntero global para acceder a la instancia
    if (masterInstance) {
        // Activamos la bandera para indicar que es momento de consultar al siguiente nodo
        masterInstance->Next = true;
        
        // También podríamos ejecutar lógica adicional aquí, pero es mejor mantener
        // las ISRs lo más cortas posible
    }
}
void Master::Nodo_REQUEST() {
    /**
     * @brief Determina el siguiente nodo a consultar en secuencia cíclica
     */
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
        estadosNodos[Nodo_Consultado].responde = false; // Resetear bandera de respuesta
        
        // Información de depuración
        // Serial.print("Consultando nodo: ");
        // Serial.println(Nodo_Consultado);
        
        // Configurar temporizador de timeout DESPUÉS de resetear las banderas
        timer_No_Response.once_ms(timeout_NoResponse, [this]() {
            Serial.println("Timeout: Verificando respuesta del nodo");
            
            // Solo marcar como no responde si efectivamente no respondió
            if (!estadosNodos[Nodo_Consultado].responde) {
                Serial.print("Nodo ");
                Serial.print(Nodo_Consultado);
                Serial.println(" no respondió a tiempo");
                nodeNoResponde = true;
            } else {
                // Serial.print("Nodo ");
                // Serial.print(Nodo_Consultado);
                // Serial.println(" respondió correctamente antes del timeout");
            }
        });
    }
}
void Master::Master_Nodo() {
    /**
     * @brief Prepara la consulta al siguiente nodo
     */
    
    
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
    
    

    
    // Registramos el intento de comunicación
    // Serial.print("Master consulta a nodo: ");
    // Serial.println(Nodo_Consultado);
}
void Master::MasterMessage() {
    /**
     * @brief Prepara el mensaje para el nodo consultado
     */
  //1. Preparamos paquete para enviar
    tx_remitente        = Master_Address;                  // Direccion del maestro.
    tx_destinatario     = String(Nodo_Proximo);                // Direccion del nodo local.
    tx_mensaje          = ".";                           // Mensaje de consulta de estado.



  //2. Armamos el mensaje para enviar.
    mensaje = String(  tx_remitente + tx_destinatario + tx_mensaje + tx_funct_mode + tx_funct_num + tx_funct_parameter1 + tx_funct_parameter2 );
  //3. Borramos Variables de envio.
    nodo_consultado = tx_destinatario.charAt(0);
    tx_remitente=' ';
    tx_destinatario=' ';
    tx_mensaje=' ';
    tx_funct_mode=' ';
    tx_funct_num=' ';
    tx_funct_parameter1=' ';
    tx_funct_parameter2=' ';
    // Aquí podrías implementar lógica adicional para mensajes especiales
    // Por ejemplo: comandos específicos para cada nodo según su estado
}
void Master::SerializeObjectToJson() {
  doc[nodeJS]     = Node_Num_str;     // Numero de Nodo consultado
  doc[commJS]     = Node_Status_str;  // Estado de la comunicacion
  doc[zoneAJS]    = rx_master_lora_3; // Estado de la zona A
  doc[zoneBJS]    = rx_master_lora_4; // Estado de the zona B
  doc[output1JS]  = rx_master_lora_5; // Estado de the salida 1
  doc[output2JS]  = rx_master_lora_6; // Estado de the salida 2
  doc[fuenteJS]   = rx_master_lora_7; // Estado de the fuente
  serializeJson(doc, jsonString);

  // Serial.print("LORA_JSON String:");
  // Serial.println(jsonString);
 }
void Master::NodeStatusUpdate(){
      /**
   * @brief Actualiza el estado del nodo consultado y serializa la información a JSON
   *
   * Este método puede ser llamado por los siguientes eventos:
   * -1 Cuando un Nodo Responde correctamente
   * -2 Cuando un Nodo no responde a la consulta.
   * -3 Cuando un Nodo cambia el estado de sus entradas (Zonas) nodo en Alerta.
   */
  
  if(nodeResponde){                            // Si el nodo respondió correctamente
    Node_Status_str = "1";                              // Comunicacion ok
    Node_Num_str    = String(Nodo_Consultado); // Numero de Nodo consultado
    Serial.println("Nodo responde timer activo");
    SerializeObjectToJson();                            // Serializar para enviar al servidor/DB
    nodeResponde = false;                      // Resetear la bandera para la próxima consulta
  }
  if(nodeNoResponde){      // Si el nodo NO respondió a la consulta
    Node_Status_str = "0"; // Nodo no responde
    Node_Num_str    = String(Nodo_Consultado); // Numero de Nodo consultado
    
    // Poner los estados de las zonas y salidas como ="0" (desconocido)
    rx_master_lora_3 = "0"; // Estado de la zona A
    rx_master_lora_4 = "0"; // Estado de la zona B
    rx_master_lora_5 = "0"; // Estado de la salida 1
    rx_master_lora_6 = "0"; // Estado de la salida 2
    rx_master_lora_7 = "0"; // Estado de la fuente
    
    // Serializar para enviar al servidor/DB
    SerializeObjectToJson();
    Serial.print("Nodo ");
    Serial.print(Nodo_Consultado);
    Serial.println(" no respondió a la consulta anterior");
    nodeNoResponde = false; // Resetear la bandera para la próxima consulta
  }
  if(nodeAlerta){          // Si el nodo cambió el estado de sus entradas (Zonas)
    nodeAlerta = false;            // Resetear la bandera para la próxima consulta
    Node_Status_str = "1";                  // Comunicacion ok
    Node_Num_str    = String(Nodo_Actual); // Numero de Nodo consultado
    SerializeObjectToJson();                // Serializa el objeto a JSON
  }
  F_ServerUpdate = true;            // Resetear la bandera de actualización del servidor
  F_NodeStatusUpdate = false; 
}
void Master::MasterDecodificar(String mensaje_loraRX) {
    /**
     * @brief Procesa un mensaje recibido y determina acciones
     */
    // Esta función analiza mensajes recibidos y determina acciones especiales
    // Desglosar el mensaje recibido en los nueve substrings como en la clase Lora
    timer_No_Response.detach();             // 1. Detener el temporizador de no respuesta
    Lora_Rxdata         = mensaje_loraRX;
    rx_remitente        = Lora_Rxdata.substring(0, 1);
    rx_destinatario     = Lora_Rxdata.substring(1, 2);
    rx_mensaje          = Lora_Rxdata.substring(2, 3);
    rx_funct_mode       = Lora_Rxdata.substring(3, 4);
    rx_funct_num        = Lora_Rxdata.substring(4, 5);
    rx_funct_parameter1 = Lora_Rxdata.substring(5, 6);
    rx_funct_parameter2 = Lora_Rxdata.substring(6, 7);
    rx_funct_parameter3 = Lora_Rxdata.substring(7, 8);
    rx_funct_parameter4 = Lora_Rxdata.substring(8, 9);

    Serial.print("Remitente: ");
    Serial.print(rx_remitente);
    Serial.print(" | Nodo Consultado: ");
    Serial.println(Nodo_Consultado);

    // Registrar que el nodo ha respondido
    if (rx_remitente.toInt() == Nodo_Consultado) { // Comparar correctamente convirtiendo String a int
        // Serial.println("*** NODO CONSULTADO RESPONDIÓ CORRECTAMENTE ***");
        nodeResponde = true;
        nodeNoResponde = false;
        
        int rx_remitente_int = rx_remitente.toInt();
        estadosNodos[rx_remitente_int].responde = true;
        estadosNodos[rx_remitente_int].ultimaRespuesta = millis();
        estadosNodos[rx_remitente_int].intentos = 0;

        // Buscar comandos especiales en el mensaje
        if (rx_mensaje == "A") {
            // Mensaje de alerta - requiere atención especial
            Serial.print("¡ALERTA RECIBIDA DE NODO ");
            Serial.print(rx_remitente);
            Serial.println("!");
        } 
        else if (rx_mensaje == "R") {
            // Comando de reinicio
            Serial.print("Comando de RESET recibido de nodo ");
            Serial.println(rx_remitente);
        }
    }
    else {
        Serial.print("*** MENSAJE DE NODO INESPERADO: ");
        Serial.print(rx_remitente);
        Serial.print(" (Se esperaba nodo: ");
        Serial.print(Nodo_Consultado);
        Serial.println(") ***");
        
        Nodo_Actual = rx_remitente.toInt(); // Convertir String a int antes de asignar
        nodeAlerta = true; // Mensaje inesperado, posible alerta
        
        // IMPORTANTE: No cambiar nodeResponde/nodeNoResponde aquí
        // porque este mensaje no es del nodo que estamos consultando
        // El temporizador seguirá corriendo para el nodo consultado
    }

    NodeStatusUpdate();
}
void Master::Master_Counter(){
    ++MasterCounter;
    counterStr = String(MasterCounter, DEC);
    tx_mensaje = counterStr; // Contador de mensajes enviados.
 }
// 👑👑MASTR PROTOCOL👑👑
void Master::Master_Protocol() {
    /**
     * @brief Ejecuta las funciones principales del protocolo Master
     */
    // Verificar si es momento de consultar al siguiente nodo
    nodeRef->Lora_RX();

    if(Next) {
        // Primero determinamos cuál es el siguiente nodo a consultar
        Nodo_REQUEST();
        Master_Nodo(); // Verifica si hay un nodo en alerta
        
        MasterMessage();// Preparamos el mensaje para el nodo seleccionado
        nodeRef->Lora_TX(mensaje); // Enviar el mensaje
        Next = false;    // Resetear la bandera
    }
    if(nodeRef->F_Recibido){ // Si se recibió un mensaje por Lora
        MasterDecodificar(nodeRef->rxdata); // Procesar el mensaje recibido
        nodeRef->F_Recibido = false; // Resetear la bandera de recepción
        F_ServerUpdate = true; // Indicar que se debe actualizar el servidor
    }
    if(nodeRef->F_Master_Excecute){
        if(message_type=="M"){
            correrRef->Functions_Request(tx_funct_mode + tx_funct_num + tx_funct_parameter1 + tx_funct_parameter2);
            correrRef->Functions_Run();
            nodeRef->F_Master_Excecute=false;         // 4. Se Desactiva la bandera Master_Excecute.
            Serial.println("🚀Server->Master");
        }
        if(message_type != "M"){
                    // Lora_Master_Frame();             // 2. Se prepara el mensaje a enviar.
            nodeRef->Lora_TX(mensaje);                       // 3. Se envia el mensaje.
            nodeRef->F_Master_Excecute=false;         // 4. Se Desactiva la bandera Master_Excecute.
            Serial.println("🚀Server->Master->Node");
        }
  }
}


// ✅✅PROTOCOL PRINCIPAL✅✅
void Master::Preguntar() {
    /**
     * @brief Inicia el protocolo Chisme
     */
    
    if(MasterMode && !F_Calibration){
        Master_Protocol();
    }
    if(NodeMode){
        Node_Protocol();
    }
    if(F_Calibration){
        Calibration_Protocol();
    }
}