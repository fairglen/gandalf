/* WiFi scan Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/soc.h"          // Disable brownout problems
#include "soc/rtc_cntl_reg.h" // Disable brownout problems
#include "driver/rtc_io.h"
#include <StringArray.h>
#include <SPIFFS.h>
#include <FS.h>

// CAMERA PINS
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASSWORD
#define FILE_PHOTO "/photo.jpg"

#define GPIO_GATE_PIN GPIO_NUM_12

IPAddress local(192, 168, 101, 111);
IPAddress gateway(192, 168, 101, 1);
IPAddress subnet(255, 255, 0, 0);

AsyncWebServer server(80);
int gateState;

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void connectWiFi()
{
    WiFi.config(local, gateway, subnet);
    WiFi.mode(WIFI_STA);
    WiFi.begin(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.printf("WiFi connection Failed!\n");
        return;
    }

    Serial.println("WiFi connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setupFileSystem()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        ESP.restart();
    }
    else
    {
        delay(500);
        Serial.println("SPIFFS mounted successfully");
    }
}

void setupCamera()
{
    // Turn-off the 'brownout detector'
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // OV2640 camera module
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 6;
    config.fb_count = 1;

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        ESP.restart();
    }
}

bool checkPhoto(fs::FS &fs)
{
    File f_pic = fs.open(FILE_PHOTO);
    unsigned int pic_sz = f_pic.size();
    return (pic_sz > 100);
}

void capturePhoto()
{
    camera_fb_t *fb = NULL; // pointer
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
        return;
    }

    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

    // Insert the data in the photo file
    if (!file)
    {
        Serial.println("Failed to open file in writing mode");
    }
    else
    {
        file.write(fb->buf, fb->len); // payload (image), payload length
        Serial.print("The picture has been saved in ");
        Serial.print(FILE_PHOTO);
        Serial.print(" - Size: ");
        Serial.print(file.size());
        Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);
}

void toggleGate()
{
    Serial.print("1GateState is : ");
    Serial.println(gateState);
    digitalWrite(GPIO_GATE_PIN, !digitalRead(GPIO_GATE_PIN));
    gateState = digitalRead(GPIO_GATE_PIN);
    Serial.print("2GateState is : ");
    Serial.println(gateState);
    delay(1000);
    digitalWrite(GPIO_GATE_PIN, !digitalRead(GPIO_GATE_PIN));
    gateState = digitalRead(GPIO_GATE_PIN);
    Serial.print("3GateState is : ");
    Serial.println(gateState);
}

void setupWebServer()
{
    server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request) {
        boolean hasPhoto = false;
        do
        {
            capturePhoto();
            hasPhoto = checkPhoto(SPIFFS);

        } while (!hasPhoto);
        request->send(SPIFFS, FILE_PHOTO, "image/jpg");
    });

    server.on("/toggle", HTTP_POST, [](AsyncWebServerRequest *request) {
        toggleGate();
        request->send(200, "text/plain", "Gate is:" + gateState);
    });

    server.onNotFound(notFound);

    server.begin();
}

void setupGateToggle()
{
    pinMode(GPIO_GATE_PIN, INPUT);
    gateState = digitalRead(GPIO_GATE_PIN);
    pinMode(GPIO_GATE_PIN, OUTPUT);
}
void setup()
{
    Serial.begin(115200);
    setupGateToggle();
    connectWiFi();
    setupFileSystem();
    setupCamera();
    setupWebServer();
}

void loop()
{
}