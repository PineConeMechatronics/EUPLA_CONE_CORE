/*
 * ==============================================================================
 * Project:  Shield de Arduino versátil de bajo coste con fines académicos (TFG)
 * File:     [IMU_unificado].ino
 * Author:   [David Martínez Pérez]
 * Year:     2026
 * Brief:    [Script de lectura de los valores del acelerometro y el Gyro]
 * ==============================================================================
 */

#include <Wire.h>
#include <math.h>

// Dirección I2C del sensor. Verifica la configuración de tu módulo (pin SDO/SA0).
// La dirección común para la LSM6DSO32X es 0x6A o 0x6B. Usaremos 0x6A como ejemplo.
#define IMU_ADDRESS 0x6b

#define I2C_READ 0x80

// Dirección del registro WHO_AM_I, se usa para verificar la conexión.
#define WHO_AM_I_REG 0x0F
#define WHO_AM_I_VALUE 0x6C  // El valor esperado

// Direcciones de los registros de control (para encender y configurar el sensor)
#define CTRL2_G 0x11   // Registro de control del giróscopo: ODR y Full Scale
#define CTRL1_XL 0x10  // Registro de control del acelerómetro: ODR y Full Scale

// Direcciones de los primeros registros de salida (Low Byte)
#define OUTX_L_G 0x22   // Primer byte del Giróscopo X
#define OUTX_L_XL 0x28  // Primer byte del Acelerómetro X

// Constante para convertir de radianes a grados (180 / PI)
const float SENSITIVITY_FACTOR = 1;
const float RAD_A_DEG = 57.295779513;

// Variables para almacenar los valores crudos del sensor
int16_t accX, accY, accZ;
int16_t gyroX, gyroY, gyroZ;

// =========================================================
// PARÁMETROS DE CALIBRACIÓN
// =========================================================

const float bias_g[3] = { -23.9619, -13.87, 40.4504 }; //cabiar por tus propias bias
const float bias_dps[3] = { 23.9235, 43.935, 1.195 }; //cabiar por tus propias bias


const float cal_matrix[3][3] = { //cabiar por tu matriz de calibracion
  { 0.9711, 0.0514, 0.0001 },
  { -0.0004, 0.9697, -0.0057 },
  { -0.0168, 0.0296, 0.9646 }
};

// Almacena los valores convertidos a unidades físicas (float)
float accX_cal, accY_cal, accZ_cal;
float gyroX_cal, gyroY_cal, gyroZ_cal;

  // Nuevas variables para los ángulos Roll y Pitch
float roll_angle_deg, pitch_angle_deg;

void setup() {
  Wire.begin();          // Inicializa la comunicación I2C
  Serial.begin(115200);  // Inicializa la comunicación serial para mostrar los datos

  // 1. Verificación de la conexión
  if (checkID()) {
    Serial.println("LSM6DSO32X encontrado. Inicializando...");
    initializeIMU();
  } else {
    Serial.println("¡Error! LSM6DSO32X no encontrado o ID incorrecto.");
    while (1)
      ;  // Detiene el programa si falla la conexión
  }
}

void loop() {
  readAllData();
  applyACCCalibration();
  calculateAngles();

  //valores raw
  Serial.println("|acelerometro|");
  Serial.print("valores raw: ");
  Serial.print(accX);
  Serial.print("g ");
  Serial.print(accY);
  Serial.print("g ");
  Serial.print(accZ);
  Serial.print("g ");

  // Muestra los valores de aceleración
  Serial.print("valores calibrados: ");
  Serial.print("ACC: ");
  Serial.print("X:");
  Serial.print(accX_cal, 3);
  Serial.print("g ");
  Serial.print("Y:");
  Serial.print(accY_cal, 3);
  Serial.print("g ");
  Serial.print("Z:");
  Serial.print(accZ_cal, 3);
  Serial.println("g ");

  // Muestra los ángulos Roll y Pitch
  /* Serial.print(" | Roll: ");
  Serial.print(roll_angle_deg, 2);
  Serial.print(" deg | Pitch: ");
  Serial.print(pitch_angle_deg, 2);
  Serial.println(" deg");  
  */

  // muestra los valores RAW del GYRO
  Serial.println(" |GYRO| ");
  Serial.print("valores raw: ");
  Serial.print(gyroX);
  Serial.print(" dps\t");
  Serial.print(gyroY);
  Serial.print(" dps\t");
  Serial.print(gyroZ);
  Serial.print(" dps\t");
  //muestra los valores calibrados del GYRO
  Serial.print("calibrated data GYRO: ");
  Serial.print(gyroX_cal);
  Serial.print(" dps\t");
  Serial.print(gyroY_cal);
  Serial.print(" dps\t");
  Serial.print(gyroZ_cal);
  Serial.println(" dps\t");

  delay(100);  // Pequeña pausa
}

