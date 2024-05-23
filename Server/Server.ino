#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID              "9d537c81-4b1f-406c-b12f-a9aa49af6332"
#define CHARACTERISTIC_LEFT_UUID  "ac61fa72-e2de-42fb-9605-d0c7549b1c39"
#define CHARACTERISTIC_RIGHT_UUID "b3f4eb92-9ceb-4317-90ed-373a36164d2b"

BLEServer *pServer;
BLEService *pService;

BLECharacteristic *pCharacteristicLeft;
BLECharacteristic *pCharacteristicRight;

int deviceConnected = 0;

class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) 
    {
        deviceConnected++;
        BLEDevice::startAdvertising();
        Serial.print("Device connected. Now there are ");
        Serial.print(deviceConnected);
        Serial.println(" devices connected.");
    };

    void onDisconnect(BLEServer* pServer) 
    {
        deviceConnected--;
        pServer->startAdvertising(); // restart advertising
        Serial.print("Device disconnected. Now there are ");
        Serial.print(deviceConnected);
        Serial.println(" devices connected.");
    };
};

class CharacteristicChangeCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      std::string key = pCharacteristic->getUUID().toString();

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
  Serial.begin(115200);
  Serial.println("BLE Server has started");

  BLEDevice::init("Proto Server :3");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  pService = pServer->createService(SERVICE_UUID);

  CharacteristicChangeCallbacks *callbacks = new CharacteristicChangeCallbacks();

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

  pCharacteristicLeft->setCallbacks(callbacks);
  pCharacteristicRight->setCallbacks(callbacks);

  pCharacteristicLeft->setValue("0");
  pCharacteristicRight->setValue("0");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void loop() {
  delay(2000);
}
