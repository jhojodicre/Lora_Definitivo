//1. librerias.
  //- 1.1 Librerias****
    // #include "images.h"
    #include <Arduino.h>
    #include "General.h"
    #include "Functions.h"
    #include "Node.h"
    #include "Master.h"

//2. Definicion de Pinout.
  //  Las Etiquetas para los pinout son los que comienzan con GPIO
  //  Es decir, si queremos activar la salida 1, tenemos que buscar la referencia GPIO 1, Pero solomante Escribir 1 sin "GPIO"
  //  NO tomar como referencia las etiquetas D1, D2,D3, ....Dx.
  
  //-2.1 Definicion de etiquetas para las Entradas.
  //-2.2 Definicion de etiquetas para las Salidas.
    #define LED_azul      2

  //-2.3 ZONAS
  //-2.4 Constantes.
  //-2.5 timer

//3. Variables Globales.
  //-3.1 Variables Interrupciones
    volatile bool flag_ISR_prueba=false;             // Flag: prueba para interrupcion serial.
    volatile bool falg_ISR_stringComplete=false;    // Flag: mensaje Serial Recibido completo.
    volatile bool flag_ISR_temporizador_1=false;
    volatile bool flag_ISR_temporizador_2=false;
    volatile bool flag_ISR_temporizador_3=false;        // pra actualizar los dato al servidor.
    volatile bool flag_ISR_temporizador_0=false;
    volatile bool flag_ISR_LORA=false;

    String        inputString;           // Buffer recepcion Serial.
    String        funtion_Mode;          // Tipo de funcion para ejecutar.
    String        funtion_Number;        // Numero de funcion a EJECUTAR.
    String        funtion_Parmeter1;     // Parametro 1 de la Funcion.
    String        funtion_Parmeter2;     // Parametro 2 de la Funcion.
    String        funtion_Parmeter3;     // Parametro 3 de la Funcion.
    String        funtion_Parmeter4;      // Parametro para las Funciones remotas.
    String        function_Remote;
    String        function_Enable;
    volatile int  x1=0;
    volatile int  x2=0;
    volatile int  x3=0;
    volatile int  x4=0;
  //-3.2 Variables Banderas.
    bool          flag_F_codified_funtion=false;   // Notifica que la funcion ha sido codificada.
    bool          flag_F_Un_segundo=false;         // Se activa cuando Pasa un Segundo por Interrupcion.
    bool          F_iniciado=false;              // Habilitar mensaje de F_iniciado por unica vez
    bool          flag_F_responder=false;          // Se activa cuando recibe un mensaje para luego responder.
    bool          flag_F_modo_Continuo=false;
    bool          flag_F_depurar=false;
    bool          flag_F_once=true;
    bool          flag_F_updateServer=false;
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

  
  //-3.3 Variables NODOS y ZONAS.
      
      // TIEMPO DE ZONA EN FALLA.
      String      Fuente_in_str;



      int         te_toca=1;           // Prueba para comunicacion continua con el servidor.      
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
    // Alarmas
    // Eventos
      

    // Otras.  
      String      codigo="";
      String      info_1="";
      char        incomingFuntion;
//4. Intancias.
  //-4.1 Clases.
    Functions Correr(true);
    General General(false);
    Node Nodo(true);
    Master Master(true);
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
//   Zonas=0;

    // }
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
      // if(flag_F_Nodos_Incompletos){
      //   analisar();
      // }
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
  //2. Condiciones Iniciales.
    //-2.1 Estado de Salidas.
    //-2.3 Timer Answer.
      tokenTime       = 1000;
      baseTime        = 1000;
      updateTime      = 1000;
      masterTime      = cycleTime*2;
      wakeUpTime      = 30.0;

  //S.2 Nodo Setup.
    Nodo.Lora_Setup();

}
void loop(){
  //1. Start Function
    if (!F_iniciado){
      F_iniciado=General.Iniciar();
    }
  //2. Functions Decode
    if(falg_ISR_stringComplete){
      flag_F_codified_funtion=Correr.Functions_Request(inputString);
      falg_ISR_stringComplete=false;
    }
  //3. Function Run
      if(flag_F_codified_funtion){
        flag_F_codified_funtion=Correr.Functions_Run();
        inputString="";
        flag_F_codified_funtion=false;
      }
  //4. Atender Las fucniones activadas desde ISR FLAGS.
    //-4.0 Bandera de Prueba.
      if(flag_ISR_prueba){
      // flag_ISR_prueba=false;
        // a1_Nodo_Destellos(1,3);
      }
    //-4.1 EJ-  REVISO Y ACTUALIZO.
      reviso();
      actualizar();

    //-4.2 F- Timer 1.
      if(flag_ISR_temporizador_1){
        analizar();
        //MASTER
        //NODOS.
      }
    //-4.3 F- Timer 2.
      if(flag_ISR_temporizador_2){
      }
    //-4.4 F- Timer 0.
      if(flag_ISR_temporizador_0){
      }
    //-4.5 F- Timer 3.
      if(flag_ISR_temporizador_3){
      }

    //-4.6 F- Server Update.
      if(flag_F_updateServer){
      }
    //-4.7 F- Recepcion de Paquete.
      if(flag_F_PAQUETE){
        flag_F_PAQUETE=false;
        secuencia();
      }
    //L.1 Lora RX.
        Nodo.Lora_RX();
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
  
//5. Funciones de Dispositivos Externos.
  //-5.1 

//https://resource.heltec.cn/download/package_heltec_esp32_index.json