# RETO-IoT: Sistema de Monitoreo Biomédico de Constantes Vitales y Detección de Caídas 

Equipo:
- Maximiliano Tinajero Rojas
- Roberto Emiliano Hernández Aguilar

1. Descripción del Proyecto:    
Este proyecto aborda el desarrollo de un sistema wearable de tecnología IoT diseñado para el seguimiento de ciertas mediciones, siendo la oximetría y frecuencia cardíaca, temperatura corporal en tiempo real y la identificación de accidentes por caídas. Todo esto mediantw un microcontrolador local, su envío por vía inalámbrica y el almacenamiento estructurado en una base de datos local

2. Arquitectura del Sistema (5 Capas)    
  Capa de Percepción:  
Oximetría y Frecuencia Cardíaca: MAX30102   
Temperatura Corporal: DS18B20, MLX90614 o Termistor NTC   
Caídas/Actividad: MPU6050

  Capa de Procesamiento Local:    
Microcontrolador: Arduino UNO   
Capa de red:
Módulo de Comunicación: ESP8266 (Wi-Fi) o HC-05 (Bluetooth) conectado al Arduino UNO para transmisión inalámbrica
Protocolo: Envío de datos desde el módulo hacia la red local

  Capa de Servidor y Base de Datos:     
Base de Datos: MySQL en XAMPP para almacenamiento relacional e historial persistente de las lecturas.
Gestión de Base de Datos: phpMyAdmin (XAMPP) para la creación, administración y consulta de las tablas.

  Capa de Aplicación:
Interfaz de Usuario: Consulta de datos en phpMyAdmin o un Dashboard local conectado directamente a MySQL en XAMPP para analizar las medidas vitales y alertas de caídas


3. Ubicación Corporal y Render del Dispositivo

  Ubicación: wearable para colocarse en la muñeca o el antebrazo.
