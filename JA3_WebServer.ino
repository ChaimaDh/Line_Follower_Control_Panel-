
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// ===== JA3 WIFI SETTINGS =====

const char* ssid = "JA3_Control";
const char* password = "ja3robot26";

// ===== WEB SERVER =====

WiFiServer server(80);

// ===== ROBOT SETTINGS =====

// PID values
float kp = 0;
float ki = 0;
float kd = 0;

// Speed values
int baseSpeed = 0;
int maxSpeed = 0;
int turnSpeed = 0;

// ===== ROBOT COMMANDS =====

bool robotRunning = false;
bool calibrationRequested = false;


// ===== SEND FILE FUNCTION =====

void sendFile(WiFiClient &client, const char* path,
              const char* contentType) {

    File file = LittleFS.open(path, "r");

    if (!file) {
        client.println("HTTP/1.1 404 Not Found");
        client.println("Connection: close");
        client.println();
        return;
    }

    client.println("HTTP/1.1 200 OK");
    client.print("Content-Type: ");
    client.println(contentType);
    client.println("Connection: close");
    client.println();

    while (file.available()) {
        client.write(file.read());
    }

    file.close();
}


// ===== SETUP =====

void setup() {

    Serial.begin(115200);

    if (!LittleFS.begin()) {
        Serial.println("LittleFS Mount Failed!");
        return;
    }

    Serial.println("LittleFS Mounted Successfully!");

    // Start Wi-Fi Access Point
    WiFi.softAP(ssid, password);

    // Start HTTP server
    server.begin();

    Serial.println("JA3 WiFi Started!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
}


// ===== MAIN LOOP =====

void loop() {

    WiFiClient client = server.available();

    if (!client) {
        return;
    }

    Serial.println("Client connected!");

    client.setTimeout(5000);


    // ===== 1. READ HTTP REQUEST =====

    String request = client.readStringUntil('\n');
    request.trim();

    Serial.println(request);


    // ===== 2. READ HTTP HEADERS =====

    int contentLength = 0;

    while (client.connected()) {

        String line = client.readStringUntil('\n');
        line.trim();

        if (line.length() == 0) {
            break;
        }

        if (line.startsWith("Content-Length:")) {
            contentLength = line.substring(15).toInt();
        }
    }

    Serial.print("Content Length: ");
    Serial.println(contentLength);


    // ===== 3. READ HTTP BODY =====

    String body = "";

    unsigned long lastDataTime = millis();

    while (body.length() < contentLength &&
           millis() - lastDataTime < 5000) {

        while (client.available() &&
               body.length() < contentLength) {

            body += (char)client.read();

            lastDataTime = millis();
        }

        delay(1);
    }


    // ===== 4. CHECK BODY COMPLETENESS =====

    if (contentLength > 0 &&
        body.length() != contentLength) {

        Serial.println("Incomplete request body!");

        client.println("HTTP/1.1 400 Bad Request");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("Incomplete request body!");

        client.stop();
        return;
    }

    Serial.println("Received body:");
    Serial.println(body);


    // ===== 5. HANDLE ROBOT SETTINGS =====

    if (request.startsWith("POST /settings")) {

        JsonDocument doc;

        DeserializationError error =
            deserializeJson(doc, body);

        if (error) {

            Serial.println("JSON parsing failed!");

            client.println("HTTP/1.1 400 Bad Request");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            client.println("Invalid JSON!");

        }

        else {

            // Check required fields and types
            bool valid =
                doc["pid"]["kp"].is<float>() &&
                doc["pid"]["ki"].is<float>() &&
                doc["pid"]["kd"].is<float>() &&
                doc["speed"]["base"].is<int>() &&
                doc["speed"]["max"].is<int>() &&
                doc["speed"]["turn"].is<int>();


            // Check speed limits
            if (valid) {

                int receivedBase = doc["speed"]["base"];
                int receivedMax = doc["speed"]["max"];
                int receivedTurn = doc["speed"]["turn"];

                if (receivedBase < 0 || receivedBase > 255 ||
                    receivedMax < 0 || receivedMax > 255 ||
                    receivedTurn < 0 || receivedTurn > 255 ||
                    receivedBase > receivedMax) {

                    valid = false;
                }
            }


            // Check PID limits
            if (valid) {

                float receivedKp = doc["pid"]["kp"];
                float receivedKi = doc["pid"]["ki"];
                float receivedKd = doc["pid"]["kd"];

                if (receivedKp < 0 || receivedKp > 1000 ||
                    receivedKi < 0 || receivedKi > 1000 ||
                    receivedKd < 0 || receivedKd > 1000) {

                    valid = false;
                }
            }


            // Accept or reject settings
            if (!valid) {

                Serial.println("Invalid settings!");

                client.println("HTTP/1.1 400 Bad Request");
                client.println("Content-Type: text/plain");
                client.println("Connection: close");
                client.println();
                client.println("Invalid settings!");

            }

            else {

                kp = doc["pid"]["kp"];
                ki = doc["pid"]["ki"];
                kd = doc["pid"]["kd"];

                baseSpeed = doc["speed"]["base"];
                maxSpeed = doc["speed"]["max"];
                turnSpeed = doc["speed"]["turn"];

                Serial.println("Settings received successfully!");

                Serial.println(kp);
                Serial.println(ki);
                Serial.println(kd);

                Serial.println(baseSpeed);
                Serial.println(maxSpeed);
                Serial.println(turnSpeed);

                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/plain");
                client.println("Connection: close");
                client.println();
                client.println("Settings received!");
            }
        }
    }


    // ===== 6. HANDLE ROBOT COMMANDS =====

    else if (request.startsWith("POST /command")) {

        JsonDocument doc;

        DeserializationError error =
            deserializeJson(doc, body);

        if (error) {

            client.println("HTTP/1.1 400 Bad Request");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            client.println("Invalid JSON!");

        }

        else {

            String action = doc["action"].as<String>();

            bool validAction = true;

            if (action == "start") {

                robotRunning = true;
                calibrationRequested = false;

            }

            else if (action == "stop") {

                robotRunning = false;
                calibrationRequested = false;

            }

            else if (action == "calibrate") {

                robotRunning = false;
                calibrationRequested = true;

            }

            else {

                validAction = false;

            }


            if (validAction) {

                Serial.print("Command received: ");
                Serial.println(action);

                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/plain");
                client.println("Connection: close");
                client.println();
                client.println("Command received!");

            }

            else {

                client.println("HTTP/1.1 400 Bad Request");
                client.println("Content-Type: text/plain");
                client.println("Connection: close");
                client.println();
                client.println("Unknown command!");

            }
        }
    }


    // ===== 7. SERVE WEBSITE =====

    else if (request.startsWith("GET / ")) {

        sendFile(client, "/index.html", "text/html");

    }

    else if (request.startsWith("GET /Style.css")) {

        sendFile(client, "/Style.css", "text/css");

    }

    else if (request.startsWith("GET /script.js")) {

        sendFile(client, "/script.js", "application/javascript");

    }

    else if (request.startsWith("GET /status")) {

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("connected");

    }


    // ===== 8. UNKNOWN REQUEST =====

    else {

        client.println("HTTP/1.1 404 Not Found");
        client.println("Connection: close");
        client.println();

    }


    // ===== 9. CLOSE CONNECTION =====

    delay(10);

    client.stop();

    Serial.println("Request completed!");
}
