
// FORMATO DE TRAMA RECIBIDA DEL CANSAT (11 datos)
//   NUBERU;x;Te1;Te2;Te3;Vbat;Text;Pext;altitud;neb;rssi
//      NUBERU: nombre del equipo
//      x: número de paquete
//      Te1-4: temperatura interior (sensores 1 a 4)
//      Text,Pext: temperatura y presión exteriores
//      altitud
//      neb: funcionamiento del nebulizador (1=ON)
//      rssi: calidad de la señal



import org.gicentre.utils.stat.*;
import processing.serial.*;

Serial puertoSerie;
PImage logo_nuberu;

XYChart grafico_t, grafico_p, grafico_h, grafico_T1, grafico_T2, grafico_T3, grafico_Vbat;


//======= RELLENAR MANUALMENTE CON LOS DATOS DEL LUGAR DE LANZAMIENTO
float altitud_real = 10; 
float presion_real = 1007.2;  // Datos servicio meteorológico
//=======

float n=0.0;
int numero_x=0; // El nº paq se define como global porque se utiliza para calcular el tiempo de misión en otra función

color borde = color(2,143,157);
color relleno = color(174,242,252);
color texto = color(10);

String cadena;
String[] datos={};
float x, t, p, h;     // datos recibidos
float T1, T2, T3, Vbat;
String nebulizador;    // estado del nebulizador
String rssi;          // calidad de la señal
String estado_nebulizador="Desactivado";

float[] valoresX={};
float[] valoresY_t={};
float[] valoresY_p={};
float[] valoresY_h={};
float[] valoresY_T1={};
float[] valoresY_T2={};
float[] valoresY_T3={};
float[] valoresY_Vbat={};


PrintWriter archivo;
PrintWriter DatosExcel;
String nombre_del_archivo="";


void setup(){
  size(2000,900); 
  fill(255);
  rect(0,0,width,height);
  puertoSerie=new Serial(this,Serial.list()[0],9600);
  logo_nuberu=loadImage("LOGONuberu.png");
  logo_nuberu.resize(200,200);
  image(logo_nuberu,35,35);  
  
  nombre_del_archivo=obtener_timestamp()+".txt";
  archivo=createWriter(nombre_del_archivo);
  
  dibujarMarcos();
  
  grafico_t = new XYChart(this);
  grafico_p = new XYChart(this);
  grafico_h = new XYChart(this);
  grafico_T1 = new XYChart(this);
  grafico_T2 = new XYChart(this);
  grafico_T3 = new XYChart(this);
  grafico_Vbat = new XYChart(this);
}


// *********************************************************************
// FUNCIÓN PARA DIBUJAR LOS MARCOS BLANCOS DE LOS GRÁFICOS Y LOS CUADROS
// *********************************************************************
void dibujarMarcos(){
  // Marco tiempo de misión
  stroke(borde);  
  noFill();
  rect(400,50,870,50);
  textSize(20);fill(borde);
  text("Tiempo de misión: ",440,80);

  fill(255); stroke(borde);  

  // Marco datos en tiempo real en formato texto
  rect(400,120,870,160);
  
  // Marcos para los gráficos Te1 y Te2
  rect(30,300,440,240);
  rect(30,610,440,240);

  // Marco para la lata de colores
  rect(500,350,300,450);
  
  // Marcos para los gráficos Te3 y Te4
  rect(830,300,440,240);
  rect(830,610,440,240);
  
  // Marcos para los gráficos Text, Pext, altitud
  rect(1350,100,440,240);
  rect(1350,370,440,240);
  rect(1350,640,440,240);
  
  fill(borde);  // Títulos de los gráficos
  textSize(18);
  text("Sensor de temperatura #1 (ºC)",120,320);
  text("Sensor de temperatura #2 (ºC)",120,630);
  text("Sensor de temperatura #3 (ºC)",930,320);
  text("Voltaje de la batería (V)",930,630);
  
  text("Temperatura exterior (ºC)",1500,120);
  text("Presión exterior (kPa)",1500,390);
  text("Altitud (msnm)",1500,660);
  
}


