#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "MAX30105.h"    
#include "spo2_algorithm.h" // Usaremos esta para oximetría y pulso
#include <OneWire.h>
#include <DallasTemperature.h>


//OBJETOS DE SENSORES
Adafruit_MPU6050 mpu;
MAX30105 particleSensor;


#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);


//VARIABLES GLOBALES DE BIOMETRÍA Y FILTROS
uint32_t irBuffer[100];
uint32_t redBuffer[100];
int bufferIndex = 0;


int32_t spo2 = 0;
int32_t heartRate = 0;
int promedioBPM = 0;


// Variables para el filtro del pulso
const int numLecturas = 5;      
int lecturas[numLecturas];      
int indiceLectura = 0;          
int totalBPM = 0;              
bool filtroIniciado = false;    


//VARIABLES DE TIEMPO
unsigned long ultimoTiempoTemp = 0;
unsigned long ultimoTiempoPrint = 0;
float temperaturaActual = 0.0;


// Filtro de suavizado para el pulso
int filtrarBPM(int bpmCrudo) {
  if (bpmCrudo < 40 || bpmCrudo > 160) return promedioBPM;
 
  if (!filtroIniciado) {
    totalBPM = 0;
    indiceLectura = 0;
    for (int i = 0; i < numLecturas; i++) {
      lecturas[i] = bpmCrudo;
      totalBPM += bpmCrudo;
    }
    filtroIniciado = true;
  }
 
  totalBPM = totalBPM - lecturas[indiceLectura];  
  lecturas[indiceLectura] = bpmCrudo;            
  totalBPM = totalBPM + lecturas[indiceLectura];  
  indiceLectura++;
  if (indiceLectura >= numLecturas) indiceLectura = 0;
 
  return totalBPM / numLecturas;
}


void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("Iniciando sensores...");


  if (!mpu.begin()) {
    Serial.println("Error MPU6050");
    while (1) yield();
  }


  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("Error MAX30102");
    while (1) yield();
  }
 
  // CONFIGURACIÓN FIFO (El sensor nos da 1 dato cada 40ms = 25Hz)
  // 100 muestras = 4 segundos de análisis
  byte ledBrightness = 60;
  byte sampleAverage = 4;  
  byte ledMode = 2;        
  int sampleRate = 100;    
  int pulseWidth = 411;    
  int adcRange = 4096;    


  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);


  tempSensor.begin();
  tempSensor.setWaitForConversion(false);
  tempSensor.requestTemperatures();
  ultimoTiempoTemp = millis();


  Serial.println("Sistema listo. Pon tu dedo firmemente.");
}


void loop() {
  //LEER TEMPERATURA
  if (millis() - ultimoTiempoTemp >= 1000) {
    temperaturaActual = tempSensor.getTempCByIndex(0);
    tempSensor.requestTemperatures();                  
    ultimoTiempoTemp = millis();
  }


  //LEER MAX30102
  particleSensor.check();
 
  while (particleSensor.available()) {
    long irValue = particleSensor.getFIFOIR();
    long redValue = particleSensor.getFIFORed();
    particleSensor.nextSample();


    if (irValue < 50000) {
      // Dedo retirado: limpiar variables
      filtroIniciado = false;
      promedioBPM = 0;
      spo2 = 0;
      bufferIndex = 0;
    } else {
      // Guardar datos en el arreglo
      redBuffer[bufferIndex] = redValue;
      irBuffer[bufferIndex] = irValue;
      bufferIndex++;


      // Al llenar 100 muestras (4 segundos de historial), ejecutar la matemática
      if (bufferIndex == 100) {
        int32_t tempHR;
        int8_t validHR;
        int32_t tempSPO2;
        int8_t validSPO2;


        maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &tempHR, &validHR, &tempSPO2, &validSPO2);
       
        // Actualizar variables globales solo si el cálculo matemático fue exitoso
        if (validHR == 1) {
          promedioBPM = filtrarBPM(tempHR);
        }
        if (validSPO2 == 1 && tempSPO2 > 0 && tempSPO2 <= 100) {
          spo2 = tempSPO2;
        }


        for (byte i = 25; i < 100; i++) {
          redBuffer[i - 25] = redBuffer[i];
          irBuffer[i - 25] = irBuffer[i];
        }
        bufferIndex = 75;
      }
    }
  }


  // 3. IMPRIMIR DATOS Y LEER MPU6050
  if (millis() - ultimoTiempoPrint >= 200) {
    sensors_event_t a, g, temp_mpu;
    mpu.getEvent(&a, &g, &temp_mpu);
   
    Serial.print("Temp: ");
    Serial.print(temperaturaActual);
    Serial.print("C | AccX: ");
    Serial.print(a.acceleration.x);
    Serial.print(" | ");
   
    if (bufferIndex == 0) {
      Serial.println("-- (Esperando dedo)");
    } else if (bufferIndex < 100 && promedioBPM == 0) {
      // Mensaje mientras recolecta los primeros 4 segundos de datos
      Serial.println("Recolectando datos (4 seg)...");
    } else {
      Serial.print("BPM: ");
      Serial.print(promedioBPM);
      Serial.print(" | SpO2: ");
     
      if (spo2 > 0) {
        Serial.print(spo2);
        Serial.println("%");
      } else {
        Serial.println("Calculando...");
      }
    }
   
    ultimoTiempoPrint = millis();
  }
}
