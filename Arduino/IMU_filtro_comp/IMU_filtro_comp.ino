#include <Wire.h> // I2C library, gyroscope

// Accelerometer ADXL345
#define ACC (0x53)    //ADXL345 ACC address
#define A_TO_READ (6)        //num of bytes we are going to read each time (two bytes for each axis)


// Gyroscope ITG3200 
#define GYRO 0x69 // gyro address, binary = 11101001 when AD0 is connected to Vcc (see schematics of your breakout board)
#define G_SMPLRT_DIV 0x15
#define G_DLPF_FS 0x16
#define G_INT_CFG 0x17
#define G_PWR_MGM 0x3E

#define G_TO_READ 8 // 2 bytes for each axis x, y, z

// offsets are chip specific. 
int g_offx = 120;
int g_offy = 20;
int g_offz = 93;

char str[512]; 


boolean firstSample = true;

float RwAcc[3];     //projection of normalized gravitation force vector on x/y/z axis, as measured by accelerometer
float Gyro_ds[3];   //Gyro readings         
float RwGyro[3];    //Rw obtained from last estimated value and gyro movement
float Awz[2];       //angles between projection of R on XZ/YZ plane and Z axis (deg)
float RwEst[3];

int lastTime = 0;
int interval = 0;
float wGyro = 10.0;

void initAcc() {
  //Turning on the ADXL345
  writeTo(ACC, 0x2D, 0);      
  writeTo(ACC, 0x2D, 16);
  writeTo(ACC, 0x2D, 8);
  //by default the device is in +-2g range reading
}

void getAccelerometerData(int * result) {
  int regAddress = 0x32;    //first axis-acceleration-data register on the ADXL345
  byte buff[A_TO_READ];

  readFrom(ACC, regAddress, A_TO_READ, buff); //read the acceleration data from the ADXL345

  //each axis reading comes in 10 bit resolution, ie 2 bytes.  Least Significat Byte first!!
  //thus we are converting both bytes in to one int
  result[0] = (((int)buff[1]) << 8) | buff[0];   
  result[1] = (((int)buff[3]) << 8) | buff[2];
  result[2] = (((int)buff[5]) << 8) | buff[4];
}

void rawAccToG(int * raw, float * RwAcc) {
  RwAcc[0] = ((float) raw[0]) / 256.0;
  RwAcc[1] = ((float) raw[1]) / 256.0;
  RwAcc[2] = ((float) raw[2]) / 256.0;
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
  writeTo(GYRO, G_PWR_MGM, 0x00);
  writeTo(GYRO, G_SMPLRT_DIV, 0x07); // EB, 50, 80, 7F, DE, 23, 20, FF
  writeTo(GYRO, G_DLPF_FS, 0x1E); // +/- 2000 dgrs/sec, 1KHz, 1E, 19
  writeTo(GYRO, G_INT_CFG, 0x00);
}


void getGyroscopeData(int *result)
{
  /**************************************
  Gyro ITG-3200 I2C
  registers:
  temp MSB = 1B, temp LSB = 1C
  x axis MSB = 1D, x axis LSB = 1E
  y axis MSB = 1F, y axis LSB = 20
  z axis MSB = 21, z axis LSB = 22
  *************************************/

  int regAddress = 0x1B;
  int temp, x, y, z;
  byte buff[G_TO_READ];

  readFrom(GYRO, regAddress, G_TO_READ, buff); //read the gyro data from the ITG3200

  result[0] = ((buff[2] << 8) | buff[3]) + g_offx;
  result[1] = ((buff[4] << 8) | buff[5]) + g_offy;
  result[2] = ((buff[6] << 8) | buff[7]) + g_offz;
  result[3] =  (buff[0] << 8) | buff[1]; // temperature 
}


// convert raw readings to degrees/sec
void rawGyroToDegsec(int *raw, float *gyro_ds) {
  gyro_ds[0] = ((float) raw[0]) / 14.375;
  gyro_ds[1] = ((float) raw[1]) / 14.375;
  gyro_ds[2] = ((float) raw[2]) / 14.375;
}


void normalize3DVec(float *vector) {
  float R;
  R = sqrt(vector[0]*vector[0] + vector[1]*vector[1] + vector[2]*vector[2]);
  vector[0] /= R;
  vector[1] /= R;  
  vector[2] /= R;  
}


float squared(float x){
  return x*x;
}


