long lastJob1s = 0, lastJob5s = 0, lastJob10s = 0, lastJob30s = 0, lastJob1min = 0, lastJob5min = 0, lastJob10min = 0;
float myTeplota1, myTeplota2, myTeplota3, myTlak1, myVlhkost1, myTeplota1x, myTeplota2x, myTeplota3x, myTlak1x, myVlhkost1x, kompenzace = 0;
int myZvuk, myZvukx, thod = 0, tmin = 0, tsec = 0;
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
String webpage = "", mylog = "";

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <FS.h>   //Include File ystem Headers
#include <Wire.h>
#include <DallasTemperature.h>
#include "Timer.h"

Timer t;

const char* imagefile = "/itrubec.jpg";
const char* htmlfile = "/index.html";

//ESP AP Mode
//IPAddress ip(192, 168, 4, 1);
char ssid[] = "iTRUBEC";    // your network SSID (name)
char pass[] = "varroa01";

ESP8266WebServer server(80);

// nastaveni adresy senzoru
#define BME280_ADRESA (0x76)
// inicializace senzoru BME z knihovny
Adafruit_BME280 bme;

//Nastaveni teplotnich cidel DS18B20
#define ONE_WIRE_BUS_PIN D4 // onewire pro Cidla na D4
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress Probe01 = { 0x28, 0xFF, 0x29, 0x15, 0x04, 0x17, 0x03, 0x04 };
DeviceAddress Probe02 = { 0x28, 0xFF, 0x29, 0x13, 0x04, 0x17, 0x03, 0x0D };

// SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-SETUP-
void setup() {
  Serial.begin(9600);
  Serial.println();

  //zahajeni komunikace s cidly DS18B20
  Wire.begin();
  sensors.begin();

  // nastaveni presnosti senzoru DS18B20 na 11 bitu (muze byt 9 - 12)
  sensors.setResolution(Probe01, 11);
  sensors.setResolution(Probe02, 11);

  // zahajeni komunikace se senzorem BME280,
  // v pripade chyby je vypsana hlaska po seriove lince
  // a zastaven program
  if (!bme.begin(BME280_ADRESA)) {
    Serial.println("BME280 senzor nenalezen, zkontrolujte zapojeni!");
    while (1);
  } else {
    Serial.println("BME280 senzor inicializovan");
  }

  //Inicializace File ystemu
  SPIFFS.begin();
  Serial.println("File ystem inicializovan");
  //SPIFFS.format();
  //Serial.println("Spiffs formatted");
  //SPIFFSDirlist();

  //Inicializace AP Modu
  WiFi.mode(WIFI_AP);
  WiFi.hostname("iTRUBEC");
  //WiFi.softAP(ssid, pass); //Password muze byt vynechano pro otevrene pripojeni
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Web Server IP: ");
  Serial.println(myIP);

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("itrubec")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  //Initialize Webserver
  server.on("/", handleRoot);
  server.on("/monitor", mymonitor);
  server.on("/datalog", datalog);
  server.on("/deletelog", deletelog);
  server.on("/deletelogQ", deletelogQ);
  server.on("/mysetup", mysetup);
  server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI
  server.begin();

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

  int tickEvent = t.every(1000, updateClock);

} // setup konec

// LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-LOOP-
void loop() {
  server.handleClient();
  t.update();

  // LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s-LOOP-BLOCK-1s
  if (millis() > (1000 + lastJob1s))
  {
    // kod vykonany kazdou 1 vterinu (1000 ms)


    lastJob1s = millis();
  } // 1s konec

  // LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-LOOP-BLOCK-5s-
  if (millis() > (5000 + lastJob5s))
  {
    // kod vykonany kazdych 5 vterin (5000 ms)


    lastJob5s = millis();
  } // 5s konec

  // LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10-LOOP-BLOCK-10s-LOOP-BLOCK-10s-LOOP-BLOCK-10s
  if (millis() > (10000 + lastJob10s))
  {
    // kod vykonany kazdych 10 vterin (10000 ms)
    ReadBME(); //Cteni BME280
    sensors.requestTemperatures(); //Cteni DS18B20
    myTeplota2x = myTeplota2;
    myTeplota3x = myTeplota3;
    myTeplota2 = sensors.getTempC(Probe01);
    myTeplota3 = sensors.getTempC(Probe02);
    SamplujZvuk(); //Ziskani vzorku zvuku
    Serial.print("Teploty: ");
    Serial.print(myTeplota1);
    Serial.print(", ");
    Serial.print(myTeplota2);
    Serial.print(", ");
    Serial.println(myTeplota3);
    Serial.print("Vlhkost: ");
    Serial.println(myVlhkost1);
    Serial.print("Tlak: ");
    Serial.println(myTlak1);
    Serial.print("Zvuk: ");
    Serial.println(myZvuk);
    Serial.println("----------------");

    //SPIFFSDirlist();

    lastJob10s = millis();
  } // 10s konec

  // LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-LOOP-BLOCK-30s-
  if (millis() > (30000 + lastJob30s))
  {
    // kod vykonany kazdych 30 vterin (30000 ms)


    lastJob30s = millis();
  } //30s konec

  // LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min-LOOP-BLOCK-1min
  if (millis() > (60000 + lastJob1min))
  {
    // kod vykonany kazdou 1 minutu (60000 ms)
    zapisLog();

    lastJob1min = millis();
  } // 1min konec

  // LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-LOOP-BLOCK-10min-
  if (millis() > (300000 + lastJob10min))
  {
    // kod vykonany kazdych 5 minut (300000 ms)


    lastJob10min = millis();
  } // 5min konec

  // LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-LOOP-BLOCK-
} // loop konec

// Vlastni funkce

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
  if (path.endsWith("/")) path += "index.html";

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".csv")) dataType = "text/csv";
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
  // Cteni vsech dostupnych informaci ze senzoru BMP
  // teplota
  myTeplota1x = myTeplota1;
  myTeplota1 = (bme.readTemperature());
  // relativni vlhkost
  myVlhkost1x = myVlhkost1;
  myVlhkost1 = (bme.readHumidity());
  // tlak s prepoCtem na hektoPascaly
  myTlak1x = myTlak1;
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
  myZvukx = myZvuk;
  myZvuk = peakToPeak;
  //Serial.println(volts);
}

void append_HTML_header(int refresh, char* title) {
  webpage += "<html><head>";
  webpage += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>";
  if (refresh > 0)
  {
    webpage += "<meta http-equiv=\"refresh\" content=\"";
    webpage += refresh;
    webpage += "\">";
  }
  webpage += "<link href=\"/favicon.ico\" rel=\"shortcut icon\" type=\"image/vnd.microsoft.icon\" />";
  webpage += "<title>iTRUBEC - ";
  webpage += title;
  webpage += "</title>";
  webpage += "<style>";
  webpage += "body {font-family: Arial, Helvetica, sans-serif;}";
  webpage += "a {text-decoration: none;}";
  webpage += ".menu td {font-size: 200%;}";
  webpage += ".note {font-size: 200%; color: darkgreen;}";
  webpage += ".ukahead {font-size: 200%;}";
  webpage += ".warn {font-size: 200%; color: red;}";
  webpage += ".mon {font-size: 400%; font-weight: bold;}";
  webpage += ".ico {}";
  webpage += ".chart {}";
  webpage += ".chartdiv {background-color: red; border: 1px solid black; height: 25px;}";
  webpage += ".btn {border: 3px outset #f4bf42; text-align: center; width: 200px; background-color: #f4bf42; text-decoration: none;}";
  webpage += "thead td {font-weight: bolder; background-color: #ffe3a3;}";
  webpage += "</style>";
  webpage += "</head><body><h1><img src=\"/itrubec.jpg\" width=\"200\" height=\"200\" alt=\"iTRUBEC\"> iTRUBEC – ";
  webpage += title;
  webpage += "</h1>";
  append_HTML_menu();
}

void append_HTML_menu() { // Saves repeating many lines of code for HTML page menus
  webpage += "<hr/>";
  webpage += "<table class=\"menu\" width=\"90%\" align=\"center\"><tr>";
  webpage += "<td align=\"center\" width=\"33%\"><a href=\"/monitor\"><div class=\"btn\">Monitor</div></a></td>";
  webpage += "<td align=\"center\"><a href=\"/datalog\"><div class=\"btn\">Log</div></a></td>";
  webpage += "<td align=\"center\" width=\"33%\"><a href=\"/mysetup\"><div class=\"btn\">Setup</div></a></td>";
  webpage += "</tr></table>";
  webpage += "<hr/>";
}

