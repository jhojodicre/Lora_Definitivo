/**
 * Send and receive LoRa-modulation packets with a sequence number, showing RSSI
 * and SNR for received packets on the little display.
 *
 * Note that while this send and received using LoRa modulation, it does not do
 * LoRaWAN. For that, see the LoRaWAN_TTN example.
 *
 * This works on the stick, but the output on the screen gets cut off.
 */
#include "Lora.h"
#include "Master.h"
#include <Arduino.h>
#include <Ticker.h>
#include <ArduinoJson.h>

// Turns the 'PRG' button into the power button, long press is off
#define HELTEC_POWER_BUTTON // must be before "#include <heltec_unofficial.h>"
#include <heltec_unofficial.h>



volatile    bool rxFlag = false;

// ✅ CONSTANTES PARA MEMORIA NO VOLÁTIL
const char* Lora::NVS_NAMESPACE = "lora_config";
const char* Lora::NVS_RADIO_CONFIG_KEY = "radio_config";

//Instancias
Ticker      Timer_Nodo_Answer;
Ticker      Timer_ZoneA_Enable;
Ticker      Timer_ZoneB_Enable;
// Nuevos timers para múltiples niveles temporales
Ticker      Timer_ZoneA_Extended;   // Timer para 6 segundos (tiempo alcanzado)
Ticker      Timer_ZoneA_Error;      // Timer para 9 segundos (error)
Ticker      Timer_ZoneB_Extended;   // Timer para 6 segundos (tiempo alcanzado)
Ticker      Timer_ZoneB_Error;      // Timer para 9 segundos (error)
Lora*       nodeInstance = nullptr; // Puntero global al objeto Master

Lora::Lora(bool IO_Simulated, int NumNodes, char nodeNumber){
    // Inicializa el atributo Master correctamente
  
  F_IO_Simulated = IO_Simulated;
  local_Address = nodeNumber; // Direccion del nodo local.
  Num_Nodos     = NumNodes;

  // Constructor de la clase Node
    //1. Configuracion de Hardware
      pinMode(Zona_A_in, INPUT);
      pinMode(Zona_B_in, INPUT);

      pinMode(Fuente_in, INPUT);

      pinMode(PB_ZA_in, INPUT);
      pinMode(PB_ZB_in, INPUT);
      pinMode(PB_ZC_in, INPUT);

      pinMode(Rele_1_out, OUTPUT);
      pinMode(Rele_2_out, OUTPUT);

    //2. Condicion Inicial.
      digitalWrite(Rele_1_out, LOW);
      digitalWrite(Rele_2_out, LOW);
      
      // Inicializar estados de zonas
      Zone_A_ST = false;
      Zone_B_ST = false;
      Zone_A_ERR = false;
      Zone_B_ERR = false;
      
      F_Nodo_Excecute=false;
      nodeInstance = this; // Asignar la instancia actual al puntero global
      Serial.println("📡 Clase Lora instanciada.");
 }
void Lora::Lora_Setup()
{

    // 🛠⚙ USAR CONFIGURACIÓN GUARDADA O POR DEFECTO
    // -1 = Usar configuración guardada si existe, si no usar por defecto (0)
    Lora_Configure(-1);
 }
