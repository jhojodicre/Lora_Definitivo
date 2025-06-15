#include "General.h"
#include <Arduino.h>

General::General(bool ready)
{
  // Constructor de la clase General
  firstScan = ready;
  // 1. Configuracion de Hardware:
  pinMode(_LED_Azul, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  // 2. Condicion Inicial.
  // digitalWrite(_LED_Azul, LOW); // Led Azul OFF.
  digitalWrite(LED_1, HIGH);     // Led 1 OFF.
  digitalWrite(LED_2, HIGH);     // Led 2 OFF.
  digitalWrite(LED_3, HIGH);     // Led 3 OFF.
  // 3. Configuracion de Perifericos:
  //-3.1 Comunicacion Serial:
  Serial.begin(115200);
  delay(10);

  //-3.2 Interrupciones Habilitadas.
  //****************************
  // attachInterrupt (digitalPinToInterrupt (PB_zonas_in), ISR_0, FALLING);  // attach interrupt handler for D2
  // attachInterrupt (digitalPinToInterrupt (Zona_A_in), ISR_1, FALLING);      // attach interrupt handler for D2
  // attachInterrupt (digitalPinToInterrupt (Zona_B_in), ISR_2, FALLING);      // attach interrupt handler for D2
  // attachInterrupt (digitalPinToInterrupt (PB_ZA_in), ISR_3, FALLING);      // attach interrupt handler for D2
  // attachInterrupt (digitalPinToInterrupt (PB_ZB_in), ISR_4, FALLING);      // attach interrupt handler for D2

  // interrupts ();
}

bool General::Iniciar()
{
  // Implementación del método Iniciar
  Led_Monitor(3);
  Welcome();
  return true;
}
void General::Dale(int repeticiones)
{
  Led_Monitor(repeticiones);
}
void General::Configuracion()
{
  // Implementación del método Configuracion
}

void General::Gestion()
{
  // Implementación del método Gestion
}

// 1. Funciones de Logic interna del Micro.
void General::Welcome()
{
  Serial.println("LORA SECURE");
}
void General::Led_Monitor(int repeticiones)
{
  // Deshabilitamos Banderas
  int repetir = repeticiones;
  for (int encender = 0; encender < repetir; ++encender)
  {
    digitalWrite(_LED_Azul, HIGH);  // Led ON.
    delay(500);                    // pausa 1 seg.
    digitalWrite(_LED_Azul, LOW); // Led OFF.
    delay(500);                    // pausa 1 seg.
  }
}
void General::Led_Status(int status)
{
  // Deshabilitamos Banderas
  if (status == 1)
  {
    digitalWrite(_LED_Azul, HIGH); // Led ON.
  }
  else
  {
    digitalWrite(_LED_Azul, LOW); // Led OFF.
  }
}
void General::Led_1(int status)
{
  // Deshabilitamos Banderas
  if (status == 1)
  {
    digitalWrite(LED_1, LOW); // Led ON.
  }
  else
  {
    digitalWrite(LED_1, HIGH); // Led OFF.
  }
}
void General::Led_2(int status)
{
  // Deshabilitamos Banderas
  if (status == 1)
  {
    digitalWrite(LED_2, LOW); // Led ON.
  }
  else
  {
    digitalWrite(LED_2, HIGH); // Led OFF.
  }
}
void General::Led_3(int status)
{
  // Deshabilitamos Banderas
  if (status == 1)
  {
    digitalWrite(LED_3, LOW); // Led ON.
  }
  else
  {
    digitalWrite(LED_3, HIGH); // Led OFF.
  }
}