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
#include <Functions.h>
#include <ArduinoJson.h>

// Turns the 'PRG' button into the power button, long press is off
#define HELTEC_POWER_BUTTON // must be before "#include <heltec_unofficial.h>"
#include <heltec_unofficial.h>


// Pause between transmited packets in seconds.
// Set to zero to only transmit a packet when pressing the user button
// Will not exceed 1% duty cycle, even if you set a lower value.
#define PAUSE 300

// Frequency in MHz. Keep the decimal point to designate float.
// Check your own rules and regulations to see what is legal where you are.
#define FREQUENCY 866.3 // for Europe
// #define FREQUENCY           905.2       // for US


// Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
#define BANDWIDTH 250.0 // Bandwidth in kHz. Higher means more data throughput, but also more noise.


// meaning (in nutshell) longer range and more robust against interference.
#define SPREADING_FACTOR 9 // Number from 5 to 12. Higher means slower but higher "processor gain",


// set anywhere between -9 dBm (0.125 mW) to 22 dBm (158 mW). Note that the maximum ERP
// (which is what your antenna maximally radiates) on the EU ISM band is 25 mW, and that
// transmissting without an antenna can damage your hardware.
#define TRANSMIT_POWER 0 // Transmit power in dBm. 0 dBm = 1 mW, enough for tabletop-testing. This value can be increased for longer range. 
volatile    bool rxFlag = false;

//Instancias
Functions   Update (false);
Ticker      Timer_Nodo_Answer;
Ticker      Timer_ZoneA_Enable;
Ticker      Timer_ZoneB_Enable;
Lora*       nodeInstance = nullptr; // Puntero global al objeto Master

Lora::Lora(bool isMaster, int NumNodes, char nodeNumber)
  : Protocol(isMaster, NumNodes) // Inicializa el atributo Master correctamente
{
  F_MasterMode  = isMaster;
  F_NodeMode    = !isMaster;
  local_Address = nodeNumber; // Direccion del nodo local.
  Num_Nodos     = NumNodes;

  // Constructor de la clase Node
    //1. Configuracion de Hardware
      pinMode(Zona_A_in, INPUT);
      pinMode(Zona_B_in, INPUT);
      pinMode(PB_ZA_in, INPUT);
      pinMode(PB_ZB_in, INPUT);
      pinMode(PB_ZC_in, INPUT);

      pinMode(Rele_1_out, OUTPUT);
      pinMode(Rele_2_out, OUTPUT);

    //2. Condicion Inicial.
      digitalWrite(Rele_1_out, LOW);
      digitalWrite(Rele_2_out, LOW);
      Zone_A = false;
      Zone_B = false;
      Protocol.Iniciar();
      F_Nodo_Excecute=false;
      nodeInstance = this; // Asignar la instancia actual al puntero global
}

void Lora::Lora_Setup(Functions* correr)
{
    correrRef = correr;
    heltec_setup();
    RADIOLIB_OR_HALT(radio.begin());
    // Set the callback function for received packets
    radio.setDio1Action(rx);
    // Set radio parameters
    RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
    RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
    RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
    RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
    // Start receiving
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
    
    // Inicializar el temporizador de Master si estamos en modo Master
    if (F_MasterMode) {
        Protocol.Iniciar(); // Esto inicia el temporizador dentro de la clase Master
    }

    // Set the display to show the radio status
    both.println("Radio init");
    both.printf("Frequency: %.2f MHz\n", FREQUENCY);
    both.printf("Bandwidth: %.1f kHz\n", BANDWIDTH);
    both.printf("Spreading Factor: %i\n", SPREADING_FACTOR);
    both.printf("TX power: %i dBm\n", TRANSMIT_POWER);
 }
