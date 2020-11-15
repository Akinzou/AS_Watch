#include <Arduino.h>
#include <Wire.h>   
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "AS_WatchV1.h"
#include <Adafruit_BME280.h>
#include <Arduino-MAX17055.h>

MAX17055 bat;
Adafruit_BME280 bme;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
int hall, temp, pressure, humidity, altidute, soc, pressDuration;
int SEALEVELPRESSURE_HPA = 1013;
int lastState = LOW;
int currentState;
int pressedTime  = 0;
int releasedTime = 0;


void ActualizeSensors(void * parameter)
{
  for(;;){
    soc = bat.getSOC();
    hall = hallRead();   
    temp = bme.readTemperature();
    pressure = bme.readPressure();
    humidity = bme.readHumidity();
    altidute = bme.readAltitude(SEALEVELPRESSURE_HPA);
    vTaskDelay(750 / portTICK_PERIOD_MS);
  }
}

void CheckButton(void * parameter)
{
  for(;;){
    currentState = digitalRead(PanicButton);
    if(lastState == HIGH && currentState == LOW)
    {
      pressedTime = millis();
    }
    else if(lastState == LOW && currentState == HIGH)
    {
      releasedTime = millis();
      pressDuration = releasedTime - pressedTime;
      Serial.println(pressDuration);
        if( pressDuration >= 3000 && pressDuration <= 4000 )
        {
          Serial.println("Go to deep sleep");
          esp_deep_sleep_start();
        }
    }
    lastState = currentState;
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void setup() {
  bat.setCapacity(1200);
  Serial.begin(9600);
  Serial.println("WakeUp");
  bme.begin();
  pinMode(PanicButton, INPUT);
  pinMode(LCD_Switch, OUTPUT);
  //digitalWrite(LCD_Switch, HIGH);
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(3);

  xTaskCreate
  (
  CheckButton,    // Function that should be called
  "CheckButton",   // Name of the task (for debugging)
  1000,            // Stack size (bytes)
  NULL,            // Parameter to pass
  2,               // Task priority
  NULL             // Task handle
  );

  xTaskCreate
  (
  ActualizeSensors,    // Function that should be called
  "ActualizeSensors",   // Name of the task (for debugging)
  2000,            // Stack size (bytes)
  NULL,            // Parameter to pass
  1,               // Task priority
  NULL             // Task handle
  );

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, 0);
  
}

void loop() {

  Serial.println(voltage);
  Serial.println(temp);
  Serial.println(soc);
  tft.fillRect(0, 0, 60, 17, ILI9341_BLACK);         //Hall
  tft.fillRect(70, 40, 180, 55, ILI9341_BLACK);      //Hour
  tft.fillRect(270, 0, 320, 17, ILI9341_BLACK);      //battery %
  tft.fillRect(230, 220, 320, 240, ILI9341_BLACK);   //Accel
  tft.fillRect(0, 130, 135, 230, ILI9341_BLACK);     //heigh & temp & pressure & humidity


  tft.setCursor(70, 45);      //Hour
  tft.setTextSize(6);
  tft.println("14:10"); 

  tft.setTextSize(2);         //Battery %
  tft.setCursor(270, 0);
  tft.print(soc);
  tft.println("%");

  tft.setTextSize(2);
  tft.setCursor(230, 225);    //Accel
  tft.print("16");
  tft.println("m/s^2");

  tft.setTextSize(2);         //Hall
  tft.setCursor(0, 0);
  tft.print(hall);
  tft.setTextSize(1);
  tft.println("*");

  tft.setTextSize(2);        //altidute
  tft.setCursor(0, 135); 
  tft.print(altidute);
  tft.println("m a.s.l");

  tft.setTextSize(2);        //Temp
  tft.setCursor(0, 165); 
  tft.print(temp);
  tft.setTextSize(1);
  tft.print("*");
  tft.setTextSize(2);
  tft.println("C");

  tft.setTextSize(2);        //Pressure
  tft.setCursor(0, 195); 
  tft.print(pressure/100);
  tft.println("hPa");

  tft.setTextSize(2);        //Humidity
  tft.setCursor(0, 225); 
  tft.print(humidity);
  tft.println("%");
  

  delay(300);
}