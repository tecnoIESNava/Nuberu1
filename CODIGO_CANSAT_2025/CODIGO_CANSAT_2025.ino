
// DATOS A MODIFICAR EL DÍA DEL LANZAMIENTO

#define presion_real 1027.0  // Consultar presión atmosférica del lugar (ventusky)
#define BAND 866E6           // Frecuencia de emisión LoRa (es un dato de tipo "long")


// PONIENDO EL PIN A NIVEL BAJO (LOW) SE SIMULA LA PULSACIÓN DEL BOTÓN DEL MÓDULO QUE CONTROLA EL NEBULIZADOR ULTRASÓNICO
// EL MÓDULO FUNCIONA ASÍ:
//  1ª PULSACION DEL BOTÓN: ACTIVA NEBULIZADOR
//  2ª PULSACIÓN: -
//  3ª PULSACIÓN: DESACTIVA NEBULIZADOR.

//  - Para simular la pulsación, pasamos de nivel alto (por defecto) a nivel bajo, mantenemos unos instantes y volvemos a nivel alto.
//  - Después de la pulsación, hacemos una pequeña espera.


#include <OneWire.h>

#include "SD.h"

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Librerías para los sensores
#include <Adafruit_BMP280.h>
#include <DallasTemperature.h>

// Pin de datos para los sensores internos. ¡¡ DESCONECTAR EL CABLE PARA PROGRAMAR !!
const int oneWirePin = 12;
const int pinNebulizador = 4;


#define SDSCK 14
#define SDMOSI 15
#define SDMISO 2
#define SDCS 13

// Datos de configuración que se leerán de la tarjeta SD (valores por defecto si no se consigue leer la tarjeta)
long frecuencia_emision = 868200000;  // Frecuencia de emision en Hz
float altitud_referencia=180;         // Aerodromo de La Morgal
float presion_referencia=101700;      // Presión atmosferica en el Aerodromo (Pascales)

// float altitud_referencia = 26;
// float presion_referencia = 1018.0;


File dataFile;
SPIClass sdSPI(HSPI);

OneWire oneWireBus(oneWirePin);

DallasTemperature sensors(&oneWireBus);

int x = 0;  // Variable contador de trama

const int trama_escudo_on = 4;     // Trama en la que se va a activar el escudo térmico
const int tiempoNebulizacion = 5;  // Tiempo de nebulizacion (en segundos)

Adafruit_BMP280 bmp;

String string_cadena;


//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 23
#define DIO0 26


//OLED pins
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

//contador de tiempo
int counter = 0;

int OLED_OK = 1;    // Controla si funciona la pantalla OLED
int envio_OK = 1;   // Controla si se ha enviado una trama correctamente
int grabar_OK = 1;  // Controla si se ha grabado una trama en el archivo correctamente
int escudo = 0;     // Controla la activación del nebulizador (escudo térmico)


// Variables para las medidas de los sensores
float Te1, Te2, Te3, Te4;
float Text, Pext, altitud;


// Variables para medir el voltaje de la batería
float tension_alimentacion = 3.3;
float lectura, voltaje_medido, voltaje_real;

//creacion de pantalla (objeto display)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// Leemos la configuración del archivo config.txt alojado en la tarjeta SD
//  La sintaxis del archivo es: frecuenciaEmision;altitudLocal;presionLocal

void leer_configuracion() {
  String s = "";

  dataFile = SD.open("/config.txt", FILE_READ);
  if (dataFile) {

    if (OLED_OK) {
      display.setCursor(0, 20);
      display.print("Abierto config.txt");
      display.display();
    }

    while (dataFile.available()) {
      char c = (char)dataFile.read();
      if (c == '\n') {
        s = "";
      } else if (c != '\r') {
        s += c;
      }
    }
    Serial.println(s);

    // Separar los datos de la cadena

    int pos = 0;
    int lastPos = 0;
    int index = 0;
    String valores[3];

    while ((pos = s.indexOf(';', lastPos)) != -1) {
      valores[index] = s.substring(lastPos, pos);
      lastPos = pos + 1;
      index++;
    }
    valores[index] = s.substring(lastPos);

    frecuencia_emision = valores[0].toInt();
    altitud_referencia = valores[1].toFloat();
    presion_referencia = valores[2].toFloat();

    Serial.print("Frecuencia: ");
    Serial.println(frecuencia_emision);
    Serial.print("Altitud referencia: ");
    Serial.println(altitud_referencia);
    Serial.print("Presion referencia: ");
    Serial.println(presion_referencia);

    if (OLED_OK) {
      display.setCursor(0, 30);
      display.print("Frecuencia: " + String(frecuencia_emision));
      display.setCursor(0, 40);
      display.print("Altitud ref: " + String(altitud_referencia));
      display.setCursor(0, 50);
      display.print("Presion ref: " + String(presion_referencia));
      display.display();
    }

    dataFile.close();
  } 
  else {
    Serial.println("Error en la lectura SD");
    if (OLED_OK) {
      display.setCursor(0, 20);
      display.print("Datos por defecto");
      display.setCursor(0, 30);
      display.print("Frecuencia: " + String(frecuencia_emision));
      display.setCursor(0, 40);
      display.print("Altitud ref: " + String(altitud_referencia));
      display.setCursor(0, 50);
      display.print("Presion ref: " + String(presion_referencia));
      display.display();
    }
  }
  delay(2000);
  
}



