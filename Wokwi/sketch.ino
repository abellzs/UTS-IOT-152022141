#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Konfigurasi Pin
#define DHTPIN 8
#define DHTTYPE DHT22
#define GREEN_LED 5
#define YELLOW_LED 10
#define RED_LED 12
#define PUMP_RELAY 7
#define BUZZER 9

// Inisialisasi DHT Sensor
DHT dht(DHTPIN, DHTTYPE);

// WiFi dan MQTT
const char* ssid ="WOKWI_WIFI_SSID";    // Ganti dengan SSID WiFi Anda
const char* password = "WOKWI_WIFI_PASSWORD"; // Ganti dengan password WiFi Anda
const char* mqtt_server = "broker.hivemq.com"; // Broker MQTT publik

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  // Inisialisasi Sensor dan Aktuator
  dht.begin();
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Koneksi ke WiFi
  setup_wifi();

  // Setup MQTT
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  Serial.begin(9600);
  Serial.print("Connecting to WiFi");
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba koneksi MQTT...");
    if (client.connect("HydroponicDevice")) {
      Serial.println("terhubung");
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Membaca data sensor DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }

  // Logika threshold untuk LED dan Relay
  if (t > 30) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(PUMP_RELAY, HIGH); // Nyalakan pompa
  } else if (t > 25) {
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(PUMP_RELAY, LOW); // Matikan pompa
  } else {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(PUMP_RELAY, LOW); // Matikan pompa
  }

  // Streaming data ke MQTT
  client.publish("hydroponics/temperature", String(t).c_str());
  client.publish("hydroponics/humidity", String(h).c_str());

  // Buzzer untuk alert kelembapan rendah
  if (h < 30) {
    digitalWrite(BUZZER, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);
  }

  delay(2000); // Sesuaikan waktu delay jika diperlukan
}