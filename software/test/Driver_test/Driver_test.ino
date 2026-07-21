/*
 * ==============================================================================
 * Project:  Shield de Arduino versátil de bajo coste con fines académicos (TFG)
 * File:     [Driver_test].ino
 * Author:   [David Martínez Pérez]
 * Year:     2026
 * Brief:    [Scrip de control del Driver de potencia del EUPLA_CONE_CORE]
 * ==============================================================================
 */

int IN1 = 9;
int IN2 = 10;
int VISEN = A1;

// Corriente máxima permitida (A)
#define MAX_CURRENT 2.0   // límite de seguridad

// Relación voltaje/corriente para R_ISET = 7.5kΩ
#define V_PER_AMP 1.5

 bool direcion = 1;
unsigned long lastPrint = 0;

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(VISEN, INPUT);
  Serial.begin(9600); 

}

void loop() {
  // for (int speed = 0; speed <= 255; speed += 5) {
  int speed = 120; 
    setMotor(direcion, speed);
    monitorCurrent();
 // }
}

void setMotor (bool dir, int pwm){
  if (dir== true){
    analogWrite(IN1 , pwm  ) ;
    analogWrite(IN2 , 0) ;
    
  }
  else{
   analogWrite(IN1 , 0) ;
   analogWrite(IN2 , pwm) ;
  }

}
// Detiene el motor (modo Hi-Z)
void stopMotor() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
}

// Lee VISEN y calcula corriente (A)
float readCurrent() {
  float voltage = analogRead(VISEN) * (5.0 / 1023.0);
  float current = voltage / V_PER_AMP;
  return current;
}

// Muestra corriente y aplica protección
void monitorCurrent() {
  float current = readCurrent();

  // Muestra cada 200 ms
  if (millis() - lastPrint > 200) {
    Serial.print("Corriente: ");
    Serial.print(current, 2);
    Serial.println(" A");
    lastPrint = millis();
  }

  // Protección contra sobrecorriente
  if (current > MAX_CURRENT) {
    Serial.println("⚠️  Corriente > 2.0 A — Motor apagado por seguridad");
    stopMotor();
    delay(1000);
  }
}
