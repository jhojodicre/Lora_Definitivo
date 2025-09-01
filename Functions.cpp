#include "Functions.h"
#include "Lora.h"
#include <Arduino.h>
#include "Master.h"
Functions::Functions(bool ready)
{
    // Constructor de la clase Functions
    // Inicializar variables
    firstScan = true;
    LED_Azul = 6; // 46 Pin del LED azul
    LED_Verde = 35; // 45 Pin del LED verde
    LED_Rojo = 5; // 44 Pin del LED rojo
    LED_ROJO = 7; // 7 Pin del LED ROJO DE LA PLACA

    // Configuración de la instancia
    
    // Configuración de pines
    pinMode(LED_Azul, OUTPUT);
    pinMode(LED_Verde, OUTPUT);
    pinMode(LED_Rojo, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);

    // Condición Inicial
    digitalWrite(LED_Azul, HIGH);
    digitalWrite(LED_Verde, HIGH);
    digitalWrite(LED_Rojo, HIGH);
    digitalWrite(LED_ROJO, HIGH);
}
void Functions::Function_begin(Lora* node){
    nodeRef = node;
}
void Functions::Functions_Request(String rxString)
{
    // Implementación del método Functions_Request
    // Deshabilitamos Banderas
    // falg_ISR_stringComplete=false;
    // flag_F_codified_funtion=true;
    function_Mode       = char(rxString.charAt(0));
    function_Number     = char(rxString.charAt(1));
    function_Parameter1 = rxString.substring(2, 3);
    function_Parameter2 = rxString.substring(3, 4);
    function_Parameter3 = rxString.substring(4, 5);
    function_Parameter4 = rxString.substring(5, 6);
    x1                  = function_Parameter1.toInt();
    x2                  = function_Parameter2.toInt();
    x3                  = function_Parameter3.toInt();
    x4                  = function_Parameter4.toInt();
}
void Functions::Functions_Run()
{
    switch (function_Mode)
    {
    case 'A':
        switch (function_Number)
        {
        case '0':
            a0();
            break;
        case '1':
            a1(x1, x2);
            break;
        case '2':
            a2();
            break;
        case '3':
            a3();
            break;
        case '4':
            a4();
            break;
        case '5':
            a5();
            break;
        case '6':
            a6();
            break;
        case '7':
            a7();
            break;
        case '8':
            a8();
            break;
        case '9':
            a9();
            break;
        default:
            // Handle invalid function_Number
            break;
        }
    case 'B':
        switch (function_Number)
        {
        case '0':
            b0();
            break;
        case '1':
            b1();
            break;
        case '2':
            b2();
            break;
        case '3':
            b3();
            break;
        case '4':
            b4();
            break;
        case '5':
            b5();
            break;
        case '6':
            b6();
            break;
        case '7':
            b7();
            break;
        case '8':
            b8();
            break;
        case '9':
            b9();
            break;
        default:
            // Handle invalid function_Number
            break;
        }
    case 'C':
        switch (function_Number)
        {
        case '0':
            c0();
            break;
        case '1':
            c1(function_Parameter1); // Pass the parameter to c1
            break;
        case '2':
            c2(function_Parameter1);
            break;
        case '3':
            c3(function_Parameter1);
            break;
        case '4':
            c4();
            break;
        case '5':
            c5();
            break;
        case '6':
            c6(function_Parameter1);
            break;
        case '7':
            c7(function_Parameter1);
            break;
        case '8':
            c8(function_Parameter1);
            break;
        case '9':
            c9();
            break;
        default:
            // Handle invalid function_Number
            break;
        }
    case 'N':
        switch (function_Number)
        {
        case '0':
            n0();
            break;
        case '1':
            n1();
            break;
        case '2':
            n2();
            break;
        case '3':
            n3();
            break;
        case '4':
            n4();
            break;
        case '5':
            n5();
            break;
        case '6':
            n6();
            break;
        case '7':
            n7();
            break;
        case '8':
            n8();
            break;
        case '9':
            n9();
            break;
        default:
            // Handle invalid function_Number
            break;
        }
    case 'M':
        switch (function_Number)
        {
        case '0':
            m0();
            break;
        case '1':
            m1();
            break;
        case '2':
            m2();
            break;
        case '3':
            m3();
            break;
        case '4':
            m4();
            break;
        case '5':
            m5();
            break;
        case '6':
            m6();
            break;
        case '7':
            m7();
            break;
        case '8':
            m8();
            break;
        case '9':
            m9();
            break;
        default:
            // Handle invalid function_Number
            break;
        }
    case 'S':
        switch (function_Number)
        {
        case '0':
            s0();
            break;
        case '1':
            s1();
            break;
        case '2':
            s2();
            break;
        case '3':
            s3();
            break;
        case '4':
            s4();
            break;
        case '5':
            s5();
            break;
        case '6':
            s6();
            break;
        case '7':
            s7();
            break;
        case '8':
            s8();
            break;
        case '9':
            s9();
            break;
        default:
            // Handle invalid function_Number
            break;
        }
    case 'Z':
        switch (function_Number)
        {
        case '0':
            z0();
            break;
        case 'A':
            z1();
            break;
        case 'B':
            z2();
            break;
        default:
            // Handle invalid function_Number
            break;
        }
    default:
        // Handle invalid function_Mode
        break;
    }
}
void Functions::a0()
{
    // Implementación del método a0
}
void Functions::A1()
{
    // FUNCION PROBADA CORRECTAMENTE    
    int veces = x1;
    int retardo = 100 * x2;
    for (int repetir = 0; repetir < veces; ++repetir)
    {
        delay(retardo);                     // pausa 1 seg.
        digitalWrite(LED_Azul, HIGH);      // Led ON.
        delay(retardo);                     // pausa 1 seg.
        digitalWrite(LED_Azul, LOW);       // Led OFF.
    }
}
void Functions::a1(int repeticiones, int tiempo)
{
    Serial.println("a1: ejecutada");
    Serial.println(repeticiones);
    Serial.println(tiempo);
    // FUNCION PROBADA CORRECTAMENTE    
    int veces = repeticiones;
    int retardo = tiempo * 100;
    for (int repetir = 0; repetir < veces; ++repetir)
    {
        delay(retardo);                     // pausa 1 seg.
        digitalWrite(LED_Azul, HIGH);      // Led ON.
        delay(retardo);                     // pausa 1 seg.
        digitalWrite(LED_Azul, LOW);       // Led OFF.
    }
}
void Functions::a2()
{
    // Implementación del método a2
}
void Functions::a3()
{
    // Implementación del método a3
}
void Functions::a4()
{
    // Implementación del método a4
}
void Functions::a5()
{
    // Implementación del método a5
}

