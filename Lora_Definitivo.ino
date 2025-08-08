//1. librerias.
  //- 1.1 Librerias****
    // #include "images.h"
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

//2. Definicion de Pinout.
  //  Las Etiquetas para los pinout son los que comienzan con GPIO
  //  Es decir, si queremos activar la salida 1, tenemos que buscar la referencia GPIO 1, Pero solomante Escribir 1 sin "GPIO"
  //  NO tomar como referencia las etiquetas D1, D2,D3, ....Dx.

  //-2.2 Definicion de etiquetas para las Salidas.
    #define LED_azul      2

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
    bool          flag_F_Un_segundo=false;         // Se activa cuando Pasa un Segundo por Interrupcion.
    bool          F_iniciado=false;              // Habilitar mensaje de F_iniciado por unica vez
    bool          flag_F_responder=false;          // Se activa cuando recibe un mensaje para luego responder.
    bool          flag_F_modo_Continuo=false;
    bool          flag_F_depurar=false;
    bool          flag_F_once=true;
    bool          flag_F_respondido=false;
    bool          flag_F_masteRequest=false;
    bool          flag_F_masterIniciado=false;
    bool          flag_F_nodoRequest=false;
    bool          flag_F_masterNodo=false;          // Habilitada para solicitar informacion a un Nodo Especifico    
    bool          flag_F_PAQUETE=false;
    bool          flag_F_tokenTime=false;
    bool          flag_F_cycleTime=false;
    bool          flag_F_T2_run=false;
    bool          flag_F_T1_run=false;
    bool          flag_F_Nodos_Completos=false;
    bool          flag_F_Nodos_Incompletos=false;
    bool          flag_F_Nodo_Iniciado=false;
    bool          flag_F_Nodo_Ultimo=false;
    bool          flag_F_nodo_Anterior=false;       // Indica cuando el nodo anterior se a comunicado con el nodo actual.
    bool          flag_F_token=false;               // Se habilita caundo el nodo responde por token
    bool          flag_F_analizar=false;
    bool          Nodo_waiting=false;
    bool          flag_F_totalTime=false;
    bool          flag_F_contar_tiempo=false;

  
 
  //-3.4 Variables TIME.
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

  //-3.5 Variables Para Conection WiFi and MQTT.
    
    // Add your MQTT Broker IP address, example:
    //const char* mqtt_server = "192.168.1.144";
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
    Lora      Node('3');
    Master    Master(true,3);      // Clase para el Maestro, con el numero de nodos que va a controlar.
  
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
  //-5.2 Extern Function
    // ICACHE_RAM_ATTR void ISR_0(){
    //   flag_ISR_prueba=true;
 
    // ICACHE_RAM_ATTR void ISR_1(){
      // }
      // ICACHE_RAM_ATTR void ISR_2(){
      // }
      // ICACHE_RAM_ATTR void ISR_3(){
      //   bitClear(Zonas, Zona_A);
      // }
      // ICACHE_RAM_ATTR void ISR_4(){
      //   bitClear(Zonas, Zona_B);
      // }
  //-5.3 Interrupciones por Timers.
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
    //-2.1 Estado de Salidas.
    //-2.3 Timer Answer.
      tokenTime       = 1000;
      baseTime        = 1000;
      updateTime      = 1000;
      masterTime      = cycleTime*2;
      wakeUpTime      = 30.0;
  //S2. Nodo Setup.
    Node.Lora_Setup();
    //S2.1 Nodo Dummy Se;ales Simuladas
    if(!Master.Mode){
      Node.Lora_Dummy_Simulate(); // Se simulan las señales de entrada.
    }
  //S4. MQTT
    if(Master.Mode){
      // client.setServer(mqtt_server, 1883);
      // client.setCallback(callback);
    }
  //S5. HTTTP Client
    if(Master.Mode){
    //S-5.2 MQTT
      // client.setServer(mqtt_server, 1883);
      // client.setCallback(callback);

    //S-5.4 Web Server
    }
      webServer.begin(&Node, &Master, &Correr);
      Serial.println("🌐 Servidor web iniciado en puerto 80");
}
void loop(){
  //L1. Function Start
    if (!F_iniciado){
      F_iniciado=General.Iniciar();
      if(Master.Mode){
        Master.Iniciar();
      }
    }
  
    // ✅ AGREGAR: Manejo del Web Server
    if(Master.Mode){
    }
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
    //L-2.2 Function Run
      if(flag_F_codified_funtion){
        // Correr.Functions_Run();
        inputString="";
        flag_F_codified_funtion=false;
      }
  //L3. Funciones de Dispositivos Externos.
    //-L3.1 MQTT Reconnect.
      // if(Master.Mode){
      //   if (!client.connected()) {
      //     reconnect();
      //   }
      //   client.loop();
      // }
  //L4. Funciones del Nodo.
    //-L4.0 Node Function Test.
      if(flag_ISR_prueba){
      // flag_ISR_prueba=false;
        // a1_Nodo_Destellos(1,3);
      }
    //-L4.1 Node IO ENTRADAS DEL NODO.
      if(!Master.Mode){
        Node.Lora_IO_Zones(); // Se actualizan los estados de las zonas.
        // Node.Lora_Dummy_Simulate(); // Se simulan las señales de entrada.
      }
    //-L4.2 Nodo TX.
      if(Node.F_Responder && !Master.Mode){
        Node.Lora_TX();       // Se envia el mensaje.
        Serial.println("Node TX");
      }
    //-L4.3 Nodo Ejecuta Funciones.
      if(Node.F_Nodo_Excecute && !Master.Mode){
        if(Node.F_function_Special){
          Node.F_function_Special=false; // Bandera activada en Lora_Nodo_Decodificar.
        }
        //-L4.3.1 Ejecuta la funcion.
        Nodo_Ejecutar_Funciones(Node.rx_funct_mode, Node.rx_funct_num, Node.rx_funct_parameter1, Node.rx_funct_parameter2);
      // Flag activado desde Lora_Nodo_Decodificar Se resetea la bandera de ejecucion.
        Node.F_Nodo_Excecute=false;
        if(Correr.F_Correr_Dale) {
          Node.Lora_Node_Print(Correr.function_Exct); // Imprime la funcion ejecutada.
          Correr.F_Correr_Dale=false;
        }
      }
    //-L4.4 Nodo RX.
      if(Node.F_Recibido && !Master.Mode){
        Node.Lora_Nodo_Decodificar();       // Se recibe el mensaje.
        Node.F_Recibido=false; // Flag activado desde Lora_Nodo_Decodificar Se resetea la bandera de recepcion.
        // Node_Print_LORA_RX(); // Imprime los datos recibidos por Lora.
      }
    //-L4.5 Nodo Ejecuta Funciones.
      if(Node.F_Nodo_Excecute && !Master.Mode){
        if(Node.F_function_Special){
          Node.F_function_Special=false; // Bandera activada en Lora_Nodo_Decodificar.
        }
        Node.F_Nodo_Excecute=false;  // Flag activado desde Lora_Nodo_Decodificar Se resetea la bandera de ejecucion.
        //-L4.5.1 Ejecuta la funcion.
        Nodo_Ejecutar_Funciones(Node.rx_funct_mode, Node.rx_funct_num, Node.rx_funct_parameter1, Node.rx_funct_parameter2);
      }

  //L5. Funciones del Master.
    //-L5.1 F- Master.
        //
    //-L5.2 Master TX
      if(Master.Next){
        Master.Master_Nodo();         // Se prepara el nodo maestro.
        Master_RX_Request();          // Se cargan los datos recibidos desde via serial.
        Node.Lora_Master_Frame();     // Se prepara el mensaje a enviar.
        Node.Lora_TX();               // Se envia el mensaje.
        Master.Next=false;
        Serial.println("Master TXed");
        //-L5.2.7 Simular
          // Master_Dummy_Simulate(); // Simula el envio de datos del nodo maestro.
        //-L.5.2.8 Probamos el envio de datos a la WEB.
        }
    //-L5.3 F- Master RX.
      if(Node.F_Recibido && Master.Mode){
        Node.Lora_Master_Decodificar();       // Se recibe el mensaje.
        Serial.print("Lora RX: ");
        Serial.println(String(Node.rxdata));
        Node.F_Recibido=false;                // Flag activado desde Lora_Nodo_Decodificar Se resetea la bandera de recepcion.
      }
    //-L5.4 F- Server Update.
      if(Node.F_Master_Update){
        //-L5.4.1 MQTT Publish.
          // Master_MQTT_Publish();              // Se publica el mensaje en el servidor MQTT.
          //Update Server.
          updateServer();
        Node.F_Master_Update=false;
      }
    //-L5.5 F- Master Execute order from Server
      if(Node.F_Master_Excecute && Master.Mode){
        //-L5.5.1 Ejecuta la funcion.
          // Master_RX_Request_2();                // 1. Carga los datos recibidos desde el servidor.
          Node.Lora_Master_Frame();             // 2. Se prepara el mensaje a enviar.
          Node.Lora_TX();                       // 3. Se envia el mensaje.
          Node.F_Master_Excecute=false;         // 4. Se Desactiva la bandera Master_Excecute.
          Serial.println("Master Executed: testing");
      }
  //L6. Function Lora RX.
    //-L6.1 lora RX.
      Node.Lora_RX();
}
//4. Funciones UPDATE.
  //-4.1 Estados de Zonas.
    void reviso(){
    }
  //-4.2 Secuencia.
    void secuencia(){
    }
  //-4.4 Actualizar.  
    void actualizar(){
    }
  //-4.5 Analizar.
    void analizar(){
    }
  //-4.7 Master RX Request.
    //-4.7.1 Master RX Request.
      void Master_RX_Request(){
        Node.nodo_a_Consultar=String(Master.Nodo_Proximo);
        Node.tx_funct_mode=Correr.function_Mode; // Tipo de funcion a ejecutar.
        Node.tx_funct_num=Correr.function_Number; // Numero de funcion a ejecutar.
        Node.tx_funct_parameter1=Correr.x1; // Parametro 1 de la Funcion.
        Node.tx_funct_parameter2=Correr.x2; // Parametro 2 de la Funcion.
      }
    //-4.7.2 Master RX Request 2.
      void Master_RX_Request_2(){
        Node.nodo_a_Consultar=Nodo_a_Pedir;
        Node.tx_funct_mode=function_Mode; // Tipo de funcion a ejecutar.
        Node.tx_funct_num=function_Number; // Numero de funcion a ejecutar.
        Node.tx_funct_parameter1=parameter_1; // Parametro 1 de la Funcion.
        Node.tx_funct_parameter2=parameter_2; // Parametro 2 de la Funcion.
      }
    //-4.7.3 Master Dummy Simulate.
      void Master_Dummy_Simulate(){
        // 1 o se envia a MQTT.
          Node.Lora_Dummy_Simulate(); // Se simulan las señales de entrada.
          Node.SerializeObjectToJson(); // Serializa el objeto a JSON
          // Master_MQTT_Publish(); // Se publica el mensaje en el servidor MQTT.  
        // 2 o se envia a MongoDB.
              // sendJsonToMongoDB(); // Envio de Json a MongoDB.
      }
    //-4.7.4 Update Server.
      void updateServer() {
        // Obtener los datos del objeto Node (clase Lora)
        jsonString = Node.jsonString; // Suponiendo que Node ya tiene el método para serializar sus datos

        // Llamar a la función de la clase LoRaWebServer para enviar los datos al servidor externo
        bool dale = webServer.enviarDatosAlServidorExterno(jsonString);
      }
  //-4.8 Node Functions Complementary.
    //-4.8.1 Nodo Muestra msg Lora_RX
      void Node_Print_LORA_RX(){
          Serial.print("RX: ");
          Serial.println(String(Node.rx_destinatario));
          Serial.print("LA: ");
          Serial.println(String(Node.local_Address));
          Serial.print("ms: ");
          Serial.println(String(Node.rx_mensaje));
          Serial.print("md: ");
          Serial.println(String(Node.rx_funct_mode));
          Serial.print("nf: ");
          Serial.println(String(Node.rx_funct_num));
          Serial.print("p1: ");
          Serial.println(String(Node.rx_funct_parameter1));
          Serial.print("p2: ");
          Serial.println(String(Node.rx_funct_parameter2));
        
      }
    //-4.8.2 Nodo Ejecuta Funciones.
      void Nodo_Ejecutar_Funciones(String mode, String number, String parameter_1, String parameter_2) {
        // Aquí se implementa la lógica para ejecutar las funciones del nodo
        // dependiendo de los parámetros recibidos.
        Correr.Functions_Request(mode + number + parameter_1 + parameter_2);
        Correr.Functions_Run(); // Ejecuta la función correspondiente.
      }
//5. Funciones de Dispositivos Externos. 
  //-5.1 
  //-5.2 MQTT Reconnect.
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
  //-5.3 MQTT RX Callback.
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
  //-5.4 MQTT TX PUBLISH
    void Master_MQTT_Publish(){
      //-5.4.1 MQTT Publish.
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