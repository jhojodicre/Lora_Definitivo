//1. librerias.
  //-1.1 Librerias
    #include <Arduino.h>
    #include "Functions.h"
    #include "General.h"
    #include "Lora.h"
    #include "Master.h"
    #include <PubSubClient.h>
    #include <WiFi.h>
    #include <ArduinoJson.h>
    #include <HTTPClient.h>
    #include "NodeWebServer.h"

//3. Variables Globales.
  //-3.1 Variables Interrupciones
    volatile bool flag_ISR_prueba=false;             // Flag: prueba para interrupcion serial.
    volatile bool falg_ISR_stringComplete=false;    // Flag: mensaje Serial Recibido completo.
    String        inputString;           // Buffer recepcion Serial.
    String        function_Remote;
    String        function_Enable;
    bool          flag_ISR_temporizador_0=false;
    bool          flag_ISR_temporizador_1=false;
    bool          flag_ISR_temporizador_2=false;
    bool          flag_ISR_temporizador_3=false;
  //-3.2 Variables Banderas. 
    bool          flag_F_codified_funtion=false;   // Notifica que la funcion ha sido codificada.
    bool          F_iniciado=false;              // Habilitar mensaje de F_iniciado por unica vez
    bool          flag_F_depurar=false;
    bool          flag_F_T2_run=false;
    bool          flag_F_T1_run=false;
    bool          flag_F_Nodo_Iniciado=false;
    bool          flag_F_token=false;               // Se habilita caundo el nodo responde por token
    bool          F_updateServer=false;
 
  //-3.3 Variables TIME.
      long        initialTime= 0;

      long        currentTime_1 = 0;
      long        elapseTime_1 = 0;
      long        afterTime_1  = 0;
      long        beforeTime_1 = 0;
      long        beforeTime_GAP;         // Tiempo transcurrido entre mensajes entrantes para el MASTER.
      long        currentTime_GAP;
      long        elapseTime_GAP;

      long        currentTime_2 = 0;
      long        elapseTime_2 = 0;
      long        afterTime_2  = 0;
      long        beforeTime_2 = 0;

      long        chismeTime = 1000;
      long        baseTime   = 1000;
      long        cycleTime = 1000;
      long        tokenTime  = 2000;
      long        updateTime = 2000;
      long        masterTime = 10000;
      float       wakeUpTime = 90.0;
      long        firstTime;
      long        tokenLast;
      long        totalTime;
      long        waitTime   = 0;
      uint32_t    remainT1;
      uint32_t    remainT2;
      int         fastTime    =   1;

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
    Lora      Node('1');
    Master    Master(false,5);      // Clase para el Maestro, con el numero de nodos que va a controlar.
  //-4.2 Clases de Protocolos.
    WiFiClient    espClient;
    PubSubClient  client(espClient);
    LoRaWebServer webServer(80);  // ✅ AGREGAR ESTA LÍNEA
  //-4.3 Timer.
    Ticker timer_0;
    Ticker timer_1;
    Ticker timer_2;
    Ticker timer_3;
    Ticker timer_4;
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
          falg_ISR_stringComplete = true;
          flag_F_codified_funtion=false;
        }
      }
    }
  //-5.2 Interrupciones por Timers.
    void ISR_temporizador_0(){
      flag_ISR_temporizador_0=true;
    }
    void ISR_temporizador_1(){
        beforeTime_1 = millis();
        flag_ISR_temporizador_1=true;
    }
    void ISR_temporizador_2(){
      currentTime_2 = millis();
      flag_ISR_temporizador_2=true;
      flag_F_token=true;
      elapseTime_2=0;
      flag_F_T2_run=false;
    }
    void ISR_temporizador_3(){
      if(flag_F_Nodo_Iniciado) return;
      flag_ISR_temporizador_3=true;
    }