void getInclination() {
  int w = 0;
  float tmpf = 0.0;
  int currentTime, signRzGyro;


  currentTime = millis();
  interval = currentTime - lastTime;
  lastTime = currentTime;

  if (firstSample) { // the NaN check is used to wait for good data from the Arduino
    for(w=0;w<=2;w++) {
      RwEst[w] = RwAcc[w];    //initialize with accelerometer readings
    }
  }
  else{
    //evaluate RwGyro vector
    if(abs(RwEst[2]) < 0.1) {
      //Rz is too small and because it is used as reference for computing Axz, Ayz it's error fluctuations will amplify leading to bad results
      //in this case skip the gyro data and just use previous estimate
      for(w=0;w<=2;w++) {
        RwGyro[w] = RwEst[w];
      }
    }
    else {
      //get angles between projection of R on ZX/ZY plane and Z axis, based on last RwEst
      for(w=0;w<=1;w++){
        tmpf = Gyro_ds[w];                              //get current gyro rate in deg/s
        tmpf *= interval / 1000.0f;                     //tmpf = tmpf*(interval/1000.0f); get angle change in deg (tambien convierte millis a segundos)
        Awz[w] = atan2(RwEst[w],RwEst[2]) * 180/PI;     //get angle and convert to deg
        Awz[w] += tmpf;                                 //Awz[w]=Awz[w]+tmpf;  get updated angle according to gyro movement
      }

      //estimate sign of RzGyro by looking in what quadrant the angle Axz is, 
      //RzGyro is positive if  Axz in range -90 ..90 => cos(Awz) >= 0
      signRzGyro = ( cos(Awz[0] * PI / 180) >=0 ) ? 1 : -1;

      //reverse calculation of RwGyro from Awz angles, for formulas deductions see  http://starlino.com/imu_guide.html
      for(w=0;w<=1;w++){
        RwGyro[0] = sin(Awz[0] * PI / 180);
        RwGyro[0] /= sqrt( 1 + squared(cos(Awz[0] * PI / 180)) * squared(tan(Awz[1] * PI / 180)) );
        RwGyro[1] = sin(Awz[1] * PI / 180);
        RwGyro[1] /= sqrt( 1 + squared(cos(Awz[1] * PI / 180)) * squared(tan(Awz[0] * PI / 180)) );        
      }
      RwGyro[2] = signRzGyro * sqrt(1 - squared(RwGyro[0]) - squared(RwGyro[1]));
    }

    //combine Accelerometer and gyro readings
    for(w=0;w<=2;w++) RwEst[w] = (RwAcc[w] + wGyro * RwGyro[w]) / (1 + wGyro);

    normalize3DVec(RwEst);
  }

  firstSample = false;
}


void setup()
{
  Serial.begin(19200);
  Wire.begin();
  initAcc();
  initGyro();
}


void loop()
{
  if(!Serial.available()) {
    int acc[3];
    int gyro[4];

    getAccelerometerData(acc);
    rawAccToG(acc, RwAcc);
    normalize3DVec(RwAcc);

    getGyroscopeData(gyro);
    rawGyroToDegsec(gyro, Gyro_ds);

    getInclination();
	
    Serial.print(interval);  //microseconds since last sample, please note that printing more data will increase interval    
    Serial.print(",  ");    
    Serial.print(RwAcc[0]);  //Inclination X axis (as measured by accelerometer)
    Serial.print(",  ");
    Serial.print(RwGyro[0]);  //Inclination X axis (calculado a partir del giroscopio)
    Serial.print(",  ");
    Serial.print(RwEst[0]);  //Inclination X axis (estimated / filtered)
    Serial.println("");


    //sprintf(str, "%d,%d,%d,%d,%d,%d,", acc[0], acc[1], acc[2], gyro[0], gyro[1], gyro[2]);  
    //Serial.print(str);
    //Serial.print(10, BYTE);
	/*
    serialPrintFloatArr(RwAcc, 3);    //Valor del acelerometro en g
    serialPrintFloatArr(Gyro_ds, 3);  //valor del giro en grados/seg
    serialPrintFloatArr(RwGyro, 3);
    serialPrintFloatArr(Awz, 2);
    serialPrintFloatArr(RwEst, 3);    // Rw estimada por la combinacion de RwAcc y RwGyro
    Serial.println();
	*/
    //Serial.print(10, BYTE);
    //delay(50);
  }
}

void serialPrintFloatArr(float *arr, int length) {
  for(int i=0; i<length; i++) {
    serialFloatPrint(arr[i]);
    Serial.print(",");
  }
}


void serialFloatPrint(float f) {
  byte *b = (byte *) &f;
  Serial.print("f:");
  for(int i=0; i<4; i++) {

    byte b1 = (b[i] >> 4) & 0x0f;
    byte b2 = (b[i] & 0x0f);

    char c1 = (b1 < 10) ? ('0' + b1) : 'A' + b1 - 10;
    char c2 = (b2 < 10) ? ('0' + b2) : 'A' + b2 - 10;

    Serial.print(c1);
    Serial.print(c2);
  }
}


//---------------- Functions
//Writes val to address register on ACC
void writeTo(int DEVICE, byte address, byte val) {
   Wire.beginTransmission(DEVICE);  //start transmission to ACC 
   Wire.write(address);             //send register address
   Wire.write(val);                 //send value to write
   Wire.endTransmission();          //end transmission
}


//reads num bytes starting from address register on ACC in to buff array
void readFrom(int DEVICE, byte address, int num, byte buff[]) {
  Wire.beginTransmission(DEVICE);   //start transmission to ACC 
  Wire.write(address);              //sends address to read from
  Wire.endTransmission();           //end transmission

  Wire.beginTransmission(DEVICE);   //start transmission to ACC
  Wire.requestFrom(DEVICE, num);    //request 6 bytes from ACC

  int i = 0;
  while(Wire.available())           //ACC may send less than requested (abnormal)
  { 
    buff[i] = Wire.read();          //receive a byte
    i++;
  }
  Wire.endTransmission();           //end transmission
}
