#include <WiFi.h> // Einbinden der WiFi-Bibliothek für die Netzwerkfunktionen
#include <Adafruit_GFX.h> // Einbinden der Adafruit_GFX-Bibliothek für die Grafikfunktionen des Displays
#include <Adafruit_SSD1306.h> // Einbinden der Adafruit_SSD1306-Bibliothek für die Ansteuerung des OLED-Displays

// OLED Display Einstellungen
#define SCREEN_WIDTH 128 // Breite des OLED-Displays in Pixel
#define SCREEN_HEIGHT 32 // Höhe des OLED-Displays in Pixel
#define OLED_RESET    -1 // Reset-Pin des OLED-Displays (nicht verwendet)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Initialisierung des Display-Objekts

// LED Pins
const int ledRed = 12;    // Pin für die rote LED
const int ledGreen = 14;  // Pin für die grüne LED
const int ledYellow = 27; // Pin für die gelbe LED

// Access Point SSID und Passwort
const char* ssid = "ESP32_AP";       // Name des Access Points
const char* password = "12345678";   // Passwort des Access Points

// Webserver auf Port 80
WiFiServer server(80);  // Initialisierung des Webservers auf Port 80

// Anzeigeinformationen
String currentTime = "11:02";  // Aktuelle Uhrzeit
String trainNumber = "506";    // Zugnummer
String trainName = "ICE";      // Zugname
String trackNumber = "11";     // Gleisnummer
String infoText = "+++ Zug verspaetung ca 60min +++"; // Informationstext
String routeInfo = "Basel Sbb - Frankfurt - Berlin HBF"; // Streckeninformation
int x1, x2, minX1, minX2; // Variablen für die Position der scrollenden Texte

// Struktur zur Steuerung der LEDs
struct LEDControl {
  int pin;               // Pin der LED
  unsigned long endTime; // Zeit, wann die LED ausgeschaltet werden soll
  bool isOn;             // Status der LED (an/aus)
};

// Initialisierung der LED-Steuerung für jede LED
LEDControl ledRedControl = {12, 0, false};
LEDControl ledGreenControl = {14, 0, false};
LEDControl ledYellowControl = {27, 0, false};

void setup() {
  Serial.begin(115200); // Initialisierung der seriellen Kommunikation

  // Initialisierung des OLED-Displays
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed")); // Fehlermeldung, wenn das Display nicht initialisiert werden kann
    for (;;); // Endlosschleife bei Fehler
  }

  // Initialisierung der LED-Pins
  pinMode(ledRed, OUTPUT);   // Pin für rote LED als Ausgang definieren
  pinMode(ledGreen, OUTPUT); // Pin für grüne LED als Ausgang definieren
  pinMode(ledYellow, OUTPUT); // Pin für gelbe LED als Ausgang definieren
  digitalWrite(ledRed, LOW);    // Rote LED aus
  digitalWrite(ledGreen, LOW);  // Grüne LED aus
  digitalWrite(ledYellow, LOW); // Gelbe LED aus

  // Start des Access Points
  WiFi.softAP(ssid, password); // Starten des Access Points mit SSID und Passwort
  Serial.println("Access Point gestartet"); // Ausgabe zur Bestätigung

  // IP-Adresse des Access Points ausgeben
  Serial.print("IP Adresse: ");
  Serial.println(WiFi.softAPIP()); // Ausgabe der IP-Adresse des Access Points

  // Start des Webservers
  server.begin(); // Starten des Webservers

  // Initialisierung der Scroll-Positionen
  display.setTextWrap(false); // Textumbruch deaktivieren
  x1 = display.width(); // Startposition für scrollenden Text initialisieren
  x2 = display.width(); // Startposition für scrollenden Text initialisieren

  // Anzeige der initialen statischen Zuginformationen
  displayStaticInfo(); // Statische Informationen auf dem Display anzeigen
}

