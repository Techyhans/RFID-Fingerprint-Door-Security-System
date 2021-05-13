#include <Adafruit_Fingerprint.h>
#include <Servo.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

const char *ssid =  "XxX";     // replace with your wifi ssid and wpa2 key
const char *pass =  "hansheng0512";

bool FP = false;

char auth[] = "oAPv8tW5-G0j99LmXr4zeN2Z8a-0CM1w";
WidgetLCD vlcd(V1);

Servo myservo;  // create servo object to control a servo

volatile int finger_status = -1;

SoftwareSerial mySerial(D2, D3); // TX/RX on fingerprint sensor

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()  
{
  Serial.begin(9600);

  Serial.println("Connecting to ");
  Serial.println(ssid); 
 
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected"); 

  Blynk.begin(auth, ssid, pass);
  pinMode(D0,OUTPUT);
  vlcd.clear();

  myservo.attach(D4);
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
  Blynk.run();
  finger_status = getFingerprintIDez();
  if ((finger_status!=-1 and finger_status!=-2) or (digitalRead(D0) == HIGH)){
    FP = true;
    digitalWrite(D0, HIGH);
    myservo.write(90);

  } else{
    if (finger_status==-2){
      for (int ii=0;ii<5;ii++){
        FP = false;
        digitalWrite(D0, LOW);
        delay(100);
      }
    }
  }
  delay(50);            //don't ned to run this at full speed.

  vlcd.clear();
  if (FP == true){
    vlcd.print(0,0,"Valid FP");
  }
  else{
    vlcd.print(0,0,"Invalid FP");
  }
  if (digitalRead(D0) == LOW){
    vlcd.print(0,1,"Door Close Now");
    myservo.write(0);
  }
  else{
    vlcd.print(0,1,"Door Open Now");
  }
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
