#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Copy over "_constants.h.example" to "_constants.h" and update it with values suitable for your network
#include "_constants.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER_IP;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
const long scanInterval = 60000*5; // scan every 5 minutes
char msg[50];
int ssidCount;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  client.setServer(mqtt_server, 1883);
  setupWifi();
}

void setupWifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266 light sensor")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

char ** scanWifi() {
  Serial.println("Scan start ... ");
  int n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.println(" network(s) found");
  char** ssids = new char*[n];

  for (int i = 0; i < n; i++) {
    ssids[i] = new char[40];
    String ssid = WiFi.SSID(i);
    ssid.toCharArray(ssids[i], 40);
  }
  WiFi.scanDelete();
  Serial.println();
  ssidCount = n;
  return ssids;
}

void loop() {
  client.loop();

  long now = millis();
  if (now - lastMsg > scanInterval) {
    lastMsg = now;

    char **ssids = scanWifi();

    if (!client.connected()) {
      reconnect();
    }

    Serial.println("Publish message: ");
    int i;
    for (i = 0; i < ssidCount; i++) {
      Serial.println(ssids[i]);
      client.publish(MQTT_CHANNEL, ssids[i]);
      delete [] ssids[i];
    }
    delete [] ssids;
    Serial.println();
  }
}