void Lora::Lora_Configure(int numero_de_configuracion){
  // ✅ VERIFICAR CONFIGURACIÓN GUARDADA EN MEMORIA NO VOLÁTIL
  int stored_config = LoadRadioConfigFromNVS();
  
  // Si se pasa -1, usar configuración guardada; si no hay guardada, usar por defecto
  if (numero_de_configuracion == -1) {
    if (stored_config != -1) {
      numero_de_configuracion = stored_config;
      Serial.printf("📄 Usando configuración guardada en NVS: %d\n", stored_config);
    } else {
      numero_de_configuracion = 0; // Por defecto
      Serial.println("🔧 No hay configuración guardada, usando por defecto");
    }
  } else {
    // Si se especifica una configuración nueva, guardarla
    if (numero_de_configuracion != 0) {
      SaveRadioConfigToNVS(numero_de_configuracion);
      Serial.printf("💾 Nueva configuración guardada en NVS: %d\n", numero_de_configuracion);
    }
  }
  
  // Actualizar variable de estado
  Node_Configuration_Radio = numero_de_configuracion;
  
  // Configuracion inicial de Lora con selección automática según distancia
  heltec_setup();
  // ESP32-S3: el rango ADC por defecto (0dB ~950mV) es insuficiente para VBAT_ADC
  // (el divisor entrega hasta ~2.1V). ADC_11db extiende el rango a ~3.1V.
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
  RADIOLIB_OR_HALT(radio.begin());
  
  // Set the callback function for received packets
  radio.setDio1Action(rx);
  
  // ✅ CONFIGURACIÓN COMPATIBLE - Solo varía potencia TX según distancia
  // TODOS los dispositivos mantienen los mismos parámetros de modulación
  switch(numero_de_configuracion) {
    case 1: // NODO CERCANO - Potencia baja para ahorro energético
      frequency = FREQUENCY;           // 915.0 MHz (IGUAL PARA TODOS)
      bandwidth = BANDWIDTH;           // 125.0 kHz (IGUAL PARA TODOS)
      spreading_factor = SPREADING_FACTOR; // SF 12 (IGUAL PARA TODOS)
      transmit_power = 5;              // 🔹 SOLO CAMBIA LA POTENCIA
      config_name = "CERCANO (5 dBm)";
      break;
      
    case 2: // NODO MEDIO - Potencia media
      frequency = FREQUENCY;           // 915.0 MHz (IGUAL PARA TODOS)
      bandwidth = BANDWIDTH;           // 125.0 kHz (IGUAL PARA TODOS)
      spreading_factor = SPREADING_FACTOR; // SF 12 (IGUAL PARA TODOS)
      transmit_power = 10;             // 🔹 SOLO CAMBIA LA POTENCIA
      config_name = "MEDIO (10 dBm)";
      break;
      
    case 3: // NODO LEJANO - Potencia alta para máximo alcance
      frequency = FREQUENCY;           // 915.0 MHz (IGUAL PARA TODOS)
      bandwidth = BANDWIDTH;           // 125.0 kHz (IGUAL PARA TODOS)
      spreading_factor = SPREADING_FACTOR; // SF 12 (IGUAL PARA TODOS)
      transmit_power = 20;             // 🔹 SOLO CAMBIA LA POTENCIA
      config_name = "LEJANO (20 dBm)";
      break;
      
    case 4: // CONFIGURACIÓN PERSONALIZADA
      frequency = FREQUENCY;           // 915.0 MHz (IGUAL PARA TODOS)
      bandwidth = BANDWIDTH;           // 125.0 kHz (IGUAL PARA TODOS)
      spreading_factor = SPREADING_FACTOR; // SF 12 (IGUAL PARA TODOS)
      transmit_power = 15;             // 🔹 SOLO CAMBIA LA POTENCIA
      config_name = "PERSONALIZADO (15 dBm)";
      break;
      
    default: // CONFIGURACIÓN POR DEFECTO (caso 0 o inválido)
      frequency = FREQUENCY;           // 915.0 MHz
      bandwidth = BANDWIDTH;           // 125.0 kHz
      spreading_factor = SPREADING_FACTOR; // SF 12
      transmit_power = TRANSMIT_POWER; // 1 dBm
      config_name = "POR DEFECTO (1 dBm)";
      break;
  }
  
  // Aplicar configuración seleccionada
  RADIOLIB_OR_HALT(radio.setFrequency(frequency));
  RADIOLIB_OR_HALT(radio.setBandwidth(bandwidth));
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(spreading_factor));
  RADIOLIB_OR_HALT(radio.setOutputPower(transmit_power));
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF)); // Start receiving
  
  // Mostrar configuración aplicada
  both.println("Radio init - " + config_name);
  both.printf("Frequency: %.2f MHz\n", frequency);
  both.printf("Bandwidth: %.1f kHz\n", bandwidth);
  both.printf("Spreading Factor: %i\n", spreading_factor);
  both.printf("TX power: %i dBm\n", transmit_power);
  both.printf("Configuracion: %i (%s)\n", numero_de_configuracion, config_name.c_str());
}

void Lora::Lora_TX(String mensaje){
    // Transmit a packet
    txdata = mensaje;
  // both.printf("TX [%s] ", String(mensaje).c_str());
    // both.printf("TX [%s] ", txdata.c_str());
    radio.clearDio1Action();

    unsigned long txStartTime = millis();

    heltec_led(50); // 50% brightness is plenty for this LED
    RADIOLIB(radio.transmit(txdata.c_str()));

    unsigned long txEndTime = millis();

    // both.printf("%lu ms\n", txEndTime - txStartTime);
    heltec_led(0);
    if (_radiolib_status == RADIOLIB_ERR_NONE)
    {
        both.printf("📻 TX: %s\n", txdata.c_str());
    }
    else
    {
        both.printf("fail (%i)\n", _radiolib_status);
    }
    radio.setDio1Action(rx);
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
    F_Responder = false;      // Bandera activada en Lora_Nodo_Decodificar.
    nodo_consultado=nodo_a_Consultar.charAt(0);
    F_Node_Atiende=false;    // Flag desactivado en Lora_Nodo_Decodificar.
    // Protocol.nodeResponde=F_Node_Atiende;
}
void Lora::Lora_RX(){
    // If a packet was received, display it and the RSSI and SNR
    if (rxFlag)
    {
      rxFlag = false;
      radio.readData(rxdata);
      if (_radiolib_status == RADIOLIB_ERR_NONE)
      {
        both.printf("📻 RX: [%s]\n", rxdata.c_str());
        // both.printf("  RSSI: %.2f dBm\n", radio.getRSSI());
        // both.printf("  SNR: %.2f dB\n", radio.getSNR());

        SNR = String(radio.getSNR()).toFloat();
        RSSI = String(radio.getRSSI()).toFloat();
      }
      RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
      F_Recibido = true;  // Bandera activada en Lora_RX.
    }
  }
void Lora::rx(){
  rxFlag = true;
 }

