#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <SD.h>
#include <Servo.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10
#define DELAY_MILLISECONDS (20000)
#define SEALEVELPRESSURE_HPA (1013.25)
#define NEUTRAL_SERVO_POSITION (90)
#define TARGET_ALTITUDE_METERS (120)
#define LAUNCH_CONDITION_ALTITUDE_METERS (3)

Adafruit_BMP3XX bmp;
File myFile;
Servo servo;

double launchAltitude;
double adjustedTargetAltitude;
double currentAltitude;
int servoPosition;
boolean ejectedParachute = false;

void setup() {
  Serial.begin(9600);
  servoPosition = NEUTRAL_SERVO_POSITION;
  servo.attach(9);
  servo.write(servoPosition);

  while (!Serial);

  if (!SD.begin(10)) {
    Serial.print("No SD Card");
    while (1);
  }


  if (!bmp.begin_I2C()) { 
    myFile = SD.open("LOGGER.TXT", FILE_WRITE);
    myFile.print(millis());
    myFile.println(" No BMP3 sensor");
    myFile.close(); 
  }
  
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_7);
  bmp.setOutputDataRate(BMP3_ODR_200_HZ);

  myFile = SD.open("ALTITUDE.TXT", FILE_WRITE);
  myFile.println("---------------------------------------------------------------");
  myFile.close();

  myFile = SD.open("LOGGER.TXT", FILE_WRITE);
  myFile.print(millis());
  myFile.println(" Started Logging");
  myFile.close();

  while(millis() < DELAY_MILLISECONDS) {
    logger(millis(), bmp.readAltitude(SEALEVELPRESSURE_HPA), bmp.pressure, servoPosition);
  } 

  servoPosition = servoPosition + 90;
  servo.write(servoPosition);

  myFile = SD.open("LOGGER.TXT", FILE_WRITE);
  myFile.print(millis());
  myFile.println(" LOADED");
  myFile.close();

  launchAltitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  adjustedTargetAltitude = launchAltitude + TARGET_ALTITUDE_METERS;
  currentAltitude = launchAltitude;

  myFile = SD.open("LOGGER.TXT", FILE_WRITE);
  myFile.print(launchAltitude);
  myFile.println(" Launch Altitude");
  myFile.close();

  //Launch condition
  while(currentAltitude - launchAltitude < LAUNCH_CONDITION_ALTITUDE_METERS) {
    logger(millis(), bmp.readAltitude(SEALEVELPRESSURE_HPA), bmp.pressure, servoPosition);
    currentAltitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  }
  
  myFile = SD.open("LOGGER.TXT", FILE_WRITE);
  myFile.print(millis());
  myFile.print(" ");
  myFile.println(" Launched");
  myFile.close();
  
}

void loop() {
  currentAltitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  if(!ejectedParachute && currentAltitude >= adjustedTargetAltitude) {
    servo.write(NEUTRAL_SERVO_POSITION);
    myFile = SD.open("LOGGER.TXT", FILE_WRITE);
    myFile.print(millis());
    myFile.println(" Ejected");
    myFile.close();
    ejectedParachute = true;
  }

  logger(millis(), bmp.readAltitude(SEALEVELPRESSURE_HPA), bmp.pressure, servoPosition);
}

void logger(unsigned long time, float altitude, float pressure, int servoPosition) {
  myFile = SD.open("ALTITUDE.TXT", FILE_WRITE);
  
  if (myFile) {
    myFile.print(time);
    myFile.print(" ");
    myFile.print(altitude);
    myFile.print(" ");
    myFile.print(pressure);
    myFile.print(" ");
    myFile.println(servoPosition); 
    myFile.close();
  } else {
    myFile.println("ERROR");
    myFile.close();
  }

}