void append_HTML_logmenu(int velikost) { // Saves repeating many lines of code for HTML page logmenus
  webpage += "<table class=\"menu\" width=\"90%\" align=\"center\"><tr>";
  webpage += "<td align=\"center\" width=\"33%\"><a href=\"/itrubec.csv\"><div class=\"btn\">Download</div></a></td>";
  webpage += "<td align=\"center\"><a href=\"/deletelogQ\"><div class=\"btn\">Delete log</div></a></td>";
  webpage += "<td align=\"center\" width=\"33%\">Size: ";
  webpage += velikost;
  webpage += " B</td>";
  webpage += "</tr></table>";
}

void append_HTML_footer() { // Saves repeating many lines of code for HTML page footers
  append_HTML_menu();
  webpage += "</body></html>";
}

void datalog() {
  webpage = "";
  append_HTML_header(65, "Log");
  if (SPIFFS.exists("/itrubec.csv")) {
    File f = SPIFFS.open("/itrubec.csv", "r");
    if (f && f.size()) {
      int velikost = f.size();
      append_HTML_logmenu(velikost);
      webpage += "<table width=\"100%\"><thead><tr><td rowspan=\"2\" valign=\"middle\" align=\"center\">Time</td><td colspan=\"3\" align=\"center\">Temperature [°C]</td><td align=\"center\">Pressure</td><td align=\"center\">Humidity</td><td align=\"center\">Noise</td></tr>";
      webpage += "<tr><td align=\"center\">Temp1</td><td align=\"center\">Temp2</td><td align=\"center\">Temp3</td><td align=\"center\">[hPa]</td><td align=\"center\">[%]</td><td align=\"center\">[0-1024]</td></tr></thead><tbody>";
      while (f.available()) {
        webpage += "<tr>";
        for (int i = 0; i <= 5; i++) {
          webpage += "<td align=\"center\">";
          webpage += f.readStringUntil(';');
          webpage += "</td>";
        }
        webpage += "<td align=\"center\">";
        webpage += f.readStringUntil('\n');
        webpage += "</td></tr>";
      }
      f.close();
      webpage += "<tbody></table>";
      append_HTML_logmenu(velikost);
    }
  } else {
    webpage += "<br/><br/><br/>";
    webpage += "<center>";
    webpage += "<span class=\"warn\">No log data available</span>";
    webpage += "</center>";
    webpage += "<br/><br/><br/>";
  }
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

void deletelogQ() {
  webpage = "";
  append_HTML_header(0, "Log");
  webpage += "<br/><br/><br/>";
  webpage += "<center>";
  webpage += "<span class=\"warn\">Really delete all stored logs?!</span>";
  webpage += "<br/><br/><br/>";
  webpage += "<a href=\"/deletelog\"><div class=\"btn\">Delete log!</div></a>";
  webpage += "<center>";
  webpage += "<br/><br/><br/>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

void mysetup() {
  webpage = "";
  append_HTML_header(0, "Setup");
  webpage += "<br/><br/><br/>";
  webpage += "<center>";
  webpage += "<span class=\"note\">...some setup stuff to be added here...</span>";
  webpage += "</center>";
  webpage += "<br/><br/><br/>";
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

void deletelog() {
  webpage = "";
  append_HTML_header(0, "Delete log");
  if (SPIFFS.remove("/itrubec.csv")) {
    webpage += "<br/><br/><br/>";
    webpage += "<center>";
    webpage += "<span class=\"note\">Log file deleted</span>";
    webpage += "</center>";
    webpage += "<br/><br/><br/>";
  } else {
    webpage += "<br/><br/><br/>";
    webpage += "<center>";
    webpage += "<span class=\"warn\">ERROR deleting log file</span>";
    webpage += "</center>";
    webpage += "<br/><br/><br/>";
  }
  append_HTML_footer();
  server.send(200, "text/html", webpage);
}

void mymonitor() {
  webpage = "";
  webpage += "<img src=\"/beemonitor.gif\" width=\"195\" height=\"168\" align=\"right\" alt=\"...monitoring...\">";
  append_HTML_header(10, "Monitor");
  /*
    webpage += "Teplota 1: <b>";
    webpage +=  kompenzuj(myTeplota1);
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
  */
  // ukazatel(char* nazev, char* jednotka, float hodnota, float multi, float ico)
  ukazatel("Temperature 1", "°C", kompenzuj(myTeplota1), 5, (myTeplota1x - myTeplota1));
  ukazatel("Temperature 2", "°C", myTeplota2, 5, (myTeplota2x - myTeplota2));
  ukazatel("Temperature 3", "°C", myTeplota3, 5, (myTeplota3x - myTeplota3));
  ukazatel("Atmospheric pressure", "hPa", myTlak1, 0.4, (myTlak1x - myTlak1));
  ukazatel("Relative humidity", "%", myVlhkost1, 5, (myVlhkost1x - myVlhkost1));
  ukazatel("Noise level", " ", myZvuk, 0.4, (myZvukx - myZvuk));
  append_HTML_footer();
  //Serial.println(webpage);
  server.send(200, "text/html", webpage);
}

void SPIFFSDirlist() {
  String str = "";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    str += dir.fileName();
    str += " / ";
    str += dir.fileSize();
    str += "\r\n";
  }
  Serial.println("Vypis SPIFFS:");
  Serial.println(str);
  Serial.println("Konec SPIFFS...");
}

float kompenzuj(float teplota) {
  if (kompenzace == 0) { //na začátku
    kompenzace = myTeplota1 - ((myTeplota2 + myTeplota3) / 2);
  }
  Serial.print("Kompenzace cidla 1: ");
  Serial.print(teplota);
  Serial.print(" - ");
  Serial.print(kompenzace);
  Serial.print(" - ");
  Serial.println(teplota - kompenzace);
  return (teplota - kompenzace);
}

void updateClock() {
  tsec += 1;
  if (tsec >= 60) {
    tsec = 0;
    tmin += 1;
  }
  if (tmin >= 60) {
    tmin = 0;
    thod += 1;
  }
  if (thod >= 24) {
    thod = 0;
  }
}

void zapisLog() {
  mylog = "";
  mylog += thod;
  mylog += ":";
  mylog += tmin;
  mylog += ":";
  mylog += tsec;
  mylog += ";";
  mylog +=  kompenzuj(myTeplota1);
  mylog += ";";
  mylog += myTeplota2;
  mylog += ";";
  mylog += myTeplota3;
  mylog += ";";
  mylog += myTlak1;
  mylog += ";";
  mylog += myVlhkost1;
  mylog += ";";
  mylog += myZvuk;
  // open file for writing
  File f = SPIFFS.open("/itrubec.csv", "a");
  if (!f) {
    Serial.println("Log file open failed");
  }
  f.println(mylog);
  f.close();
}

void ukazatel(char* nazev, char* jednotka, float hodnota, float multi, float ico) {
  webpage += "<table width=\"100%\">";
  webpage += "<thead>";
  webpage += "<tr>";
  webpage += "<td colspan=\"3\" align=\"center\" class=\"ukahead\">";
  webpage += nazev;
  webpage += "</td>";
  webpage += "</tr>";
  webpage += "</thead>";
  webpage += "<tbody>";
  webpage += "<tr>";
  webpage += "<td valign=\"middle\" align=\"right\" width=\"38%\" height=\"50\" class=\"mon\">";
  webpage += hodnota;
  webpage += "&nbsp";
  webpage += jednotka;
  webpage += "</td>";
  webpage += "<td valign=\"middle\" align=\"center\" width=\"50\" height=\"50\" class=\"ico\">";
  if (ico > 0) { // klesa
    webpage += "<img src=\"/arr-down.gif\" width=\"37\" height=\"48\" />";
  }
  if (ico == 0) { //stabilni
    webpage += "<img src=\"/arr-stab.gif\" width=\"48\" height=\"37\" />";
  }
  if (ico < 0) { //roste
    webpage += "<img src=\"/arr-up.gif\" width=\"37\" height=\"48\" />";
  }
  webpage += "</td>";
  webpage += "<td valign=\"middle\" align=\"left\" width=\"*\" width=\"50\" class=\"chart\">";
  webpage += "<div style=\"width: ";
  webpage += round(hodnota * multi);
  webpage += "px;\" class=\"chartdiv\">";
  webpage += "</td>";
  webpage += "</tr>";
  webpage += "</tbody>";
  webpage += "</table>";
}
