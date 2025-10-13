//1. librerias.
  //-1.1 Librerias
    #include "Functions.h"
    #include "General.h"
    #include "Lora.h"
    #include <HTTPClient.h>
    #include "NodeWebServer.h"

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
    Lora      Node(false,5,'3');
  //-4.2 Clases de Protocolos.
    LoRaWebServer webServer(80);  // ✅ AGREGAR ESTA LÍNEA
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
  //S2. Class Setup.
    Node.Lora_Setup(&Correr);
    Correr.Function_begin(&Node);
    webServer.begin(&Node, &Correr);
}
void loop(){
  //L1. Function Start
    if (!F_iniciado){
      F_iniciado=General.Iniciar();
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
        Serial.println(Node.local_Address);
        Serial.println("RX_SERIAL: "+inputString);
        flag_ISR_stringComplete=false;
      }
    //-L2.2 Function Run
      if(flag_F_codified_funtion){
        // Correr.Functions_Run();
        inputString="";
        flag_F_codified_funtion=false;
      }
  //L4. Funciones del Protocolo.
    Node.Lora_Protocol();
  //L5. Funciones del Master.
    if(Node.F_ServerUpdate){
      updateServer();
    }
}
//5 Master RX Request.
  //-5.1 Master RX Request.
    void Master_RX_Request(){
      // Node.nodo_a_Consultar=String(Master.Nodo_Proximo);
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
  //-5.4 Update Server.
    void updateServer() {
      // Obtener los datos del objeto Node (clase Lora)
      jsonString = Node.jsonString; // Suponiendo que Node ya tiene el método para serializar sus datos
      // Llamar a la función de la clase LoRaWebServer para enviar los datos al servidor externo
      bool dale = webServer.enviarDatosAlServidorExterno(jsonString);
      Node.F_ServerUpdate=false;
    }
