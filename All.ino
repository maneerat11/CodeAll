#include <Scheduler.h>
#include <TridentTD_LineNotify.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

//DHT11
#define DHTPIN D6
#define DHTTYPE DHT11
#define SW D4

//FAN
#define FAN1 D3
#define FAN2 D7

//Connect WiFi
//SSID
#define SSID "NATTARIKA-2.4G"
//Password
#define PASSWORD "0968354188"

//LINE TOKEN
#define LINE_TOKEN "EoEsFlWHogGKL0gXo86NTSGHiOpUywVaDZx8H4SoRNH"

//Thingspeak API key
String apiKey = "S6SOCGTMTJQ0PM40";
const char* ssid = "Tingsdt";
const char* password = "123456789"; 
const char* server = "api.thingspeak.com";

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient client;

int analogPin = A0; //define analogpin
int analog_val = 0;

//For DHT11
class FirstTask : public Task {
protected:
    void setup() {

    }

    void loop() {
        String dht11_val = "";
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (isnan(h) || isnan(t)) {
            Serial.println("FirstTask_Failed to read from DHT sensor!");
        }

        if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com
            String postStr = apiKey;
            postStr +="&field1=";
            postStr += String(t);
            postStr +="&field2=";
            postStr += String(h);
            postStr += "\r\n\r\n";

            client.print("POST /update HTTP/1.1\n"); 
            client.print("Host: api.thingspeak.com\n"); 
            client.print("Connection: close\n"); 
            client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n"); 
            client.print("Content-Type: application/x-www-form-urlencoded\n"); 
            client.print("Content-Length: "); 
            client.print(postStr.length()); 
            client.print("\n\n"); 
            client.print(postStr);

            Serial.print("Temperature: ");
            Serial.print(t);
            Serial.print(" degrees Celcius Humidity: "); 
            Serial.print(h);
            Serial.println("% send to Thingspeak");    
        }
        client.stop();

        dht11_val = dht11_val + "อุณหภูมิ "+t+"°C"+ " ความชื้น "+h+"%";
        Serial.println(dht11_val);
        LINE.notify(dht11_val);

        delay(60000);
    }

private:
    uint8_t state;
} first_task;

//For RainDrop
class SecondTask : public Task {
protected:
    void setup() {
       pinMode(SW,INPUT);
       Serial.begin(9600); 
    }

    void loop() {
        analog_val = analogRead(analogPin);
        Serial.print("analog val = ");
        Serial.println(analog_val); 

        if (digitalRead(SW) == HIGH){
          while(digitalRead(SW) == HIGH) delay(10);
          LINE.notify("ตอนนี้มีน้ำหกหรืองูปัสสาวะนะจ๊ะ");
        }
        delay(10);
  }
    
private:
    uint8_t state;
} second_task;

//For LCD AND FAN
class ThirdTask : public Task {
protected:
    void setup() {
        
    }

    void loop() {
        String dht11_val = "";
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (isnan(h) || isnan(t)) {
            Serial.println("ThirdTask_Failed to read from DHT sensor!");
            return;
        }

        if (t > = 30 ) {
            digitalWrite(FAN1, HIGH);
            digitalWrite(FAN2, LOW);
        } else {
            digitalWrite(FAN1, LOW);
            digitalWrite(FAN2, HIGH);
        }

        if (t < = 27 ) {
            digitalWrite(FAN1, LOW);
            digitalWrite(FAN2, HIGH);
        } else {
            digitalWrite(FAN1, HIGH);
            digitalWrite(FAN2, LOW);
        }

        lcd.setCursor(0, 0);
        lcd.print("Temp:     ");
        lcd.setCursor(4, 0);
        lcd.print(t);
        lcd.setCursor(9, 0);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Hum:     ");
        lcd.setCursor(4, 1);
        lcd.print(h);
        lcd.setCursor(9, 1);
        lcd.print("%");

        delay(2000);
    }
    
private:
    uint8_t state;
} third_task;

void setup() {
    Serial.begin(9600);

    dht.begin();
    lcd.begin();

    pinMode(FAN1,OUTPUT);
    pinMode(FAN2,OUTPUT);
    digitalWrite(FAN1, LOW);
    digitalWrite(FAN2, LOW);

    Serial.println();
    Serial.println(LINE.getVersion());
    WiFi.begin(SSID, PASSWORD);
    Serial.printf("WiFi connecting to %s\n", SSID);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(2000);
    }

    Serial.printf("\nWiFi connected\nIP : ");
    Serial.println(WiFi.localIP());
    LINE.setToken(LINE_TOKEN);
    LINE.notify("เซนเซอร์วัดอุณหภูมิ ความชื้นและเซนเซอร์ตรวจจับน้ำหกหรืองูปัสสาวะ เริ่มทำงานแล้ว");

    Scheduler.start(&first_task);
    Scheduler.start(&second_task);
    Scheduler.start(&third_task);
    Scheduler.begin();
}

void loop() {

}