void Lora::Lora_TX(){
    // both.printf("TX [%s] ", String(mensaje).c_str());
    both.printf("TX [%s] ", txdata.c_str());
    radio.clearDio1Action();
    heltec_led(50); // 50% brightness is plenty for this LED
    RADIOLIB(radio.transmit(txdata.c_str()));
    heltec_led(0);
    if (_radiolib_status == RADIOLIB_ERR_NONE)
    {
        both.printf("OK (%i ms)\n");
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
    Protocol.nodeResponde=F_Node_Atiende;
 }
void Lora::Lora_RX(){
    // If a packet was received, display it and the RSSI and SNR
    if (rxFlag)
    {
      rxFlag = false;
      radio.readData(rxdata);
      if (_radiolib_status == RADIOLIB_ERR_NONE)
      {
        both.printf("RX [%s]\n", rxdata.c_str());
        // both.printf("  RSSI: %.2f dBm\n", radio.getRSSI());
        // both.printf("  SNR: %.2f dB\n", radio.getSNR());
      }
      RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
      F_Recibido = true;  // Bandera activada en Lora_RX.
    }
    
    rx_remitente        = rxdata.charAt(0); // Nodo que envia el mensaje.
    rx_destinatario     = rxdata.charAt(1); // Nodo que recibe el mensaje.
    rx_mensaje          = rxdata.substring(2, 3);         // Mensaje recibido.
    rx_funct_mode       = rxdata.substring(3,4);         // Tipo de funcion a ejecutar.
    rx_funct_num        = rxdata.substring(4,5);         // Numero de funcion a ejecutar.
    rx_funct_parameter1 = rxdata.substring(5, 6); // Parametro 1 de la Funcion.
    rx_funct_parameter2 = rxdata.substring(6, 7); // Parametro 2 de la Funcion.
    rx_funct_parameter3 = rxdata.substring(7, 8); // Parametro 3 de la Funcion.
    rx_funct_parameter4 = rxdata.substring(8, 9); // Parametro 4 de la Funcion.

    rx_master_lora_1 = rxdata.substring(0, 1); // Direccion del nodo que responde.
    rx_master_lora_2 = rxdata.substring(1, 2); // Direccion del maestro.
    rx_master_lora_3 = rxdata.substring(2, 3); // Estado de la zona A.
    rx_master_lora_4 = rxdata.substring(3, 4); // Estado de la zona B.
    rx_master_lora_5 = rxdata.substring(4, 5); // Estado de la salida 1.
    rx_master_lora_6 = rxdata.substring(5, 6); // Estado de la salida 2.
    rx_master_lora_7 = rxdata.substring(6, 7); // Estado de la fuente.
    rx_master_lora_8 = rxdata.substring(7, 8); // Tipo de mensaje, si es de emergencia.

    rx_mensaje_DB       = rxdata.substring(2, 3);         // Mensaje recibido.
    // rx_ST_ZA_DB         = rxdata.substring(3, 4);         // Estado de la Zona A.
    rx_ST_ZA_DB         = rxdata.substring(5, 6);         // Estado de la Zona B.
    rx_ST_ZB_DB         = rxdata.substring(6, 7);         // Estado de la Fuente.
  }
void Lora::rx(){
  rxFlag = true;
 }
void Lora::Lora_IO_Zones(){
  // 1. Lectura de Pulsadores A y B.
    Zone_A_ACK    = digitalRead(PB_ZA_in);       // pulsador A. PB_ZA_in
    Zone_B_ACK    = digitalRead(PB_ZB_in);       // pulsador B.
    Zone_AB_ACK   = digitalRead(PB_ZC_in);       // pulsador C. Pulsador por defecto PRG.

  // 2. Lectura Zona A y Zona B.
    Zone_A        = digitalRead(Zona_A_in);
    Zone_B        = digitalRead(Zona_B_in);

  // 3. Lectura de Estado de Salidas.
    Rele_1_out_ST = digitalRead(Rele_1_out);
    Rele_2_out_ST = digitalRead(Rele_2_out);
    
  // 3.1 LLamada a la Funcion de Forazado.
      // Lora_IO_Zones_Force();
  // 4. ________________  
    bitClear(Zonas_Fallan, Zone_A);     // ZONA A FALLA Reset.
    bitClear(Zonas_Fallan, Zone_B);     // ZONA B FALLA Reset.

    Zone_A_F_str='.';
    Zone_B_F_str='.';

    Zone_A_ERR=false;
    Zone_B_ERR=false;
  //4 ZONES AB RESET con el pulsador C.
    if(!Zone_AB_ACK){
      bitClear(Zonas, Zone_A);
      bitClear(Zonas, Zone_B);
      bitClear(Zonas_Fallan, Zone_A);
      bitClear(Zonas_Fallan, Zone_B);
      Zone_A_ERR=false;
      Zone_B_ERR=false;
      Zone_A_F_str='.';
      Zone_B_F_str='.';        
      Zone_A_ST=false;
      Zone_B_ST=false;
      F_Event_Enable = true;
    }
  // 5. ZONA A RESET= Zona A aceptada desde el pulsador activo en bajo "0"
    if(!Zone_A_ACK){
      bitClear(Zonas, Zone_A);
      bitClear(Zonas_Fallan, Zone_A);
      Zone_A_ERR=false;
      Zone_A_F_str='.';
      Zone_A_ST=false;
      F_Event_Enable = true;
    }
  // 6. ZONE B RESET= Zone B aceptada desde el pulsador activo en bajo "0"
    if(!Zone_B_ACK){
      bitClear(Zonas, Zone_B);
      bitClear(Zonas_Fallan, Zone_B);
      Zone_B_ERR=false;
      Zone_B_F_str='.';        
      Zone_B_ST=false;
      F_Event_Enable = true;
    }
  // 7. ZONA A ACTIVA.
    if(!Zone_A){
      if(timer_ZA_En){
        Timer_ZoneA_Enable.once_ms(3000, Lora_time_ZoneA_reach); // Si pasan mas de 3 segundos.
        timer_ZA_En=false;
      }
    }
  // 8. ZONE B ACTIVA.
    if(!Zone_B){
      if(timer_ZB_En){
        Timer_ZoneB_Enable.once_ms(3000, Lora_time_ZoneB_reach); // Si pasan mas de 3 segundos.
        timer_ZB_En=false;
      }
    }
  // 9. Evento en Zonas.
    if(F_Event_Enable){
      msg_enviar = true;
      msg_enviado=0;
      Tipo_de_Mensaje="U";
    } 
    else{
      Tipo_de_Mensaje="A";
    }
  // 10 ZONAS para mostrar en Pantalla  OLED
    //ZONES INPUTS
    Zone_A_str=String(Zone_A_ST, BIN);
    Zone_B_str=String(Zone_B_ST, BIN);
    //PUSHBUTTON INPUTS
    Zone_A_ACK_str=String(!Zone_A_ACK, BIN);
    Zone_B_ACK_str=String(!Zone_B_ACK, BIN);
    //SORUCE INPUT
    Fuente_in_str=String(Fuente_in_ST, BIN);
    // SALIDAS
    Rele_1_out_str=String(Rele_1_out_ST, BIN);
    Rele_2_out_str=String(Rele_2_out_ST, BIN);
 }
void Lora::Lora_IO_Zones_Force(){
  // 1. Fuerza de Zonas A y B.
  if(Zone_A_Forzar) Zone_A = Zone_A_Force;
  if(Zone_B_Forzar) Zone_B = Zone_B_Force;
  if(Fuente_in_Forzar) Fuente_in_ST = Fuente_in_Force;
}
void Lora::Lora_IO_Zone_A_ACK(){
  Zone_A_ST=false;
  F_Event_Enable = false;
}
void Lora::Lora_IO_Zone_B_ACK(){
  Zone_B_ST=false;
  F_Event_Enable = false;
}
void Lora::Lora_Nodo_Frame(){
  // 0. Function Llamada desde Lora_Nodo_Decodificar.
  // 1. Preparamos paquete para enviar
    //Estados de Entradas.
    // bitWrite(nodo_local,0, );
    bitWrite(nodo_local,0, Zone_A_ST);
    bitWrite(nodo_local,1, Zone_B_ERR);
    bitWrite(nodo_local,2, Zone_B_ST);
    bitWrite(nodo_local,3, Zone_A_ERR);
    bitWrite(nodo_local,4, false);
    bitWrite(nodo_local,5, false);
    bitWrite(nodo_local,6, true);
    bitWrite(nodo_local,7, false);
    nodo_status=char(nodo_local);

    tx_nodo_lora_1          =String(local_Address);   // Direccion del nodo local.
    tx_nodo_lora_2          =String(Master_Address);  // Direccion del maestro.
    tx_nodo_lora_3          =Zone_A_str;              // Estado de la zona A      
    tx_nodo_lora_4          =Zone_B_str;              // Estado de la zona B
    tx_nodo_lora_5          =Rele_1_out_str;          // Estado de la Salida 1
    tx_nodo_lora_6          =Rele_2_out_str;          // Estado de la Salida 2
    tx_nodo_lora_7          =Fuente_in_str;           // Estado de la Fuente
    tx_nodo_lora_8          =Tipo_de_Mensaje;

  // 2. Armamos el paquete a enviar.
    txdata = String(  tx_nodo_lora_1 + tx_nodo_lora_2 + tx_nodo_lora_3 + tx_nodo_lora_4 + tx_nodo_lora_5 + tx_nodo_lora_6 + tx_nodo_lora_7 + tx_nodo_lora_8);
 }
void Lora::Lora_Nodo_Decodificar(){
  // 1. Preparamos mensaje para enviar.
    if(rx_destinatario==local_Address){
      Serial.println("Nodo_Atiende");
      if(rx_funct_mode!="0"){
        Serial.println("Peticion escuchada");
        F_Nodo_Excecute=true;  //Flag Desactivado en L-4.3
      }
      F_Responder=true;
      F_Node_Atiende=true;
      Protocol.nodeResponde=F_Node_Atiende;
    }
  }
void Lora::Lora_Node_Print(String z_executed){
  both.printf(z_executed.c_str());
  }
void Lora::Lora_Master_Frame(){
  //0. Funcion Llamada desde L5.2
  //1. Preparamos paquete para enviar
    tx_remitente        = Master_Address;                  // Direccion del maestro.
    tx_destinatario     = nodo_a_Consultar;                // Direccion del nodo local.
    tx_mensaje          = ".";                             // Estado del nodo en este byte esta el estado de las entradas si esta en error o falla


  //2. Armamos el mensaje para enviar.
    txdata = String(  tx_remitente + tx_destinatario + tx_mensaje + tx_funct_mode + tx_funct_num + tx_funct_parameter1 + tx_funct_parameter2 );
  //3. Borramos Variables de envio.
    nodo_consultado = tx_destinatario.charAt(0);
    tx_remitente=' ';
    tx_destinatario=' ';
    tx_mensaje=' ';
    tx_funct_mode=' ';
    tx_funct_num=' ';
    tx_funct_parameter1=' ';
    tx_funct_parameter2=' ';
  }
void Lora::Lora_Master_Decodificar(){
  if(rx_remitente==nodo_consultado){
    Node_Status = true; // UPDATE FLAG Comunicacion Ok
    Node_Status_str = "1"; // Comunicacion ok
  }
  else{
    if(rx_master_lora_8=="U"){
      txdata=(Master_Address + rx_remitente + "0" + "N" + "3" + "0" + "0" + "A");
      Lora_TX();
    }
  }
  Node_Num_str = String(rx_remitente); // Numero de Nodo consultado.
  SerializeObjectToJson(); // Serializa el objeto a JSON
  // Lora_Master_DB();
  F_ServerUpdate=true;
}
void Lora::Lora_Node_Print_RX(){
  Serial.print("RX: ");
  Serial.println(String(rx_destinatario));
  Serial.print("LA: ");
  Serial.println(String(local_Address));
  Serial.print("ms: ");
  Serial.println(String(rx_mensaje));
  Serial.print("md: ");
  Serial.println(String(rx_funct_mode));
  Serial.print("nf: ");
  Serial.println(String(rx_funct_num));
  Serial.print("p1: ");
  Serial.println(String(rx_funct_parameter1));
  Serial.print("p2: ");
  Serial.println(String(rx_funct_parameter2));
}

void Lora::Lora_Dummy_Simulate(){
  // 1. Simulacion de Paquete.
    Node_Num_str = String(random(3, 6)); // Random between "3" and "5"
    Node_Status_str = String(random(0, 2)); // Random between "0" and "1"
    Zone_A_str = String(random(0, 2));    // Random between "0" and "1"
    Zone_B_str = String(random(0, 2));    // Random between "0" and "1"
    Fuente_in_str = String(random(0, 2)); // Random between "0" and "1"
    Rele_2_out_str = String(random(0, 2)); // Random between "0" and "1"
    Rele_1_out_str = String(random(0, 2)); // Random between "0" and "1"
}
void Lora::Lora_timerNodo_Answer(){
  // 1. Timer para enviar el mensaje al maestro.
    if (nodeInstance) {
      nodeInstance->F_Responder = true; // Acceder a la variable de instancia a través del puntero global
  }
}
void Lora::Lora_time_ZoneA_reach(){
  nodeInstance->timer_ZA_En=true;
  if(!(nodeInstance->Zone_A)){
    nodeInstance->Zone_A_ST=true;
    nodeInstance->F_Event_Enable=true;
  }
}
void Lora::Lora_time_ZoneB_reach(){
  nodeInstance->timer_ZB_En=true;
  if(!(nodeInstance->Zone_B)){
    nodeInstance->Zone_B_ST=true;
    nodeInstance->F_Event_Enable=true;
  }
}
void Lora::Lora_Master_DB(){
  switch (rx_remitente){
    case '1':
      nodo_DB = jsonString; // Serializa el objeto a JSON
      // nodo_DB = "{\"comm\":\"" + String(rx_remitente) +\"node\":\"" + String(rx_remitente) + "\",\"zoneA\":\"" + String(rx_ST_ZA_DB) + "\",\"zonaB\":\"" + String(rx_ST_ZB_DB) + "\",\"output1\":\"" + Rele_1_out_str +"\",\"output2\":\"" + Rele_2_out_str +"\",\"fuente\":\"" + String(rx_ST_FT_DB) + "\"}";
      break;
    case '2':
      nodo_DB = jsonString; // Serializa el objeto a JSON

      break;
    default:
      break;
  }
}
void Lora::SerializeObjectToJson() {
  
    doc[nodeJS]     = Node_Num_str;     // Numero de Nodo consultado
    doc[commJS]     = Node_Status_str;  // Estado de la comunicacion
    doc[zoneAJS]    = rx_master_lora_3; // Estado de la zona A
    doc[zoneBJS]    = rx_master_lora_4; // Estado de the zona B
    doc[output1JS]  = rx_master_lora_5; // Estado de the salida 1
    doc[output2JS]  = rx_master_lora_6; // Estado de the salida 2
    doc[fuenteJS]   = rx_master_lora_7; // Estado de the fuente
    serializeJson(doc, jsonString);

    Serial.print("LORA_JSON String:");
    Serial.println(jsonString);
}
void Lora::Lora_WebMessage(String mensaje) {
    tx_funct_mode=mensaje.charAt(2);          // Modo de Funcion a ejecutar.
    tx_funct_num=mensaje.charAt(3);           // Numero de Funcion a ejecutar.
    tx_funct_parameter1=mensaje.charAt(4);    // Primer parametro de Funcion a ejecutar.
    tx_funct_parameter2=mensaje.charAt(5);    // Segundo parametro de Funcion a ejecutar.
    F_Master_Excecute=true; // Flag desactivado en L5.4
}
void Lora::Lora_Timer_Enable(int answerTime){
    Timer_Nodo_Answer.once(answerTime,Lora_timerNodo_Answer);
}
void Lora::Lora_Event_Disable(){
  Timer_Nodo_Answer.detach();
  F_Event_Enable = false;
}

void Lora::Lora_Protocol(){
  /**
   * @brief Gestiona el protocolo de comunicación según el modo (Master o Nodo)
   * 
   * Este es el punto de entrada principal para la gestión del protocolo
   * y se debe llamar regularmente desde el loop principal.
   */
  Lora_RX();
  
  // En modo Nodo, ejecuta el protocolo para nodos
  if (F_NodeMode) {
    Lora_Node_Protocol();
  }
  // En modo Master, gestiona el ciclo del protocolo
  if (F_MasterMode) {
    Lora_Master_Protocol();
  }
}
void Lora::Lora_Node_Protocol(){
  //-P.1 LORA RX
  //-P.2 Node IO.
    Lora_IO_Zones(); // Se actualizan los estados de las zonas.  // Node.Lora_Dummy_Simulate(); // Se simulan las señales de entrada.
  //-P.3 Nodo Evento en Zonas
    if(F_Event_Enable && msg_enviar){
      Serial.println("event");
      while(msg_enviado<2){
        Lora_TX();
        delay(100);
        ++ msg_enviado;
      }
      msg_enviar=false;
      F_Event_Enable = false;
    }
  //-P.4 Nodo RX.
    if(F_Recibido){
      Lora_Nodo_Decodificar();        // Se recibe el mensaje.
      F_Recibido=false;               // Flag activado desde Lora_Nodo_Decodificar Se resetea la bandera de recepcion.
    }
  //-P.5 Nodo Ejecuta Funciones.
    if(F_Nodo_Excecute){
      correrRef->Functions_Request(rx_funct_mode + rx_funct_num + rx_funct_parameter1 + rx_funct_parameter2);
      correrRef->Functions_Run();
      F_Nodo_Excecute=false;
    }
  //-P.6 Nodo TX.
    if(F_Responder){
      Lora_Nodo_Frame();    // Antes de enviar el mensaje se prepara la trama del nodo.
      Lora_TX();            // Se envia el mensaje.
    }
}
void Lora::Lora_Master_Protocol(){
   /**
   * @brief Implementa el protocolo para el modo Master
   * 
   * Este método maneja el ciclo completo del protocolo Master:
   * 1. Revisa si a llegado un Nuevo Mensaje.
   * 2. Prepara El siguiente nodo a ser consultado.
   * 3. Actualiza el Servidor.
   * 4. Ejecuta ordenes desde el Servidor.
   */
    // Procesar mensajes recibidos en modo Master usando el método dedicado
    if (F_Recibido) {
      Protocol_ProcesarMensajesRecibidos();
    }
    // El temporizador en Master.cpp activa la bandera Protocol.Next para consultar el siguiente nodo
    if (Protocol.Next) {
      Protocol_ConsultarNodoSiguiente();
    }
    // Actualizar el status del nodo que no responde
    if (F_ServerUpdate){
      Protocol_UpdateServer();
    }
    //-Master Execute order from Server
    if(F_Master_Excecute){
      Protocol_ExecuteOrderFromServer();
    }
}
void Lora::Protocol_ConsultarNodoSiguiente(){
  Protocol.Master_Nodo();
  nodo_a_Consultar = String(Protocol.Nodo_Proximo); // Convertir el número de nodo a String
  Lora_Master_Frame();      // Prepara la trama del maestro
  Lora_TX();                // Envía el mensaje
  F_ServerUpdate = true; // Indicar que se debe actualizar el servidor
  Protocol.Next = false;    // Resetear la bandera
}
void Lora::Protocol_ProcesarMensajesRecibidos() {
  /**
   * @brief Procesa mensajes recibidos en modo Master
   */
    Serial.println("Procesando mensaje recibido...");
    // Decodificar el mensaje recibido
    Protocol.ProcesarMensaje(rxdata);
    F_ServerUpdate = true; // Indicar que se debe actualizar el servidor    
    
    F_Recibido = false;// Reset de la bandera de recepción
    
    Serial.print("Lora RX: ");
    Serial.println(String(rxdata));
    // Si el mensaje requiere acción especial, tomar medidas adicionales
    // if (requiereAccionEspecial) {
    //   Serial.println("¡ACCIÓN ESPECIAL REQUERIDA!");
    //   // Implementar acciones especiales aquí
    //   // Ejemplos:
    //   // - Notificar a un servidor
    //   // - Activar alguna alerta
    //   // - Enviar comandos adicionales
    // }
}
void Lora::Protocol_UpdateServer(){
  /**
   * @brief Actualiza el estado del nodo en el servidor
   *
   * Este método puede ser llamado por los siguientes eventos:
   * -1 Cuando un Nodo Responde correctamente
   * -2 Cuando un Nodo no responde a la consulta.
   * -3 Cuando un Nodo cambia el estado de sus entradas (Zonas) nodo en Alerta.
   */
  if(Protocol.nodeResponde){    // Si el nodo respondió correctamente
    Protocol.nodeResponde = false; // Resetear la bandera para la próxima consulta
    Node_Status_str = "1"; // Comunicacion ok
    Node_Num_str    = String(Protocol.Nodo_Consultado); // Numero de Nodo consultado
    Serial.println("Nodo responde timer activo");
    SerializeObjectToJson();// Serializar para enviar al servidor/DB
    return; // No es necesario actualizar el servidor si el nodo respondió
  }
  if(Protocol.nodeNoResponde){  // Si el nodo NO respondió a la consulta
    Protocol.nodeNoResponde = false; // Resetear la bandera para la próxima consulta
    Node_Status_str = "0"; // Nodo no responde
    Node_Num_str    = String(Protocol.Nodo_Consultado); // Numero de Nodo consultado
    
    // Poner los estados de las zonas y salidas como ="0" (desconocido)
    rx_master_lora_3 = "0"; // Estado de la zona A
    rx_master_lora_4 = "0"; // Estado de la zona B
    rx_master_lora_5 = "0"; // Estado de la salida 1
    rx_master_lora_6 = "0"; // Estado de la salida 2
    rx_master_lora_7 = "0"; // Estado de la fuente

    // Serializar para enviar al servidor/DB
    SerializeObjectToJson();
    Serial.print("Nodo ");
    Serial.print(Protocol.Nodo_Consultado);
    Serial.println(" no respondió a la consulta anterior");
    return;
  }
  if(Protocol.nodeAlerta){                  // Si el nodo cambió el estado de sus entradas (Zonas)
    Protocol.nodeAlerta = false;            // Resetear la bandera para la próxima consulta
    Node_Status_str = "1";                  // Comunicacion ok
    Node_Num_str    = String(Protocol.Nodo_Actual); // Numero de Nodo consultado
    SerializeObjectToJson();                // Serializa el objeto a JSON
    return;
  }
  F_ServerUpdate = true;
}
void Lora::Protocol_ExecuteOrderFromServer() {
  /**
   * @brief Ejecuta órdenes recibidas desde el servidor
   * 
   */
  Lora_Master_Frame();             // 2. Se prepara el mensaje a enviar.
  Lora_TX();                       // 3. Se envia el mensaje.
  F_Master_Excecute=false;         // 4. Se Desactiva la bandera Master_Excecute.
  Serial.println("Server->Master->Node");
}

void Lora::Protocol_porImplementar(){
  /**
   * @brief Método placeholder para futuras implementaciones del protocolo
   * 
   * Este método está reservado para futuras expansiones o modificaciones
   * del protocolo de comunicación.
   */
  // Implementar futuras funcionalidades del protocolo aquí
    // Ejecutar la gestión periódica del protocolo
    Protocol.Gestion();

  // Gestionar la base de datos del Master periódicamente
  static unsigned long ultimaActualizacionDB = 0;
  if (millis() - ultimaActualizacionDB > 30000) { // Cada 30 segundos
    Protocol.Master_DB(); // Actualizar/mostrar base de datos de nodos
    ultimaActualizacionDB = millis();
  }
}

