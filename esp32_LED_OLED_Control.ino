#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LED Pins
const int ledRed = 12;
const int ledGreen = 14;
const int ledYellow = 27;

// Access Point SSID und Passwort
const char* ssid = "ESP32_AP";
const char* password = "12345678";

// Webserver auf Port 80
WiFiServer server(80);

// Display information
String currentTime = "11:02";
String trainNumber = "506";
String trainName = "ICE";
String trackNumber = "11";
String infoText = "+++ Zug verspaetung ca 60min +++";
String routeInfo = "Basel Sbb - Frankfurt - Berlin HBF";
int x1, x2, minX1, minX2;

void setup() {
  Serial.begin(115200);

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Initialize LED pins
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledYellow, LOW);

  // Start Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point gestartet");

  // Print IP Address
  Serial.print("IP Adresse: ");
  Serial.println(WiFi.softAPIP());

  // Start Web Server
  server.begin();

  // Initialize scroll positions
  display.setTextWrap(false);
  x1 = display.width();
  x2 = display.width();

  // Display initial static train information
  displayStaticInfo();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Neuer Client verbunden");
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // Parse the HTTP request and control LED
    if (request.indexOf("/LED=R_ON") != -1) {
      digitalWrite(ledRed, HIGH);
    }
    if (request.indexOf("/LED=R_OFF") != -1) {
      digitalWrite(ledRed, LOW);
    }
    if (request.indexOf("/LED=G_ON") != -1) {
      digitalWrite(ledGreen, HIGH);
    }
    if (request.indexOf("/LED=G_OFF") != -1) {
      digitalWrite(ledGreen, LOW);
    }
    if (request.indexOf("/LED=Y_ON") != -1) {
      digitalWrite(ledYellow, HIGH);
    }
    if (request.indexOf("/LED=Y_OFF") != -1) {
      digitalWrite(ledYellow, LOW);
    }

    // Update display information based on HTTP request
    if (request.indexOf("updateDisplay") != -1) {
      updateDisplayInfo(request);
    }

    // Send response to client
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<p>LED Steuerung</p>");
    client.println("</html>");
    client.stop();
    Serial.println("Client Verbindung geschlossen");
  }

  // Update OLED display with scrolling text
  display.clearDisplay();

  // Static information
  displayStaticInfo();

  // Scrolling text
  scrollText(infoText, x1, 10); // Adjust position to avoid overlap
  scrollText(routeInfo, x2, 25); // Adjust position to avoid overlap

  display.display();
  delay(50); // Adjust scrolling speed
}

void displayStaticInfo() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Time and train number
  display.setCursor(0, 0);
  display.print(currentTime);
  display.setCursor(35, 00);
  display.print(trainName + " " + trainNumber);

  // Track
  display.setCursor(95, 0);
  display.print("Gl.");
  display.println(trackNumber);
}

void scrollText(String message, int &x, int y) {
  minX1 = -12 * message.length(); // 12 = 6 pixels/character * text size 2
  display.setTextSize(1); // Use the same text size as in the static info
  display.setCursor(x, y);
  display.print(message);
  x = x - 2; // Scroll speed
  if (x < minX1) x = display.width();
}

void updateDisplayInfo(String request) {
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

String getValue(String data, String key) {
  int startIndex = data.indexOf(key + "=") + key.length() + 1;
  int endIndex = data.indexOf("&", startIndex);
  if (endIndex == -1) {
    endIndex = data.indexOf(" ", startIndex);
  }
  return data.substring(startIndex, endIndex);
}

String urlDecode(String str) {
  String decoded = "";
  char temp[] = "00";
  unsigned int len = str.length();
  for (unsigned int i = 0; i < len; i++) {
    if (str[i] == '%') {
      if (i + 2 < len) {
        temp[0] = str[i + 1];
        temp[1] = str[i + 2];
        decoded += (char) strtol(temp, NULL, 16);
        i += 2;
      }
    } else if (str[i] == '+') {
      decoded += ' ';
    } else {
      decoded += str[i];
    }
  }
  return decoded;
}

