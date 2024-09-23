/*********
  Fernanda Ratola, Luan Bravo, Amanda Joris, Betina.
  IFSC Florianopolis, Product Design 2023
  Adapted from Rui Santos' project. Details at https://randomnerdtutorials.com
*********/

// Load Wi-Fi library
#include <WiFi.h>

// Red, green, and blue pins for PWM control
#define RED_PIN 25
#define GREEN_PIN 26
#define BLUE_PIN 27

#define BUTTON_ON 22

// MELODY BLOCK
#define MELODY_PIN 23

// Set web server port number to 80
WiFiServer server(80);

// Decode HTTP GET value
String red_string = "0";
String green_string = "0";
String blue_string = "0";
int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int pos4 = 0;

// Variable to store the HTTP request
String header;

// Setting PWM frequency, channels and bit resolution
const int freqColor = 5000;
const int red_channel = 0;
const int green_channel = 1;
const int blue_channel = 2;
// Bit resolution 2^8 = 256
const int resolution = 8;

// Current time
unsigned long current_time = millis();
// Previous time
unsigned long previous_time = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeout_times = 2000;

int lamp_on = 0;
int button, button_prev;

int freqMelody = 2000;
int channel = 3;
//int resolution = 8;

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();

    // Wait for SmartConfig to complete
    while (!WiFi.smartConfigDone()) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nSmartConfig received.");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!\n");

    // configure LED PWM functionalitites
    /*
    * ledcSetup(red_channel, freqColor, resolution);
    * ledcSetup(green_channel, freqColor, resolution);
    * ledcSetup(blue_channel, freqColor, resolution);
    * // Configure BUZZER PWMs
    * ledcSetup(channel, freqMelody, resolution);
    */
    // attach the channel to the GPIO to be controlled
    ledcAttach(MELODY_PIN, freqMelody, resolution);
    ledcAttach(RED_PIN, freqColor, resolution);
    ledcAttach(GREEN_PIN, freqColor, resolution);
    ledcAttach(BLUE_PIN, freqColor, resolution);
    //Configure BUTTON_ON
    pinMode(BUTTON_ON, INPUT);
    server.begin();
    button_prev = HIGH;
}

void loop() {
    WiFiClient client = server.available();   // Listen for incoming clients

    if (client) {                             // If a new client connects,
        current_time = millis();
        previous_time = current_time;
        Serial.println("New Client.");          // print a message out in the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        while (client.connected() && current_time - previous_time <= timeout_times) {            // loop while the client's connected
            current_time = millis();
            if (client.available()) {             // if there's bytes to read from the client,
                char c = client.read();             // read a byte, then
                Serial.write(c);                    // print it out the serial monitor
                header += c;
                if (c == '\n') {                    // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();
                        // Display the HTML web page
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                        client.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">");
                        client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js\"></script>");
                        client.println("</head><body><div class=\"container\"><div class=\"row\"><h1>ESP Color Picker</h1></div>");
                        client.println("<a class=\"btn btn-primary btn-lg\" href=\"#\" id=\"change_color\" role=\"button\">Change Color</a> ");
                        client.println("<input class=\"jscolor {onFineChange:'update(this)'}\" id=\"rgb\"></div>");
                        client.println("<script>function update(picker) {document.getElementById('rgb').innerHTML = Math.round(picker.rgb[0]) + ', ' +  Math.round(picker.rgb[1]) + ', ' + Math.round(picker.rgb[2]);");
                        client.println("document.getElementById(\"change_color\").href=\"?r\" + Math.round(picker.rgb[0]) + \"g\" +  Math.round(picker.rgb[1]) + \"b\" + Math.round(picker.rgb[2]) + \"&\";}</script></body></html>");
                        // The HTTP response ends with another blank line
                        client.println();
                        // Request sample: /?r201g32b255&
                        // Red = 201 | Green = 32 | Blue = 255
                        if (header.indexOf("GET /?r") >= 0) {
                            pos1 = header.indexOf('r');
                            pos2 = header.indexOf('g');
                            pos3 = header.indexOf('b');
                            pos4 = header.indexOf('&');
                            red_string = header.substring(pos1+1, pos2);
                            green_string = header.substring(pos2+1, pos3);
                            blue_string = header.substring(pos3+1, pos4);
                            Serial.println(red_string.toInt());
                            Serial.println(green_string.toInt());
                            Serial.println(blue_string.toInt());
                            if (lamp_on = 1) {
                                ledcWrite(red_channel, red_string.toInt());
                                ledcWrite(green_channel, green_string.toInt());
                                ledcWrite(blue_channel, blue_string.toInt());
                            } else {
                                red_string = "0";
                                green_string = "0";
                                blue_string = "0";
                                ledcWrite(red_channel, red_string.toInt());
                                ledcWrite(green_channel, green_string.toInt());
                                ledcWrite(blue_channel, blue_string.toInt());
                             }
                        }
                        // Break out of the while loop
                        break;
                    } else { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                } else if (c != '\r') {  // if you got anything else but a carriage return character,
                    currentLine += c;      // add it to the end of the currentLine
                }
            }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
    }

    button = digitalRead(BUTTON_ON);
    if (button == LOW) {
        if (button_prev == HIGH) {
            button_prev = LOW;
            lamp_on = 1;
            red_string = "255";
            green_string = "255";
            blue_string = "255";
            ledcWrite(red_channel, red_string.toInt());
            ledcWrite(green_channel, green_string.toInt());
            ledcWrite(blue_channel, blue_string.toInt());
            singTheme();

        } else {
            button_prev = HIGH;
            delay(400);
            lamp_on = 0;
            red_string = "0";
            green_string = "0";
            blue_string = "0";
            ledcWrite(red_channel, red_string.toInt());
            ledcWrite(green_channel, green_string.toInt());
            ledcWrite(blue_channel, blue_string.toInt());
            singPipe();
        }
    }
}

