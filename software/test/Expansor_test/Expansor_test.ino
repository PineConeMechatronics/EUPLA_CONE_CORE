/*******************************************************
 * @file       [Expansor_test].ino
 * @brief      Control y lectura del I/O Expander MCP23017 a través de I2C 
 *             para el Arduino Shield Multifunción.
 *
 * Este script inicializa el MCP23017 (utilizado en la shield para expansión 
 * de E/S por I2C), configurando sus pines para la gestión de periféricos 
 * de la placa.
 *
 * Key features demonstrated:
 * - Comunicación I2C con expansor MCP23017 en el Shield
 * - Gestión de pines de E/S (pinMode, digitalRead, digitalWrite)
 * - Verificación de funcionamiento en placa propia
 *
 * @section author Author
 * Adaptado por [David Martínez Pérez]
 * Original library code by 7Semi
 *
 * @section project Project
 * Shield de Arduino versátil de bajo coste con fines académicos(TFG )
 *
 * @section version Version
 * 1.0 - 2026
 *
 * @section license License
 * @license MIT
 * Copyright (c) 2025 7Semi (Original Library)
 * Modifications and implementation for Shield de Arduino versátil de bajo coste con fines académicos TFG (c) 2026 [David Martínez Pérez]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *******************************************************/
#include <7Semi_MCP23017.h>

// MCP23017 at default address 0x20 on default Wire bus
MCP23017_7Semi mcp(0x20);

void setup() {
  Serial.begin(115200);

  // Initialize MCP23017
  if (!mcp.begin()) {
    Serial.println("MCP23017 init failed!");
    while (1);
  }
  Serial.println("MCP23017 initialized.");

  // Set GPA0 as output (LED)
  mcp.pinMode(GPB0, OUTPUT);
  mcp.pinMode(GPB1, OUTPUT);
  mcp.pinMode(GPB2, OUTPUT);
  mcp.pinMode(GPB3, OUTPUT);

  // Set GPB0 as input (Button) with pull-up enabled
  mcp.pinMode(GPB5, INPUT); //boton 2
  mcp.pullUp(GPB5, true);
  mcp.pinMode(GPA0, INPUT);//boton 4
  mcp.pullUp(GPA0,true);
  mcp.pinMode(GPB4, INPUT);//boton 1
  mcp.pullUp(GPB4,true);
  mcp.pinMode(GPB6, INPUT);//boton 3
  mcp.pullUp(GPB6,true);

}

void loop() {
  // Toggle LED every 500 ms
 // mcp.togglePin(GPB0);

  // Read the button state
  uint8_t botonpu = mcp.digitalRead(GPB5);
  uint8_t bottonpd = mcp.digitalRead(GPA0);
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
