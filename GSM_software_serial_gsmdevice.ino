#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <Wire.h>
#define si7021Addr 0x40
#include "bitmaps.h"

String apn = "APN";                    //APN
String apn_u = "";                     //APN-Username
String apn_p = "";                     //APN-Password
String url = "http ://www.infernoinfosec.in/test.php";  //URL of Server

SoftwareSerial SWserial(33, 25); // RX, TX
// These pins will also work for the 1.8" TFT shield

//ESP32-WROOM
#define TFT_DC 12 //A0
#define TFT_CS 13 //CS
#define TFT_MOSI 14 //SDA
#define TFT_CLK 27 //SCK
#define TFT_RST 0
#define TFT_MISO 0
// REST 3.3v

//sda 21 si7021
//scl 22

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);
String apiKeyValue = "tPmAT5Ab3j7F9";

void setup()
{
  Wire.begin();
  Wire.beginTransmission(si7021Addr);
  Wire.endTransmission();
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_WHITE);
  tft.drawBitmap(0, 50, logo, 128, 64, ST77XX_BLACK);
  Serial.begin(115200);
  Serial.println("SIM800 AT CMD Test");
  SWserial.begin(9600);
  delay(15000);
  while (SWserial.available()) {
    Serial.write(SWserial.read());
  }
  delay(2000);
  gsm_config_gprs();
}

void loop() {
  unsigned int data[2];
  Wire.beginTransmission(si7021Addr);
  //Send humidity measurement command
  Wire.write(0xF5);
  Wire.endTransmission();
  delay(500);

  // Request 2 bytes of data
  Wire.requestFrom(si7021Addr, 2);
  // Read 2 bytes of data to get humidity
  if (Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  // Convert the data
  float humidity  = ((data[0] * 256.0) + data[1]);
  humidity = ((125 * humidity) / 65536.0) - 6;

  Wire.beginTransmission(si7021Addr);
  // Send temperature measurement command
  Wire.write(0xF3);
  Wire.endTransmission();
  delay(500);

  // Request 2 bytes of data
  Wire.requestFrom(si7021Addr, 2);

  // Read 2 bytes of data for temperature
  if (Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  // Convert the data
  float temp  = ((data[0] * 256.0) + data[1]);
  float celsTemp = ((175.72 * temp) / 65536.0) - 46.85;
  float fahrTemp = celsTemp * 1.8 + 32;

  // Output data to serial monitor
  Serial.print("Humidity : ");
  Serial.print(humidity);
  Serial.println(" % RH");
  Serial.print("Celsius : ");
  Serial.print(celsTemp);
  Serial.println(" C");
  Serial.print("Fahrenheit : ");
  Serial.print(fahrTemp);
  Serial.println(" F");

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.setCursor(0, 60);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("Temp:");
  tft.print(celsTemp);
  tft.setCursor(0, 80);
  //  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("Hum:");
  tft.print(humidity);
  tft.setCursor(0, 150);
  //  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print("By CloudStakes");
  gsm_http_post("api_key=" + apiKeyValue + "&value1=" + String(celsTemp) + "&value2=" + String(humidity) + "&value3=" + String("100%") + "&value4=" + String("100%") + "&value5=" + String("01") + "&value6=" + String("0123456789") + "&value7=" + String("0123456789") + "&value8=" + String("Direct") + "&value9=" + String("ACTIVE") + "&value10=" + String("19:06") + "");
  delay(30000);
}

void gsm_http_post( String postdata) {
  Serial.println(" --- Start GPRS & HTTP --- ");
  gsm_send_serial("AT+SAPBR=1,1");
  gsm_send_serial("AT+SAPBR=2,1");
  gsm_send_serial("AT+HTTPINIT");
  gsm_send_serial("AT+HTTPPARA=CID,1");
  gsm_send_serial("AT+HTTPPARA=URL," + url);
  gsm_send_serial("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
  gsm_send_serial("AT+HTTPDATA=192,5000");
  gsm_send_serial(postdata);
  gsm_send_serial("AT+HTTPACTION=1");
  gsm_send_serial("AT+HTTPREAD");
  gsm_send_serial("AT+HTTPTERM");
  gsm_send_serial("AT+SAPBR=0,1");
}

void gsm_config_gprs() {
  Serial.println(" --- CONFIG GPRS --- ");
  gsm_send_serial("AT+SAPBR=3,1,Contype,GPRS");
  gsm_send_serial("AT+SAPBR=3,1,APN," + apn);
  if (apn_u != "") {
    gsm_send_serial("AT+SAPBR=3,1,USER," + apn_u);
  }
  if (apn_p != "") {
    gsm_send_serial("AT+SAPBR=3,1,PWD," + apn_p);
  }
}

void gsm_send_serial(String command) {
  Serial.println("Send ->: " + command);
  SWserial.println(command);
  long wtimer = millis();
  while (wtimer + 3000 > millis()) {
    while (SWserial.available()) {
      Serial.write(SWserial.read());
    }
  }
  Serial.println();
}