void Functions::a6()
{
    // Implementación del método a6
}
void Functions::a7()
{
    // Implementación del método a7
}
void Functions::a8()
{
    // Implementación del método a8
}
void Functions::a9()
{
    // Implementación del método a9
}
void Functions::b0()
{
    // Implementación del método b0
}
void Functions::b1()
{
    // Implementación del método b1
}
void Functions::b2()
{
    // Implementación del método b2
}
void Functions::b3()
{
    // Implementación del método b3
}
void Functions::b4()
{
    // Implementación del método b4
}
void Functions::b5()
{
    // Implementación del método b5
}
void Functions::b6()
{
    // Implementación del método b6
}
void Functions::b7()
{
    // Implementación del método b7
}
void Functions::b8()
{
    // Implementación del método b8
}
void Functions::b9()
{
    // Implementación del método b9
}
void Functions::c0()
{
    // Implementación del método c0
}
void Functions::c1(String argumento1){
    // Implementación del método c1
    if (function_Parameter1 == "1") {
        digitalWrite(LED_Rojo, LOW);
        Serial.println("LR ON");
    } else if (function_Parameter1 == "0") {
        digitalWrite(LED_Rojo, HIGH);
        Serial.println("LR OFF");
    }
}
void Functions::c2(String argumento1)
{
    // Implementación del método c2
    if (function_Parameter1 == "1") {
        digitalWrite(LED_Azul, LOW);
    } else if (function_Parameter1 == "0") {
        digitalWrite(LED_Azul, HIGH);
    }
}
void Functions::c3(String argumento1)
{
    // Implementación del método c3
    if (function_Parameter1 == "1") {
        digitalWrite(LED_Verde, LOW);
    } else if (function_Parameter1 == "0") {
        digitalWrite(LED_Verde, HIGH);
    }
}
void Functions::c4()
{
    // Implementación del método c4
    nodeRef->Lora_IO_Zone_A_ACK();
}
void Functions::c5()
{
    // Implementación del método c5
    nodeRef->Lora_IO_Zone_B_ACK();
}
void Functions::c6(String argumento1)
{
    // Implementación del método c6
}
void Functions::c7(String argumento1)
{
    // Implementación del método c7
}
void Functions::c8(String argumento1)
{
    // Implementación del método c8
    if (function_Parameter1 == "1") {
        digitalWrite(LED_ROJO, LOW);
    } else if (function_Parameter1 == "0") {
        digitalWrite(LED_ROJO, HIGH);
    }
}
void Functions::c9()
{
    // Implementación del método c9
}
void Functions::n0()
{
    // Implementación del método n0
}
void Functions::n1()
{
    // Implementación del método n1
    ESP.restart();
    

}
void Functions::n2()
{
    // Implementación del método n2
}
void Functions::n3()
{
    // Implementación del método n3
    nodeRef->Lora_Event_Disable();

}
void Functions::n4()
{
    // Implementación del método n4
}
void Functions::n5()
{
    // Implementación del método n5
}
void Functions::n6()
{
    // Implementación del método n6
}
void Functions::n7()
{
    // Implementación del método n7
}

