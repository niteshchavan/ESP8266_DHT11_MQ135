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
const char* ssid = "OZONE1";
const char* password = "123456789";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<html>
<title>Sensor Dashboard</title>
<head>
  <meta charset="UTF-8">
  <meta content="IE=edge" http-equiv="X-UA-Compatible">
  <meta content="width=device-width, initial-scale=1" name="viewport">
  <style>
    body {
      background-color: #555888;
      font-family: sans-serif;
      color: #fff;
      text-align: center;
    }
	.sensor-container {
	  display: flex;
	  flex-direction: column; /* Stack items vertically */
	  align-items: center;    /* Center align items horizontally */
	  margin: 20px auto;
	  max-width: 1200px;
	}
    .sc-gauge {
      width: 200px;
      height: 150px;
      margin: 20px;
	  
    }
    .sc-background {
      position: relative;
      height: 100px;
      margin-bottom: 10px;
      background-color: #fff;
      border-radius: 150px 150px 0 0;
      overflow: hidden;
      text-align: center;
    }
    .sc-mask {
      position: absolute;
      top: 20px;
      right: 20px;
      left: 20px;
      height: 80px;
      background-color: #555888;
      border-radius: 150px 150px 0 0;
    }
    .sc-percentage {
      position: absolute;
      top: 100px;
      left: -200%;
      width: 400%;
      height: 400%;
      margin-left: 100px;
      background-color: #00aeef;
      transform: rotate(0deg);
      transform-origin: top center;
      transition: background-color 0.5s ease-in-out, transform 0.5s ease-in-out;
    }
    .sc-min {
      float: left;
    }
    .sc-max {
      float: right;
    }
    .sc-value {
      position: absolute;
      top: 50%;
      left: 0;
      width: 100%;
      font-size: 48px;
      font-weight: 700;
    }
    .sc-label {
      font-size: 20px;
      font-weight: bold;
      margin-top: 10px;
    }
    .aqi-table {
      background-color: #222;
      color: #fff;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.3);
      margin: 20px;
      width: 300px;
    }
    .aqi-table table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 10px;
    }
    .aqi-table th,
    .aqi-table td {
      padding: 8px 12px;
      text-align: left;
      border-bottom: 1px solid #666;
    }
    .aqi-table th {
      font-weight: bold;
      text-transform: uppercase;
      font-size: 14px;
    }
    .aqi-table td {
      font-size: 13px;
    }
  </style>
</head>
<body>
  <h1>Sensor Dashboard</h1>

  <div class="sensor-container">
    <!-- Smoke Gauge -->
    <div class="sc-gauge">
      <div class="sc-background">
        <div class="sc-percentage" id="smoke-percentage"></div>
        <div class="sc-mask"></div>
        <span class="sc-value" id="smoke-value">--</span>
      </div>
      <span class="sc-min">0 ppm</span>
      <span class="sc-max">500 ppm</span>
      <div class="sc-label">AQI</div>
    </div>

  <!-- Temperature Gauge -->
  <div class="sc-gauge">
    <div class="sc-background">
      <div class="sc-percentage" id="temperature-percentage"></div>
      <div class="sc-mask"></div>
      <span class="sc-value" id="temperature-value">--</span>
    </div>
    <span class="sc-min">0°C</span>
    <span class="sc-max">80°C</span>
    <div class="sc-label">Temperature</div>
  </div>

  <!-- Humidity Gauge -->
  <div class="sc-gauge">
    <div class="sc-background">
      <div class="sc-percentage" id="humidity-percentage"></div>
      <div class="sc-mask"></div>
      <span class="sc-value" id="humidity-value">--</span>
    </div>
    <span class="sc-min">0%</span>
    <span class="sc-max">100%</span>
    <div class="sc-label">Humidity</div>
  </div>

    <!-- AQI Table -->
    <div class="aqi-table">
      <h3>AQI Levels</h3>
      <table>
        <thead>
          <tr>
            <th>Range</th>
            <th>Category</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td>0-50</td>
            <td style="color: green;">Good</td>
          </tr>
          <tr>
            <td>51-100</td>
            <td style="color: orange;">Moderate</td>
          </tr>
          <tr>
            <td>101-150</td>
            <td style="color: red;">Unhealthy for Sensitive Groups</td>
          </tr>
          <tr>
            <td>151-200</td>
            <td style="color: darkred;">Unhealthy</td>
          </tr>
          <tr>
            <td>201-300</td>
            <td style="color: purple;">Very Unhealthy</td>
          </tr>
          <tr>
            <td>301-500</td>
            <td style="color: maroon;">Hazardous</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
  <script>
    async function fetchData() {
      try {
        const response = await fetch('/getData'); // Replace '/getData' with your endpoint
        const data = await response.json();
        console.log("Fetched data:", data);

        // Parse and update Smoke
        const smoke = parseFloat(data.smoke) || 0;
        const smokePercentage = Math.min(smoke, 500) * (180 / 500); // Map 0-500 PPM to 0-180 degrees
        const smokeGauge = document.getElementById("smoke-percentage");
		smokeGauge.style.transform = `rotate(${smokePercentage}deg)`;
        document.getElementById("smoke-value").textContent = `${smoke.toFixed(1)} PPM`;
		
		// Change smoke gauge color based on value
		if (smoke <= 50) {
		  smokeGauge.style.backgroundColor = "green"; // Safe levels
		} else if (smoke > 50 && smoke <= 100) {
		  smokeGauge.style.backgroundColor = "orange"; // Moderate levels
		} else {
		  smokeGauge.style.backgroundColor = "red"; // High levels
		}
		
        // Parse and update Temperature
        const temperature = parseFloat(data.temperature) || 0;
        const temperaturePercentage = Math.min(temperature, 80) * (180 / 80); // Map 0-80°C to 0-180 degrees
        document.getElementById("temperature-percentage").style.transform = `rotate(${temperaturePercentage}deg)`;
        document.getElementById("temperature-value").textContent = `${temperature.toFixed(1)}°C`;

        // Parse and update Humidity
        const humidity = parseFloat(data.humidity) || 0;
        const humidityPercentage = Math.min(humidity, 100) * (180 / 100); // Map 0-100% to 0-180 degrees
        document.getElementById("humidity-percentage").style.transform = `rotate(${humidityPercentage}deg)`;
        document.getElementById("humidity-value").textContent = `${humidity.toFixed(1)}%`;
      } catch (error) {
        console.error("Error fetching data:", error);
      }
    }

    setInterval(fetchData, 2000); // Fetch data every 2 seconds
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
  const unsigned long readInterval = 1000;  // 10 seconds

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
