#include "BLEDevice.h"


// The remote service we wish to connect to.
#define SERVICE_UUID "9d537c81-4b1f-406c-b12f-a9aa49af6333"

// The characteristics we want to update on the server
#define CHARACTERISTIC_LEFT_UUID  "ac61fa72-e2de-42fb-9605-d0c7549b1c39"
#define CHARACTERISTIC_RIGHT_UUID "b3f4eb92-9ceb-4317-90ed-373a36164d2b"

#define CHARACTERISTIC_LEFT_BATTERY  "1a703249-0446-448b-98d4-980a1d10da21"
#define CHARACTERISTIC_RIGHT_BATTERY "2de2e09d-5ccd-4341-a148-eb7422401c98"

// https://www.uuidgenerator.net/


// Reformat the strings as a BLEUUID
static BLEUUID serviceUUID(SERVICE_UUID);
static BLEUUID charLeftUUID(CHARACTERISTIC_LEFT_UUID);
static BLEUUID charRightUUID(CHARACTERISTIC_RIGHT_UUID);
static BLEUUID charLeftBatt(CHARACTERISTIC_LEFT_BATTERY);
static BLEUUID charRightBatt(CHARACTERISTIC_RIGHT_BATTERY);

// If he have found an advertised device and should attempt to connect to it
static boolean doConnect = false;
// If we are connected to a server
static boolean connected = false;
// If we should scan for other servers
static boolean doScan = false;

// The device with our service found during a scan
static BLEAdvertisedDevice* pFoundDevice;

// The characteristics on the server for us to update
static BLERemoteCharacteristic* pRemoteCharLeft;
static BLERemoteCharacteristic* pRemoteCharRight;
static BLERemoteCharacteristic* pRemoteCharLeftBatt;
static BLERemoteCharacteristic* pRemoteCharRightBatt;

bool right = false;
bool thumb, pointer, middle;
uint8_t flexData;

float batteryVolt;
uint32_t Vbatt;


// Callbacks for events that occur to the BLE Client
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  // When the client is disconnected
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Client disconnected :c");
  }
};

// Helper function to try to connect to a server once the advertised device has been found
bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(pFoundDevice->getAddress().toString().c_str());
    
    BLEClient* pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    // Adds the callback methods so they can get called
    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(pFoundDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristics in the service of the remote BLE server.
    pRemoteCharLeft = pRemoteService->getCharacteristic(charLeftUUID);
    if (pRemoteCharLeft == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charLeftUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRemoteCharRight = pRemoteService->getCharacteristic(charRightUUID);
    if (pRemoteCharRight == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charLeftUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRemoteCharLeftBatt = pRemoteService->getCharacteristic(charLeftBatt);
    if (pRemoteCharLeftBatt == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charLeftBatt.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRemoteCharRightBatt = pRemoteService->getCharacteristic(charRightBatt);
    if (pRemoteCharRightBatt == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charRightBatt.toString().c_str());
      pClient->disconnect();
      return false;
    }
    
    Serial.println(" - Found our characteristics");

    connected = true;
    return true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      pFoundDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  // setting up pins to use for flex sensor, A0 for checking battery voltage, A1-3 for flex sensors
  // D8 used for determining left of right client, connect 10kΩ to ground for left, power for right
  pinMode(A0, INPUT);

  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  pinMode(D8, INPUT);


  Serial.begin(115200);
  Serial.println("Proto Glove Client starting...");
  BLEDevice::init("Proto Glove Client");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  if (digitalRead(D8) == HIGH) right = true;
  Serial.print("Client is right? ");
  Serial.println(digitalRead(D8));
} // End of setup.


// This is the Arduino main loop function.
void loop() {

  // reading A0 for voltage
  batteryVolt = readBatt();

  // reading flex sensors 
  (analogRead(A0) >= 2047) ? thumb = true : thumb = false;
  (analogRead(A1) >= 2047) ? pointer = true : pointer = false;
  (analogRead(A2) >= 2047) ? middle = true : middle = false;

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("Connected to the BLE Proto Server.");
      doConnect = false;
    } else {
      Serial.println("Failed to connect to the server :c");
    }
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {

    // dont know if this actually works
    flexData = 48 + ((int) thumb << 0 | (int) pointer << 1 | (int) middle << 2);
    //Serial.println(flexData);
    
    if (right) {
      pRemoteCharRight->writeValue(flexData, false);
      pRemoteCharRightBatt->writeValue(batteryVolt, false);
    } else {
      pRemoteCharLeft->writeValue(flexData, false);
      pRemoteCharLeftBatt->writeValue(batteryVolt, false);
    }

  } else if (doScan) {
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  
  delay(1000); // Delay a second between loops.
} // End of loop

float readBatt() {
  uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(A3); // ADC with correction   
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0;     // attenuation ratio 1/2, mV --> V
  return Vbattf;
}
