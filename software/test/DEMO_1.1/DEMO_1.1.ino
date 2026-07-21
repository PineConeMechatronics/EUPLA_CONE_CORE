/*
 * ==============================================================================
 * Project:  Shield de Arduino versátil de bajo coste con fines académicos(TFG)
 * File:     [DEMO_1.1].ino
 * Author:   [David Martínez Pérez]
 * Year:     2026
 * Brief:    [Este script se trarta de una demo de funcionamiento del EUPLA_CONE_CORE]
 * ==============================================================================
 */
#include <7Semi_MCP23017.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_VL53L1X.h"

//Pin seleccion de esclavo
const int SLV_DAC = 8;
const int SLV_ADC = A0;
const int PIN_MODO = A2; // pin de habilitacion de control de RGB
// COMANDOS DAC7562
#define WRT_UPLD_REG_AB 0x1F
// MCP23017 at default address 0x20 on default Wire bus
MCP23017_7Semi mcp(0x20);
Adafruit_VL53L1X vl53;
//Pinnes PWM RGB y potenciometro
const int RED = 5;
const int GREEN = 6;
const int BLUE = 3;
const int POT = A3; // poteciometro idependiente al ADC
//variables
float pot0 = 0.0;
float pot1 = 0.0;
float NTC = 0.0;
float LM35 = 0.0;
float distanciaActual = 0.0;
// Distancia límite arbitraria en milímetros
const int DISTANCIA_UMBRAL = 600;
void setup() {
  //configuracion ADC-DAC
  Serial.begin(115200);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);            //ELEGIMOS CUAL VA A SER EL PRIMER BIT S EL MAS SIGNIFICATIVO O EL MENOS
  SPI.setDataMode(SPI_MODE1);           //elegr el modo de reloj
  SPI.setClockDivider(SPI_CLOCK_DIV8);  // divisiones del reloj ajustar a la frecuencia mas conveniente
  pinMode(SLV_ADC, OUTPUT);             // configuracion esclavo ADC
  digitalWrite(SLV_ADC, HIGH);          //ADC Slave pin inactivo
  pinMode(SLV_DAC, OUTPUT);             // configuracion esclavo DAC
  digitalWrite(SLV_DAC, HIGH);          //DAC Slave pin inactivo

  // configuracion de pines RGB y POT
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(POT, INPUT);
  pinMode(PIN_MODO, INPUT);

  //Configuracion Expansor de puertos
  // Initialize MCP23017
  if (!mcp.begin()) {
    Serial.println("MCP23017 init failed!");
    while (1)
      ;
  }
  Serial.println("MCP23017 initialized.");
  // Set GPA0 as output (LED)
  mcp.pinMode(GPB0, OUTPUT);
  mcp.pinMode(GPB1, OUTPUT);
  mcp.pinMode(GPB2, OUTPUT);
  mcp.pinMode(GPB3, OUTPUT);
  // Set GPB0 as input (Button) with pull-up enabled
  mcp.pinMode(GPB5, INPUT);  //boton 2
  mcp.pullUp(GPB5, true);
  mcp.pinMode(GPB7, INPUT);  //boton 4
  mcp.pullUp(GPB7, true);
  mcp.pinMode(GPB4, INPUT);  //boton 1
  mcp.pullUp(GPB4, true);
  mcp.pinMode(GPB6, INPUT);  //boton 3
  mcp.pullUp(GPB6, true);

  // Inicialización del sensor de distancia VL53L1X (Dirección estándar 0x29)
  if (!vl53.begin(0x29, &Wire)) {
    Serial.println("Error al iniciar el sensor VL53L1X!");
  } else {
    Serial.println("VL53L1X OK!");
    vl53.startRanging();
    vl53.setTimingBudget(50); 
  }
}

void loop() {
  pulsadores();
  actualizarDistancia(); 
  evaluarModoYRGB();
  conversores();

  Serial.print(" | NTC:");
  Serial.print(NTC); Serial.print(" ºC");
  Serial.print(" | LM35:");
  Serial.print(LM35); Serial.print(" ºC");
  Serial.print(" | Dist: "); Serial.print(distanciaActual);
  Serial.print(" mm | A2: "); Serial.println(digitalRead(PIN_MODO));

  delay(200);
}

// --- EVALUACIÓN PIN A2 Y CONTROL RGB ---
void evaluarModoYRGB() {
  int estadoA2 = digitalRead(PIN_MODO);

  if (estadoA2 == HIGH) {
    // MODO SENSOR: El color depende del sensor láser
    if (distanciaActual > 0 && distanciaActual <= DISTANCIA_UMBRAL) {
      analogWrite(RED, 0);
      analogWrite(GREEN, 0);
      analogWrite(BLUE, 255); // Azul si está cerca
    } else {
      analogWrite(RED, 255);
      analogWrite(GREEN, 0);
      analogWrite(BLUE, 0);   // Rojo si está lejos
    }
  } else {
    // MODO MANUAL: El RGB responde a los potenciómetros
    Control_RGB();
  }
}

