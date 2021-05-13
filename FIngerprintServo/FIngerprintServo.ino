#include <Adafruit_Fingerprint.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

volatile int finger_status = -1;

SoftwareSerial mySerial(D2, D3); // TX/RX on fingerprint sensor

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()  
{
  Serial.begin(9600);

  myservo.attach(D5);
  myservo.write(0);
  
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
}

void loop()                     // run over and over again
{
  finger_status = getFingerprintIDez();
  if (finger_status!=-1 and finger_status!=-2){
    Serial.println("Hello Han Sheng");
    myservo.write(90);
  } else{
    if (finger_status==-2){
      for (int ii=0;ii<5;ii++){
        Serial.println("Invalid Fingerprint");
        delay(100);
      }
    }
  }
  delay(50);            //don't ned to run this at full speed.
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p!=2){
    Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;
  
  p = finger.image2Tz();
  if (p!=2){
    Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -2;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}