void loop() {
  WiFiClient client = server.available(); // Überprüfen, ob ein neuer Client verbunden ist
  if (client) { // Wenn ein neuer Client verbunden ist
    Serial.println("Neuer Client verbunden"); // Ausgabe zur Bestätigung
    String request = client.readStringUntil('\r'); // HTTP-Anfrage des Clients lesen
    Serial.println(request); // Anfrage zur Überprüfung ausgeben
    client.flush(); // Puffer leeren

    // HTTP-Anfrage parsen und LED steuern
    if (request.indexOf("/LED=R_ON") != -1) {
      int duration = parseDuration(request); // Dauer aus der Anfrage parsen
      controlLED(ledRedControl, duration);   // Rote LED steuern
    }
    if (request.indexOf("/LED=R_OFF") != -1) {
      digitalWrite(ledRedControl.pin, LOW); // Rote LED ausschalten
      ledRedControl.isOn = false;           // Status der LED aktualisieren
    }
    if (request.indexOf("/LED=G_ON") != -1) {
      int duration = parseDuration(request);
      controlLED(ledGreenControl, duration); // Grüne LED steuern
    }
    if (request.indexOf("/LED=G_OFF") != -1) {
      digitalWrite(ledGreenControl.pin, LOW); // Grüne LED ausschalten
      ledGreenControl.isOn = false;
    }
    if (request.indexOf("/LED=Y_ON") != -1) {
      int duration = parseDuration(request);
      controlLED(ledYellowControl, duration); // Gelbe LED steuern
    }
    if (request.indexOf("/LED=Y_OFF") != -1) {
      digitalWrite(ledYellowControl.pin, LOW); // Gelbe LED ausschalten
      ledYellowControl.isOn = false;
    }

    // Anzeigeinformationen basierend auf der HTTP-Anfrage aktualisieren
    if (request.indexOf("updateDisplay") != -1) {
      updateDisplayInfo(request); // Anzeigeinformationen aktualisieren
    }

    // Antwort an den Client senden
    client.println("HTTP/1.1 200 OK"); // HTTP-Header für die Antwort
    client.println("Content-Type: text/html"); // Inhaltstyp der Antwort
    client.println("Connection: close"); // Verbindung nach der Antwort schließen
    client.println();
    client.println("<!DOCTYPE HTML>"); // HTML-Inhalt der Antwort
    client.println("<html>");
    client.println("<p>LED Steuerung</p>");
    client.println("</html>");
    client.stop(); // Verbindung zum Client schließen
    Serial.println("Client Verbindung geschlossen"); // Ausgabe zur Bestätigung
  }

  // OLED-Display mit scrollendem Text aktualisieren
  display.clearDisplay(); // Display löschen

  // Statische Informationen anzeigen
  displayStaticInfo(); // Funktion zur Anzeige statischer Informationen aufrufen

  // Scrollenden Text anzeigen
  scrollText(infoText, x1, 10); // Scrollenden Informationstext anzeigen
  scrollText(routeInfo, x2, 25); // Scrollende Streckeninformation anzeigen

  display.display(); // Änderungen auf dem Display anzeigen
  delay(50); // Scrollgeschwindigkeit anpassen

  // LEDs aktualisieren
  updateLED(ledRedControl); // Status der roten LED überprüfen und aktualisieren
  updateLED(ledGreenControl); // Status der grünen LED überprüfen und aktualisieren
  updateLED(ledYellowControl); // Status der gelben LED überprüfen und aktualisieren
}

// Funktion zur Anzeige statischer Informationen auf dem OLED-Display
void displayStaticInfo() {
  display.setTextSize(1); // Textgröße setzen
  display.setTextColor(SSD1306_WHITE); // Textfarbe setzen

  // Zeit und Zugnummer anzeigen
  display.setCursor(0, 0); // Cursorposition setzen
  display.print(currentTime); // Uhrzeit anzeigen
  display.setCursor(35, 0); // Cursorposition setzen
  display.print(trainName + " " + trainNumber); // Zugname und -nummer anzeigen

  // Gleis anzeigen
  display.setCursor(95, 0); // Cursorposition setzen
  display.print("Gl."); // "Gleis" abkürzen
  display.println(trackNumber); // Gleisnummer anzeigen
}

// Funktion zum Scrollen des Textes
void scrollText(String message, int &x, int y) {
  minX1 = -12 * message.length(); // Minimale X-Position basierend auf der Länge des Textes
  display.setTextSize(1); // Gleiche Textgröße wie in den statischen Informationen verwenden
  display.setCursor(x, y); // Cursor auf die aktuelle Position setzen
  display.print(message); // Nachricht drucken
  x = x - 2; // Scrollgeschwindigkeit
  if (x < minX1) x = display.width(); // Wenn der Text das Ende erreicht, zurücksetzen
}

