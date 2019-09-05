// Libraries for DHT sensor
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 3
#define DHTTYPE    DHT11     // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);

// Librariers and setup for real time clock
#include<Wire.h>
#include<RTClib.h>
RTC_DS1307 RTC;
void printDateTime(DateTime dateTime);

// SD card libraries and setup
#include <SPI.h>
#include <SD.h>
#define SDCardPin 4
#define dataFileName "data.csv"

// Pin assignments
#define SoilSensorPin A0
#define photoCellPin A1
#define delayBetweenRead 2000

#define timeBetweenReads 60000

void setup() {
  Serial.begin(9600);
  setupDHT();
  setupRTC();
  setupSDCard();
}

void setupSDCard() {
  Serial.print("Initializing SD card...");

  if (!SD.begin(SDCardPin)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  if (SD.exists(dataFileName)) {
    Serial.println("data.csv exists.");
  } else {
    // If the file doesn't exist yet we want to create it and set the first row to be column headers
    Serial.println("data.csv does not exist");
    createDataFile();
  }
}

void setupRTC() {
  Wire.begin();
  RTC.begin();
  // If the clock is currently running we want to set it, once set the module will keep time even if the arduino is unpowered.
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}

// This funcion just starts the sensor and prints out some details on the specific sensor
void setupDHT() {
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
}

void loop() {
  delay(timeBetweenReads);
  writeLineToDataFile();
}

String getOutputString() {
  String output;
  
  output += "Humidity (%): ";
  output += String(getHumidity(), 2);
  output += "\n";

  output += "Temperature (°C): ";
  output += String(getTemperature(), 2);
  output += "\n";

  output += "Light reading: ";
  output += String(getPhotocellReading());
  output += "\n";

  output += "Soil sensor reading: ";
  output += String(getSoilSensorReading());
  output += "\n";

  output += "Time: ";
  output += getTimeStamp();
  output += "\n";

  output += "=============================================";
  output += "\n";

  return output;
}

String getDataString() {
  String output;

  output += getTimeStamp();
  output += ',';
  output += getTemperature();
  output += ',';
  output += getHumidity();
  output += ',';
  output += getPhotocellReading();
  output += ',';
  output += getSoilSensorReading();

  return output;
}

void writeLineToDataFile() {
  String data = getDataString();
  File myFile = SD.open(dataFileName, FILE_WRITE);
  myFile.println(data);
  myFile.close();

  Serial.println(data);
}

String getTimeStamp() {
  String output;
  DateTime now = RTC.now(); 

  output += now.year();
  output += '/';
  output += now.month();
  output += '/';
  output += now.day();
  output += ' ';
  output += now.hour();
  output += ':';
  output += now.minute();
  output += ':';
  output += now.second();

  return output;
}

float getTemperature() {
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
    return -1;
  }
  else {
   return event.temperature;
  }
}

float getHumidity() {
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
    return -1;
  }
  else {
    return event.relative_humidity;
  }
}

/* The return value from this is an analog reading from the sensor, this will range from 0 - 1023
it is not practical to get a reading from a photocell that is in specific units so this reading is more
for seeeing the relative sunlight at one time compared to another. */
int getPhotocellReading() {
  return analogRead(photoCellPin);
}

/* The return value from this is an analog reading from the sensor, this will range from 0 - 1023
There is no practical units to give this reading in, a rough guide though would be: 
  0  ~300     dry soil
  300~700     humid soil
  700~950     in water
  */
int getSoilSensorReading() {
  return analogRead(SoilSensorPin);
}

void printDateTime(DateTime dateTime){
  Serial.print(dateTime.year(),DEC);
  Serial.print('/');
  Serial.print(dateTime.month(),DEC);
  Serial.print('/');
  Serial.print(dateTime.day(),DEC);
  Serial.print(' ');
  Serial.print(dateTime.hour(),DEC);
  Serial.print(':');
  Serial.print(dateTime.minute(),DEC);
  Serial.print(':');
  Serial.print(dateTime.second(),DEC);
  Serial.println();
}

void createDataFile() {
  Serial.println("Creating data.csv...");
  File myFile = SD.open(dataFileName, FILE_WRITE);
  myFile.println("Time, Temp, Humidity, Photocell, Soil");
  myFile.close();
}
