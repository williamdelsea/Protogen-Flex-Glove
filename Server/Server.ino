#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


// Service ID, should be the same as the client so the client knows which device to connect to.
#define SERVICE_UUID              "9d537c81-4b1f-406c-b12f-a9aa49af6333"

// Keys for the clients to update, left client should use the CHARACTERISTIC_LEFT_UUID 
// to set values and the right client should use CHARACTERISTIC_RIGHT_UUID
#define CHARACTERISTIC_LEFT_UUID  "ac61fa72-e2de-42fb-9605-d0c7549b1c39"
#define CHARACTERISTIC_RIGHT_UUID "b3f4eb92-9ceb-4317-90ed-373a36164d2b"

#define CHARACTERISTIC_LEFT_BATTERY  "1a703249-0446-448b-98d4-980a1d10da21"
#define CHARACTERISITC_RIGHT_BATTERY "2de2e09d-5ccd-4341-a148-eb7422401c98"

// https://www.uuidgenerator.net/

// The bluetooth low energy server and its default service.
// See https://learn.adafruit.com/introduction-to-bluetooth-low-energy/gatt#services-and-characteristics-640989
BLEServer *pServer;
BLEService *pService;

// Characteristic definitions for the left and right state values.
BLECharacteristic *pCharacteristicLeft;
BLECharacteristic *pCharacteristicRight;
BLECharacteristic *pCharacteristicLeftBatt;
BLECharacteristic *pCharacteristicRightBatt;

// Number of devices currently connected, used for logging.
int deviceConnected = 0;

int maxDevices = 2;

String flexValueLeft, flexValueRight;

float leftBattPercent, rightBattPercent;

bool leftConnected, rightConnected;

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// 'paw_connected', 16x16px
const unsigned char b_paw_connected [] PROGMEM = {
	0x0c, 0x30, 0x1e, 0x78, 0x1e, 0x78, 0x0e, 0x70, 0x60, 0x06, 0xf0, 0x0f, 0xf0, 0x0f, 0x73, 0xce, 
	0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x1c, 0x38
};
// 'paw_disconnected', 16x16px
const unsigned char b_paw_disconnected [] PROGMEM = {
	0x0c, 0x30, 0x12, 0x48, 0x12, 0x48, 0x0e, 0x70, 0x60, 0x06, 0x90, 0x09, 0x90, 0x09, 0x73, 0xce, 
	0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x23, 0xc4, 0x1c, 0x38
};

// Extends the server callbacks class to define code that runs on certain events in the bluetooth server.
class ServerCallbacks: public BLEServerCallbacks {
    // Runs whenever a new device connects to the server.
    void onConnect(BLEServer* pServer) 
    {
        deviceConnected++;
        // By default the server stops advertising when a device connects, the below line makes it continue advertising and hence allow multiple connections.
        // if there is less than the max amount of devices connecting, the serve will continue to advertise
        if (deviceConnected < maxDevices) BLEDevice::startAdvertising();

        // Debug messages
        Serial.print("Device connected. Now there are ");
        Serial.print(deviceConnected);
        Serial.println(" devices connected.");
    };

    void onDisconnect(BLEServer* pServer) 
    {
        deviceConnected--;
        pServer->startAdvertising(); // restart advertising
        leftConnected = false;
        rightConnected = false;
        // Rebug messages
        Serial.print("Device disconnected. Now there are ");
        Serial.print(deviceConnected);
        Serial.println(" devices connected.");
    };
};

// Extends the characteristics callbacks to define code that runs on certain events to a defined characteristic
class CharacteristicChangeCallbacks: public BLECharacteristicCallbacks {

    // Whenever this characteristic is written to by a client, and the value is updated
    void onWrite(BLECharacteristic *pCharacteristic) {

      // Get the key value pair, the key is one of the characteristic UUIDs defined earlier
      std::string key = pCharacteristic->getUUID().toString();
      std::string value = pCharacteristic->getValue();
      String keyString = key.c_str();

      if (keyString.equals("ac61fa72-e2de-42fb-9605-d0c7549b1c39") || keyString.equals("1a703249-0446-448b-98d4-980a1d10da21")) { // left UUID
        leftConnected = true;
      } else if (keyString.equals("b3f4eb92-9ceb-4317-90ed-373a36164d2b") || keyString.equals("2de2e09d-5ccd-4341-a148-eb7422401c98")) { // right UUID
        rightConnected = true;
      }
      // Debug messages
      if (value.length() > 0) {
        Serial.println("*********");

        Serial.print("Key:   ");
        for (int i = 0; i < key.length(); i++) Serial.print(key[i]);
        Serial.println();

        Serial.print("Value: ");
        for (int i = 0; i < value.length(); i++) Serial.print(value[i]);
        Serial.println();

        Serial.println("*********");
      }
    }

