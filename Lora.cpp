/**
 * Send and receive LoRa-modulation packets with a sequence number, showing RSSI
 * and SNR for received packets on the little display.
 *
 * Note that while this send and received using LoRa modulation, it does not do
 * LoRaWAN. For that, see the LoRaWAN_TTN example.
 *
 * This works on the stick, but the output on the screen gets cut off.
 */
#include "Lora.h"
#include <Arduino.h>
// Turns the 'PRG' button into the power button, long press is off
#define HELTEC_POWER_BUTTON // must be before "#include <heltec_unofficial.h>"
#include <heltec_unofficial.h>

// Pause between transmited packets in seconds.
// Set to zero to only transmit a packet when pressing the user button
// Will not exceed 1% duty cycle, even if you set a lower value.
#define PAUSE 300

// Frequency in MHz. Keep the decimal point to designate float.
// Check your own rules and regulations to see what is legal where you are.
#define FREQUENCY 866.3 // for Europe
// #define FREQUENCY           905.2       // for US

// LoRa bandwidth. Keep the decimal point to designate float.
// Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
#define BANDWIDTH 250.0

// Number from 5 to 12. Higher means slower but higher "processor gain",
// meaning (in nutshell) longer range and more robust against interference.
#define SPREADING_FACTOR 9

// Transmit power in dBm. 0 dBm = 1 mW, enough for tabletop-testing. This value can be
// set anywhere between -9 dBm (0.125 mW) to 22 dBm (158 mW). Note that the maximum ERP
// (which is what your antenna maximally radiates) on the EU ISM band is 25 mW, and that
// transmissting without an antenna can damage your hardware.
#define TRANSMIT_POWER 0
volatile    bool rxFlag = false;
Lora::Lora(uint nodeNumber)
{
    // Constructor de la clase Node
    //1. Configuracion de Hardware
        pinMode(Zona_A_in, INPUT);
        pinMode(Zona_B_in, INPUT);
        pinMode(PB_ZA_in, INPUT);
        pinMode(PB_ZB_in, INPUT);
        pinMode(PB_ZAB_in, INPUT);

        pinMode(Rele_1_out, OUTPUT);
        pinMode(Rele_2_out, OUTPUT);
        Zone_A = false;
        Zone_B = false;
    //2. Condicion Inicial.
        digitalWrite(Rele_1_out, LOW);
        digitalWrite(Rele_2_out, LOW);
}
void Lora::Lora_Setup()
{
    heltec_setup();
    both.println("Radio init");
    RADIOLIB_OR_HALT(radio.begin());
    // Set the callback function for received packets
    radio.setDio1Action(rx);
    // Set radio parameters
    both.printf("Frequency: %.2f MHz\n", FREQUENCY);
    RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
    both.printf("Bandwidth: %.1f kHz\n", BANDWIDTH);
    RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
    both.printf("Spreading Factor: %i\n", SPREADING_FACTOR);
    RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
    both.printf("TX power: %i dBm\n", TRANSMIT_POWER);
    RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
    // Start receiving
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
}
void Lora::Lora_TX()
{
        both.printf("TX [%s] ", String(mensaje).c_str());
        radio.clearDio1Action();
        heltec_led(50); // 50% brightness is plenty for this LED
        RADIOLIB(radio.transmit(String(mensaje++).c_str()));
        heltec_led(0);
        if (_radiolib_status == RADIOLIB_ERR_NONE)
        {
            both.printf("OK (%i ms)\n", (int)tx_time);
        }
        else
        {
            both.printf("fail (%i)\n", _radiolib_status);
        }
        delay(100); // Debounce the button
        radio.setDio1Action(rx);
        RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
    F_Responder = false;
    F_Enviado = true;
}
void Lora::Lora_RX()
{
    // If a packet was received, display it and the RSSI and SNR
    if (rxFlag)
    {
        rxFlag = false;
        radio.readData(rxdata);
        if (_radiolib_status == RADIOLIB_ERR_NONE)
        {
            both.printf("RX [%s]\n", rxdata.c_str());
            both.printf("  RSSI: %.2f dBm\n", radio.getRSSI());
            both.printf("  SNR: %.2f dB\n", radio.getSNR());
        }
        RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
    }
}

// Can't do Serial or display things here, takes too much time for the interrupt
void Lora::rx()
{
    rxFlag = true;
}
