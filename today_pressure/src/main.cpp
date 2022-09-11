#include <Arduino.h>

// 网络
#ifdef ESP32

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <U8g2lib.h>

#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include "images.h"

#define gpio_pressure D10

// 初始化OLED
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R1, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/
                                         U8X8_PIN_NONE);   // All Boards without Reset of the Display
// 初始化web服务器
AsyncWebServer server(80);



const char *ssid = "ormissia-tp";
const char *password = "1234qweASD";

const char *PARAM_VOLTAGE = "pressure";
const char *IP = "192.168.13.9";


int progress = 0;


void initPinMode();

void initOLED();

void initWifi();

void serverUp();


void setup() {
    // 设置串口
    Serial.begin(115200);
    // 设置引脚
    initPinMode();
    // 设置屏幕
    initOLED();
    // 设置Wifi
    initWifi();
    // 设置服务
    serverUp();

    Serial.printf("setup end!\n");
}

void loop() {
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_wqy13_t_gb2312b);
        u8g2.drawXBM(0, 15, 64, 23, bitmap_progress);
        u8g2.drawBox(2, 27, progress, 9);

        u8g2.setCursor(7, 60);
        u8g2.print("今日压力");

        u8g2.drawXBM(20, 80, 24, 24, bitmap_jira);

    } while (u8g2.nextPage());
}

void initPinMode() {
    pinMode(gpio_pressure, OUTPUT);
}

void initOLED() {
    u8g2.enableUTF8Print();
    u8g2.setColorIndex(1);
    u8g2.begin();
}

void initWifi() {
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    // IP=WiFi.localIP().toString().c_str();
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void serverUp() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "success");
    });

    server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request) {
        String message;
        if (request->hasParam(PARAM_VOLTAGE)) {
            message = request->getParam(PARAM_VOLTAGE)->value();
            int voltage = message.toInt();
            analogWrite(gpio_pressure, voltage);

            int  progressTemp = voltage*3;
            if (progressTemp <= 60) {
                progress = progressTemp;
            } else {
                progress = 60;
            }
        } else {
            message = "No message sent";
        }
        message = "ok " + message;
        // displayString(message);
        request->send(200, "text/plain", message);
    });

    server.onNotFound(notFound);

    server.begin();
}