    void onNotify(BLECharacteristic* pCharacteristic) {
      std::string key = pCharacteristic->getUUID().toString();
      
      display.display();
    }
};

void setup() {
  // Starts serial logging
  Serial.begin(115200);
  Serial.println("BLE Server has started");


  // Initializes the server with the discoverable name of "Proto Server :3"
  BLEDevice::init("Proto Server :3");
  pServer = BLEDevice::createServer();

  // Tells the server to use the callbacks we defined above
  pServer->setCallbacks(new ServerCallbacks());

  // Creates a service within our server using the SERVICE_UUID
  pService = pServer->createService(SERVICE_UUID);

  // Creates the characteristics for left and right using the UUIDs defined above, and allows them to be both read and written to
  pCharacteristicLeft = pService->createCharacteristic(
    CHARACTERISTIC_LEFT_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristicRight = pService->createCharacteristic(
    CHARACTERISTIC_RIGHT_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristicLeftBatt = pService->createCharacteristic(
    CHARACTERISTIC_LEFT_BATTERY,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristicRightBatt = pService->createCharacteristic(
    CHARACTERISITC_RIGHT_BATTERY,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  // Tells both characteristics to use the same callback methods we defined above to listen for changes
  CharacteristicChangeCallbacks *callbacks = new CharacteristicChangeCallbacks();
  pCharacteristicLeft->setCallbacks(callbacks);
  pCharacteristicRight->setCallbacks(callbacks);
  pCharacteristicLeftBatt->setCallbacks(callbacks);
  pCharacteristicRightBatt->setCallbacks(callbacks);

  // Sets the default value for each characteristic
  pCharacteristicLeft->setValue("0");
  pCharacteristicRight->setValue("0");
  pCharacteristicLeftBatt->setValue("0");
  pCharacteristicRightBatt->setValue("0");

  // Starts the service, however no client can connect until the device starts advertising
  pService->start();

  // Starts advertising to other devices, modifing the default advertising settings
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  pinMode(19, OUTPUT);
  pinMode(32, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.setTextColor(WHITE);
  
  display.display();
  display.clearDisplay();
}

void loop() {
  
  leftBattPercent = getBatteryPercent(pCharacteristicLeftBatt);
  rightBattPercent = getBatteryPercent(pCharacteristicRightBatt);

  flexValueLeft = pCharacteristicLeft->getValue().c_str();
  flexValueRight = pCharacteristicRight->getValue().c_str();
  analogWrite(19, map(flexValueRight.toInt(), 0, 7, 0, 255));
  analogWrite(32, map(flexValueLeft.toInt(), 0, 7, 0, 255));

  displayInfo();
  delay(500); // Keeps the server running
  display.clearDisplay();
}

float getBatteryPercent(BLECharacteristic* c) {
  String batteryPercent = c->getValue().c_str();
  return mapfloat(batteryPercent.toFloat(), 3.6, 4.2, 0.0, 100.0);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void displayInfo() {
  //display.drawBitmap();
  (rightConnected) ? display.drawBitmap(111, 0, b_paw_connected, 16, 16, WHITE) : display.drawBitmap(111, 0, b_paw_disconnected, 16, 16, WHITE);
  (leftConnected) ? display.drawBitmap(91, 0, b_paw_connected, 16, 16, WHITE) : display.drawBitmap(91, 0, b_paw_disconnected, 16, 16, WHITE);
  display.setCursor(91, 20);
  display.print(flexValueLeft.toInt());
  display.setCursor(91, 28);
  display.print(max(0, (int)leftBattPercent));
  display.println("%");

  display.setCursor(111, 20);
  display.print(flexValueRight.toInt());
  display.setCursor(111, 28);
  display.print(max(0, (int)rightBattPercent));
  display.print("%");

  display.setCursor(0, 0);
  display.print(pCharacteristic)
  display.display();
}
