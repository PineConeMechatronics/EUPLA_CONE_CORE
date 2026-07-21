/*
 * ==============================================================================
 * Project:  Shield de Arduino versátil de bajo coste con fines académicos (TFG)
 * File:     [Frecuencia_muestreo_test].ino
 * Author:   [David Martínez Pérez]
 * Year:     2026
 * License:  MIT License
 * Brief:    [script para analizar la frecuencia de muestreo de los los conversores]
 * ==============================================================================
 */

#include <SPI.h>
// --- CONFIGURACIÓN DE HARDWARE ---
const int SLV_ADC = A0;  // esclavo ADC
const int SLV_DAC = 8;   // esclavo DAC
// COMANDOS DAC7562
#define WRT_UPLD_REG_AB 0x1F
//dato escritura fijo
float dato;
void setup() {
  Serial.begin(115200);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);            //ELEGIMOS CUAL VA A SER EL PRIMER BIT S EL MAS SIGNIFICATIVO O EL MENOS
  SPI.setDataMode(SPI_MODE1);           //elegr el modo de reloj
  SPI.setClockDivider(SPI_CLOCK_DIV8);  // divisiones del reloj ajustar a la frecuencia mas conveniente
// Para  reducir el impacto del tiempo de procesado sobre la frecuencia de muestreo se han tomado medidas como cambiar ciertas sintansix por manipulacion de registros
  DDRB |= (1 << PB0); //equivale a pinMode(slc_DAC, OUTPUT);
  PORTB |= (1 << PB0); //equivale a un digitalWrite(slv_DAC, LOW); 
  DDRC |= (1 << PC0); //equivale a pinMode(slc_ADC, OUTPUT);
  PORTC |= (1 << PC0); //  equivale a un digitalWrite(slv_ADC, LOW); 
  dato = 2048;
}
void loop() {
  // --- 1. BENCHMARK: CÁLCULO DE TIEMPO DE MUESTREO (1000 CICLOS) ---
  unsigned long inicioBench = micros();
  for (int i = 0; i < 1000; i++) {
    // Realizamos una lectura fantasma para medir el tiempo del hardware
    WriteDAC(WRT_UPLD_REG_AB, dato); // Para poder hacer una medicion individual solo hay que comentar alguna de las llamadas
    ReadADC(2);
  }
  unsigned long finBench = micros();
  unsigned long tiempoTotal = finBench - inicioBench; //Tiempo de muestreo  
  float dt = (float)tiempoTotal / 1000.0;  // Tiempo promedio por muestra en microsegundos
  float fs = 1e6 / dt; // calculo de la frecuencia de muestro a partir del tiempo de muestreo
  Serial.println("--- Benchmark Conjunto DAC+ADC ---");
  Serial.print("dt = "); Serial.print(dt); Serial.println(" us/ciclo");
  Serial.print("fs = "); Serial.print(fs); Serial.println(" Hz");
  delay(5000);  // Pausa de 5 segundos antes de repetir el benchmark
}

// --- FUNCIONES DE COMUNICACIÓN MANTENIDAS ---
void WriteDAC(uint16_t comando, uint16_t dato) {
  PORTB &= ~(1 << PB0);  //  equivale a un digitalWrite(slv_DAC, LOW); 
  SPI.transfer(comando);
  uint16_t dato_ajustado = (dato & 0x0FFF) << 4;  // Ajuste a 12 bits
  SPI.transfer16(dato_ajustado);
  PORTB |= (1 << PB0);  //  equivale a un digitalWrite(slv_DAC, HIGH); 
}
int ReadADC(int canal) {
  PORTC &= ~(1 << PC0);  //  equivale a un digitalWrite(slv_ADC, LOW); 
  // Comando para MCP3204/3208 o similar
  SPI.transfer(0x06 | ((canal & 0x07) >> 2));
  byte msb = SPI.transfer((canal & 0x03) << 6);
  byte lsb = SPI.transfer(0x00);
  PORTC |= (1 << PC0);  //  equivale a un digitalWrite(slv_ADC, HIGH); 
  return ((int)(msb & 0x0F) << 8) | lsb; 
}