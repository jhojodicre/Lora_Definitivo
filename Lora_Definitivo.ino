//1. librerias.
  //-1.1 Librerias
    #include "Functions.h"
    #include "General.h"
    #include "Lora.h"
    #include <HTTPClient.h>
    #include "NodeWebServer.h"
    #include <BluetoothSerial.h>  // ✅ Librería Bluetooth para ESP32

//3. Variables Globales.
  //-3.1 Variables Interrupciones
    volatile bool flag_ISR_prueba=false;             // Flag: prueba para interrupcion serial.
    volatile bool flag_ISR_stringComplete=false;    // Flag: mensaje Serial Recibido completo.
    String        inputString;           // Buffer recepcion Serial.
    String        function_Remote;
    String        function_Enable;
    bool          flag_ISR_temporizador_0=false;
    bool          flag_ISR_temporizador_1=false;
    bool          flag_ISR_temporizador_2=false;
    bool          flag_ISR_temporizador_3=false;
  //-3.2 Variables Banderas. 
    bool          flag_F_codified_funtion=false;    // Notifica que la funcion ha sido codificada.
    bool          F_iniciado=false;                 // Habilitar mensaje de F_iniciado por unica vez
    bool          flag_F_depurar=false;
    bool          flag_F_T2_run=false;
    bool          flag_F_T1_run=false;
    bool          flag_F_Nodo_Iniciado=false;
    bool          flag_F_token=false;               // Se habilita caundo el nodo responde por token
    bool          F_updateServer=false;
    
    // Variables para debug y monitoreo
    unsigned long last_status_time = 0;
    unsigned long status_interval = 30000; // Mostrar estado cada 30 segundos
  //-3.4 Variables Para Conection WiFi and MQTT.
      const char* mqtt_server = "192.168.1.27";
    // JSON Variables.
      String  jsonString; // Cadena JSON para enviar a MongoDB
      int     nombre;
      int     valueJson;
      String  nodoJson="";

      // const char* serverName = "http://192.168.1.27:3000/api/data"; // URL de tu API de MongoDB

    long lastMsg = 0;
    char msg[50];
    int value = 0;

    String MQTT_Frame_TX="";
    String Lora_RX=""; // Mensaje recibido por Lora.
    StaticJsonDocument<200> doc;

    char Nodo_a_Pedir     = ' '; // Nodo a consultar por el Maestro.
    char function_Mode    = ' '; // Modo de Funcion a ejecutar.
    char function_Number  = ' ';
    char parameter_1      = ' ';
    char parameter_2      = ' ';
    // Otras.
      String      codigo="";
      String      info_1="";
      char        incomingFuntion;
//4. Intancias.
  //-4.1 Clases propias.
    Functions Correr(true);         // Funciones a Ejecutar
    General   General(false);       // Configuraciones Generales del Nodo.
    Lora      Node(false,5,'1');
  //-4.2 Clases de Protocolos.
    LoRaWebServer webServer(80);    // Servidor Web
    BluetoothSerial SerialBT;       // ✅ Instancia Bluetooth Serial
//5. Funciones ISR.
  //-5.1 Serial Function.
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
    Serial.printf("📍 Nodo: %c\n", Node.local_Address);
    Serial.println("✅ Puerto serie iniciado a 115200 baudios");
    
  //S2. Iniciar Bluetooth.
    SerialBT.begin("ESP32_LoRa_Node"); // Nombre del dispositivo Bluetooth
    Serial.println("📱 Bluetooth iniciado - Nombre: ESP32_LoRa_Node");
    
  //S3. Class Setup.
    Serial.println("📡 Iniciando configuración LoRa...");
    Node.Lora_Setup(&Correr);
    Serial.println("⚙️ Iniciando funciones del sistema...");
    Correr.Function_begin(&Node);
    Serial.println("🌐 Iniciando servidor web...");
    webServer.begin(&Node, &Correr);
    Serial.println("✅ Sistema iniciado correctamente!");
    Serial.println("=====================================\n");
}
void loop(){
  //L1. Function Start
    if (!F_iniciado){
      F_iniciado=General.Iniciar();
      Serial.println("💡 Sistema en funcionamiento - esperando comandos...");
      Serial.println("📝 Comandos disponibles por serie:");
      Serial.println("   - CFG0-4: Cambiar configuración radio");
      Serial.println("   - CFGSTATUS: Ver estado configuración");
      Serial.println("   - Otros comandos según protocolo\n");
    }
    
  //L2. Bluetooth Serial Handler
    // Leer comandos desde Bluetooth y redirigir a Serial para procesamiento
    if (SerialBT.available()) {
      String btCommand = SerialBT.readStringUntil('\n');
      btCommand.trim();
      if (btCommand.length() > 0) {
        Serial.println("📱 BT RX: " + btCommand);
        SerialBT.println("📱 Comando recibido: " + btCommand);
        // Procesar comando como si viniera del Serial normal
        inputString = btCommand + "\n";
        flag_ISR_stringComplete = true;
      }
    }

}
//A 📎 Funciones Ausiliares
//A1 Master RX Request.
  //-1.1  Update Server.
    void updateServer() {
      // Obtener los datos del objeto Node (clase Lora)
      jsonString = Node.jsonString; // Suponiendo que Node ya tiene el método para serializar sus datos
      // Llamar a la función de la clase LoRaWebServer para enviar los datos al servidor externo
      bool dale = webServer.enviarDatosAlServidorExterno(jsonString);
    }
