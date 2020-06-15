#include <LiquidCrystal_I2C.h>      //LCD part (1)
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //LCD part (1)

#define SS_PIN D4                   //RFID part(1)  
#define RST_PIN 0
#include <SPI.h>
#include <MFRC522.h>
MFRC522 mfrc522(SS_PIN, RST_PIN);
int statuss = 0;
int out = 0;                        //RFID part(1)
int statuss_02 = 0;

#include <Adafruit_Sensor.h> // sensor part (1)
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN D3             // declare pin for sensor
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);     //sensor part(1)

//
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
//#include <SPI.h>
#include <ESP8266WiFi.h>      
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
char auth[] = "b19f2ba99d5a4d2aa6f7fe1356bec8a7";
char ssid[] = "LA"; 
char pass[] = "worldhello123";

SimpleTimer timer;

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V5, h);  //V5 is for Humidity
  Blynk.virtualWrite(V6, t);  //V6 is for Temperature
}
//

unsigned long previousMillis_01 = 0; // has not been use (1)
int interval_01 = 5000;             // has not been use (2)

int counter_i001 = 0;

//Input & Button Logic
const int numOfInputs = 2;
const int inputPins[numOfInputs] = {15, 10};
int inputState[numOfInputs];
int lastInputState[numOfInputs] = {LOW, LOW};
int inputFlags[numOfInputs] = {LOW, LOW};
long lastDebounceTime[numOfInputs] = {0, 0};
long debounceDelay = 5;

//LCD Menu Logic
const int numOfScreens = 5;//2;
int currentScreen = 4;//1;  // we have 0 and 1 screens
String screens[numOfScreens][2]= {{"=> 1.Temp+Hum","   2.Send SMS"}, {"   1.Temp+Hum","=> 2.Send SMS"},
{"=> 3.Add Key","   4.Delete Key"}, {"   3.Add Key","=> 4.Delete Key"}, {"=> 5.Exit",""}}; 

//Belongs to Function ADD KEY
byte save0[9]; byte save1[9]; byte save2[9]; byte save3[9]; byte save4[9]; 
byte save5[9]; byte save6[9]; byte save7[9]; byte save8[9]; byte save9[9]; 
byte save10[9];

bool iskeyexist(byte a[]){
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    if (save0[i] != a[i]) {return false;}
  }
  return true;
}
//Belongs to Function ADD KEY

//Belongs to sim800L
#include <SoftwareSerial.h>
//char saying[] = "Hello World!";
SoftwareSerial sim(D1,D2);
//Belongs to sim800L

void setup() {
  dht.begin();
    
  for (int i = 0; i < numOfInputs; i++){  //for the button and menu;
    pinMode(inputPins[i], INPUT);
    digitalWrite(inputPins[i], HIGH);
  }  

  Wire.begin(1,3);
  lcd.begin(16,2);                  //LCD part (2) 
  lcd.init();
  lcd.backlight();
  lcd.clear();                      //LCD part (2)
//  lcd.setCursor(0,0);
//  lcd.print("Verify Yourself");     //LCD 
  
  SPI.begin();                      //RFID part(2)
  mfrc522.PCD_Init();               //RFID part(2)
  statuss = 0;
  statuss_02 = 0;

//
// Blynk.begin(auth, ssid, pass);
// timer.setInterval(1000L, sendSensor);
//

}

void loop() {
  while (statuss == 0){
  lcd.setCursor(0,0);
  lcd.print("Verify Yourself");
  delay(450);
  if ( ! mfrc522.PICC_IsNewCardPresent())   //RFID part(3) 
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    save0[i] = mfrc522.uid.uidByte[i];
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX)); 
  }
  content.toUpperCase();
  if (content.substring(1) == "7A B9 62 A3" || iskeyexist(save1) == true||
                                               iskeyexist(save2) == true||
                                               iskeyexist(save3) == true||
                                               iskeyexist(save4) == true||
                                               iskeyexist(save5) == true||
                                               iskeyexist(save6) == true||
                                               iskeyexist(save7) == true||
                                               iskeyexist(save8) == true||
                                               iskeyexist(save9) == true||
                                               iskeyexist(save10) == true)
  {
    lcd.clear();                     
    lcd.setCursor(0,0);
    lcd.print("Access Granted");
    delay(2000);
    statuss = 1;
  }
  else
  {
    lcd.clear();           
    lcd.setCursor(0,0);
    lcd.print("Access Denied");
    delay(3000);
    return;
  }