void singPipe(void) {
    ledcAttach(MELODY_PIN, freqMelody, resolution);

    ledcWriteNote(channel, NOTE_E, 6);
    delay(100);
    ledcWriteNote(channel, NOTE_D, 6);
    delay(100);
    ledcWriteNote(channel, NOTE_C, 6);
    delay(100);
    ledcWriteNote(channel, NOTE_B, 5);
    delay(100);
    ledcWriteNote(channel, NOTE_A, 5);
    delay(100);

    delay(100);

    ledcWriteNote(channel, NOTE_E, 6);
    delay(100);
    ledcWriteNote(channel, NOTE_D, 6);
    delay(100);
    ledcWriteNote(channel, NOTE_C, 6);
    delay(100);
    ledcWriteNote(channel, NOTE_B, 5);
    delay(100);
    ledcWriteNote(channel, NOTE_A, 5);
    delay(100);

    delay(100);

    ledcWriteNote(channel, NOTE_E, 6);
    delay(100);
    ledcWriteNote(channel, NOTE_D, 6);
    delay(100);
    ledcWriteNote(channel, NOTE_C, 6);
    delay(100);
    ledcWriteNote(channel, NOTE_B, 5);
    delay(100);
    ledcWriteNote(channel, NOTE_A, 5);
    delay(100);

    ledcWriteTone(channel, 0);
    ledcDetach(MELODY_PIN);
}

void singTheme(void) {
    ledcAttach(MELODY_PIN, freqMelody, resolution);
    ledcWriteNote(channel, NOTE_E, 7);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);

    ledcWriteNote(channel, NOTE_E, 7);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteTone(channel, 0);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);

    ledcWriteNote(channel, NOTE_E, 7);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteTone(channel, 0);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteNote(channel, NOTE_C, 7);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);

    ledcWriteNote(channel, NOTE_E, 7);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteTone(channel, 0);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);

    ledcWriteNote(channel, NOTE_G, 7);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteTone(channel, 0);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteTone(channel, 0);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteTone(channel, 0);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);

    ledcWriteNote(channel, NOTE_G, 6);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteTone(channel, 0);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteTone(channel, 0);
    delay(100);
    ledcWriteTone(channel, 0);
    delay(30);
    ledcWriteTone(channel, 0);
    delay(3000);
    ledcDetach(MELODY_PIN);
}
