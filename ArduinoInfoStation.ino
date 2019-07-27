//https://github.com/ZinggJM/GxEPD/tree/master/examples/GxEPD_Example

//Board einstellen --> ESP32 DEV

#include <GxEPD.h> 
#include <GxGDE0213B1/GxGDE0213B1.h>      // 2.13" b/w

#if !defined(_GxFont_GFX_TFT_eSPI_H_)
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
//#include <Fonts/FreeMonoBold24pt7b.h>
#endif

#if defined(_ADAFRUIT_TF_GFX_H_)
#include <Fonts/Open_Sans_Bold_12pt.h>
#endif

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

//Wifi
#include <WiFi.h>

//MQTT
#include <PubSubClient.h>

//ESP32
GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

//Wifi
const char* ssid     = "UPC3971618";
const char* password = "passwd";

//MQTT
const char* mqtt_server = "192.168.0.100";
const char* mqtt_username = "";
const char* mqtt_password = "";
//char* topic = "InfoBoxWz";

WiFiClient espClient;
PubSubClient client(espClient);

//Display Textausrichtung
typedef enum
{
  RIGHT_ALIGNMENT = 0,
  LEFT_ALIGNMENT,
  CENTER_ALIGNMENT,
} Text_alignment;

//globale Variablen
String tempA = "nA";
String humidityA = "nA";
String tempI = "nA";
String humidityI = "nA";
String tempA_Old;
String humidityA_Old;
String tempI_Old;
String humidityI_Old;


  
void setup()
{
  
  Serial.begin(115200);
  display.init(115200); // enable diagnostic output on Serial
  display.eraseDisplay();

  Serial.println("Start Setup..");
  
  // WiFi-Connection //https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi/examples/WiFiClient
  WIFI_Connect();
  
  //MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  //Settings Display
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextSize(1);

  Serial.println("Setup Ready!");
}



int z=1;
int mqttcounter=0;
int wificounter=0;
long lastMsg=-99999999;

void loop()
{ /*
  Serial.print("MqttConnections:");
  Serial.println(mqttcounter);
  Serial.print("WifiConnections:");
  Serial.println(wificounter);
  
  Serial.println(z);
  Serial.println("Check WifiStatus");
  */
  if (WiFi.status() != WL_CONNECTED){
      WIFI_Connect();
      wificounter+=1;
    } 
  //Serial.println("Wifi OK");
    
  if (!client.connected()) {
    Serial.println("Mainloop Client not connected");
    MQTT_Connect();
    mqttcounter+=1;
  }

  client.loop();
  
  //Alle 5min Display aktualisieren sofern neue Werte vorhanden
  long now = millis();
  if (now - lastMsg > 5*1000*60) { 
    lastMsg = now;
    if (DisplayNichtAktuell()){
      displayVorbereitung();
      client.loop();
      displayUpdate();    
    }
    
  }
  
  
  z+=1;
  if(z>10){
    z=0;
  }
  
  
   
}

String RoundInput(String Stringinput){
  Serial.println("RoundInput...");
  /*float floatValue;
  Serial.println(Stringinput);
  floatValue = Stringinput.toFloat();
  Serial.println(floatValue);
  int intValue = (int)(floatValue*10);
  Serial.println(intValue);
  floatValue = (float)intValue/10;
  Serial.println(floatValue);
  String StringOutput;
  StringOutput = String(floatValue);
  Serial.println(StringOutput);
  StringOutput = StringOutput.substring(0, 4);
  Serial.println(StringOutput);
  return StringOutput;*/
  Serial.println(Stringinput.substring(0, 4));
  return Stringinput.substring(0, 4);
}