// Funktion zum Aktualisieren der Anzeigeinformationen basierend auf der HTTP-Anfrage
void updateDisplayInfo(String request) {
  // Anfrage parsen und entsprechende Anzeigeinformationen aktualisieren
  if (request.indexOf("time=") != -1) {
    currentTime = urlDecode(getValue(request, "time"));
  }
  if (request.indexOf("trainNumber=") != -1) {
    trainNumber = urlDecode(getValue(request, "trainNumber"));
  }
  if (request.indexOf("trainName=") != -1) {
    trainName = urlDecode(getValue(request, "trainName"));
  }
  if (request.indexOf("trackNumber=") != -1) {
    trackNumber = urlDecode(getValue(request, "trackNumber"));
  }
  if (request.indexOf("infoText=") != -1) {
    infoText = urlDecode(getValue(request, "infoText"));
  }
  if (request.indexOf("routeInfo=") != -1) {
    routeInfo = urlDecode(getValue(request, "routeInfo"));
  }
}

// Hilfsfunktion zum Extrahieren eines Werts aus einer HTTP-Anfrage
String getValue(String data, String key) {
  int startIndex = data.indexOf(key + "=") + key.length() + 1; // Startindex des Werts nach dem Schlüssel
  int endIndex = data.indexOf("&", startIndex); // Endindex des Werts (bei nächstem '&' oder Leerzeichen)
  if (endIndex == -1) {
    endIndex = data.indexOf(" ", startIndex);
  }
  return data.substring(startIndex, endIndex); // Wert aus der Anfrage extrahieren
}

// Funktion zur URL-Dekodierung eines Strings
String urlDecode(String str) {
  String decoded = ""; // Dekodierter String
  char temp[] = "00"; // Temporäres Array zur Umwandlung von Hex in Char
  unsigned int len = str.length(); // Länge des Eingabestrings
  for (unsigned int i = 0; i < len; i++) {
    if (str[i] == '%') { // Wenn ein Prozentzeichen gefunden wird, dekodieren
      if (i + 2 < len) {
        temp[0] = str[i + 1]; // Erstes Hex-Zeichen
        temp[1] = str[i + 2]; // Zweites Hex-Zeichen
        decoded += (char) strtol(temp, NULL, 16); // Hex in Char umwandeln und zum dekodierten String hinzufügen
        i += 2; // Zwei zusätzliche Zeichen überspringen
      }
    } else if (str[i] == '+') {
      decoded += ' '; // Pluszeichen durch Leerzeichen ersetzen
    } else {
      decoded += str[i]; // Anderes Zeichen unverändert übernehmen
    }
  }
  return decoded; // Dekodierten String zurückgeben
}

// Funktion zur Parsierung der Dauer aus der HTTP-Anfrage
int parseDuration(String request) {
  int startIndex = request.indexOf("&duration="); // Startindex der Dauer in der Anfrage finden
  if (startIndex != -1) {
    int endIndex = request.indexOf(" ", startIndex); // Endindex der Dauer finden
    String durationString = request.substring(startIndex + 10, endIndex); // Dauer aus der Anfrage extrahieren
    return durationString.toInt(); // Dauer in eine Ganzzahl umwandeln und zurückgeben
  }
  return 0; // Wenn keine Dauer gefunden wird, 0 zurückgeben
}

// Funktion zur Steuerung der LED
void controlLED(LEDControl &led, int duration) {
  digitalWrite(led.pin, HIGH); // LED einschalten
  led.isOn = true; // LED-Status auf "an" setzen
  if (duration > 0) {
    led.endTime = millis() + duration * 1000; // Endzeit basierend auf der aktuellen Zeit und der Dauer berechnen
  } else {
    led.endTime = 0; // Wenn keine Dauer angegeben ist, Endzeit auf 0 setzen
  }
}

// Funktion zur Aktualisierung des LED-Status
void updateLED(LEDControl &led) {
  if (led.isOn && led.endTime > 0 && millis() > led.endTime) { // Überprüfen, ob die Endzeit erreicht ist
    digitalWrite(led.pin, LOW); // LED ausschalten
    led.isOn = false; // LED-Status auf "aus" setzen
    led.endTime = 0; // Endzeit zurücksetzen
  }
}

