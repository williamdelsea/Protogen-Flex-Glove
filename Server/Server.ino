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

String flexValue;

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

  pinMode(12, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
}

void loop() {
  flexValue = pCharacteristicLeft->getValue().c_str();
  switch (flexValue.toInt()) {
    case 0:
      digitalWrite(12, LOW);
      digitalWrite(27, LOW);
      digitalWrite(25, LOW);
      digitalWrite(33, LOW);
      digitalWrite(32, LOW);
      digitalWrite(22, LOW);
      digitalWrite(23, LOW);
      break;
    case 1:
      digitalWrite(12, HIGH);
      digitalWrite(27, LOW);
      digitalWrite(25, LOW);
      digitalWrite(33, LOW);
      digitalWrite(32, LOW);
      digitalWrite(22, LOW);
      digitalWrite(23, LOW);
      break;
    case 2:
      digitalWrite(12, LOW);
      digitalWrite(27, HIGH);
      digitalWrite(25, LOW);
      digitalWrite(33, LOW);
      digitalWrite(32, LOW);
      digitalWrite(22, LOW);
      digitalWrite(23, LOW);
      break;
    case 3:
      digitalWrite(12, LOW);
      digitalWrite(27, LOW);
      digitalWrite(25, HIGH);
      digitalWrite(33, LOW);
      digitalWrite(32, LOW);
      digitalWrite(22, LOW);
      digitalWrite(23, LOW);
      break;
    case 4:
      digitalWrite(12, LOW);
      digitalWrite(27, LOW);
      digitalWrite(25, LOW);
      digitalWrite(33, HIGH);
      digitalWrite(32, LOW);
      digitalWrite(22, LOW);
      digitalWrite(23, LOW);
      break;
    case 5:
      digitalWrite(12, LOW);
      digitalWrite(27, LOW);
      digitalWrite(25, LOW);
      digitalWrite(33, LOW);
      digitalWrite(32, HIGH);
      digitalWrite(22, LOW);
      digitalWrite(23, LOW);
      break;
    case 6:
      digitalWrite(12, LOW);
      digitalWrite(27, LOW);
      digitalWrite(25, LOW);
      digitalWrite(33, LOW);
      digitalWrite(32, LOW);
      digitalWrite(22, HIGH);
      digitalWrite(23, LOW);
      break;
    case 7:
      digitalWrite(12, LOW);
      digitalWrite(27, LOW);
      digitalWrite(25, LOW);
      digitalWrite(33, LOW);
      digitalWrite(32, LOW);
      digitalWrite(22, LOW);
      digitalWrite(23, HIGH);
      break;
  }
  delay(2000); // Keeps the server running
}