// --- LECTURA CONTINUA ASÍNCRONA (POLLING) ---
void actualizarDistancia() {
  if (vl53.dataReady()) {
    int16_t dist = vl53.distance();
    if (dist != -1) {
      distanciaActual = (float)dist;
      vl53.clearInterrupt();
    }
  }
}

void pulsadores() {
  // Read the button state
  uint8_t botonpu = mcp.digitalRead(GPB5);
  uint8_t bottonpd = mcp.digitalRead(GPB7);
  uint8_t bottonnp = mcp.digitalRead(GPB4);
  uint8_t bottonnp1 = mcp.digitalRead(GPB6);
  // Print button state
  if (botonpu == LOW) {
    // Serial.println("Button Pressed");
    mcp.digitalWrite(GPB0, true);
  } else {
    //Serial.println("Button Released");
    mcp.digitalWrite(GPB0, false);
  }
  // Print button state
  if (bottonpd == HIGH) {
    // Serial.println("Button Pressed");
    mcp.digitalWrite(GPB1, true);
  } else {
    //Serial.println("Button Released");
    mcp.digitalWrite(GPB1, false);
  }
  // Print button state
  if (bottonnp == LOW) {
    // Serial.println("Button Pressed");
    mcp.digitalWrite(GPB2, true);
  } else {
    //Serial.println("Button Released");
    mcp.digitalWrite(GPB2, false);
  }
  // Print button state
  if (bottonnp1 == LOW) {
    // Serial.println("Button Pressed");
    mcp.digitalWrite(GPB3, true);
  } else {
    //Serial.println("Button Released");
    mcp.digitalWrite(GPB3, false);
  }
}
//control de conversores
void WriteDAC(uint16_t comando, uint16_t dato) {
  digitalWrite(SLV_DAC, LOW);  // Activar PIN esclavo
  SPI.transfer(comando);
  uint16_t dato_ajustado = (dato & 0x0FFF) << 4;  // Ajuste a 12 bits
  SPI.transfer16(dato_ajustado);
  digitalWrite(SLV_DAC, HIGH);  // Desactivar pin
}
int ReadADC(int canal) {
  byte msb, lsb;  // Para almacenar los datos del ADC
  long valor = 0;
  digitalWrite(SLV_ADC, LOW);                  //activamos el pin esclavo del ADc
  SPI.transfer(0x06 | ((canal & 0x07) >> 2));  // Enviar los primeros 3 bits
  msb = SPI.transfer((canal & 0x03) << 6);     // Enviar los 2 bits restantes del canal
  lsb = SPI.transfer(0x00);                    // Leer el primer byte de datos (MSB)
  // Finalizar la comunicación SPI
  digitalWrite(SLV_ADC, HIGH);  // Desactivar el CS (no seleccionado)
  // Combinar los bytes para obtener el valor de 12 bits
  valor = ((long)(msb & 0x0F) << 8) | lsb;  // Los 4 bits MSB se usan de manera correcta
  return valor;
}
void conversores() {
   pot0 = ((ReadADC(0) * 4.096) / 4096.0);                   // Leer el valor potenciometro_0
   pot1 = ((ReadADC(1) * 4.096) / 4096.0);                   // Leer el valor potenciometro_1
   NTC = (resistenciantc(((ReadADC(3) * 4.096) / 4096.0))-273.15);  // Leer el valor de la NTC
   LM35 = ((ReadADC(4) * 4.096) / 4096.0) / 0.010;           // Leer el valor de la LM35

  //WriteDAC(WRT_UPLD_REG_AB, 2.5);
}
float resistenciantc(float voltntc) {
  float Rterm = 0.0;
  float temp = 0.0;
  Rterm = (10000 * 5.0 / voltntc) - 10000;
  //return Rterm;
  temp = (3380 / log(Rterm / (10000 * exp(-3380 / 298.15))));
  return temp;
}
//  funcion control de RGB potenciometros
void Control_RGB() {
  analogWrite(RED, map(analogRead(POT), 0, 1023, 0, 255));
  analogWrite(GREEN, map(constrain(pot0 * 1000, 0, 4096), 0, 4096, 0, 255));
  analogWrite(BLUE, map(constrain(pot1 * 1000, 0, 4096), 0, 4096, 0, 255));
}