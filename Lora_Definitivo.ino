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
    // Replace the next variables with your SSID/Password combination
    const char* ssid = "ANTEL_0322";
    const char* password = "xBKJ474S";
    // Add your MQTT Broker IP address, example:
    //const char* mqtt_server = "192.168.1.144";
    const char* mqtt_server = "192.168.1.27";

    long lastMsg = 0;
    char msg[50];
    int value = 0;

    String MQTT_Frame_TX="";
    String Lora_RX=""; // Mensaje recibido por Lora.
    // Otras.  
      String      codigo="";
      String      info_1="";
      char        incomingFuntion;
//4. Intancias.
  //-4.1 Clases propias.
    Functions Correr(true);         // Funciones a Ejecutar
    General   General(false);         // Configuraciones Generales del Nodo.
    Lora      Node('1');
    Master    Master(true,2);
  //-4.2 Clases de Dispositivos Externos.
    WiFiClient espClient;
    PubSubClient client(espClient);
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
  //S.3 WiFi
    setup_wifi();
  //S4. MQTT
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}
void loop(){
  //L1. Function Start
    if (!F_iniciado){
      F_iniciado=General.Iniciar();
      if(Master.Mode){
        Master.Iniciar();
      }

    }
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
    // -L3.1 MQTT Reconnect.
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
  //L4. Funciones del Nodo.
    //-L4.0 Function Test.
      if(flag_ISR_prueba){
      // flag_ISR_prueba=false;
        // a1_Nodo_Destellos(1,3);
      }
    //-L4.1 IO ENTRADAS DEL NODO sino es el maestro.
      if(!Master.Mode){
        // Node.Lora_IO_Zones(); // Se actualizan los estados de las zonas.
        Node.Lora_Dummy_Simulate(); // Se simulan las señales de entrada.
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
          General.Led_Monitor(Node.rx_funct_parameter1); // Se ejecuta la funcion.
        }
        //-L4.3.1 Ejecuta la funcion.
      // Flag activado desde Lora_Nodo_Decodificar Se resetea la bandera de ejecucion.
        Node.F_Nodo_Excecute=false;  
      }
    //-L4.4 Nodo RX.
      if(Node.F_Recibido && !Master.Mode){
        Node.F_Recibido=false; // Flag activado desde Lora_Nodo_Decodificar Se resetea la bandera de recepcion.
        Node.Lora_Nodo_Decodificar();       // Se recibe el mensaje.
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
  //L5. Funciones del Master.
    //-L5.1 F- Master.
        //
    //-L5.2 Master TX
      if(Master.Next){
        Master.Master_Nodo();       //
        Master_RX_Request();          // Se cargan los datos recibidos desde via serial.
        Node.Lora_Master_Frame();
        Node.Lora_TX();
        Master.Next=false;
        Serial.println("Master TXed");
        Correr.a1(3,5);// 1 veces, 100 milesegundos.
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
        // Master_MQTT_Publish(); // Se publica el mensaje en el servidor MQTT.
        Node.F_Master_Update=false;
        Node.Lora_Master_DB(); // Se actualizan los datos del nodo.
        Master_MQTT_Publish(); // Se publica el mensaje en el servidor MQTT.
        //-L5.4.0 Debug.
      }
    //-L5.5 F- Master Execute order from Server
      if(Node.F_Master_Excecute && Master.Mode){
        //-L5.5.1 Ejecuta la funcion.
          // General.Led_Monitor(1);
          // Correr.a1(2,1);// 1 veces, 100 milesegundos. 
          // General.Led_Monitor(5); // Led ON.
          // Correr.a1(Node.tx_funct_parameter1,Node.tx_funct_parameter2); // Se ejecuta la funcion.

          Node.F_Master_Excecute=false; // Bandera activada en Lora_Nodo_Decodificar.
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
  //-4.6 Master RX Request.
    void Master_RX_Request(){
      Node.nodo_a_Consultar=String(Master.Nodo_Proximo);
      Node.tx_funct_mode=Correr.function_Mode; // Tipo de funcion a ejecutar.
      Node.tx_funct_num=Correr.function_Number; // Numero de funcion a ejecutar.
      Node.tx_funct_parameter1=Correr.x1; // Parametro 1 de la Funcion.
      Node.tx_funct_parameter2=Correr.x2; // Parametro 2 de la Funcion.
    }
//5. Funciones de Dispositivos Externos. 
  //-5.1 WiFi
    void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    }
  //-5.2 MQTT Callback
    void callback(char* topic, byte* payload, unsigned int length) {
      Serial.print("Message arrived [");
      Serial.print(topic);
      Serial.print("] ");
      String messageTemp;
      for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        messageTemp += (char)payload[i];
      }
      Serial.println();
      // Feel free to add more if statements to control more GPIOs with MQTT

      // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
      // Changes the output state according to the message
      if (String(topic) == "test/topic") {
        Serial.print("Changing output to ");
        if(messageTemp == "on"){
          Serial.println("on");
          // General.Led_Status(1); // Led ON.
          // General.Led_Monitor(2); // Se ejecuta la funcion.
          // Node.F_Master_Excecute=true; // Bandera activada para ejecutar la funcion.
          General.Led_1(1); // Led ON.
        }
        else if(messageTemp == "off"){
          Serial.println("off");
          General.Led_1(0); // Led OFF.
        }
      }
    }
  //-5.3 MQTT Reconnect.
    void reconnect() {
      // Loop until we're reconnected
      while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("ESP32Client")) {
          Serial.println("connected");
          // ... and subscribe to topic
          client.subscribe("test/topic");
        } else {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
          // Wait 5 seconds before retrying
          delay(5000);
        }
      }
    }
  //-5.4 MQTT PUBLISH
    void Master_MQTT_Publish(){
      //-5.4.0 Debud.
        MQTT_Frame_TX=String(Node.nodo_DB);
        Serial.print("MQTT TX: ");
        Serial.println(MQTT_Frame_TX);
      //-5.4. MQTT Publish.
      client.publish("test/topic", Node.nodo_DB.c_str());
    }





//10. Miscelanius
  //https://resource.heltec.cn/download/package_heltec_esp32_index.json