void Lora::Lora_Status_RadioConfig(){
  // ✅ GENERAR JSON CON ESTADO COMPLETO DE RADIO
  StaticJsonDocument<512> statusDoc;
  
  // === 📡 CONFIGURACIÓN DE RADIO ===
  statusDoc["radio"]["frequency"] = frequency;
  statusDoc["radio"]["bandwidth"] = bandwidth;
  statusDoc["radio"]["spreading_factor"] = spreading_factor;
  statusDoc["radio"]["transmit_power"] = transmit_power;
  statusDoc["radio"]["config_id"] = Node_Configuration_Radio;
  statusDoc["radio"]["config_name"] = config_name;
  
  // === 📊 ESTADO DE COMUNICACIÓN ===
  statusDoc["communication"]["rssi"] = RSSI;
  statusDoc["communication"]["snr"] = SNR;
  statusDoc["communication"]["last_rx"] = millis(); // Timestamp último mensaje
  
  // === 🔧 INFORMACIÓN DEL DISPOSITIVO ===
  statusDoc["device"]["type"] = F_MasterMode ? "MASTER" : "NODE";
  statusDoc["device"]["address"] = String(local_Address);
  statusDoc["device"]["uptime"] = millis() / 1000; // Segundos de funcionamiento
  statusDoc["device"]["free_heap"] = ESP.getFreeHeap();
  
  // === ⚙️ CONFIGURACIÓN PERSISTENTE ===
  statusDoc["storage"]["has_stored_config"] = HasStoredRadioConfig();
  if (HasStoredRadioConfig()) {
    statusDoc["storage"]["stored_config"] = LoadRadioConfigFromNVS();
  }
  
  // === 🎯 ESTADO ESPECÍFICO DEL MASTER ===
  if (F_MasterMode) {
    statusDoc["master"]["total_nodes"] = Num_Nodos;
    // statusDoc["master"]["current_node"] = String(Protocol.Nodo_Consultado);
    // statusDoc["master"]["next_node"] = String(Protocol.Nodo_Proximo);
    statusDoc["master"]["calibration_active"] = F_MasterCalibration;
    statusDoc["master"]["master_address"] = Master_Address;
  }
  
  // === 📍 ESTADO ESPECÍFICO DEL NODO ===
  if (F_NodeMode) {
    statusDoc["node"]["zone_a_status"] = Zone_A_ST;
    statusDoc["node"]["zone_b_status"] = Zone_B_ST;
    statusDoc["node"]["zone_a_error"] = Zone_A_ERR;
    statusDoc["node"]["zone_b_error"] = Zone_B_ERR;
    statusDoc["node"]["relay_1"] = Rele_1_out_ST;
    statusDoc["node"]["relay_2"] = Rele_2_out_ST;
    statusDoc["node"]["event_enabled"] = F_IO_Event_Enable;
  }
  
  // === 📝 SERIALIZAR A STRING GLOBAL ===
  serializeJson(statusDoc, radioStatusJSON);
  
  // === 🖥️ MOSTRAR EN SERIAL PARA DEBUG ===
  Serial.println("=== 📊 ESTADO DEL SISTEMA ===");
  serializeJsonPretty(statusDoc, Serial);
  Serial.println("\n==============================");
}
void Lora::Lora_Status_SystemInfo(){
  // ✅ INFORMACIÓN GENERAL DEL SISTEMA
  StaticJsonDocument<256> sysDoc;
  
  sysDoc["timestamp"] = millis();
  sysDoc["uptime_seconds"] = millis() / 1000;
  sysDoc["free_heap"] = ESP.getFreeHeap();
  sysDoc["chip_model"] = ESP.getChipModel();
  sysDoc["chip_revision"] = ESP.getChipRevision();
  sysDoc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
  sysDoc["flash_size"] = ESP.getFlashChipSize();
  sysDoc["sketch_size"] = ESP.getSketchSize();
  sysDoc["free_sketch_space"] = ESP.getFreeSketchSpace();
  
  serializeJson(sysDoc, systemStatusJSON);
}
void Lora::Lora_Status_NodeSpecific(){
  // ✅ ESTADO ESPECÍFICO DEL NODO
  StaticJsonDocument<512> nodeDoc;
  
  // === 🔌 ENTRADAS DIGITALES ===
  nodeDoc["inputs"]["Zone_A_ST"] = Zone_A_ST;
  nodeDoc["inputs"]["Zone_A_ST"] = Zone_A_ST;
  nodeDoc["inputs"]["zone_a_ack"] = Zone_A_ACK;
  nodeDoc["inputs"]["zone_b_ack"] = Zone_B_ACK;
  nodeDoc["inputs"]["zone_ab_ack"] = Zone_AB_ACK;
  
  // === 📊 ESTADOS DE ZONAS ===
  nodeDoc["zones"]["zone_a_status"] = Zone_A_ST;
  nodeDoc["zones"]["zone_b_status"] = Zone_B_ST;
  nodeDoc["zones"]["zone_a_error"] = Zone_A_ERR;
  nodeDoc["zones"]["zone_b_error"] = Zone_B_ERR;
  nodeDoc["zones"]["zone_a_extended"] = Zone_A_Extended;
  nodeDoc["zones"]["zone_b_extended"] = Zone_B_Extended;
  
  // === 🔌 SALIDAS ===
  nodeDoc["outputs"]["relay_1"] = Rele_1_out_ST;
  nodeDoc["outputs"]["relay_2"] = Rele_2_out_ST;
  
  // === ⏰ TIMERS ===
  nodeDoc["timers"]["za_enabled"] = timer_ZA_En;
  nodeDoc["timers"]["za_reached"] = timer_ZA_Reached;
  nodeDoc["timers"]["zb_enabled"] = timer_ZB_En;
  nodeDoc["timers"]["zb_reached"] = timer_ZB_Reached;
  
  // === 📡 EVENTOS ===
  nodeDoc["events"]["event_enabled"] = F_IO_Event_Enable;
  nodeDoc["events"]["responder_flag"] = F_Responder;
  nodeDoc["events"]["execute_flag"] = F_Nodo_Excecute;
  
  // === 📈 CONTADORES ===
  nodeDoc["counters"]["node_counter"] = Node_Counter;
  
  serializeJson(nodeDoc, nodeStatusJSON);
}
void Lora::Lora_Status_MasterSpecific(){
  // ✅ ESTADO ESPECÍFICO DEL MASTER
  if (!F_MasterMode) return;
  
  StaticJsonDocument<512> masterDoc;
  
  // === 🎯 CONFIGURACIÓN MASTER ===
  masterDoc["config"]["total_nodes"] = Num_Nodos;
  masterDoc["config"]["master_address"] = Master_Address;
  masterDoc["config"]["local_address"] = String(local_Address);
  
  // === 📡 ESTADO DE CONSULTA ===
  // masterDoc["polling"]["current_node"] = String(Protocol.Nodo_Consultado);
  // masterDoc["polling"]["next_node"] = String(Protocol.Nodo_Proximo);
  masterDoc["polling"]["node_to_query"] = nodo_a_Consultar;
  // masterDoc["polling"]["query_active"] = Protocol.Next;
  
  // === 🔬 CALIBRACIÓN ===
  masterDoc["calibration"]["active"] = F_MasterCalibration;
  masterDoc["calibration"]["node_calibrating"] = Node_to_Calibrate;
  masterDoc["calibration"]["samples_taken"] = validSamples;
  masterDoc["calibration"]["avg_rssi"] = avgRSSI;
  masterDoc["calibration"]["total_rssi"] = totalRSSI;
  
  // === 📊 ESTADÍSTICAS ===
  masterDoc["stats"]["master_counter"] = Master_Counter;
  // masterDoc["stats"]["node_responds"] = Protocol.nodeResponde;
  // masterDoc["stats"]["node_no_response"] = Protocol.nodeNoResponde;
  // masterDoc["stats"]["node_alert"] = Protocol.nodeAlerta;
  
  // === 🌐 SERVIDOR ===
  masterDoc["server"]["update_pending"] = F_ServerUpdate;
  masterDoc["server"]["execute_pending"] = F_Master_Excecute;
  
  serializeJson(masterDoc, systemStatusJSON); // Usar systemStatusJSON para Master
}
void Lora::Lora_Status_CommunicationStats(){
  // ✅ ESTADÍSTICAS DE COMUNICACIÓN
  StaticJsonDocument<256> commDoc;
  
  // === 📡 CALIDAD DE SEÑAL ===
  commDoc["signal"]["rssi"] = RSSI;
  commDoc["signal"]["snr"] = SNR;
  commDoc["signal"]["avg_rssi"] = avgRSSI;
  
  // === 📊 CONTADORES ===
  if (F_MasterMode) {
    commDoc["counters"]["master_counter"] = Master_Counter;
    commDoc["counters"]["total_nodes"] = Num_Nodos;
  } else {
    commDoc["counters"]["node_counter"] = Node_Counter;
    commDoc["counters"]["local_address"] = String(local_Address);
  }
  
  // === 🔄 ESTADO DE COMUNICACIÓN ===
  commDoc["status"]["received_flag"] = F_Recibido;
  commDoc["status"]["respond_flag"] = F_Responder;
  commDoc["status"]["last_update"] = millis();
  
  String commJSON;
  serializeJson(commDoc, commJSON);
  // Añadir a radioStatusJSON para comunicación
  radioStatusJSON = commJSON;
}
String Lora::Lora_GetStatus(String type){                                             
  // ✅ MÉTODO UNIFICADO PARA OBTENER DIFERENTES TIPOS DE ESTADO
  StaticJsonDocument<1024> unifiedDoc;
  
  if (type == "all" || type == "radio") {
    Lora_Status_RadioConfig();
    StaticJsonDocument<512> radioDoc;
    deserializeJson(radioDoc, radioStatusJSON);
    unifiedDoc["radio_status"] = radioDoc;
  }
  
  if (type == "all" || type == "system") {
    Lora_Status_SystemInfo();
    StaticJsonDocument<256> sysDoc;
    deserializeJson(sysDoc, systemStatusJSON);
    unifiedDoc["system_info"] = sysDoc;
  }
  
  if (type == "all" || type == "node") {
    if (F_NodeMode) {
      Lora_Status_NodeSpecific();
      StaticJsonDocument<512> nodeDoc;
      deserializeJson(nodeDoc, nodeStatusJSON);
      unifiedDoc["node_status"] = nodeDoc;
    }
  }
  
  if (type == "all" || type == "master") {
    if (F_MasterMode) {
      Lora_Status_MasterSpecific();
      StaticJsonDocument<512> masterDoc;
      deserializeJson(masterDoc, systemStatusJSON);
      unifiedDoc["master_status"] = masterDoc;
    }
  }
  
  if (type == "all" || type == "communication") {
    Lora_Status_CommunicationStats();
    StaticJsonDocument<256> commDoc;
    deserializeJson(commDoc, radioStatusJSON);
    unifiedDoc["communication_stats"] = commDoc;
  }
  
  // === 📝 SERIALIZAR RESULTADO ===
  String result;
  serializeJson(unifiedDoc, result);
  return result;
}
void Lora::Lora_UpdateAllStatus(){
  // ✅ ACTUALIZAR TODOS LOS ESTADOS
  Lora_Status_RadioConfig();
  Lora_Status_SystemInfo();
  
  if (F_NodeMode) {
    Lora_Status_NodeSpecific();
  }
  
  if (F_MasterMode) {
    Lora_Status_MasterSpecific();
  }
  
  Lora_Status_CommunicationStats();
  
  Serial.println("✅ Todos los estados actualizados");
}