void setup(){
  //S1. Condiciones Iniciales.
    //-1.1 Timer Answer.
      tokenTime       = 1000;
      baseTime        = 1000;
      updateTime      = 1000;
      masterTime      = cycleTime*2;
      wakeUpTime      = 30.0;
  //S2. Class Setup.
    Node.Lora_Setup(&Correr);
    Correr.Function_begin(&Node);
    webServer.begin(&Node, &Master, &Correr);
}
void loop(){
  //L1. Function Start
    if (!F_iniciado){
      F_iniciado=General.Iniciar();
      if(Master.Mode) Master.Iniciar();
    }
    //-L1.1Manejo del Web Server
      webServer.handle();
  
  //L2. Functions Serial RX
    //-L2.1 Decode
      if(falg_ISR_stringComplete){
        Correr.Functions_Request(inputString);
        flag_F_codified_funtion=true;
        Serial.println(inputString);
        Serial.println("RX_SERIAL: "+inputString);
        falg_ISR_stringComplete=false;
      }
    //-L2.2 Function Run
      if(flag_F_codified_funtion){
        // Correr.Functions_Run();
        inputString="";
        flag_F_codified_funtion=false;
      }
  //L4. Funciones del Nodo.
    if(!Master.Mode){
      //-L4.1 Node Protocol.
        Node.Lora_Node_Protocol();
    }
  //L5. Funciones del Master.
    if(Master.Mode){
      Node.Lora_RX();
      //-L5.1 F- Master TX
        if(Master.Next){
          if(!F_updateServer){
            Node.Node_Status_str="0";
            Node.Node_Num_str=Node.nodo_consultado;
            Node.SerializeObjectToJson();
            updateServer();
          }
          Master.Master_Nodo();         // Se prepara el nodo maestro.
          Master_RX_Request();          // Se cargan los datos recibidos desde via serial.
          Node.Lora_Master_Frame();     // Se prepara el mensaje a enviar.
          Node.Lora_TX();               // Se envia el mensaje.
          Master.Next=false;
          F_updateServer=false;
          Serial.println("Master TXed");
          //-L5.2.7 Simular
            // Master_Dummy_Simulate(); // Simula el envio de datos del nodo maestro.
          //-L.5.2.8 Probamos el envio de datos a la WEB.
          }
      //-L5.2 F- Master RX.
        if(Node.F_Recibido){
          Node.Lora_Master_Decodificar();       // Se recibe el mensaje.
          Serial.print("Lora RX: ");
          Serial.println(String(Node.rxdata));
          Node.F_Recibido=false;                // Flag activado desde Lora_Nodo_Decodificar Se resetea la bandera de recepcion.
        }
      //-L5.3 F- Server Update.
        if(Node.F_Master_Update){
          //-L5.4.1 MQTT Publish.
            // Master_MQTT_Publish();              // Se publica el mensaje en el servidor MQTT.
            //Update Server.
          updateServer();
          Node.F_Master_Update=false;
        }
      //-L5.4 F- Master Execute order from Server
        if(Node.F_Master_Excecute){
          //-L5.5.1 Ejecuta la funcion.
            // Master_RX_Request_2();                // 1. Carga los datos recibidos desde el servidor.
            Node.Lora_Master_Frame();             // 2. Se prepara el mensaje a enviar.
            Node.Lora_TX();                       // 3. Se envia el mensaje.
            Node.F_Master_Excecute=false;         // 4. Se Desactiva la bandera Master_Excecute.
            Serial.println("Master Executed: testing");
        }

    }
      
}
//5 Master RX Request.
  //-5.1 Master RX Request.
    void Master_RX_Request(){
      Node.nodo_a_Consultar=String(Master.Nodo_Proximo);
      Node.tx_funct_mode=Correr.function_Mode; // Tipo de funcion a ejecutar.
      Node.tx_funct_num=Correr.function_Number; // Numero de funcion a ejecutar.
      Node.tx_funct_parameter1=Correr.x1; // Parametro 1 de la Funcion.
      Node.tx_funct_parameter2=Correr.x2; // Parametro 2 de la Funcion.
    }
  //-5.2 Master RX Request 2.
    void Master_RX_Request_2(){
      Node.nodo_a_Consultar=Nodo_a_Pedir;
      Node.tx_funct_mode=function_Mode; // Tipo de funcion a ejecutar.
      Node.tx_funct_num=function_Number; // Numero de funcion a ejecutar.
      Node.tx_funct_parameter1=parameter_1; // Parametro 1 de la Funcion.
      Node.tx_funct_parameter2=parameter_2; // Parametro 2 de la Funcion.
    }
  //-5.3 Master Dummy Simulate.
    void Master_Dummy_Simulate(){
      // 1 o se envia a MQTT.
        Node.Lora_Dummy_Simulate(); // Se simulan las señales de entrada.
        Node.SerializeObjectToJson(); // Serializa el objeto a JSON
        // Master_MQTT_Publish(); // Se publica el mensaje en el servidor MQTT.  
      // 2 o se envia a MongoDB.
            // sendJsonToMongoDB(); // Envio de Json a MongoDB.
    }
  //-5.4 Update Server.
    void updateServer() {
      // Obtener los datos del objeto Node (clase Lora)
      jsonString = Node.jsonString; // Suponiendo que Node ya tiene el método para serializar sus datos
      // Llamar a la función de la clase LoRaWebServer para enviar los datos al servidor externo
      bool dale = webServer.enviarDatosAlServidorExterno(jsonString);
      F_updateServer=true;
    }