void envia_lora() {  //Send LoRa packet to receiver
  LoRa.beginPacket();
  if (LoRa.print(string_cadena)) {
    envio_OK = 1;
  } else {
    envio_OK = 0;
  }
  LoRa.endPacket();
}


void muestra_datos() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.print("T1:" + String(Te1) + " T2:" + String(Te2));
  display.setCursor(0, 12);
  display.print("T3:" + String(Te3) + " Vbat:" + String(voltaje_real));
  display.setCursor(0, 22);
  display.print("Text:" + String(Text) + " C");
  display.setCursor(0, 32);
  display.print("Pext:" + String(Pext) + " Pa");
  display.setCursor(0, 42);
  display.print("h:" + String(altitud) + " m");
  display.setCursor(0, 52);
  display.print(" -- NUBERU TEAM " + String(envio_OK) + String(grabar_OK));
  display.display();
}


void leer_sensores() {
  // Lectura de sensores de temperatura interior (DS18B20)
  sensors.requestTemperatures();
  Te1 = sensors.getTempCByIndex(0);
  Te2 = sensors.getTempCByIndex(1);
  Te3 = sensors.getTempCByIndex(2);
//  Te4 = sensors.getTempCByIndex(3); // Vamos a prescindir del 4º sensor de temperatura

  // Lectura de sensores de parámetros exteriores (BMP280)
  Text = bmp.readTemperature();
  Pext = bmp.readPressure();  // Presión en Pascales
  altitud = bmp.readAltitude(presion_referencia);


  // Lectura del voltaje de la batería
  lectura=analogRead(35);
  voltaje_medido=lectura*tension_alimentacion/4096;
  voltaje_real=2*voltaje_medido;
}


void escribirSD() {
  dataFile = SD.open("/data1.txt", FILE_APPEND);
  if (dataFile) {
    dataFile.println(string_cadena);
    dataFile.close();
    grabar_OK = 1;
  } else {
    grabar_OK = 0;
  }
}



// Función para emular la pulsación del botón del módulo del nebulizador
void pulsar() {
  digitalWrite(pinNebulizador, LOW);
  delay(80);
  digitalWrite(pinNebulizador, HIGH);
  delay(200);
}

void activar_escudo() {
  pulsar();
}

void desactivar_escudo() {
  pulsar();
  pulsar();
}



void setup() {

  pinMode(pinNebulizador, OUTPUT);
  //digitalWrite(pinNebulizador,HIGH);
  activar_escudo();
  desactivar_escudo();

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {  //, false, false)) { // Address 0x3C for 128x32
    OLED_OK = 0;
  }

  if (OLED_OK == 1) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    // display.setCursor(0, 0);
    // display.print("DISPLAY OK ");
    display.display();
  }

  // Lector de tarjetas SD
  uint8_t cardType;
  sdSPI.begin(SDSCK, SDMISO, SDMOSI, SDCS);

  if (!SD.begin(SDCS, sdSPI)) {
    if (OLED_OK == 1) {
      display.setCursor(0, 0);
      display.print("SD ERROR");
      display.display();
    }
  }

  else {
    if (OLED_OK == 1) {
      display.setCursor(0, 0);
      display.print("Modulo SD OK");
      display.setCursor(0, 10);
      display.print("Leyendo config");
      display.display();
    }
  }

  Serial.begin(115200);
  Serial.println("Leyendo config");
  leer_configuracion();

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);


  //  if (!LoRa.begin(BAND)) {
  display.clearDisplay();
  if (!LoRa.begin(frecuencia_emision)) {
    if (OLED_OK == 1) {
      display.setCursor(0, 10);
      display.print("LORA ERROR");
      display.display();
    }
  }

  else {
    if (OLED_OK == 1) {
      display.setCursor(0, 10);
      display.print("LORA OK");
      display.display();
      //delay(2000);
    }
  }


  sensors.begin();
  if (OLED_OK == 1) {
    display.setCursor(0, 20);
    display.print("DS18B20 OK");
    display.display();
  }


  if (!bmp.begin()) {
    if (OLED_OK == 1) {
      display.setCursor(0, 30);
      display.print("BMP ERROR");
      display.display();
    }
  } else {
    if (OLED_OK == 1) {
      display.setCursor(0, 30);
      display.print("BMP OK");
      display.display();
    }
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Modo de operación */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Presion oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtrado. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Tiempo Standby. */



  dataFile = SD.open("/data1.txt", FILE_WRITE);
  dataFile.close();
}



void loop() {

  leer_sensores();

  x++;

  if (x == trama_escudo_on) {
    escudo = 1;
    activar_escudo();
  }
  if (x == (trama_escudo_on + tiempoNebulizacion)) {
    escudo = 0;
    desactivar_escudo();
  }

  string_cadena = "NUBERU;" + String(x) + ";" + String(Te1) + ";" + String(Te2) + ";" + String(Te3) + ";" + String(voltaje_real) + ";" + String(Text) + ";" + String(Pext) + ";" + String(altitud) + ";" + String(escudo);

  escribirSD();

  envia_lora();
  if (OLED_OK == 1) {
    muestra_datos();
  }

  delay(1000);
}