void Lora::Lora_IO_Battery(){
  // Lectura de la Fuente de Alimentación.
    // VBAT_CTRL=GPIO37 habilita el divisor; VBAT_ADC=GPIO1 lee el voltaje.
    // Se configura aqui para asegurar que SIEMPRE quede activo en el camino real del loop.
    static bool vbatAdcConfigured = false;
    if (!vbatAdcConfigured) {
      analogSetAttenuation(ADC_11db);
      analogReadResolution(12);
      vbatAdcConfigured = true;
    }

    int rawLow = -1;
    int rawHigh = -1;
    bool fallbackUsed = false;
    bool ctrlActiveLow = true;

    // Si por cualquier motivo la libreria devuelve ~0, hacemos una lectura manual robusta
    // y detectamos la polaridad real de VBAT_CTRL en tu hardware.
      fallbackUsed = true;
      pinMode(VBAT_CTRL, OUTPUT);

      digitalWrite(VBAT_CTRL, LOW);
      delay(8);
      rawLow = analogRead(VBAT_ADC);

      digitalWrite(VBAT_CTRL, HIGH);
      delay(8);
      rawHigh = analogRead(VBAT_ADC);

      pinMode(VBAT_CTRL, INPUT);

      int bestRaw = rawLow;
      if (rawHigh > rawLow) {
        bestRaw = rawHigh;
        ctrlActiveLow = false;
      }

      batteryVoltage = bestRaw / 238.7f;

    int batteryPercent = heltec_battery_percent(batteryVoltage);
    // >4.18V = batería al 100% / cargando activamente | <4.18V = batería descargando
    // Nota: con divisor resistivo no se puede distinguir USB vs batería llena con certeza
    Fuente_in_ST = (batteryVoltage < 3.95f);

    static unsigned long lastPrintTime = 0;
    // if (millis() - lastPrintTime >= 3000) {
      
    //     both.printf("🔋 Bat: %.2fV (%d%%) | %s\n",
    //                   batteryVoltage, batteryPercent,
    //                   Fuente_in_ST ? "USB/Fuente" : "Bateria");
    //   lastPrintTime = millis();
    // }
  }
