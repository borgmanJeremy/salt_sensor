#include <ESP8266WiFi.h>
#include <MQTT.h>
#include "Ultrasonic.h"
#include "passwords.h"

static const uint8_t TRIGGER_PIN = 12; // pin D6 is the trigger pin
static const uint8_t ECHO_PIN = 14;    // pin D5 is the echo pin

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN, 200000UL);
long int range = 0;

const char ssid[] = "its_a_trap";
const char pass[] = WIFI_PASSWORD;

WiFiClient net;
MQTTClient client;

int battery = A0;

void connect()
{
  WiFi.forceSleepWake();
  delay(1);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }

  client.begin("192.168.1.219", net);
  while (!client.connect("salt_sensor", "jeremy", HA_PASSWORD))
  {
    delay(1000);
  }
}

int getRange()
{
  return ultrasonic.read();
}

void setup()
{
  WiFi.mode(WIFI_OFF);

  WiFi.forceSleepBegin();
  delay(1);
}

void loop()
{

  int sensor_count = 0;
  range = getRange();
  while (range > 500)
  {
    range = getRange();
    sensor_count += 1;

    delay(1000);
    if (sensor_count > 10)
    {
      range = 0;
      break;
    }
  }

  int batt_lsb_count = analogRead(battery);
  float batt_v = (batt_lsb_count / 1024.0) * 3.3;

  connect();
  client.publish("/salt/distance", String(range));
  client.publish("/salt/battery/voltage", String(batt_v));
  client.publish("/salt/battery/percentage", String(100 * batt_v / 3.0));

  // Give 1 seconds for message to transmit
  delay(10000);

  // 1e6 = 1 second * 60seconds = 1 min*60min= 1hr * 12 = 12hr
  // ESP.deepSleep(1 * 1e6 , WAKE_RF_DISABLED);
  ESP.deepSleep(1 * 1e6 * 60 * 12, WAKE_RF_DISABLED);
}
