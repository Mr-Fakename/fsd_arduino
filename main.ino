#include <ArduinoBLE.h>
#include <Arduino_HS300x.h>

// Bluetooth Standard code for Environmental Sensing service
BLEService environmentService("181A");

// Bluetooth Standard code for 16-bit Temperature characteristic
BLEIntCharacteristic tempCharacteristic("2A6E", BLERead | BLENotify);

// Bluetooth Standard code for 16-bit Humidity characteristic
BLEUnsignedIntCharacteristic humidCharacteristic("2A6F", BLERead | BLENotify);

const int UPDATE_FREQUENCY = 5000;

int previousTemperature = 0;
unsigned int previousHumidity = 0;

long previousMillis = 0;

void setup() {
  Serial.begin(9600);

  // Set LED's pin to output mode
  pinMode(LED_BUILTIN, OUTPUT);

  // When the central disconnects, turn off the LED
  digitalWrite(LED_BUILTIN, LOW);

  // Begin initialization
  if (!BLE.begin()) {
    while (1);
  }
  if (!HS300x.begin()) {
    while (1);
  }

  // Set advertised local name and service UUID
  BLE.setLocalName("Nano 33 BLE Sense");

  // Advertise Environment service
  BLE.setAdvertisedService(environmentService);

  // Add temp and humidity characteristics to the service
  environmentService.addCharacteristic(tempCharacteristic);
  environmentService.addCharacteristic(humidCharacteristic);

  // Add service to BLE
  BLE.addService(environmentService);

  // Set characteristics' initial values
  tempCharacteristic.setValue(0);
  humidCharacteristic.setValue(0);

  // Start advertising
  BLE.advertise();
}

void loop() {
  // Listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // If a central is connected to peripheral:
  if (central) {
    digitalWrite(LED_BUILTIN, HIGH);  // turn on the LED to indicate the connection

    // While the central is still connected to peripheral:
    while (central.connected()) {

      long currentMillis = millis();
      // After UPDATE_FREQUENCY ms have passed, check temperature & humidity
      if (currentMillis - previousMillis >= UPDATE_FREQUENCY) {
        previousMillis = currentMillis;
        updateReadings();
      }
    }

    digitalWrite(LED_BUILTIN, LOW);  // when the central disconnects, turn off the LED
  }
}

int getTemperature() {
  // Get calibrated temperature as signed 16-bit int for BLE characteristic
  return (int) (HS300x.readTemperature() * 100);
}

unsigned int getHumidity() {
  // Get humidity as unsigned 16-bit int for BLE characteristic
  return (unsigned int) (HS300x.readHumidity() * 100);
}

void updateReadings() {
  int temperature = getTemperature();
  unsigned int humidity = getHumidity();

  // When values change, update and write the current values
  if (temperature != previousTemperature) {
    tempCharacteristic.writeValue(temperature);
    previousTemperature = temperature;
  }

  if (humidity != previousHumidity) {
    humidCharacteristic.writeValue(humidity);
    previousHumidity = humidity;
  }
}