void Lora::Lora_IO_Zones(){
  if(F_IO_Simulated){
    Lora_IO_Dummy_Simulate();
    return;
  }
  // 1. ZONE A y ZONE B Push Button Acknowledge.
    Zone_A_ACK    = digitalRead(PB_ZA_in);       // pulsador A. PB_ZA_in
    Zone_B_ACK    = digitalRead(PB_ZB_in);       // pulsador B.
    Zone_AB_ACK   = digitalRead(PB_ZC_in);       // pulsador C. Pulsador por defecto PRG.

  // 2. ZONE A y ZONE B Read input.
    Zone_A_in_ST  = digitalRead(Zona_A_in);
    Zone_B_in_ST  = digitalRead(Zona_B_in);

  // 3. OUTPUT A OUTPUT B Read.
    Rele_1_out_ST = digitalRead(Rele_1_out);
    Rele_2_out_ST = digitalRead(Rele_2_out);
    
  // 4. Lectura de la Fuente de Alimentación.
    Lora_IO_Battery();

  // 4  ZONES AB RESET con el pulsador C.
    if(!Zone_AB_ACK){
      delay(20); // Antirebote simple
      if(!Zone_AB_ACK){ // Verificar que el botón sigue presionado después del retardo
      Lora_IO_Zone_A_ACK();
      Lora_IO_Zone_B_ACK();
      }
      else{
        return; // Si el botón no está presionado, salir sin hacer nada
      }

    }
  // 5. ZONE  A RESET= Zona A aceptada desde el pulsador activo en bajo "0"
    if(!Zone_A_ACK){
      delay(20); // Antirebote simple
      if(!Zone_A_ACK){ // Verificar que el botón sigue presionado después del retardo
      Lora_IO_Zone_A_ACK();
      }
      else{
        return; // Si el botón no está presionado, salir sin hacer nada
      }
    }
  // 6. ZONE  B RESET= Zone B aceptada desde el pulsador activo en bajo "0"
    if(!Zone_B_ACK){
      delay(20); // Antirebote simple
      if(!Zone_B_ACK){ // Verificar que el botón sigue presionado después del retardo
      Lora_IO_Zone_B_ACK();
      }
      else{
        return; // Si el botón no está presionado, salir sin hacer nada
      }

    }
  // 7. ZONE  A ACTIVA - Timer secuencial: 3s confirmación, luego 3s más para error.
    if(!Zone_A_in_ST){
      if(!timer_ZA_En){
        // Primer timer: 3 segundos para confirmación (Zone_A_ST = true)
        Timer_ZoneA_Enable.once_ms(time_Zone_Actived, Lora_time_ZoneA_reach);
        // Iniciar segundo timer de 6 segundos para confirmar (Zone_A_ERROR = true)
        Timer_ZoneA_Error.once_ms(time_Zone_Fall, Lora_time_ZoneA_error);
        timer_ZA_En=true;
        Serial.println("ZA_Timers_EN");
      }
    }
  // 8. ZONE  B ACTIVA - Sistema de múltiples niveles temporales.
    if(!Zone_B_in_ST){
      if(!timer_ZB_En){
        // Nivel 1: 3 segundos - Zona confirmada (Zone_B_ST = true)
        Timer_ZoneB_Enable.once_ms(time_Zone_Actived, Lora_time_ZoneB_reach);
        // Nivel 2: 6 segundos - Tiempo alcanzado (Zone_B_Extended = true)
        Timer_ZoneB_Error.once_ms(time_Zone_Fall, Lora_time_ZoneB_error);
        timer_ZB_En=true;
        Serial.println("ZB_Timers_EN");
      }
    }
  // 11. Evento en Zonas.
    if(F_IO_Event_Enable){
      msg_enviar = true;
      msg_enviado=0;
    }
  // 12 ZONAS para mostrar en Pantalla  OLED
    //ZONES INPUTS
    if(!Zone_A_ERR){
      Zone_A_str=String(Zone_A_ST, BIN);
    }
    else{
      Zone_A_str="2";
    }
    if(!Zone_B_ERR){
      Zone_B_str=String(Zone_B_ST, BIN);
    }
    else{
      Zone_B_str="2";
    }
    //PUSHBUTTON INPUTS
    Zone_A_ACK_str=String(!Zone_A_ACK, BIN);
    Zone_B_ACK_str=String(!Zone_B_ACK, BIN);
    // SALIDAS
    Rele_1_out_str=String(Rele_1_out_ST, BIN);
    Rele_2_out_str=String(Rele_2_out_ST, BIN);
    //SORUCE INPUT
    Fuente_in_str=String(Fuente_in_ST, BIN);
 }
