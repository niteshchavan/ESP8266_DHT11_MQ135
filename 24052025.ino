#include <DHT.h>
#include <MQ135.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>

#define PIN_DHT11 2      // Digital pin connected to the DHT sensor
#define PIN_MQ135 A0     // MQ135 Analog Input Pin
#define DHTTYPE DHT11    // DHT 11

MQ135 gasSensor = MQ135(PIN_MQ135);
DHT dht(PIN_DHT11, DHTTYPE);

// WiFi credentials
const char* ssid = "OZONE";
const char* password = "123456789";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html { font-family: Arial; text-align: center; margin: 0 auto; }
    h2 { font-size: 3.0rem; }
    p { font-size: 2.0rem; }
    .units { font-size: 1.2rem; }
    .labels { font-size: 1.5rem; padding-bottom: 10px; }
  </style>
</head>
<body>
  <h2>ESP Environment Monitor</h2>
  <p><span class="labels">Smoke Level:</span> <span id="smoke">N/A</span><sup class="units">PPM</sup></p>
  <p><span class="labels">Humidity:</span> <span id="humidity">N/A</span><sup class="units">%</sup></p>
  <p><span class="labels">Temperature:</span> <span id="temperature">N/A</span><sup class="units">°C</sup></p>

  <script>
    async function fetchData() {
      try {
        const response = await fetch('/getData');
        const data = await response.json(); // Parse JSON response
        document.getElementById("temperature").innerHTML = data.temperature;
        document.getElementById("humidity").innerHTML = data.humidity;
        document.getElementById("smoke").innerHTML = data.smoke;
      } catch (error) {
        console.error("Error fetching data:", error);
      }
    }
    setInterval(fetchData, 10000); // Fetch every 10 seconds
    fetchData(); // Initial call
  </script>
</body>
</html>
)rawliteral";

// Global variables to store sensor data
float t = 0.0, h = 0.0, correctedPPM = 0.0;

// Function to provide JSON response with sensor data
String getSensorDataJSON() {
  String json = "{";
  json += "\"temperature\":" + String(t) + ",";
  json += "\"humidity\":" + String(h) + ",";
  json += "\"smoke\":" + String(correctedPPM);
  json += "}";
  return json;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MQ135 and DHT sensors...");

  // Initialize sensors
  dht.begin();

  // Start Wi-Fi in Access Point mode
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP: ");
  Serial.println(IP);

  // Define server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", getSensorDataJSON());
  });

  // Start the server
  server.begin();
  Serial.println("Server started!");
}

void loop() {
  static unsigned long lastRead = 0;
  const unsigned long readInterval = 10000;  // 10 seconds

  // Read sensors periodically
  if (millis() - lastRead >= readInterval) {
    lastRead = millis();

    // Read temperature and humidity from DHT sensor
    t = dht.readTemperature();
    h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Get corrected PPM value from MQ135 sensor
    correctedPPM = gasSensor.getCorrectedPPM(t, h);

    // Log the data
    Serial.printf("Temperature: %.2f°C, Humidity: %.2f%%, Smoke Level: %.2f PPM\n", t, h, correctedPPM);
  }
}
