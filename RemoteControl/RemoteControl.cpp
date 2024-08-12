
#include "Arduino.h"
#include "SD_MMC.h"
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDMouse.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "pin_config.h"

#include "html.hpp"
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

USBHIDKeyboard Keyboard;
USBHIDMouse mouse;
const char *ssid = "WS_KBD_1";
const char *password = "WSKEYBOARD";

const char *PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

const int buttonPin = 0;        // input pin for pushbutton
int previousButtonState = HIGH; // for checking the state of a pushButton
int counter = 0;                // button push counter

void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    //data[len] = 0;
    //String message = (char*)data;
    // Check if the message is "getReadings"
    //if (strcmp((char*)data, "getReadings") == 0) {
      //if it is, send current sensor readings
      String sensorReadings = "FUUUU";
    //   Serial.print(sensorReadings);
      notifyClients(sensorReadings);
    //}
  }
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
    //   Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
    //   Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}


void setup()
{
    // make the pushButton pin an input:
    pinMode(buttonPin, INPUT_PULLUP);
    // initialize control over the keyboard:
    Keyboard.begin();
    mouse.begin();
    USB.begin();

    SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
    SD_MMC.begin();

    // WiFi.mode(WIFI_STA);
    // WiFi.begin(ssid, password);
    // while (WiFi.status() != WL_CONNECTED) {
    //     Serial.print('.');
    //     delay(1000);
    // }
    // Serial.println(WiFi.localIP());
    WiFi.softAP(ssid, password);

    MDNS.begin("esp_remote");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", html);
    });

    server.on("/kbd_print", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("data", true)) {
            Keyboard.print(request->getParam("data", true)->value());
            request->send(200);
        }  else {
            request->send(400);
        }
        
    });
    server.on("/kbd_press", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("key", true)) {
            Keyboard.press(request->getParam("key", true)->value().toInt());
            request->send(200);
        }  else {
            request->send(400);
        }
        
    });
    server.on("/kbd_release", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("key", true)) {
            Keyboard.release(request->getParam("key", true)->value().toInt());
            request->send(200);
        }  else {
            request->send(400);
        }
    });
    server.on("/mouse_rel_move", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("x", true) && request->hasParam("y", true)) {
            auto x = request->getParam("x", true)->value().toInt();
            auto y = request->getParam("y", true)->value().toInt();
            mouse.move(x, y);
            request->send(200);
        }  else {
            request->send(400);
        }
        
    });
    server.on("/mouse_click", HTTP_POST, [](AsyncWebServerRequest *request){
        auto btn = MOUSE_LEFT;
        if(request->hasParam("left", true)) btn = MOUSE_RIGHT;
        mouse.click(btn);
        request->send(200);
    });

    server.on("/sd_info", HTTP_GET, [](AsyncWebServerRequest * request) {
        uint8_t cardType = SD_MMC.cardType();

        if (cardType == CARD_NONE) {
            request->send(200, "text/html", "No SD_MMC card attached");
            return;
        }

        String str;
        str = "SD_MMC Card Type: ";
        if (cardType == CARD_MMC) {
            str += "MMC";
        } else if (cardType == CARD_SD) {
            str += "SD_MMCSC";
        } else if (cardType == CARD_SDHC) {
            str += "SD_MMCHC";
        } else {
            str += "UNKNOWN";
        }

        uint32_t cardSize = SD_MMC.cardSize() / (1024 * 1024);

        str += "\nSD_MMC Card Size: ";
        str += cardSize;

        str += "\nTotal space: ";
        str += uint32_t(SD_MMC.totalBytes() / (1024 * 1024));
        str += "MB";

        str += "\nUsed space: ";
        str += uint32_t(SD_MMC.usedBytes() / (1024 * 1024));
        str += "MB";
        request->send(200, "text/html", str);
    });


    

    server.onNotFound(notFound);

    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.begin();

}

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

void loop()
{
    // read the pushbutton:
    int buttonState = digitalRead(buttonPin);
    // if the button state has changed,
    if ((buttonState != previousButtonState)
        // and it's currently pressed:
        && (buttonState == LOW))
    {
        // increment the button counter
        counter++;
        // type out a message
        Keyboard.print("You pressed the button ");
        Keyboard.print(counter);
        Keyboard.println(" times.");
    }
    // save the current button state for comparison next time:
    previousButtonState = buttonState;

    if ((millis() - lastTime) > timerDelay) {
        String sensorReadings = "FUUU";
        // Serial.print(sensorReadings);
        notifyClients(sensorReadings);
        lastTime = millis();
    }
    ws.cleanupClients();

}
