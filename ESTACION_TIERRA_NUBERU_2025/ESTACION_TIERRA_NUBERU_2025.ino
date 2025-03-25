#define BAND 866E6


#include "SD.h"

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


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


#define SDSCK 14
#define SDMOSI 15
#define SDMISO 2
#define SDCS 13

File dataFile;
SPIClass sdSPI(HSPI);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

String LoRaData;

String x, Te1, Te2, Te3, Vbat, Text, Pext, altitud;
String equipo="";
String datosSeparados[9]; // Cambia el tamaño según lo que necesites
int rssi;

// Datos de configuración que se leerán de la tarjeta SD (valores por defecto si no se consigue leer la tarjeta)
long frecuencia_emision=868E6;
float altitud_referencia=26;     
float presion_referencia=1018.0;   

int grabar_OK=1;// Controla si se ha grabado una trama en el archivo correctamente
int OLED_OK=1;  // Controla si funciona la pantalla OLED



void escribirSD() {
  dataFile = SD.open("/data.txt", FILE_APPEND);
  if (dataFile) {
    dataFile.println(LoRaData);
    dataFile.close();
    grabar_OK=1;
  }
  else{
    grabar_OK=0;
  }
}


void leer_configuracion() {
  String s = "";

  dataFile = SD.open("/config.txt", FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      char c = (char)dataFile.read();
      if (c == '\n'){
        s="";
      }
      else if (c !=  '\r') {
        s += c;
      }
    }
    Serial.println(s);

    // Separar los datos de la cadena

    int pos =0;
    int lastPos =0;
    int index=0;
    String valores[3];

    while((pos=s.indexOf(';',lastPos)) != -1){
      valores[index]=s.substring(lastPos,pos);
      lastPos =pos+1;
      index++;
    }
    valores[index]=s.substring(lastPos);

    frecuencia_emision=valores[0].toInt();
    altitud_referencia=valores[1].toFloat();
    presion_referencia=valores[2].toFloat();

    Serial.print("Frecuencia: ");Serial.println(frecuencia_emision);
    Serial.print("Altitud referencia: "); Serial.println(altitud_referencia);
    Serial.print("Presion referencia: "); Serial.println(presion_referencia);


    dataFile.close();
    } 
    else {
    Serial.println("Error en la lectura");
    }
  delay(1000);
}



void setup() {

  //initialize Serial Monitor
  Serial.begin(9600);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {  // Address 0x3C for 128x32
    //Serial.println(F("SSD1306 allocation failed"));
  }

  uint8_t cardType;
  sdSPI.begin(SDSCK, SDMISO, SDMOSI, SDCS);
  if (!SD.begin(SDCS, sdSPI))
  {
    Serial.println("SD Card Mount Failed");
  }

  leer_configuracion();

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LORA RECEIVER ");
  display.display();



  //Serial.println("LoRa Receiver Test");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(frecuencia_emision)) {
    //Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  //Serial.println("LoRa Initializing OK!");
  display.setCursor(0, 10);
  display.println("LoRa Inicializacion OK!");
  display.display();




  dataFile=SD.open("/data.txt",FILE_WRITE);
  dataFile.close();

}

void muestra_datos(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0); display.print("T1:"+String(Te1)+"  T2:"+String(Te2));      
  display.setCursor(0,10); display.print("T3:"+String(Te3)+"  Vbat:"+String(Vbat));      
  display.setCursor(0,20); display.print("Text:"+String(Text)+" C"); 
  display.setCursor(0,30); display.print("Pext:"+String(Pext)+" Pa");   
  display.setCursor(0,40); display.print("h:"+String(altitud)+"m rssi:"+String(rssi)); 
  display.setCursor(0,52); display.print(" -- NUBERU TEAM "+String(grabar_OK)); 
  display.display();
}




void separarDatos(String cadena) {
  // Convertir el String a un arreglo de caracteres
  char charArray[cadena.length() + 1];
  cadena.toCharArray(charArray, cadena.length() + 1);
  
  // Separar los datos usando strtok
  char* token = strtok(charArray, ";");
  
  // Contador para las variables
  int contador = 0;
  

  // Iterar sobre los tokens
  while (token != NULL) {
    datosSeparados[contador] = String(token);
    //Serial.println(datosSeparados[contador]); // Imprimir cada dato separado
    contador++;
    token = strtok(NULL, ";");
  }
  equipo = datosSeparados[0];
  x = datosSeparados[1];
  Te1 = datosSeparados[2];
  Te2 = datosSeparados[3];
  Te3 = datosSeparados[4];
  Vbat = datosSeparados[5];
  Text = datosSeparados[6];
  Pext = datosSeparados[7];
  altitud = datosSeparados[8];
}



void loop() {

  //try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {

    //read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      rssi = LoRa.packetRssi();
      separarDatos(LoRaData);
      LoRaData=LoRaData+";"+String(rssi);

      if(equipo=="NUBERU"){
        muestra_datos();
        Serial.println(LoRaData);
      }
      
      escribirSD();
    }
  }
}