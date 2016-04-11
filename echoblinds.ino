/*
  Control servo motors through Amazon Echo
  Based on the source code from: http://www.seeedstudio.com/wiki/WiFi_Serial_Transceiver_Module
*/

#include <ArduinoJson.h>
#include "config.h"
#include <Servo.h>

Servo servo1;
Servo servo2;
int fractionOpen = -1;

void setup()
{
  servo1.attach(9);
  servo2.attach(8);
  Serial.begin(9600);
  Serial2.begin(115200);
  Serial2.setTimeout(5000);
  Serial.println("Init");
  delay(1000);
  if(Serial2.find("ready")) {
    Serial.println("WiFi - Module is ready");
  }else{
    Serial.println("Module dosn't respond.");
  }
  delay(1000);
  // try to connect to wifi
  boolean connected=false;
  for(int i=0;i<5;i++){
    if(connectWiFi()){
      connected = true;
      Serial.println("Connected to WiFi...");
      break;
    }
  }
  if (!connected){
    Serial.println("Coudn't connect to WiFi.");
    while(1);
  }
  delay(5000);
  Serial2.println("AT+CIPMUX=0"); // set to single connection mode
}

void loop()
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += DST_IP;
  cmd += "\",";
  cmd += PORT;
  Serial2.println(cmd);
  if(Serial2.find("Error")) return;
  cmd = "GET /api/get?apiKey=";
  cmd += API_KEY;
  cmd += "&clientId=";
  cmd += CLIENT_ID;
  cmd += " HTTP/1.0\r\n\r\n";
  Serial2.print("AT+CIPSEND=");
  Serial2.println(cmd.length());
  if(Serial2.find(">")){
    Serial.print(">");
  }else{
    Serial2.println("AT+CIPCLOSE");
    Serial.println("connection timeout");
    delay(1000);
    return;
  }
  Serial2.println(cmd);
  unsigned int i = 0; //timeout counter
  int n = 1; // char counter
  char json[100]="{";
  while (!Serial2.find("{")){
    if (Serial2.read()==-1){break;}
  }
  while (i<6000) {
    if(Serial2.available()) {
      char c = Serial2.read();
      json[n]=c;
      n++;
      i=0;
    }
    i++;
  }
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  boolean success_ok = root["success"];
    
  if(success_ok) {
    JsonObject& message = root["message"];
    boolean op = message["open"];
    int microseconds;
    if (op) {
      microseconds = 1700;
    } else {
      microseconds = 1300;
    }
    if (fractionOpen < 0 || (op && fractionOpen == 0) || (!op && fractionOpen == 1)) {
      servo1.writeMicroseconds(microseconds);  // Counter clockwise
      servo2.writeMicroseconds(microseconds);  // Counter clockwise
      delay(8000);
      servo1.writeMicroseconds(1500);  // Stop
      servo2.writeMicroseconds(1500);  // Stop
      if (op) {
        fractionOpen = 1;
      } else {
        fractionOpen = 0;
      }
    }
  }
}

boolean connectWiFi()
{
  Serial2.println("AT+CWMODE=1");
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);
  Serial2.println(cmd);
  delay(2000);
  if(Serial2.find("OK")){
    Serial.println("OK, Connected to WiFi.");
    return true;
  }else{
    Serial.println("Can not connect to the WiFi.");
    return false;
  }
  if (!Serial2.available()){
    delay(5000);
    software_Reset();
    }
}

void software_Reset() {// Restarts program from beginning but does not reset the peripherals and registers
  asm volatile ("  jmp 0");  
}  
