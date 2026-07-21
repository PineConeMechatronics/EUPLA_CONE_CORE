/*
 * ==============================================================================
 * Project:  Shield de Arduino versátil de bajo coste con fines académicos (TFG)
 * File:     [LM35_test].ino
 * Author:   [David Martínez Pérez]
 * Year:     2026
 * Brief:    [Test de funcionamiento del LM35]
 * ==============================================================================
 */

#include <SPI.h>
// Pin seleccion de esclavo
const int SLV_DAC = 8; 
const int SLV_ADC = A0; 
// COMANDOS DAC7562
#define WRT_UPLD_REG_AB 0x1F 
void setup() {
  
  Serial.begin(9600);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);             //ELEGIMOS CUAL VA A SER EL PRIMER BIT S EL MAS SIGNIFICATIVO O EL MENOS
  SPI.setDataMode(SPI_MODE1);            //elegr el modo de reloj
  SPI.setClockDivider(SPI_CLOCK_DIV8);  // divisiones del reloj ajustar a la frecuencia mas conveniente
  pinMode(SLV_ADC, OUTPUT);     // configuracion esclavo ADC
  digitalWrite(SLV_ADC, HIGH);  //ADC Slave pin inactivo
  pinMode(SLV_DAC, OUTPUT); 
  digitalWrite(SLV_DAC, HIGH);
}

void loop() {

  int valorADC = ReadADC(4);  // Leer el valor del canal

  //Serial.println(((valorADC)));
  float value = ((valorADC * 4.096) / 4096.0);  //tranformar el valor leido al valor rel
  Serial.print("Voltaje LM35: ");
  Serial.print(value);
  Serial.println("V");
  // Serial.println(((value / 0.010))); // temperatura en base al slope
  delay(650);
}



int ReadADC(int canal) {
  byte msb, lsb;  // Para almacenar los datos del ADC
  long valor = 0;

  digitalWrite(SLV_ADC, LOW);  //activamos el pin esclavo del ADc

  SPI.transfer(0x06 | ((canal & 0x07) >> 2));  // Enviar los primeros 3 bits
  msb = SPI.transfer((canal & 0x03) << 6);     // Enviar los 2 bits restantes del canal
  lsb = SPI.transfer(0x00);                    // Leer el primer byte de datos (MSB)

  // Finalizar la comunicación SPI
  digitalWrite(SLV_ADC, HIGH);  // Desactivar el CS (no seleccionado)

  // Combinar los bytes para obtener el valor de 12 bits
  valor = ((long)(msb & 0x0F) << 8) | lsb;  // Los 4 bits MSB se usan de manera correcta

  return valor;
}