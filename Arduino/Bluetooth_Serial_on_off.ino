/*
 on_off
 Utilizando el puerto serie enciende y apaga el LED de la placa.
 Tambien lo he utilizado con el puerto serie del modem bluetooth y funciona correctamente.
 Logicamente los puertos serie son distintos.
 Utilizo SecureCRT como programa de comunicaciones.
 */

void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    switch (inByte) {
    case 'a':    
      digitalWrite(13, HIGH);
      break;
    case 'b':    
      digitalWrite(13, LOW);
      break;
     }
   }
}

