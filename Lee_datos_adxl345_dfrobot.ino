/*
Adaptado para ARDUINO 1.0 - Cambia el nombre de funciones en serial e I2C y tambien definiciones de tipos.
Version 3
*/

#include <Wire.h>
 
#define DEVICE (0x53)    //ADXL345 device address
#define TO_READ (6)      //num of bytes we are going to read each time (two bytes for each axis)
 
byte buff[TO_READ] ;    //6 bytes buffer for saving data read from the device
char str[512];          //string buffer to transform data before sending it to the serial port
int x, y, z;
long statex, statey, statez;
int regAddress = 0x32;    //first axis-acceleration-data register on the ADXL345

void setup()
{
  //int w;
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
 
  //Turning on the ADXL345
  writeTo(DEVICE, 0x2D, 0);     
  writeTo(DEVICE, 0x2D, 16);
  writeTo(DEVICE, 0x2D, 8);
  
  Calibrate();
  
  //readFrom(DEVICE, 0x00, 1, buff);
  //w = buff[0];
  //Serial.println(w);
}
 
void loop()
{
  readFrom(DEVICE, regAddress, TO_READ, buff); //read the acceleration data from the ADXL345
  
  //each axis reading comes in 10 bit resolution, ie 2 bytes.  Least Significat Byte first!!
  //thus we are converting both bytes in to one int
  x = ((((int)buff[1]) << 8) | buff[0]) - statex;  
  y = ((((int)buff[3]) << 8) | buff[2]) - statey;
  z = ((((int)buff[5]) << 8) | buff[4]);
  
  if ((x <= 3) && (x >= -3))  {x = 0;}        // Ventana de filtrado 
  if ((y <= 3) && (y >= -3))  {y = 0;}
  if ((z <= 3) && (z >= -3))  {z = 0;}  
 
  
  //we send the x y z values as a string to the serial port
  //sprintf(str, "x:%d y:%d z:%d", x, y, z); 
  sprintf(str, "%d %d %d ", x, y, z);
  Serial.println(str);
  //Serial.println (statex);
  //Serial.println (statey);
  //Serial.println (statez);
  //Serial.print("\n");
  //It appears that delay is needed in order not to clog the port
  delay(200);
}
 

/***************************************************************************
  Writes val to address register on device
***************************************************************************/
void writeTo(int device, byte address, byte val) {
  Wire.beginTransmission(device); //start transmission to device
  Wire.write(address);        // send register address
  Wire.write(val);        // send value to write
  Wire.endTransmission(); //end  transmission
}

 
/*************************************************************************** 
  Reads num bytes starting from address register on device in to buff array
***************************************************************************/
void readFrom(int device, byte address, int num, byte buff[]) {
  Wire.beginTransmission(device); //start transmission to device
  Wire.write(address);        //sends address to read from
  Wire.endTransmission();    //end transmission
 
  Wire.beginTransmission(device); //start transmission to device
  Wire.requestFrom(device, num);    // request 6 bytes from device
 
  int i = 0;
  while(Wire.available())    //device may send less than requested (abnormal)
  {
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}


/*******************************************************************************
The purpose of the calibration routine is to obtain the value of the reference threshold.
It consists on a 1024 samples average in no-movement condition.
********************************************************************************/
void Calibrate(void){
  unsigned int count1;
  count1 = 0;
  do{
     readFrom(DEVICE, regAddress, TO_READ, buff); //read the acceleration data from the ADXL345
     x = (((int)buff[1]) << 8) | buff[0];  
     y = (((int)buff[3]) << 8) | buff[2];
     z = (((int)buff[5]) << 8) | buff[4];
     statex = statex + x;                        // Accumulate Samples
     statey = statey + y;
     statez = statez + z;
     count1++;
  }while(count1!=0x0FF);             // 256 times
  
  statex=statex>>8;                  // division between 256
  statey=statey>>8;
  statez=statez>>8;
}  