// Función que calcula el tiempo que lleva encendido el CANSAT y lo
//  representa en pantalla
void tiempo_de_mision(){
  int horas_mision=numero_x/3600;
  String horas_mision_txt=str(horas_mision);
  if(horas_mision_txt.length()==1){
    horas_mision_txt="0"+horas_mision_txt;
  }
  int minutos_mision=(numero_x % 3600) / 60;
  String minutos_mision_txt=str(minutos_mision);
  if(minutos_mision_txt.length()==1){
    minutos_mision_txt="0"+minutos_mision_txt;
  }  
  int segundos_mision=(numero_x % 60);
  String segundos_mision_txt=str(segundos_mision);
  if(segundos_mision_txt.length()==1){
    segundos_mision_txt="0"+segundos_mision_txt;
  }
  
  stroke(borde);  
  fill(255);
  rect(400,50,870,50);
  textSize(20);fill(borde);
  text("Tiempo de misión: ",440,80);
  String tiempo_mision_entxt=horas_mision_txt+":"+minutos_mision_txt+":"+segundos_mision_txt;
  text(tiempo_mision_entxt,620,80);
}


void analizarDatos(){
  try{
    numero_x=int(datos[1]);
    T1=float(datos[2]);
    T2=float(datos[3]);
    T3=float(datos[4]);
    Vbat=float(datos[5]);
    t=float(datos[6]);
    p=float(datos[7]);
    h = float(datos[8]);
    nebulizador=datos[9];
    rssi=datos[10];
    
    if(int(nebulizador)==1){
      estado_nebulizador="Activado";
    }
    else{
      estado_nebulizador="Desactivado";
    }
    

    
    // Ampliamos los vectores con los valores de la última muestra
    valoresX=append(valoresX,numero_x);    
    valoresY_t=append(valoresY_t,t);
    valoresY_p=append(valoresY_p,p/1000.0);
    valoresY_h=append(valoresY_h,h);  
    valoresY_T1=append(valoresY_T1,T1);  
    valoresY_T2=append(valoresY_T2,T2);  
    valoresY_T3=append(valoresY_T3,T3);  
    valoresY_Vbat=append(valoresY_Vbat,Vbat);     
  }
  catch(Exception e){}
}


void representarDatos(){
  tiempo_de_mision();
  fill(borde);
  //text("DATOS EN TIEMPO REAL",700,150);
  text("Sensor de temperatura #1 (ºC): "+str(T1),420,150);
  text("Sensor de temperatura #2 (ºC): "+str(T2),420,180);
  text("Sensor de temperatura #3 (ºC): "+str(T3),420,210);
  text("Voltaje de la batería (V): "+str(Vbat),420,240);
  
  text("Nebulizador: "+estado_nebulizador,420,270);
  
  
  text("Temperatura exterior (ºC): "+str(t),900,150);
  text("Presión (kPa): "+str(p),900,180);
  text("Altitud (msnm): "+str(h),900,210);
  text("RSSI: "+rssi,900,240);
}


