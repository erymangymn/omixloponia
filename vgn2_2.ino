
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

#include <LCD_I2C.h>
#include "Wire.h"

#include "DHT.h"

#include "WiFiS3.h"
#include <ArduinoMqttClient.h>


#define DHTPIN 12     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

char ssid[] = "Xiaomi_kath";        // your network SSID (name)
char pass[] = "";  

char mqtt_user[] = "gir";
char mqtt_pass[] = "Gir12345$";


WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "girok.hopto.org"; //IP address of the EMQX broker.
int        port     = 1883;
const char subscribe_topic[]  = "/erym/omixl/pot";
const char publish_h_topic[]  = "/erym/omixl/hum";
const char publish_t_topic[]  = "/erym/omixl/temp";
const char publish_p_topic[]  = "/erym/omixl/pot";

ArduinoLEDMatrix matrix;
LCD_I2C lcd(0x27, 16, 2); // Default address of most PCF8574 modules, change according
DHT dht(DHTPIN, DHTTYPE);
int led =  LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);
int timerOn;
int timerOff;
unsigned long mytime;

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}



void setup() {

  Serial.begin(115200);
  Serial.println(F("Ομιχλοπονία Ερύμανθου"));
  pinMode(LED_BUILTIN, OUTPUT); 
  dht.begin();

  Wire.begin();
  lcd.begin(&Wire);
  lcd.display();
  lcd.backlight();

  matrix.begin();
/*
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(100);

  const char text[] = "  fogponic erymanthos by asteres  ";
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();

*/

while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    //status = WiFi.begin(ssid, pass);
    status = WiFi.begin(ssid);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();    

  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);

  Serial.print("Attempting to connect to the MQTT broker.");

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");

  Serial.print("Subscribing to topic: ");
  Serial.println(subscribe_topic);

  // subscribe to a topic
  mqttClient.subscribe(subscribe_topic);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(subscribe_topic);

  mytime=millis();

}

void loop() {


// Wait a few seconds between measurements.
  delay(500);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));



  
  // Make it scroll!
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(50);

  // add the text
  char text[] = "    Scrolling text!    ";
  sprintf (text, "%.2f  %.2f", h, t);
  //text=String(h);
  Serial.println(text);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
 
  lcd.clear();
  lcd.print(text); // You can make spaces using well... spaces
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("hum - temp     ");
    delay(500);

  int messageSize = mqttClient.parseMessage();
  if (messageSize) {
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    // use the Stream interface to print the contents
    while (mqttClient.available()) {
      char rcv;
      rcv=(char)mqttClient.read();
      Serial.print(rcv);
      if (rcv=='o') {  digitalWrite(LED_BUILTIN, HIGH);  }
      if (rcv=='f') {  digitalWrite(LED_BUILTIN, LOW);  }
    }
    Serial.println();
  }
  mqttClient.beginMessage(publish_h_topic);
  mqttClient.print(h);
  mqttClient.endMessage();
  mqttClient.beginMessage(publish_t_topic);
  mqttClient.print(t);
  mqttClient.endMessage();

  if ((millis()-mytime)<20000) { 
    digitalWrite(13,HIGH);

    } else { 
      if ((millis()-mytime)<24000) {
    mqttClient.beginMessage(publish_p_topic);
    mqttClient.print('f');
    mqttClient.endMessage();       
      } 
      digitalWrite(13,LOW);
      } 
  if ((millis()-mytime)>60000) {
    mytime=millis();
    mqttClient.beginMessage(publish_p_topic);
    mqttClient.print('o');
    mqttClient.endMessage();
    }

}