void Lora::Lora_IO_Dummy_Simulate(){
  // 1. Simulacion de Paquete.
    // Node_Num_str = String(random(3, 6)); // Random between "3" and "5"
    // Node_Status_str = String(random(0, 2)); // Random between "0" and "1"
    Node_Status_str = "1";
    // Zone_A_str = String(random(0, 2));    // Random between "0" and "1"
    Zone_A_str = "0";
    // Zone_B_str = String(random(0, 2));    // Random between "0" and "1"
    Zone_B_str = "0";
    // Rele_1_out_str = String(random(0, 2)); // Random between "0" and "1"
    Rele_1_out_str = "0";
    // Rele_2_out_str = String(random(0, 2)); // Random between "0" and "1"
    Rele_2_out_str = "0";
    // Fuente_in_str = String(random(0, 2)); // Random between "0" and "1"
    // Fuente_in_str = "0";

  // 4. Lectura de la Fuente de Alimentación.
    Lora_IO_Battery();
    Fuente_in_str=String(Fuente_in_ST, BIN);
  }

void Lora::Lora_IO_Zones_Force(){
  // 1. Fuerza de Zonas A y B.
  if(Zone_A_Forzar) Zone_A_ST = Zone_A_Force;
  if(Zone_B_Forzar) Zone_A_ST = Zone_B_Force;
  if(Fuente_in_Forzar) Fuente_in_ST = Fuente_in_Force;
 }
void Lora::Lora_IO_Zone_A_ACK(){
  Zone_A_ST=false;
  Zone_A_ERR=false;
  Zone_A_Extended=false;
  Zone_B_Extended=false;
  timer_ZA_Reached=false;
  timer_ZA_En=false;
  F_IO_Event_Enable = false;
  Zone_A_F_str='.';
  Zone_A_str=String(Zone_A_ST, BIN);
  // Implementacion Futura.
  bitClear(Zonas, Zone_A_ST);
  bitClear(Zonas_Fallan, Zone_A_ST);
 }
