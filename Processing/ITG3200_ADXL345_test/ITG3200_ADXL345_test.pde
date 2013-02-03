/**
 * Displays values coming from an ITG3200 and ADXL345 connected to Arduino
*/

import processing.serial.*;

Serial myPort;  // Create object from Serial class

int x = 0;
int y = 0; 
int z = 0; 

int a = 0;
int b = 0; 
int c = 0; 

int lf = 10; // 10 is '\n' in ASCII
byte[] inBuffer = new byte[50];
PFont font;


void setup() 
{
  size(600, 600);
  //myPort = new Serial(this, "/dev/ttyUSB0", 9600);  
  myPort = new Serial(this, "COM5", 9600);
  
  // The font must be located in the sketch's 
  // "data" directory to load successfully
  font = loadFont("CourierNewPSMT-14.vlw"); 
  
}


void draw() {
  if(myPort.available() > 0) {
    if (myPort.readBytesUntil(lf, inBuffer) > 0) {
      String inputString = new String(inBuffer);
      String [] inputStringArr = split(inputString, ',');
      
      x = int(inputStringArr[0]);
      y = int(inputStringArr[1]);
      z = int(inputStringArr[2]);
      
      a = int(inputStringArr[3]);
      b = int(inputStringArr[4]);
      c = int(inputStringArr[5]);
    }
  }
  
  background(#000000);
  
  int R = 0;
  int G = 255;
  int B = 0;
  color greencolor = color(R, G, B);
  fill(greencolor);
  
  text("Giroscopio",90, 80);
  text("Acelerometro",320, 80);
  text("+",20, 100);
  
  float x_ds = (float(x) / 14.375); // converts to degrees/sec
  float hx = x_ds * 250 / 2000;     //compute rectange higth
  rect(50, 250, 50, 2);
  rect(50, 250, 50, hx * 3);
  
  float y_ds = (float(y) / 14.375);
  float hy = y_ds * 250 / 2000;
  rect(110, 250, 50, 2);
  rect(110, 250, 50, hy * 3);
  
  float z_ds = (float(z) / 14.375);
  float hz = z_ds * 250 / 2000;
  rect(170, 250, 50, 2);
  rect(170, 250, 50, hz * 3);
  
  //Parte del acelerometro
  rect(300, 250, 50, 2);
  rect(300, 250, 50, -a/2);
  
  rect(360, 250, 50, 2);
  rect(360, 250, 50, -b/2);
  
  rect(420, 250, 50, 2);
  rect(420, 250, 50, -c/2);
  
  textFont(font);
  text("raw: " + x + " " + y + " " + z + "\n°/s: \n X:" + x_ds + "\n Y:" + y_ds + "\n Z:" + z_ds, 20, 500);
  text("Acc: \n X:" + a + "\n Y:" + b + "\n Z:" + c, 400, 520); 
  
}


