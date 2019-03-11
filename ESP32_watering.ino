#include <LiquidCrystal_I2C.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>



// เรียกใช้ไลบรารี่ WiFi.h และ IOXhop_FirebaseESP32.h มาใช้งาน
#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <time.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

// Set these to run example.
#define FIREBASE_HOST "esp32-firebase-2048a.firebaseio.com"
#define FIREBASE_AUTH "KtNAcvnrpSkhBbf8acrZiigYsHzhJ23xVBZ20Ai0"
#define WIFI_SSID "KMITL-WiFi"
#define WIFI_PASSWORD ""
//Constants
#define DHTPIN 32       // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define soilpin  35
#define pumppin  4

const int AirValue = 4000;   //you need to replace this value with Value_1
const int WaterValue = 900;  //you need to replace this value with Value_2
int intervals = (AirValue - WaterValue)/3;   
int soilMoistureValue = 3;
String s_status;
// LED Debug
#define DEBUG_WIFICONNECT 14
#define DEBUG_PUTHDATA 5

 
// Config time
int timezone = 7;

char ntp_server1[20] = "time.navy.mi.th";
char ntp_server2[20] = "clock.nectec.or.th";
char ntp_server3[20] = "th.pool.ntp.org";

int dst = 0;

DHT dht(DHTPIN, DHTTYPE);

void setup() 
{
 
  pinMode(DEBUG_WIFICONNECT, OUTPUT);
  pinMode(DEBUG_PUTHDATA, OUTPUT);
  pinMode(pumppin, OUTPUT);
  Serial.begin(9600);
    lcd.begin();
  WiFi.mode(WIFI_STA);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(DEBUG_WIFICONNECT, !digitalRead(DEBUG_WIFICONNECT));
    delay(500);
  }
  digitalWrite(DEBUG_WIFICONNECT, HIGH);
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  Serial.println("Waiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Now: " + NowString());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  dht.begin();
}

void loop() {
  pinMode(DHTPIN, INPUT_PULLUP);

  // Read temp & Humidity for DHT22
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(500);
    return;
  }
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C ");
  Serial.println();
 soilMoistureValue = analogRead(soilpin);  //put Sensor insert into soil





 
  if(soilMoistureValue > (WaterValue-400) && soilMoistureValue < (WaterValue + intervals))
  {
    Serial.println("Very Wet"); 
    s_status = "Very Wet";
 lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T: ");
  lcd.print(temperature);
  lcd.print((char)223);
  lcd.print("C");
  
  lcd.setCursor(8,0);
  lcd.print("H: ");
  lcd.print(humidity);
  lcd.print("%RH");
  
  lcd.setCursor(0,1);
  lcd.print("HS= ");
  lcd.setCursor(4,1);
  lcd.print("Very Wet (>33%)" ); 
  lcd.setCursor(0,2);
  lcd.print("WATER SYSTEM OFF");
    digitalWrite(pumppin,LOW); //Motor stops//
    delay(60000);
  }
  
  if(soilMoistureValue > (WaterValue + intervals) && soilMoistureValue < (AirValue - intervals))
  {
   Serial.println("Wet"); 
   s_status = "Wet";
     lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T: ");
  lcd.print(temperature);
  lcd.print((char)223);
  lcd.print("C");
  
  lcd.setCursor(8,0);
  lcd.print("H: ");
  lcd.print(humidity);
  lcd.print("%RH");
  
  lcd.setCursor(0,1);
  lcd.print("HS= ");
  lcd.setCursor(4,1);
  lcd.print("Wet (>33%)" ); 
  lcd.setCursor(0,2);
  lcd.print("WATER SYSTEM OFF");
   digitalWrite(pumppin,LOW); //Motor stops//
   delay(60000);
  }
  
  if(soilMoistureValue < (AirValue+900) && soilMoistureValue > (AirValue - intervals))
  {
   Serial.println("Dry"); 
   s_status = "Dry";

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T: ");
  lcd.print(temperature);
  lcd.print((char)223);
  lcd.print("C");
  
  lcd.setCursor(8,0);
  lcd.print("H: ");
  lcd.print(humidity);
  lcd.print("%RH");
  
  lcd.setCursor(0,1);
  lcd.print("HS= ");
  lcd.setCursor(4,1);
  lcd.print("Dry(<17%)" );
  lcd.setCursor(0,2);
  lcd.print("WATER SYSTEM ON");
   digitalWrite(pumppin,HIGH); //Motor stops//
   delay(60000);
   
  }
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  {
  root["time"] = NowString();
  root["temperature"] = temperature;
  root["humidity"] = humidity;
  root["soil"] = s_status;
  
  }
   Firebase.pushString("soil", s_status);
  digitalWrite(DEBUG_PUTHDATA, HIGH);
  // append a new value to /logDHT
  String name = Firebase.push("KMITL SERVER", root);
  // handle error
  if (Firebase.failed()) {
      Serial.print("pushing /KMITL SERVER failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Serial.print("pushed: /KMITL SERVER/");
  Serial.println(name);
  delay(2000);
  digitalWrite(DEBUG_PUTHDATA, LOW);
  delay(28000);
}

String NowString() 
{
 int getcount = 1;
        time_t now = time(nullptr);
        struct tm* newtime = localtime(&now);
        String myyear = String(newtime->tm_year + 1900);
        //ถ้าปียังเป็นปี 1970 ให้ดึงค่าเวลาใหม่ พยายามสูงสุด 4 ครั้ง
        while(myyear == "2018" && getcount <= 4) 
        {
         time_t now = time(nullptr);
         struct tm* newtime = localtime(&now);
         myyear = String(newtime->tm_year + 1900);
         vTaskDelay(100 / portTICK_PERIOD_MS);
         getcount++;
         }
        String tmpNow = "";
        tmpNow += String(newtime->tm_year + 1948);
        tmpNow += "-";
        tmpNow += String(newtime->tm_mon + 11);
        tmpNow += "-";
        tmpNow += String(newtime->tm_mday + 26);
        tmpNow += " ";
        tmpNow += String(newtime->tm_hour);
        tmpNow += ":";
        tmpNow += String(newtime->tm_min);
        tmpNow += ":";
        tmpNow += String(newtime->tm_sec);
        return tmpNow;  
}
