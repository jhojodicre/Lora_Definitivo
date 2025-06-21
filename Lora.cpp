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
#include <Arduino.h>
#include <Ticker.h>
#include <Functions.h>
#include "Master.h"
#include <PubSubClient.h>
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

// LoRa bandwidth. Keep the decimal point to designate float.
// Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
#define BANDWIDTH 250.0

// Number from 5 to 12. Higher means slower but higher "processor gain",
// meaning (in nutshell) longer range and more robust against interference.
#define SPREADING_FACTOR 9

// Transmit power in dBm. 0 dBm = 1 mW, enough for tabletop-testing. This value can be
// set anywhere between -9 dBm (0.125 mW) to 22 dBm (158 mW). Note that the maximum ERP
// (which is what your antenna maximally radiates) on the EU ISM band is 25 mW, and that
// transmissting without an antenna can damage your hardware.
#define TRANSMIT_POWER 0
volatile    bool rxFlag = false;
Functions Update (false);
Ticker      Timer_Nodo_Answer;


Lora* nodeInstance = nullptr; // Puntero global al objeto Master

Master  Node_1("1", "0", "0","0"); // Instancia de Nodo en el Perimetro
Master  Node_2("2", "0", "0","0"); // Instancia de Nodo en el Perimetro
Master  Node_3("3", "0", "0","0"); // Instancia de Nodo en el Perimetro

Lora::Lora(char nodeNumber){
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
        local_Address = nodeNumber; // Direccion del nodo local.
        F_Nodo_Excecute=false;
        nodeInstance = this; // Asignar la instancia actual al puntero global
}

void Lora::Lora_Setup()
{
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
    // RADIOLIB(radio.transmit(String(mensaje).c_str()));
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
    rx_funct_mode       = rxdata.charAt(3);         // Tipo de funcion a ejecutar.
    rx_funct_num        = rxdata.charAt(4);         // Numero de funcion a ejecutar.
    rx_funct_parameter1 = rxdata.substring(4, 5).toInt(); // Parametro 1 de la Funcion.
    rx_funct_parameter2 = rxdata.substring(5, 6).toInt(); // Parametro 2 de la Funcion.


    rx_mensaje_DB       = rxdata.substring(2, 3);         // Mensaje recibido.
    // rx_ST_ZA_DB         = rxdata.substring(3, 4);         // Estado de la Zona A.
    rx_ST_ZA_DB         = rxdata.substring(5, 6);         // Estado de la Zona B.
    rx_ST_ZB_DB         = rxdata.substring(6, 7);         // Estado de la Fuente.
  }
void Lora::rx(){
  rxFlag = true;
 }
