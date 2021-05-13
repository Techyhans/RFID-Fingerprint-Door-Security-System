#include <Adafruit_Fingerprint.h>
#include <Servo.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         D0          // Configurable, see typical pin layout above
#define SS_PIN          D8 

const char *ssid =  "XxX";     // replace with your wifi ssid and wpa2 key
const char *pass =  "hansheng0512";

char auth[] = "oAPv8tW5-G0j99LmXr4zeN2Z8a-0CM1w";
WidgetLCD vlcd(V1);
WidgetLED led8(V0);

MFRC522 mfrc522(SS_PIN, RST_PIN);  

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
  pinMode(D1,OUTPUT);
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

   SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();

  digitalWrite(D1, LOW);
  led8.off();
}

BLYNK_WRITE(3) // At global scope (not inside of the function)
{
    int i=param.asInt();
    if (i==1) 
    {
        digitalWrite(D1, HIGH);
    }
    else 
    {
        digitalWrite(D1, LOW);
    }
}

void loop()                     // run over and over again
{  
  Blynk.run();
  vlcd.clear();
  
  String content= "x";
  
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (( ! mfrc522.PICC_IsNewCardPresent()) or (! mfrc522.PICC_ReadCardSerial())) {
    finger_status = getFingerprintIDez();
    Serial.println("Jack");
    Serial.println(digitalRead(D1));
    
    if ((finger_status == 99) or (digitalRead(D1) == HIGH)){
      led8.off();
      vlcd.print(0,0,"Valid Access");
      digitalWrite(D1, HIGH);
      myservo.write(90);
      vlcd.print(0,1,"Door is Open");
      //delay(100);
    } else{
      Serial.println("Else 1");
      led8.on();
      vlcd.print(0,0,"Invalid Access");
      digitalWrite(D1, LOW);
      myservo.write(0);
      vlcd.print(0,1,"Door is Close");
      //delay(100);
    }
    delay(50);
    return;
  }
  
  //Show UID on serial monitor
  Serial.print("UID tag :");
  content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  if (content.substring(1) == "A2 1F 52 C5") //change here the UID of the card/cards that you want to give access
  {
    led8.off();
    vlcd.print(0,0,"Valid Access");
      digitalWrite(D1, HIGH);
      myservo.write(90);
      vlcd.print(0,1,"Door is Open");
    //delay(100);
  }
  else  
  {
    Serial.println("Else 2");
    led8.on();
    Blynk.notify("Invalid RFID Detected!!");
    vlcd.print(0,0,"Invalid Access");
      digitalWrite(D1, LOW);
      myservo.write(0);
      vlcd.print(0,1,"Door is Close");
    //delay(100);
    return;
  }
  
//  finger_status = getFingerprintIDez();
//  Serial.println(finger_status);
//  if ((finger_status!=-1 and finger_status!=-2) or (digitalRead(D0) == HIGH)){
//    FP = true;
//    digitalWrite(D0, HIGH);
//    myservo.write(90);
//  } else{
//    FP = false;
//    digitalWrite(D0, LOW);
//    delay(100);
//  }
//  delay(50);            //don't ned to run this at full speed.
//
//  vlcd.clear();
//  if (FP == true){
//    vlcd.print(0,0,"Valid Access");
//  }
//  else{
//    vlcd.print(0,0,"Invalid Access");
//  }
//  if (digitalRead(D0) == LOW){
//    vlcd.print(0,1,"Door Close Now");
//    myservo.write(0);
//  }
//  else{
//    vlcd.print(0,1,"Door Open Now");
//    myservo.write(90);
//  }
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p!=2){
    //Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;
  
  p = finger.image2Tz();
  if (p!=2){
    //Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -2;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return 99; 
}