void Lora::Lora_IO_Zone_B_ACK(){
  Zone_B_ST=false;
  Zone_B_ERR=false;
  timer_ZB_Reached=false;
  timer_ZB_En=false;
  Zone_B_Extended=false;
  F_IO_Event_Enable = false;
  Zone_B_F_str='.';
  Zone_B_str=String(Zone_B_ST, BIN);
  // Implementacion Futura.
  bitClear(Zonas, Zone_B_ST);
  bitClear(Zonas_Fallan, Zone_B_ST);
 }

void Lora::Lora_time_ZoneA_reach(){
  nodeInstance->timer_ZA_Reached=true;
  if(!(nodeInstance->Zone_A_in_ST) && !(nodeInstance->Zone_A_ST)){
    // Zona confirmada después de 3 segundos
    nodeInstance->Zone_A_ST=true;
    nodeInstance->F_IO_Event_Enable=true;
    nodeInstance->Zone_A_str=String(nodeInstance->Zone_A_ST, BIN);
    Serial.println("Zone_A_ST true");
  }
 }
void Lora::Lora_time_ZoneA_error(){
  // Si después de 3 segundos más la zona sigue activa, activar bandera de error
  nodeInstance->timer_ZA_En=false; // Desactivar timer de confirmación para evitar múltiples activaciones
  if(!(nodeInstance->Zone_A_in_ST)){
    nodeInstance->Zone_A_ERR=true;
    nodeInstance->F_IO_Event_Enable=true;
    Serial.println("ZA_ERROR true");
  }
 }
void Lora::Lora_time_ZoneB_reach(){
  nodeInstance->timer_ZB_Reached=true;
  if(!(nodeInstance->Zone_B_in_ST) && !(nodeInstance->Zone_B_ST)){
    // Zona confirmada después de 3 segundos
    nodeInstance->Zone_B_ST=true;
    nodeInstance->F_IO_Event_Enable=true;
    nodeInstance->timer_ZB_En=false; // Desactivar timer de confirmación para evitar múltiples activaciones
    nodeInstance->Zone_B_str=String(nodeInstance->Zone_B_ST, BIN);
    Serial.println("Zone_B_ST true");
  }
 }
void Lora::Lora_time_ZoneB_error(){
  // Si después de 3 segundos más la zona sigue activa, activar bandera de error
  nodeInstance->timer_ZB_En=false; // Desactivar timer de confirmación para evitar múltiples activaciones
  if(!(nodeInstance->Zone_B_in_ST)){
    nodeInstance->Zone_B_ERR=true;
    nodeInstance->F_IO_Event_Enable=true;
    Serial.println("ZB_ERROR true");
  }
 }

void Lora::Lora_Event_Disable(){
  Timer_Nodo_Answer.detach();
  F_IO_Event_Enable = false;
  }

void Lora::Lora_timerNodo_Answer(){
  // 1. Timer para enviar el mensaje al maestro.
    if (nodeInstance) {
      nodeInstance->F_Responder = true; // Acceder a la variable de instancia a través del puntero global
  }
 }
void Lora::Lora_Timer_Enable(int answerTime){
    Timer_Nodo_Answer.once(answerTime,Lora_timerNodo_Answer);
  }

void Lora::Protocol_Master_Calibration(){
  // if(Protocol.NextSurvey){
  //   Survey_Calibration_Node();
  // }
  if(F_Recibido){
    // Protocol_ProcesarMensajesRecibidos();
    Survey_MeasureNodeSignal();
  }
  // if (F_NodeStatusUpdate || Protocol.nodeNoResponde || Protocol.nodeAlerta) {
  //   Protocol_NodeStatusUpdate();
  // }
  if(F_Node_Calibrated){
    Survey_FinishCalibration();
  }
}
void Lora::Survey_Calibration_Node(){
  // ✅ CORREGIDO: Verificar que hay un nodo para calibrar
  if (nodo_a_Consultar.length() == 0) {
    Serial.println("❌ Error: No hay nodo especificado para calibrar");
    nodo_a_Consultar = "1"; // Usar nodo 1 por defecto
  }
  
  Serial.println("🎯 Enviando survey a nodo: " + nodo_a_Consultar);
  
  // Lora_Master_Counter();
  // Lora_Master_Frame();  // Antes de enviar el mensaje se prepara la trama del nodo.
  // Lora_TX();
  // Protocol.NextSurvey = false;    // Resetear la bandera
  
  Serial.printf("📊 Survey enviado - Contador: %d\n", Master_Counter);
}
void Lora::Survey_MeasureNodeSignal() {
  totalRSSI += RSSI;
  validSamples++;
  
  Serial.printf("📊 Muestra %d - RSSI: %.1f dBm - Total acumulado: %.1f\n", 
                validSamples, RSSI, totalRSSI);
  
  // ✅ CORREGIDO: Lógica de cálculo y finalización
  if(validSamples >= 10) {
    avgRSSI = totalRSSI / validSamples;
    F_Node_Calibrated = true;
    
    Serial.printf("✅ Calibración completa - RSSI promedio: %.1f dBm\n", avgRSSI);
    
    if (avgRSSI > -70) {
      // Nodo cercano - configuración rápida
      Serial.println("⚙️🛠️ Nodo " + nodo_a_Consultar + " configurado para ALTA VELOCIDAD");
    }
    else if (avgRSSI > -90) {
        // Nodo medio - configuración balanceada
        Serial.println("⚙️🛠️ Nodo " + nodo_a_Consultar + " configurado para VELOCIDAD MEDIA");
    }
    else {
        // Nodo lejano - configuración robusta
        Serial.println("⚙️🛠️ Nodo " + nodo_a_Consultar + " configurado para MÁXIMO ALCANCE");
    }
  } else {
    Serial.printf("🔄 Calibración en progreso: %d/10 muestras\n", validSamples);
  }
}
void Lora::Survey_FinishCalibration(){
  Serial.println("🏁 Finalizando calibración del Master");
  // ✅ LIMPIAR FLAGS Y VARIABLES
  // Protocol.Master_Calibration_End();
  F_Node_Calibrated = false;
  F_MasterCalibration = false;

  
  // ✅ RESETEAR VARIABLES DE CALIBRACIÓN
  totalRSSI = 0;
  validSamples = 0;
  avgRSSI = 0;
  Master_Counter = 0;
  
  Serial.println("✅ Calibración finalizada - Variables reseteadas");
  Serial.println("🔄 Retornando al modo Master normal");
  Serial.println("F_MasterCalibration: " + String(F_MasterCalibration));
  Serial.println("F_Node_Calibrated: " + String(F_Node_Calibrated));
  Serial.println("F_Master_Mode: " + String(F_MasterMode));
  // Serial.println("F_Node_Mode: " + String(Protocol.F_Calibration_Complete));
  // Serial.println("F_Node_Calibrated: " + String(Protocol.F_Calibration_EN));

}



