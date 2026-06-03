//1. librerias.
  //-1.1 Librerias
    #include "Functions.h"
    #include "General.h"
    #include "Lora.h"
    #include <HTTPClient.h>
    #include "NodeWebServer.h"
    #include "Master.h"

//2. Variables Globales.                                               
  //-2.1 Variables Interrupciones
    
    volatile bool flag_ISR_stringComplete=false;    // Flag: mensaje Serial Recibido completo.
    String        inputString;           // Buffer recepcion Serial.

  //-2.2 Variables Banderas. 
    bool          flag_F_codified_funtion=false;    // Notifica que la funcion ha sido codificada.
    bool          F_iniciado=false;                 // Habilitar mensaje de F_iniciado por unica vez
  
  //-2.3 JSON Variables.
    String  jsonString; 
    const unsigned long SERVER_UPDATE_INTERVAL_MS = 1500;
    unsigned long lastServerUpdateMs = 0;
  //-2.4 Variables del Protocolo.
    const int  CHISMOSO_TOTAL_NODOS  = 5;
    const char CHISMOSO_NODE_ADDRESS = '3';
    const bool MASTER                = true; // Cambiar a false para modo Nodo.
    const bool IO_DISABLE            = true; // Cambiar a true para deshabilitar funciones de IO (útil para pruebas sin hardware conectado)

//3. Intancias.
  //-3.1 Clases propias.
    Functions Correr(true);                 // Funciones a Ejecutar
    General   General(false);               // Configuraciones Generales del Nodo.
    Lora      Node(IO_DISABLE);               // Clase de Comunicacion Lora.
    Master    Chismoso(MASTER, CHISMOSO_TOTAL_NODOS, CHISMOSO_NODE_ADDRESS);      // Master: true, Numero de Nodos: 5, Direccion del Nodo: '1'
  //-3.2 Clases de Protocolos.
    LoRaWebServer webServer(80);            // AGREGAR ESTA LÍNEA
//4. Funciones ISR.
  //-4.1 Serial Function.
    void serialEvent (){
      while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        inputString += inChar;
        // if the incoming_function character is a newline, set a flag so the main loop can
        // do something about it:
        if (inChar == '\n') {
          flag_ISR_stringComplete = true;
          flag_F_codified_funtion=false;
        }
      }
    }
void setup(){
  //S1. Condiciones Iniciales.
    Serial.begin(115200);
    delay(1000);  // Esperar que termine el boot del ROM
    Serial.println("\n=== 🚀 INICIANDO SISTEMA LORA ===");
    Serial.printf("📍 Nodo: %c\n", Chismoso.NodeAddress);
    
  //S2. Class Setup.
    Serial.println("📡 Iniciando configuracion LoRa...");
    Node.Lora_Setup();
    Serial.println("⚙️ Iniciando funciones del sistema...");
    Correr.Function_begin(&Node, &Chismoso);
    webServer.begin(&Node, &Correr, &Chismoso);
    Serial.println("🌐 Iniciando servidor web...");
    Serial.println("✅ Sistema iniciado correctamente!");
    Serial.println("=====================================\n");
    Chismoso.Iniciar(&Node, &Correr);
}
void loop(){
  //L1. Function Start
    if (!F_iniciado){
      // F_iniciado=General.Iniciar();
      Serial.println("💡 Sistema en funcionamiento - esperando comandos...");
      // bool dale = webServer.enviarDatosAlServidorExterno(jsonString);
      if(MASTER){
        webServer.handleGetMasterStatus();
        lastServerUpdateMs = millis();
      }
      F_iniciado = true; // Ejecutar este bloque una sola vez al arranque
    }
    //-L1.1Manejo del Web Server
      webServer.handle();
  //L2. Functions Serial RX
    //-L2.1 Decode
      if(flag_ISR_stringComplete){
        Correr.Functions_Request(inputString);
        flag_F_codified_funtion=true;
        Serial.println(inputString);
        Serial.print("Nodo: ");
        Serial.println(Chismoso.NodeAddress);
        Serial.println("RX_SERIAL: "+inputString);
        flag_ISR_stringComplete=false;
      }
    //-L2.2 Function Run
      if(flag_F_codified_funtion){
        Correr.Functions_Run();
        inputString="";
        flag_F_codified_funtion=false;
      }
  //L3. Funciones del Protocolo.
    Chismoso.Preguntar();

  //L4. Funciones del Master.
    if(Chismoso.F_ServerUpdate){
      updateServer();
      Chismoso.F_ServerUpdate = false;
    }

  //L5. Envio periodico al servidor cada 1500 ms.
    if (millis() - lastServerUpdateMs >= SERVER_UPDATE_INTERVAL_MS && MASTER) {
      webServer.handleGetMasterStatus();
      lastServerUpdateMs = millis();
    }
}
//A 📎 Funciones Auxiliares

  //-A1  Update Server.
    void updateServer() {
      jsonString = Chismoso.jsonString;
      // Llamar a la función de la clase LoRaWebServer para enviar los datos al servidor externo
      bool dale = webServer.enviarDatosAlServidorExterno(jsonString);
    }