//6 Funciones de Dispositivos Externos. 
  //-6.2 MQTT Reconnect.
    void reconnect() {
      // Loop until we're reconnected
      while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("ESP32Client")) {
          Serial.println("connected");
          // ... and subscribe to topic
          client.subscribe("lora/master/cmd");
        } else {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
          // Wait 5 seconds before retrying
          delay(5000);
        }
      }
    }
  //-6.3 MQTT RX Callback.
    void callback(char* topic, byte* payload, unsigned int length) {
      Serial.print("MQTT RX: :[");
      Serial.print(topic);
      Serial.println("] ");
      String messageTemp;
      for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        messageTemp += (char)payload[i];
      }
      Serial.println();
      // Feel free to add more if statements to control more GPIOs with MQTT
          // General.Led_1(1); // Led ON.

      // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
      // Changes the output state according to the message
      if (String(topic) == "lora/master/cmd") {
        Serial.print("MQTT processing message: ");
        char firstChar = messageTemp.charAt(0);
            Nodo_a_Pedir = messageTemp.charAt(1);
            function_Mode = messageTemp.charAt(2);
            function_Number = messageTemp.charAt(3);
            parameter_1 = messageTemp.charAt(4);
            parameter_2 = messageTemp.charAt(5);
        switch (firstChar) {
          case 'N':
            Serial.println("Case N: Master command received");
            Node.F_Master_Excecute = true; // Activar bandera para ejecutar la función en el nodo.
        // Acción para 'N'
            break;
          case 'M':
            Serial.println("Case M: Master command received");
            // Acción para 'M'
            break;
          case 'C':
            Serial.println("Case C: Config command received");
            // Acción para 'C'
            break;
          case 'F':
            Serial.println("Case F: Function command received");
            // Acción para 'F'
            break;
          case 'G':
            Serial.println("Case G: General command received");
            // Acción para 'G'
            break;
            case 'Z':
              Serial.println("Case Z: Special command received");
              // Acción para 'Z'
              break;
          default:
            Serial.println("Unknown command");
            break;
        }
      }

    }
  //-6.4 MQTT TX PUBLISH
    void Master_MQTT_Publish(){
      //-6.4.1 MQTT Publish.
        jsonString = Node.jsonString; // Obtener la cadena JSON del objeto
        client.publish("lora/master/status", jsonString.c_str());
      //-5.4.10 Debug.
        Serial.print("MQTT TX: ");
        Serial.println(jsonString);
    }
//10. Miscelanius#include <HTTPClient.h>
  //https://resource.heltec.cn/download/package_heltec_esp32_index.json
  // mongodb+srv://jhojodicre:l7emAppTNpcVUTsc@cluster0.wa5aztt.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0
  // The password for jhojodicre is included in the connection string for your first time setup. This password will not be available again after exiting this connect flow.
  // jhojodicre
  // password: l7emAppTNpcVUTsc
  // ip ip (186.52.249.162)

  // ramita agregada a la rama principal