void displayVorbereitung(){
      int x = 20;
      int y = 20;
      
      //display.setTextColor(GxEPD_BLACK);
      //display.setFont(&FreeMonoBold12pt7b);
      //display.setTextSize(1);
      
      display.setCursor(x,y);

      display.eraseDisplay();
      display.fillScreen(GxEPD_WHITE);

      display.setFont(&FreeMonoBold12pt7b);
      displayText("Aussen:", y, LEFT_ALIGNMENT);  //Temp Außen
      display.setFont(&FreeMonoBold18pt7b);
      y = display.getCursorY() + 1; //Eine Zeile nach unten
      displayText(tempA, y, LEFT_ALIGNMENT);  //Temp Außen
      y = display.getCursorY() + 1; //Eine Zeile nach unten
      displayText(humidityA, y, LEFT_ALIGNMENT);  //Humidity Außen
      y = display.getCursorY() + 2; //Eine Zeile nach unten

      display.setFont(&FreeMonoBold12pt7b);
      displayText("Innen:", y, LEFT_ALIGNMENT);  //Temp Außen
      display.setFont(&FreeMonoBold18pt7b);
      y = display.getCursorY() + 1; //Eine Zeile nach unten
      displayText(tempI, y, LEFT_ALIGNMENT);  //Temp Innen
      y = display.getCursorY() + 1; //Eine Zeile nach unten
      displayText(humidityI, y, LEFT_ALIGNMENT);  //Humidity Innen
  
}
void displayText(const String &str, int16_t y, uint8_t alignment)
{
  Serial.println("Display Textfunktion");
  int16_t x = 0;
  int16_t x1, y1;
  uint16_t w, h;
  display.setCursor(x, y);
  display.getTextBounds(str, x, y, &x1, &y1, &w, &h);

  switch (alignment)
  {
  case RIGHT_ALIGNMENT:
    display.setCursor(display.width() - w - x1, y);
    break;
  case LEFT_ALIGNMENT:
    display.setCursor(0, y);
    break;
  case CENTER_ALIGNMENT:
    display.setCursor(display.width() / 2 - ((w + x1) / 2), y);
    break;
  default:
    break;
  }
  display.println(str);
  //delay(1000);
}

void displayUpdate(){
  Serial.println("Display Update..");
  display.update();
  Serial.println("Display Update Ready!");
}


void MQTT_Connect() {
  // Loop until we're reconnected
  Serial.println("Funktion MQTT Connect...");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // Create a random client ID
    String clientId = "MQTTClientInfoBox-";
    clientId += String(random(0xffff), HEX);
    Serial.print("ClientID:");
    Serial.println(clientId);
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      //if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      delay(100);
      client.subscribe("InfoBoxWzTA");
      delay(100);
      client.subscribe("InfoBoxWzHA");
      delay(100);
      client.subscribe("InfoBoxWzTI");
      delay(100);
      client.subscribe("InfoBoxWzHI");
      delay(100);
     
     
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void WIFI_Connect(){
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  for (int i = 0; i < 25; i++)
  {
    if ( WiFi.status() != WL_CONNECTED ) {
      delay(500);
      Serial.print(".");
    }else{
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP()); 
      break;   
    }    
  } 
}


int DisplayNichtAktuell(){
  
  if( (tempA != tempA_Old) || (humidityA != humidityA_Old)){
    tempA_Old = tempA;
    humidityA_Old = humidityA;
    Serial.println("Aktualisiere Werte Außen");
    return 1;
  }
  if( (tempI != tempI_Old) && (humidityI != humidityI_Old)){
    tempI_Old = tempI;
    humidityI_Old = humidityI;
    Serial.println("Aktualisiere Werte Innen");
    return 1;
  }
  return 0; //Keine aktualisierten Werte
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  
  }
  Serial.println();
  
  String s = String((char*)payload);
  s = RoundInput(s); //runden des Strings auf eine Stelle hinterm Komma todo
  if (strcmp(topic,"InfoBoxWzTA")==0){  
    tempA = s;  
    tempA += "C";
  }  
  if (strcmp(topic,"InfoBoxWzHA")==0){
    humidityA = s;  
    humidityA += "%";
  }
  if (strcmp(topic,"InfoBoxWzTI")==0){  
    tempI = s; 
    tempI += "C";
  }  
  if (strcmp(topic,"InfoBoxWzHI")==0){
    humidityI = s;
    humidityI += "%";  
  }

}