void configurarGraficos(){
  grafico_t.setData(valoresX,valoresY_t);
  grafico_p.setData(valoresX,valoresY_p);
  grafico_h.setData(valoresX,valoresY_h);
  grafico_T1.setData(valoresX,valoresY_T1);  
  grafico_T2.setData(valoresX,valoresY_T2);  
  grafico_T3.setData(valoresX,valoresY_T3);  
  grafico_Vbat.setData(valoresX,valoresY_Vbat);  
  
  grafico_t.showXAxis(true);     grafico_t.showYAxis(true);
  grafico_p.showXAxis(true);     grafico_p.showYAxis(true);
  grafico_h.showXAxis(true);     grafico_h.showYAxis(true);
  grafico_T1.showXAxis(true);    grafico_T1.showYAxis(true);
  grafico_T2.showXAxis(true);    grafico_T2.showYAxis(true);
  grafico_T3.showXAxis(true);    grafico_T3.showYAxis(true);
  grafico_Vbat.showXAxis(true);  grafico_Vbat.showYAxis(true);
  
  grafico_t.setYFormat("#.00");
  grafico_p.setYFormat("#.00");
  grafico_h.setYFormat("#.00");
  grafico_T1.setYFormat("#.00");
  grafico_T2.setYFormat("#.00");
  grafico_T3.setYFormat("#.00");
  grafico_Vbat.setYFormat("#.00");
  
  grafico_t.setPointColour(color(240,240,240,240));
  grafico_p.setPointColour(color(240,240,240,240));
  grafico_h.setPointColour(color(240,240,240,240));
  grafico_T1.setPointColour(color(240,240,240,240));
  grafico_T2.setPointColour(color(240,240,240,240));
  grafico_T3.setPointColour(color(240,240,240,240));
  grafico_Vbat.setPointColour(color(240,240,240,240));
  
  grafico_t.setLineColour(color(194,24,91));
  grafico_t.setAxisColour(borde);
  grafico_t.setAxisValuesColour(borde);
  
  grafico_p.setLineColour(color(63,81,181));
  grafico_p.setAxisColour(borde);
  grafico_p.setAxisValuesColour(borde);
  
  grafico_h.setLineColour(color(0,137,123));
  grafico_h.setAxisColour(borde);
  grafico_h.setAxisValuesColour(borde);
  
  grafico_T1.setLineColour(color(251,192,45));
  grafico_T1.setAxisColour(borde);
  grafico_T1.setAxisValuesColour(borde);

  grafico_T2.setLineColour(color(251,192,45));
  grafico_T2.setAxisColour(borde);
  grafico_T2.setAxisValuesColour(borde);

  grafico_T3.setLineColour(color(251,192,45));
  grafico_T3.setAxisColour(borde);
  grafico_T3.setAxisValuesColour(borde);

  grafico_Vbat.setLineColour(color(165,105,189));
  grafico_Vbat.setAxisColour(borde);
  grafico_Vbat.setAxisValuesColour(borde);
  
  grafico_t.setPointSize(0);    grafico_t.setLineWidth(1.5);
  grafico_p.setPointSize(0);    grafico_p.setLineWidth(1.5);
  grafico_h.setPointSize(0);    grafico_h.setLineWidth(1.5);
  grafico_T1.setPointSize(0);   grafico_T1.setLineWidth(1.5);
  grafico_T2.setPointSize(0);   grafico_T2.setLineWidth(1.5);
  grafico_T3.setPointSize(0);   grafico_T3.setLineWidth(1.5);
  grafico_Vbat.setPointSize(0); grafico_Vbat.setLineWidth(1.5);
}


void representarGraficos(){
  try{
    configurarGraficos();
    
    fill(255); // Pintamos los cuadros de los gráficos para "borrar"
    dibujarMarcos();
    
    grafico_T1.draw(40,310,400,220);
    grafico_T2.draw(40,620,400,220);
    grafico_T3.draw(840,310,400,220);
    grafico_Vbat.draw(840,620,400,220);
    grafico_t.draw(1360,110,400,220);
    grafico_p.draw(1360,380,400,220);
    grafico_h.draw(1360,650,400,220);
  }
  catch(Exception e){
    print(e);
  }
}

// ************************************************
//  FUNCIÓN PARA DIBUJAR LA LATA CON MAPA DE CALOR
// ************************************************
void dibujar_lata(){
  
  noStroke();
  float maxTemp = max(max(T1, T2), T3);
  float minTemp = min(min(T1, T2), T3);
  dibujar_mapa_calor(500,350,300,450,T1,T2,T3,minTemp,maxTemp);
  
  fill(borde);
  ellipse(560,440,8,8);
  stroke(borde);
  noFill();
  beginShape();
    vertex(560,440);
    vertex(500,400);
    vertex(470,400);
  endShape();
  
  fill(borde);
  ellipse(560,710,8,8);
  noFill();
  beginShape();
    vertex(560,710);
    vertex(500,750);
    vertex(470,750);
  endShape();
  
  fill(borde);
  ellipse(740,440,8,8);
  noFill();
  beginShape();
    vertex(740,440);
    vertex(800,400);
    vertex(830,400);
  endShape();
  
  fill(borde);
  ellipse(740,710,8,8);
  noFill();
  beginShape();
    vertex(740,710);
    vertex(800,750);
    vertex(830,750);
  endShape();
}