// Función para leer un solo registro (1 byte)
byte readRegister(byte regAddress) {
  Wire.beginTransmission(IMU_ADDRESS);  // Paso 1: Dirección del dispositivo
  Wire.write(regAddress);               // Paso 2: Dirección del registro a leer
  Wire.endTransmission(false);          // Paso 3: Termina la escritura, mantiene la conexión (false)
  Wire.requestFrom(IMU_ADDRESS, 1);     // Paso 4: Solicita 1 byte
  return Wire.read();                   // Paso 5: Lee el byte
}

// Función para escribir un valor en un registro
void writeRegister(byte regAddress, byte value) {
  Wire.beginTransmission(IMU_ADDRESS);  // Paso 1: Dirección del dispositivo
  Wire.write(regAddress);               // Paso 2: Dirección del registro
  Wire.write(value);                    // Paso 2: Valor a escribir
  Wire.endTransmission();               // Paso 3: Finaliza la transmisión
}

// Función para verificar el ID del dispositivo
bool checkID() {
  byte id = readRegister(WHO_AM_I_REG);
  return id == WHO_AM_I_VALUE;
}

// Función para configurar el sensor
void initializeIMU() {
  // Configuración del Giróscopo (CTRL2_G):
  // 0x50: ODR (Output Data Rate) de 208 Hz, Full Scale de +/- 250 dps (tasa de muestreo media)
  // Revisa el datasheet para otras configuraciones de ODR/FS.
  writeRegister(CTRL2_G, 0x50);
  // Configuración del Acelerómetro (CTRL1_XL):
  // 0x54: ODR de 208 Hz, Full Scale de +/- 16 g
  // Revisa el datasheet para otras configuraciones de ODR/FS.
  writeRegister(CTRL1_XL, 0x54);
}

// Función para leer todos los datos del sensor
void readAllData() {
  // --- Lectura de Acelerómetro (6 bytes, X, Y, Z) ---
  Wire.beginTransmission(IMU_ADDRESS);
  // Se añade 0x80 para habilitar la auto-incrementación de dirección (útil para lectura secuencial)
  Wire.write(OUTX_L_G);
  Wire.endTransmission(false);
  Wire.requestFrom(IMU_ADDRESS, 12);
  gyroX = Wire.read();
  gyroX |= Wire.read() << 8;
  gyroY = Wire.read();
  gyroY |= Wire.read() << 8;
  gyroZ = Wire.read();
  gyroZ |= Wire.read() << 8;
  // Paso 5 & 6: Lectura secuencial y combinación de bytes (LSB y MSB) contruccion de valores en los ejes a partir de dos registros de 8 bits
  accX = Wire.read();        // LSB X
  accX |= Wire.read() << 8;  // MSB X
  accY = Wire.read();        // LSB Y
  accY |= Wire.read() << 8;  // MSB Y
  accZ = Wire.read();        // LSB Z
  accZ |= Wire.read() << 8;  // MSB Z
}

void applyACCCalibration() {
  //  Aplicar matriz M * V_raw - B
  accX_cal = ((cal_matrix[0][0] * accX) + (cal_matrix[0][1] * accY) + (cal_matrix[0][2] * accZ) - bias_g[0]);
  accY_cal = ((cal_matrix[1][0] * accX) + (cal_matrix[1][1] * accY) + (cal_matrix[1][2] * accZ) - bias_g[1]);
  accZ_cal = ((cal_matrix[2][0] * accX) + (cal_matrix[2][1] * accY) + (cal_matrix[2][2] * accZ) - bias_g[2]);

  // aplicar calibracion restando las bias de los valores RAW V_raw - B
  gyroX_cal = (float(gyroX)) - bias_dps[0];
  gyroY_cal = (float(gyroY)) - bias_dps[1];
  gyroZ_cal = (float(gyroZ)) - bias_dps[2];
}

void calculateAngles() {
  // 1. Cálculo del ángulo Roll (Balanceo) - Rotación alrededor del Eje X
  // beta = atan2(g_y, sqrt(g_x^2 + g_z^2))
  roll_angle_deg = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_A_DEG;

  // 2. Cálculo del ángulo Pitch (Inclinación/Cabeceo) - Rotación alrededor del Eje Y
  // alpha = atan2(g_x, sqrt(g_y^2 + g_z^2))
  pitch_angle_deg = atan(accX / sqrt(accY * accY + accZ * accZ)) * RAD_A_DEG;
}