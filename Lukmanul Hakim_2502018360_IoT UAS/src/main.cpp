#include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>
#include <Wire.h>
#include "BH1750.h"
#include "DHTesp.h"
#include "ThingsBoard.h"

#define UPDATE_DATA_INTERVAL 5000
const char *ssid = "Nasution_plus";
const char *password = "Lukman07";
#define THINGSBOARD_ACCESS_TOKEN "TwNtrLiRoCqim9iytSSh"
#define THINGSBOARD_SERVER "demo.thingsboard.io"
#define PIN_DHT 4
#define PIN_SDA 18
#define PIN_SCL 5

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
Ticker timerSendData;

DHTesp dht;
BH1750 lightMeter;

void WifiConnect();
void onSendSensor();

void setup()
{
  Serial.begin(9600);
  dht.setup(PIN_DHT, DHTesp::DHT11);
  Wire.begin(PIN_SDA, PIN_SCL);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
  WifiConnect();
  timerSendData.attach_ms(UPDATE_DATA_INTERVAL, onSendSensor);
  Serial.println("System ready.");
}

void loop()
{
  if (!tb.connected())
  {
    if (tb.connect(THINGSBOARD_SERVER, THINGSBOARD_ACCESS_TOKEN))
      Serial.println("Connected to ThingsBoard");
    else
    {
      Serial.println("Error connecting to ThingsBoard");
      delay(3000);
    }
  }
  tb.loop();
}

void onSendSensor()
{
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  float lux = lightMeter.readLightLevel();
  if (dht.getStatus() == DHTesp::ERROR_NONE)
  {
    Serial.printf("Temperature: %.2f C, Humidity: %.2f %%, Light: %.2f\n",
                  temperature, humidity, lux);
    if (tb.connected())
    {
      tb.sendTelemetryFloat("temperature", temperature);
      tb.sendTelemetryFloat("humidity", humidity);
    }
  }
  else
    Serial.printf("Light: %.2f lx\n", lux);
  tb.sendTelemetryFloat("light", lux);
}

void WifiConnect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}