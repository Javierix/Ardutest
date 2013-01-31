/*
 Leemos la temperatura de un LM35 y la enviamos mediante Bluetooth al PC.
 Utilizo SecureCRT como programa de comunicaciones.
 */
 
float tempC;
int tempPin = 0;

void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

void loop() {
  tempC = analogRead(tempPin);           //read the value from the sensor
  tempC = (5.0 * tempC * 100.0)/1024.0;  //convert the analog data to temperature
  Serial.println((byte)tempC);           //send the data to the computer
  delay(3000);   
}