void dibujar_mapa_calor(float x, float y, float w, float h, float t1, float t2, float t3, float minTemp, float maxTemp) {
  int numPixelsX = 500; // number of pixels in the x direction
  int numPixelsY = 500; // number of pixels in the y direction
  
  // offset for the temperature points
  float offsetX = w * 0.0; // 20% from the edge
  float offsetY = h * 0.0; // 20% from the edge
  
  for (int i = 0; i < numPixelsX; i++) {
    for (int j = 0; j < numPixelsY; j++) {
      float px = x + i * w / numPixelsX;
      float py = y + j * h / numPixelsY;
      float pw = w / numPixelsX;
      float ph = h / numPixelsY;
      
      // calculate the temperature for this pixel using bilinear interpolation
      float tx = (px - x) / (w - 2 * offsetX);
      float ty = (py - y) / (h - 2 * offsetY);
      float t = t1 * (1 - tx) * (1 - ty) + t2 * tx * (1 - ty) + t3 * (1 - tx) * ty;
      
      // map the temperature to a color
      color c = obtenerColor(t, minTemp,maxTemp);
      
      // draw the pixel
      fill(c);
      rect(px, py, pw, ph);
    }
  }
}

float obtenerTemperatura(float x, float y, float w, float h, float t1, float t2, float t3) {
  // interpolación para conseguir el valor de la temperatura en un punto
  float tx = x / w;
  float ty = y / h;
  float t = t1 * (1 - tx) * (1 - ty) + t2 * tx * (1 - ty) + t3 * (1 - tx) * ty;
  return t;
}

color obtenerColor(float t, float minTemp, float maxTemp) {
  // calculate a normalized value between 0 and 1
  float norm = (t - minTemp) / (maxTemp - minTemp);
  
  // stretch the normalized value to exaggerate the differences
  norm = pow(norm, 0.2);
  
   // clamp the stretched value to avoid very low values
  norm = constrain(norm, 0.1, 0.9);
  
  // map the stretched value to a color
  float r = norm * 255;
  float g = 0;
  float b = (1 - norm) * 255;
  
  return color(r, g, b);
}


String obtener_timestamp(){
  String dia=str(day()); if(dia.length()==1){dia="0"+dia;}
  String mes=str(month()); if(mes.length()==1){mes="0"+mes;}
  String anio=str(year());
  String hora=str(hour()); if(hora.length()==1){hora="0"+hora;}
  String minuto=str(minute()); if(minuto.length()==1){minuto="0"+minuto;}
  String segundo=str(second()); if(segundo.length()==1){segundo="0"+segundo;}
  String timestamp=dia+"_"+mes+"_"+anio+"_"+hora+"_"+minuto+"_"+segundo;
  return timestamp;  
}

void grabarDatos(){
     archivo.print(cadena); 
}

void draw(){
  if(puertoSerie.available()>0){
    cadena=puertoSerie.readStringUntil('\n');
    if(cadena!=null){
      datos=cadena.split(";");
      if(datos.length>8){         
        print(cadena);  
        analizarDatos();
        grabarDatos();
        representarGraficos();
        representarDatos();
        dibujar_lata();
      }
    }
  } 
}


void guardar_cerrar_nuevo(){
  archivo.flush();
  archivo.close();
  //contador_archivo++;
    
  nombre_del_archivo=obtener_timestamp()+".txt";
  archivo=createWriter(nombre_del_archivo); 
}

void keyPressed(){
  if(key == 's'){
    guardar_cerrar_nuevo();
  }
  
  if(key == 'c'){
    for(int k=valoresX.length-1;k==0;k--){    
    }
  }
}

void exit(){
  archivo.flush();
  archivo.close();
}
