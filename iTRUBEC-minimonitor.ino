long lastJob1s = 0, lastJob5s = 0, lastJob10s = 0, lastJob30s = 0, lastJob1min = 0, lastJob5min = 0;
float myTeplota1, myTeplota2, myTeplota3, myTlak1, myVlhkost1;
int myZvuk;
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
String webpage = "";

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <FS.h>   //Include File System Headers
#include <Wire.h>
#include <DallasTemperature.h>

const char* imagefile = "/itrubec.png";
const char* htmlfile = "/index.html";

//ESP AP Mode
//IPAddress ip(192, 168, 4, 1);
char ssid[] = "iTRUBEC";    // your network SSID (name)
char pass[] = "varroa01";

ESP8266WebServer server(80);

// nastavení adresy senzoru
#define BME280_ADRESA (0x76)
// inicializace senzoru BME z knihovny
Adafruit_BME280 bme;

//Nastavení teplotních čidel DS18B20
#define ONE_WIRE_BUS_PIN D8 // onewire pro čidla na D8
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress Probe01 = { 0x28, 0xFF, 0x29, 0x15, 0x04, 0x17, 0x03, 0x04 };
DeviceAddress Probe02 = { 0x28, 0xFF, 0x29, 0x13, 0x04, 0x17, 0x03, 0x0D };

// SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-
void setup() {
  Serial.begin(9600);
  Serial.println();

  //zahájení komunikace s čidly DS18B20
  Wire.begin();
  sensors.begin();

  // nastavení přesnosti senzorů DS18B20 na 11 bitů (může být 9 - 12)
  sensors.setResolution(Probe01, 11);
  sensors.setResolution(Probe02, 11);

  // zahájení komunikace se senzorem BME280,
  // v případě chyby je vypsána hláška po sériové lince
  // a zastaven program
  if (!bme.begin(BME280_ADRESA)) {
    Serial.println("BME280 senzor nenalezen, zkontrolujte zapojeni!");
    while (1);
  }

  //Inicializace File Systemu
  SPIFFS.begin();
  Serial.println("File System inicializován");

  //Inicializace AP Modu
  WiFi.mode(WIFI_AP_STA);
  WiFi.hostname("iTRUBEC");
  WiFi.softAP(ssid, pass); //Password může být vynecháno pro otevřené připojení
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Web Server IP: ");
  Serial.println(myIP);

  //Initialize Webserver
  server.on("/", handleRoot);
  server.on("/monitor", monitor);
  server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI
  server.begin();

} // setup konec

// LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-
void loop() {
  server.handleClient();


  // LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s
  if (millis() > (1000 + lastJob1s))
  {
    // kód vykonaný každou 1 vteřinu (1000 ms)


    lastJob1s = millis();
  } // 1s konec

  // LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-
  if (millis() > (5000 + lastJob1s))
  {
    // kód vykonaný každých 5 vteřin (5000 ms)


    lastJob1s = millis();
  } // 5s konec

  // LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s
  if (millis() > (10000 + lastJob1s))
  {
    // kód vykonaný každých 10 vteřin (10000 ms)
    ReadBME(); //čtení BME280
    sensors.requestTemperatures(); //čtení DS18B20
    myTeplota2 = sensors.getTempC(Probe01);
    myTeplota3 = sensors.getTempC(Probe02);
    SamplujZvuk(); //Získání vzorku zvuku

    lastJob1s = millis();
  } // 10s konec

  // LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-
  if (millis() > (30000 + lastJob30s))
  {
    // kód vykonaný každých 30 vteřin (30000 ms)


    lastJob30s = millis();
  } //30s konec

  // LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min
  if (millis() > (60000 + lastJob1min))
  {
    // kód vykonaný každou 1 minutu (60000 ms)


    lastJob1min = millis();
  } // 1min konec

  // LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-
  if (millis() > (300000 + lastJob1min))
  {
    // kód vykonaný každých 5 minut (300000 ms)


    lastJob1min = millis();
  } // 5min konec

  // LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-
} // loop konec

// Vlastní funkce

void handleRoot() {
  server.sendHeader("Location", "/index.html", true);  //Redirect to our html web page
  server.send(302, "text/plane", "");
}

void handleWebRequests() {
  if (loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}

bool loadFromSpiffs(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) path += "index.htm";

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}

void ReadBME() {
  // čtení všech dostupných informací ze senzoru BMP
  // teplota
  myTeplota1 = (bme.readTemperature());
  // relativní vlhkost
  myVlhkost1 = (bme.readHumidity());
  // tlak s přepočtem na hektoPascaly
  myTlak1 = (bme.readPressure() / 100.0F);
}

void SamplujZvuk() {
  unsigned long startMillis = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(A0);
    if (sample < 1024)  // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  //double volts = (peakToPeak * 5.0) / 1024;  // convert to volts
  myZvuk = peakToPeak;
  //Serial.println(volts);
}

void append_HTML_header() {
  webpage += "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'><html><head>";
  webpage += "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'/><meta http-equiv='refresh' content='5'>"; // 5-sec refresh time, test needed to prevent auto updates repeating some commands
  webpage += "<title>iTRUBEC – monitor</title>";
  webpage += "</head><body><img src='\itrubec.png' alt='iTRUBEC' width='200' height='200' align='right'><h1>iTRUBEC</h1>";
}

void append_HTML_footer() { // Saves repeating many lines of code for HTML page footers
  webpage += "</body></html>";
}

void monitor() {
  webpage = "";
  append_HTML_header();
  webpage += "Teplota 1: <b>";
  webpage += myTeplota1;
  webpage += "°C</b>";
  webpage += "<br>";
  webpage += "Teplota 2: <b>";
  webpage += myTeplota2;
  webpage += "°C</b>";
  webpage += "<br>";
  webpage += "Teplota 3: <b>";
  webpage += myTeplota3;
  webpage += "°C</b>";
  webpage += "<br>";
  webpage += "Tlak: <b>";
  webpage += myTlak1;
  webpage += " hPa</b>";
  webpage += "<br>";
  webpage += "Vlhkost: <b>";
  webpage += myVlhkost1;
  webpage += " %</b>";
  webpage += "<br>";
  webpage += "Zvuk: <b>";
  webpage += myZvuk;
  webpage += "<br>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}