//  if (statuss == 0)
//  {
//    lcd.clear();                      
//    lcd.setCursor(0,0);
//    lcd.print("Verify Yourself");     
//  }
  }

  while (statuss_02 == 0){
  lcd.clear();           
  lcd.setCursor(5,0);
  lcd.print("Welcome");
  delay(2000);
  statuss_02 = 1;
//  lcd.clear();
  }
  
  setInputFlags();
  resolveInputFlags();
}

void setInputFlags(){
  for (int i = 0; i < numOfInputs; i++){
    int reading = digitalRead(inputPins[i]);
    if (reading != lastInputState[i]){
      lastDebounceTime[i] = millis();
    }
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != inputState[i]) {
        inputState[i]= reading;
        if (inputState[i] == HIGH) {
          inputFlags[i] = HIGH;
        }
      }
    }
    lastInputState[i] = reading;
  }
}

void resolveInputFlags(){
  for (int i = 0; i < numOfInputs; i++) {
    if (inputFlags[i] == HIGH) {
      inputAction(i);
      inputFlags[i] = LOW;
      printScreen();
    }
  }
}

void inputAction(int input) {
  switch (input){
    case 0:
      if (currentScreen == numOfScreens -1) {
        currentScreen = 0; 
      }else{
        currentScreen++;
      }
      break;

    case 1:
      if (currentScreen == 0){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Initializing..");
        delay(500);
        Blynk.begin(auth, ssid, pass);
        timer.setInterval(1000L, sendSensor);
        delay(2000);      
        counter_i001 = 0;
        while (counter_i001 < 2)
        {
          send_it_online();
          float h = dht.readHumidity();
          float t = dht.readTemperature();
          if (isnan(h) || isnan(t) ) {
            lcd.clear();                                  //print to LCD
            lcd.setCursor(0,0);
            lcd.print("DHT sensor:");
            lcd.setCursor(0,1);
            lcd.print("Broken!"); //print to LCD
            delay(3000);
            break;
          }
          tempandhum();
          counter_i001 = counter_i001 + 1;
        }
        delay(500);
        WiFi.disconnect(true);
        delay(1500);
        break;
      }
      if (currentScreen == 1){
        lcd.clear();                     
        lcd.setCursor(0,0);
        lcd.print("Sending SMS...");
        delay(1000);
        /*sim.begin(9600);
        sim.print("AT+CMGF=1\r\n");
        delay(450);
        //sim.print("AT+CMGS=\"0932790158\"\r\n");
        sim.print("AT+CMGS=\"0773018287\"\r\n");
        delay(450);
        //String s = String(saying);
        //sim.print(s+"\r\n");
        sim.print("Hello World!");
        delay(450);
        sim.write(26);
        delay(450);*/
        sms_800L();
        lcd.clear();                     
        lcd.setCursor(0,0);
        lcd.print("Success!");
        delay(1500);
        break;
      }

      if (currentScreen == 2){
        //lcd.clear();                     
        //lcd.setCursor(0,0);
        //lcd.print("Function Add");
        //delay(3000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Insert card...");
        delay(3500);
        if ( ! mfrc522.PICC_IsNewCardPresent()) 
        {
          break;
        }
  
        if ( ! mfrc522.PICC_ReadCardSerial()) 
        {
          break;
        } 
        String content= "";
        byte letter;
        for (byte i = 0; i < mfrc522.uid.size; i++)
        {
          save0[i] = mfrc522.uid.uidByte[i];
          content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
          content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        content.toUpperCase();
        if (content.substring(1) == "7A B9 62 A3")
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Can't add:");
          lcd.setCursor(0,1); 
          lcd.print("CORE Key");
          delay(3000);
          break;
        }

        if (iskeyexist(save1)== true||
            iskeyexist(save2)== true||
            iskeyexist(save3)== true||
            iskeyexist(save4)== true||
            iskeyexist(save5)== true||
            iskeyexist(save6)== true||
            iskeyexist(save7)== true||
            iskeyexist(save8)== true||
            iskeyexist(save9)== true||
            iskeyexist(save10)== true)
            {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Key existed!");
              delay(3000);
              break;
            }
        if (save1[0] == NULL)
        {
          memcpy(save1,save0,sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
        }

        if (save2[0] == NULL)
        {
          memcpy(save2,save0,sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
        }

        if (save3[0] == NULL)
        {
          memcpy(save3,save0,sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
        }

        if (save4[0] == NULL)
        {
          memcpy(save4,save0,sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
        }

        if (save5[0] == NULL)
        {
          memcpy(save5,save0, sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
        }

        if (save6[0] == NULL)
        {
          memcpy(save6,save0, sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
         }  

        if (save7[0] == NULL)
        {
          memcpy(save7,save0, sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
        }

        if (save8[0] == NULL)
        {
          memcpy(save8,save0, sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
        }

        if (save9[0] == NULL)
        {
          memcpy(save9,save0, sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
        }

        if (save10[0] == NULL)
        {
          memcpy(save10,save0, sizeof(save0));
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Add Success!");
          delay(2000);
          break;
        }
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Memory Full!");
        delay(2000);
        break;          
//Belongs to function ADD KEY
      }

      if (currentScreen == 3){
        //lcd.clear();                     
        //lcd.setCursor(0,0);
        //lcd.print("Function Delete");
        //delay(3000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Insert card...");
        delay(3500);
        if ( ! mfrc522.PICC_IsNewCardPresent()) 
        {
          break;
        }
  
        if ( ! mfrc522.PICC_ReadCardSerial()) 
        {
          break;
        } 
        String content= "";
        byte letter;
        for (byte i = 0; i < mfrc522.uid.size; i++)
        {
          save0[i] = mfrc522.uid.uidByte[i];
          content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
          content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        content.toUpperCase();
        if (content.substring(1) == "7A B9 62 A3")
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Can't delete:");
          lcd.setCursor(0,1); 
          lcd.print("CORE Key");
          delay(3000);
          break;
        }

        if (iskeyexist(save1)== false&&
            iskeyexist(save2)== false&&
            iskeyexist(save3)== false&&
            iskeyexist(save4)== false&&
            iskeyexist(save5)== false&&
            iskeyexist(save6)== false&&
            iskeyexist(save7)== false&&
            iskeyexist(save8)== false&&
            iskeyexist(save9)== false&&
            iskeyexist(save10)== false)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("ID doesn't exist");
          delay(2500);
          break;
        }

        if (iskeyexist(save1) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save1[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        if (iskeyexist(save2) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save2[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        if (iskeyexist(save3) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save3[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        if (iskeyexist(save4) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save4[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        if (iskeyexist(save5) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save5[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        if (iskeyexist(save6) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save6[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        if (iskeyexist(save7) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save7[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        if (iskeyexist(save8) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save8[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        if (iskeyexist(save9) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save9[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        if (iskeyexist(save10) == true){
          for (byte i = 0; i < mfrc522.uid.size; i++){
            save10[i] = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Delete Success!");
          delay(2500);
          break;
        }

        break;        
      }

      if (currentScreen == 4){
        statuss = 0;
        statuss_02 = 0;
        return;
      }
      break;    
  }
}

void printScreen(){
  lcd.clear();
  lcd.print(screens[currentScreen][0]);
  lcd.setCursor(0,1);
  lcd.print(screens[currentScreen][1]);
}

void tempandhum(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  //if (isnan(h) || isnan(t) ) {
  //  lcd.clear();                                  //print to LCD
  //  lcd.setCursor(0,0);
  //  lcd.print("DHT sensor");
  //  lcd.setCursor(0,1);
  //  lcd.print("Can't read!"); //print to LCD
  //  delay(3000);
  //  return;
  //}
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temperature: ");
  lcd.setCursor(0,1);
  lcd.print(t);
  lcd.print(" *C");
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Humidity: ");
  lcd.setCursor(0,1);
  lcd.print(h);
  lcd.print(" %");

  delay(2000);
}

void send_it_online(){
  Blynk.run(); 
  timer.run(); 
}

void sms_800L(){
  //sim.begin(9600);
  sim.print("AT+CMGF=1\r\n");
  delay(500);
  sim.print("AT+CMGS=\"0932790158\"\r\n");
  //sim.print("AT+CMGS=\"0773018287\"\r\n");
  delay(500);
  //String s = String(saying);
  //sim.print(s+"\r\n");
  sim.print("Hello World!");
  delay(500);
  //sim.write(26);
  sim.println((char)26);
  delay(90);
}
