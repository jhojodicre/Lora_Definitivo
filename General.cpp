#include "General.h"
#include <Arduino.h>

General::General(bool ready)
{
  // Constructor de la clase General
  firstScan = ready;
  pinMode(_LED_Azul, OUTPUT);
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
  Serial.println("LORA SECURE URUGUAY");
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