void Lora::Lora_IO_Zones(){
  // 1. Pulsadores A y B Lectura.
    Zone_A_ACK=digitalRead(PB_ZA_in);       // pulsador A. PB_ZA_in
    Zone_B_ACK=digitalRead(PB_ZB_in);       // pulsador B.
    Zone_AB_ACK=digitalRead(PB_ZC_in);      // pulsador C. Pulsador por defecto PRG.
  // 2. Zona A y Zona B Lectura.
    Zone_A=digitalRead(Zona_A_in);
    Zone_B=digitalRead(Zona_B_in);
  // 3. Bateria o Fuente Lectura.
    Fuente_in_ST=digitalRead(Fuente_in);

  // 4. ZONAS A y B Reconocimiento.
    // Pulsador C = Realiza reconocimiento de Ambas Zonas A y B. Reset de Ambas Zonas y Fallas.
    if(!Zone_AB_ACK){
      bitClear(Zonas, Zone_A);      // ZONA A Reset.
      bitClear(Zonas, Zone_B);      // ZONA B Reset.

      bitClear(Zonas_Fallan, Zone_A);     // ZONA A FALLA Reset.
      bitClear(Zonas_Fallan, Zone_B);     // ZONA B FALLA Reset.

      Zone_A_F_str='.';
      Zone_B_F_str='.';

      Zone_A_ERR=false;
      Zone_B_ERR=false;
    }
  // 5. ZONA A RESET= Zona A aceptada desde el pulsador activo en bajo "0"
    if(!Zone_A_ACK){
      bitClear(Zonas, Zone_A);
      bitClear(Zonas_Fallan, Zone_A);
      Zone_A_ERR=false;
      Zone_A_F_str='.';
      Zone_A_ST=false;
    }
  // 6. ZONE B RESET= Zone B aceptada desde el pulsador activo en bajo "0"
    if(!Zone_B_ACK){
      bitClear(Zonas, Zone_B);
      bitClear(Zonas_Fallan, Zone_B);
      Zone_B_ERR=false;
      Zone_B_F_str='.';        
      Zone_B_ST=false;
    }

  // 7. ZONA A ACTIVA.
    if(!Zone_A){
      bitSet(Zonas, Zone_A);
      Zone_A_ST=true;
    }
  // 8. ZONE B ACTIVA.
      if(!Zone_B){
        bitSet(Zonas, Zone_B);
        Zone_B_ST=true;
      }
  // 10 ZONAS para mostrar en Pantalla  OLED
    Zone_A_str=String(Zone_A_ST, BIN);
    Zone_B_str=String(Zone_B_ST, BIN);

    Zone_A_ACK_str=String(!Zone_A_ACK, BIN);
    Zone_B_ACK_str=String(!Zone_B_ACK, BIN);

    Fuente_in_str=String(Fuente_in_ST, BIN);
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

    tx_remitente          =String(local_Address);
    tx_destinatario       =String(Master_Address);    // Direccion del maestro.
    tx_mensaje            ="M";
    tx_funct_mode         ="A"; // Tipo de funcion a ejecutar.
    tx_funct_num          ="1";     // Numero de Nodo en el Perimetro.
    tx_funct_parameter1   ="a";//Zone_A_str; // Estado de la zona A
    tx_funct_parameter2   ="b";//Zone_B_str; // Estado de la Zona B
    
  // 2. Armamos el paquete a enviar.
    txdata = String(  tx_remitente + tx_destinatario + tx_mensaje + tx_funct_mode + tx_funct_num + tx_funct_parameter1 + tx_funct_parameter2 );
    // txdata = "123";
    Timer_Nodo_Answer.once(2, Lora_timerNodo_Answer); // 500 ms para enviar el mensaje.
 }
void Lora::Lora_Nodo_Decodificar(){
  // 0. Functon Llamada desde L4.4 Nodo.F_Recibido.
  // 1. Preparamos mensaje para enviar.
    // Serial.println(String(local_Address));
    if(rx_destinatario==local_Address){
      Lora_Nodo_Frame();
      F_Nodo_Excecute=true;  //Flag Desactivado en L-4.3
      if(rx_funct_mode=='A' && rx_funct_num=='1'){
        F_function_Special=true; // Bandera Desactivada en L4.3
      }
    }
    // 2. Ejecutamos Funcion.
}
void Lora::Lora_Master_Frame(){
  //0. Funcion Llamada desde L5.2
  //1. Preparamos paquete para enviar
    tx_remitente        = Master_Address;                      // Direccion del maestro.
    tx_destinatario     = nodo_a_Consultar;          // Direccion del nodo local.
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
    Node_Status = true; // Comunicacion Ok
    Node_Status_str = "1"; // Comunicacion ok
  }
  else{
    Node_Status = false; // No comunica
    Node_Status_str = "0"; // Comunicacion  No ok
    // Si no es el nodo consultado, no se actualiza la base de datos.
    // nodo_DB = "{\"comm\":\"" + String(commJS) + "\",\"node\":\"" + String(rx_remitente) + "\",\"zoneA\":\"" + String(rx_ST_ZA_DB) + "\",\"zonaB\":\"" + String(rx_ST_ZB_DB) + "\",\"output1\":\"" + Rele_1_out_str +"\",\"output2\":\"" + Rele_2_out_str +"\",\"fuente\":\"" + String(rx_ST_FT_DB) + "\"}";
  }
  SerializeObjectToJson(); // Serializa el objeto a JSON
  Lora_Master_DB();
  F_Master_Update=true;
}
void Lora::Lora_Dummy_Simulate(){
  // 1. Simulacion de Paquete.
    commJS   = String(random(0, 2));    // Random between "0" and "1"
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
  doc[nodeJS]     = rx_remitente;
  doc[commJS]     = Node_Status_str;
  doc[zoneAJS]    = Zone_A_str;
  doc[zoneBJS]    = Zone_B_str;
  doc[output1JS]  = Rele_1_out_str;
  doc[output2JS]  = Rele_2_out_str;
  doc[fuenteJS]    = Fuente_in_str;
  serializeJson(doc, jsonString);
    // Serial.print("LORA_JSON String:");
    // Serial.println(jsonString);
  }
