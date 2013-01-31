#include <Wire.h> // I2C library, gyroscope

#define GYRO_ADDR 0x69 // gyro address, binary = 11101001 when AD0 is connected to Vcc (see schematics of your breakout board)
#define SMPLRT_DIV 0x15
#define DLPF_FS 0x16
#define INT_CFG 0x17
#define PWR_MGM 0x3E

#define TO_READ+2 8 // 2 bytes for each axis x, y, z and temp

// Gyroscope offsets are chip specific. 
int offx = 23;
int offy = -2;
int offz = -19;


#define DEVICE 0x53    //ADXL345 device address
#define TO_READ 6      //num of bytes we are going to read each time (two bytes for each axis)
byte buff[TO_READ] ;     //6 bytes buffer for saving data read from the device
char str[512];           //string buffer to transform data before sending it to the serial port
int x, y, z;
long statex, statey, statez;
int regAddress = 0x32;    //first axis-acceleration-data register on the ADXL345


void setup()
{
  Serial.begin(9600);
  Wire.begin();
  
  //Turning on the ADXL345
  writeTo(DEVICE, 0x2D, 0);     
  writeTo(DEVICE, 0x2D, 16);
  writeTo(DEVICE, 0x2D, 8);
  
  calibrate_Acc();
  initGyro();
}


void loop()
{
  getGyroscopeData();
  getAccData();
  delay(200);
}

//initializes the gyroscope
void initGyro()
{
  /*****************************************
  * ITG 3200
  * power management set to:
  * clock select = internal oscillator
  *     no reset, no sleep mode
  *   no standby mode
  * sample rate to = 125Hz
  * parameter to +/- 2000 degrees/sec
  * low pass filter = 5Hz
  * no interrupt
  ******************************************/
  
  writeTo(GYRO_ADDR, PWR_MGM, 0x00);
  writeTo(GYRO_ADDR, SMPLRT_DIV, 0x07); // EB, 50, 80, 7F, DE, 23, 20, FF
  writeTo(GYRO_ADDR, DLPF_FS, 0x1E); // +/- 2000 dgrs/sec, 1KHz, 1E, 19
  writeTo(GYRO_ADDR, INT_CFG, 0x00);

}


void getGyroscopeData()
{
  /**************************************
  Gyro ITG-3200 I2C
  registers:
  temp MSB = 1B, temp LSB = 1C
  x axis MSB = 1D, x axis LSB = 1E
  y axis MSB = 1F, y axis LSB = 20
  z axis MSB = 21, z axis LSB = 22
  *************************************/

  int regAddress = 0x1D;
  int temp, x, y, z;
  byte buff[TO_READ];
  char str[50]; // 50 should be enough to store all the data from the gyro
  
  readFrom(GYRO_ADDR, regAddress, TO_READ, buff); //read the gyro data from the ITG3200
  
  //temp = (buff[0] << 8) | buff[1];
  x = ((buff[0] << 8) | buff[1]) - offx;
  y = ((buff[2] << 8) | buff[3]) - offy;
  z = ((buff[4] << 8) | buff[5]) - offz;

  //we send the x y z values as a string to the serial port
  sprintf(str, "%d,%d,%d,", x, y, z);  
  Serial.write(str);
  //Serial.write(10);
}


void getAccData()
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
  sprintf(str, "%d,%d,%d,", x, y, z);
  Serial.write(str);
  Serial.write(10);
 }

/*******************************************************************************
The purpose of the calibration routine is to obtain the value of the reference threshold.
It consists on a 1024 samples average in no-movement condition.
********************************************************************************/
void calibrate_Acc(void){
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
