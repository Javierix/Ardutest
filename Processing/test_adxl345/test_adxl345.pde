/*
Codigo para Processing que interpreta los datos enviados por Arduino y el ADXL345 y los muestra en la pantalla
*/

//import processing.opengl.*;
import processing.serial.*;

Serial SerialPort;
PFont fontA;

int[] nums = new int [3];
int SIZE = 360, SIZEX = 360;

void setup() {
  size(SIZEX, SIZE, P3D);
  println(Serial.list());
  SerialPort = new Serial(this, "COM5", 9600);
  //SerialPort = new Serial(this, Serial.list()[0], 9600);
  SerialPort.clear();
  
  fontA = loadFont("ComicSansMS-48.vlw");
  textAlign(CENTER);
  textFont(fontA, 50);
}

void draw() {
  if ( SerialPort.available() > 0) {   
    String inBuffer = SerialPort.readStringUntil(10); 
    if (inBuffer != null) {
      //println(inBuffer);
      nums = int(split(inBuffer, ' '));
      //println (split(inBuffer, ' '));
    }  
  }
  
  //println(nums[0]);
  //println(nums[1]);
  //println(nums[2]);
  float aax = nums[0];
  float aay = nums[1];
  float aaz = nums[2];
  
  float rho = atan(aax/sqrt((aay*aay)+(aaz*aaz))) * 57.29577951;   //angulo en radianes
  float phi = atan(aay/sqrt((aax*aax)+(aaz*aaz))) * 57.29577951;   //angulo en radianes
  //float theta = atan(sqrt((aay*aay)+(aax*aax))/aaz)*57.29577951;
  float theta = atan(aaz/sqrt((aay*aay)+(aax*aax))) * 57.29577951; //angulo en radianes
  //println (rho+" "+phi+" "+theta);
 
  translate(SIZEX/2, SIZE/2, -400);
  background(0);
  //buildShape(radians(mouseX), radians(mouseY));
  buildShape(radians(-rho*2), radians(-phi*2));
  textAlign(CENTER);
  text(int(rho), 0-150, 300);
  text(int(phi), 0, 300);
  text(int(theta), 0+150, 300);
}


void buildShape(float rotz, float rotx) {
  pushMatrix();
  scale(8,8,20);
  rotateZ(rotz/2);
  rotateX(rotx/2);
  fill(255);
  stroke(0);
  
  box(60, 10, 10);
  fill(0, 255, 0);
  box(10, 9, 40);
  translate(0, -10, 20);
  fill(255, 0, 0);
  popMatrix();
}


