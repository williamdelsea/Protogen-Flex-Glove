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

// https://www.uuidgenerator.net/

uint8_t rightADDR[] = {0x64, 0xE8, 0x33, 0x00, 0xFC, 0x3E};
uint8_t leftADDR[] = {0xD4, 0xF9, 0x8D, 0x04, 0x1D, 0xB6};


// The bluetooth low energy server and its default service.
// See https://learn.adafruit.com/introduction-to-bluetooth-low-energy/gatt#services-and-characteristics-640989
BLEServer *pServer;
BLEService *pService;

// Characteristic definitions for the left and right state values.
BLECharacteristic *pCharacteristicLeft;
BLECharacteristic *pCharacteristicRight;

// Number of devices currently connected, used for logging.
int deviceConnected = 0;

int maxDevices = 2;

int flexValueLeft, flexValueRight;

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
    void onConnect(BLEServer* pServer,  esp_ble_gatts_cb_param_t *param) 
    {
        deviceConnected++;
        Serial.print("Serial address: ");
        Serial.println(BLEAddress(param->connect.remote_bda).toString().c_str());
        if (BLEAddress(param->connect.remote_bda).equals(leftADDR)) {
          Serial.println("left");
          leftConnected = true;
        } else if (BLEAddress(param->connect.remote_bda).equals(rightADDR)) {
          Serial.println("right");
          rightConnected = true;
        }
        // By default the server stops advertising when a device connects, the below line makes it continue advertising and hence allow multiple connections.
        // if there is less than the max amount of devices connecting, the serve will continue to advertise
        if (deviceConnected < maxDevices) BLEDevice::startAdvertising();

        // Debug messages
        Serial.print("Device connected. Now there are ");
        Serial.print(deviceConnected);
        Serial.println(" devices connected.");
    };

    void onDisconnect(BLEServer* pServer,  esp_ble_gatts_cb_param_t *param) 
    {
        deviceConnected--;
        Serial.print("Serial address: ");
        Serial.println(BLEAddress(param->connect.remote_bda).toString().c_str());

        // it takes a couple seconds for the server to register that a device has disconnected 
        if (BLEAddress(param->connect.remote_bda).equals(leftADDR)) {
          Serial.println("left");
          pCharacteristicLeft->setValue("0");
          leftConnected = false;
        } else if (BLEAddress(param->connect.remote_bda).equals(rightADDR)) {
          Serial.println("right");
          pCharacteristicRight->setValue("0");
          rightConnected = false;
        }

        pServer->startAdvertising(); // restart advertising
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

  // Tells both characteristics to use the same callback methods we defined above to listen for changes
  CharacteristicChangeCallbacks *callbacks = new CharacteristicChangeCallbacks();
  pCharacteristicLeft->setCallbacks(callbacks);
  pCharacteristicRight->setCallbacks(callbacks);

  // Sets the default value for each characteristic
  pCharacteristicLeft->setValue("0");
  pCharacteristicRight->setValue("0");

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

  flexValueLeft = (int)pCharacteristicLeft->getValue().c_str();
  flexValueRight = (int)pCharacteristicRight->getValue().c_str();

  displayInfo();
  delay(100); // Keeps the server running
  display.clearDisplay();
}

void displayInfo() {
  (rightConnected) ? display.drawBitmap(111, 0, b_paw_connected, 16, 16, WHITE) : display.drawBitmap(111, 0, b_paw_disconnected, 16, 16, WHITE);
  (leftConnected) ? display.drawBitmap(91, 0, b_paw_connected, 16, 16, WHITE) : display.drawBitmap(91, 0, b_paw_disconnected, 16, 16, WHITE);
  display.setCursor(91, 20);
  display.print(flexValueLeft);

  display.setCursor(111, 20);
  display.print(flexValueRight);

  display.setCursor(0, 0);
  display.display();
}