void Functions::n8()
{
    // Implementación del método n8
}

void Functions::n9()
{
    // Implementación del método n9
}

void Functions::m0()
{
    // Implementación del método m0
}

void Functions::m1()
{
    // Implementación del método m1
}

void Functions::m2()
{
    // Implementación del método m2
}

void Functions::m3()
{
    // Implementación del método m3
}

void Functions::m4()
{
    // Implementación del método m4
}

void Functions::m5()
{
    // Implementación del método m5
}

void Functions::m6()
{
    // Implementación del método m6
}

void Functions::m7()
{
    // Implementación del método m7
}

void Functions::m8()
{
    // Implementación del método m8
}

void Functions::m9()
{
    // Implementación del método m9
}
void Functions::s0()
{
    // Implementación del método s0
}

void Functions::s1()
{
    // Implementación del método s1
}

void Functions::s2()
{
    // Implementación del método s2
}

void Functions::s3()
{
    // Implementación del método s3
}

void Functions::s4()
{
    // Implementación del método s4
}

void Functions::s5()
{
    // Implementación del método s5
}

void Functions::s6()
{
    // Implementación del método s6
}

void Functions::s7()
{
    // Implementación del método s7
}

void Functions::s8()
{
    // Implementación del método s8
}

void Functions::s9()
{
    // Implementación del método s9
}
void Functions::z0()
{
    // Implementación del método z0
    Serial.println("Función z0 ejecutada");
    function_Exct = "z0";
    F_Correr_Dale=true;
    // Ejemplo: Apaga todos los LEDs
}

void Functions::z1()
{
    // Implementación del método z1
    Serial.println("Función z1 ejecutada");
    function_Exct = "z1";
    F_Correr_Dale=true;
}

void Functions::z2()
{
    // Implementación del método z2
    Serial.println("Función z2 ejecutada");
    function_Exct = "z2";
    F_Correr_Dale=true;
    // Ejemplo: Parpadea todos los LEDs una vez
}