void Lora::SaveRadioConfigToNVS(int config) {
  preferences.begin(NVS_NAMESPACE, false); // false = modo escritura
  
  size_t bytesWritten = preferences.putInt(NVS_RADIO_CONFIG_KEY, config);
  
  if (bytesWritten > 0) {
    Serial.printf("💾 Configuración guardada en NVS: %d\n", config);
  } else {
    Serial.println("❌ Error al guardar configuración en NVS");
  }
  
  preferences.end();
}
int Lora::LoadRadioConfigFromNVS() {
  preferences.begin(NVS_NAMESPACE, true); // true = modo solo lectura
  
  // Obtener valor, -1 como valor por defecto si no existe
  int config = preferences.getInt(NVS_RADIO_CONFIG_KEY, -1);
  
  preferences.end();
  
  if (config != -1) {
    Serial.printf("📄 Configuración cargada desde NVS: %d\n", config);
  } else {
    Serial.println("📄 No hay configuración guardada en NVS");
  }
  
  return config;
}
void Lora::ClearRadioConfigNVS() {
  preferences.begin(NVS_NAMESPACE, false); // false = modo escritura
  
  bool success = preferences.remove(NVS_RADIO_CONFIG_KEY);
  
  if (success) {
    Serial.println("🗑️ Configuración borrada de NVS");
  } else {
    Serial.println("❌ Error al borrar configuración de NVS");
  }
  
  preferences.end();
}
bool Lora::HasStoredRadioConfig() {
  preferences.begin(NVS_NAMESPACE, true); // true = modo solo lectura
  
  bool hasKey = preferences.isKey(NVS_RADIO_CONFIG_KEY);
  
  preferences.end();
  
  return hasKey;
}
void Lora::SetRadioConfigFromMaster(int config) {
  Serial.printf("📡 Master solicita cambio de configuración a: %d\n", config);
  
  // Validar rango de configuración
  if (config < 0 || config > 4) {
    Serial.printf("❌ Configuración inválida: %d (debe ser 0-4)\n", config);
    return;
  }
  
  // Guardar nueva configuración
  SaveRadioConfigToNVS(config);
  
  // Aplicar inmediatamente la nueva configuración
  Serial.println("🔄 Aplicando nueva configuración de radio...");
  Lora_Configure(config);
  
  // Confirmar cambio
  Serial.printf("✅ Configuración cambiada a: %d y guardada en memoria\n", config);
  
  // Opcional: Enviar confirmación al Master
  // tx_mensaje = "CFG_OK_" + String(config);
}
void Lora::StartCalibration(String nodeToCalibrate) {
  if (!F_MasterMode) {
    Serial.println("❌ Error: Solo el Master puede iniciar calibración");
    return;
  }
  
  if (F_MasterCalibration) {
    Serial.println("⚠️ Advertencia: Calibración ya está activa");
    return;
  }
  Serial.println("🚀 Iniciando calibración del nodo: " + nodeToCalibrate);
  Serial.println("📊 Variables de calibración inicializadas");
  // Protocol.Master_Calibration_Init();
  // ✅ CONFIGURAR CALIBRACIÓN
  // nodo_a_Consultar = nodeToCalibrate;
  F_MasterCalibration = true;
  
  // ✅ RESETEAR VARIABLES
  totalRSSI         = 0;
  validSamples      = 0;
  avgRSSI           = 0;
  Master_Counter    = 0;
  F_Node_Calibrated = false;
}
bool Lora::IsCalibrationActive() {
  return F_MasterCalibration;
}