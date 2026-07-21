/*
 * ==============================================================================
 * Project: Shield de Arduino versátil de bajo coste con fines académicos (TFG)
 * File:     [ntc_test].ino
 * Author:   [David Matínez Pérez]
 * Year:     2026
 * Brief:    [Test de funcionamiento Thermistor NTC]
 * ==============================================================================
 */

#include <SPI.h>
#include <math.h>

// Pin seleccion de esclavo
const int SLV_ADC = A0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);             //ELEGIMOS CUAL VA A SER EL PRIMER BIT S EL MAS SIGNIFICATIVO O EL MENOS
  SPI.setDataMode(SPI_MODE1);            //elegr el modo de reloj
  SPI.setClockDivider(SPI_CLOCK_DIV16);  // divisiones del reloj ajustar a la frecuencia mas conveniente

  pinMode(SLV_ADC, OUTPUT);     // configuracion esclavo ADC
  digitalWrite(SLV_ADC, HIGH);  //ADC Slave pin inactivo
}

void loop() {

  int valorADC = ReadADC(3);  // Leer el valor del canal

  //Serial.println(((valorADC)));
  float value = ((valorADC * 4.096) / 4096.0);
  Serial.print(value);
  float Rntc = resistenciantc(value);
  Serial.println(" voltios");
 // Serial.print("|");
  //Serial.print(Rntc-273);
  //Serial.println("ºC");

  delay(800);
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

float resistenciantc(float voltntc) {
  float Rterm = 0.0;
  float temp = 0.0;
  Rterm = (10000 * 5.0 / voltntc) - 10000;
  //return Rterm;
  temp = (3380 / log(Rterm / (10000 * exp(-3380 / 298.15))));
  return temp;
}
