// Include the required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// WiFi credentials
const char* ssid     = "ESP8266-Access-Point";
const char* password = "123456789";

#define DHTPIN 2     // DHT sensor is connected to GPIO 2
#define SMOKE_SENSOR_PIN A0  // Smoke sensor is connected to ADC0

// Uncomment the type of DHT sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// Variables for temperature, humidity, and smoke value
float t = 0.0;
float h = 0.0;
int smokeValue = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 Environment Monitor</h2>
  <p>
    <span class="labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <span class="labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
  <p>
    <span class="labels">Smoke Level</span>
    <span id="smoke">%SMOKE%</span>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("smoke").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/smoke", true);
  xhttp.send();
}, 10000);
</script>
</html>)rawliteral";

// Function to replace placeholders in HTML
String processor(const String& var){
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  else if(var == "SMOKE"){
    return String(smokeValue);
  }
  return String();
}

void setup(){
  // Serial port for debugging
  Serial.begin(115200);
  dht.begin();
  
  // Initialize Wi-Fi Access Point
  Serial.print("Setting up AP...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Define server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });
  server.on("/smoke", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(smokeValue).c_str());
  });

  // Start the server
  server.begin();
}

void loop(){  
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;
  const long interval = 10000;  // Update interval

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read temperature and humidity
    float newT = dht.readTemperature();
    if (!isnan(newT)) t = newT;
    float newH = dht.readHumidity();
    if (!isnan(newH)) h = newH;

    // Read smoke sensor value
    smokeValue = analogRead(SMOKE_SENSOR_PIN);

    // Print values to Serial Monitor
    Serial.print("Smoke Value: ");
    Serial.println(smokeValue);
    Serial.print("Temprature: ");
    Serial.println(t);
    Serial.print("Humidity: ");
    Serial.println(h);
  }
}
