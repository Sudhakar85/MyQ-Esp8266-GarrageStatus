#define ARDUINOJSON_ENABLE_PROGMEM 0
#define LED_PIN D3

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// WIfi Settings
const char* ssid = "XXXX";
const char* password = "XXXX";
char* homeIpAddress;

unsigned long currentMillis = millis(), previousMillis = millis();
unsigned long timerDelay = 300000;

const uint8_t fingerprint[40] = {0x4f, 0x32, 0xcb, 0x4e, 0xbc, 0xdc, 0x7f, 0x19, 0xfd, 0x7e, 0x1f, 0xaf, 0x64, 0x01, 0x30, 0x5f, 0xe0, 0x9b, 0x21, 0xc6};
const char* host = "api.myqdevice.com";


void setup() {

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  WiFi.disconnect();
  delay(10);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Server Started");
  Serial.println(WiFi.localIP());
  
  delay(3000);
  digitalWrite(LED_PIN, HIGH);
  CheckDoorStatus();
  delay(2000);
}

void loop() {

  currentMillis = millis();
  if (WiFi.status() == WL_CONNECTED)
  {
    if ( (currentMillis - previousMillis) > timerDelay ) {
      previousMillis = currentMillis;
      CheckDoorStatus();
    }
  }
  else
  {
    error();
    delay(2000);
  }
}

void CheckDoorStatus()
{
  WiFiClientSecure client;
  client.setFingerprint(fingerprint);
  
  String requestData = "{\"Username\": \"XXXXX\",\"Password\": \"XXXXX\"}";  
  
  String loginStr = ExecuteMyQApi(client,"POST","/api/v5/login","",requestData, 500);

  DynamicJsonDocument doc(300);
  deserializeJson(doc, loginStr);
  JsonObject loginRes = doc.as<JsonObject>();        
  String token = loginRes[String("SecurityToken")];
  
  Serial.println("LoginToken:" + token);  
  
  if(loginRes && token != "null" )
  {
     String deviceStr = ExecuteMyQApi(client,"GET","/api/v5.1/Accounts/<DeviceId>/Devices",token,"", 1024);  

     DynamicJsonDocument doc1(1024);
     deserializeJson(doc1, deviceStr);
     JsonObject deviceStatus = doc1.as<JsonObject>();     
  
    if(deviceStatus)
    {
      JsonObject item = deviceStatus["items"][0];
      JsonObject state = item["state"];
      String doorStatus = state["door_state"];
      
      if(doorStatus == "closed")
      {
        Serial.println("Door Status : " + doorStatus);
        blinkLed(false);
      }
      else
      {
        Serial.println("Door Status : " + doorStatus);
        blinkLed(true);
      }
    }
    else
    {
     error();
    }
  }
  else
  {
    error();
  }
  client.stop();  
}

String ExecuteMyQApi(WiFiClientSecure client, char * method, String route, String token, String payload, int bufferSize)
{  
  HTTPClient http;
  http.begin(client,host,443, route, true);
  http.addHeader("MyQApplicationId", "JVM/G9Nwih5BwKgNCjLxiFUQxQijAebyyg8QUHr7JOrP+tuPb8iHfRHKwTmDzHOu");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("ApiVersion", "5.1");
  http.addHeader("User-Agent", "okhttp/3.10.0");
  http.addHeader("BrandId", "2");
  http.addHeader("Culture", "en");

  if(token.length() > 0)
  {
    http.addHeader("SecurityToken", token);
  }

  int httpResponseCode = -1;
  if(method == "GET")
  {
    httpResponseCode = http.GET();
  }
  else if(method == "POST")
  {
    httpResponseCode = http.POST(payload);
  }
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
 
  if (httpResponseCode > 0 ) {
      String payload = http.getString();
      Serial.println("Received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");     
      return payload;
  }
  else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    error();
  }
  return "";
  http.end(); 
}


void error()
{
  digitalWrite(LED_PIN, LOW);
}
void blinkLed(bool closed)
{
  if(closed)
  {
    for(int i=0;i<50;i++)
    {
      digitalWrite(LED_PIN, HIGH);                                     
      delay(100);
      digitalWrite(LED_PIN, LOW);                                     
      delay(100); 
    }   
  }
  digitalWrite(LED_PIN, HIGH);
}
