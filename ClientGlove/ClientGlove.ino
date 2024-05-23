#include "BLEDevice.h"


// The remote service we wish to connect to.
#define SERVICE_UUID "9d537c81-4b1f-406c-b12f-a9aa49af6332"

// The characteristics we want to update on the server
#define CHARACTERISTIC_LEFT_UUID  "ac61fa72-e2de-42fb-9605-d0c7549b1c39"
#define CHARACTERISTIC_RIGHT_UUID "b3f4eb92-9ceb-4317-90ed-373a36164d2b"


// Reformat the strings as a BLEUUID
static BLEUUID serviceUUID(SERVICE_UUID);
static BLEUUID charLeftUUID(CHARACTERISTIC_LEFT_UUID);
static BLEUUID charRightUUID(CHARACTERISTIC_RIGHT_UUID);

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
    pRemoteCharLeft = pRemoteService->getCharacteristic(charRightUUID);
    if (pRemoteCharRight == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charLeftUUID.toString().c_str());
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
} // End of setup.


// This is the Arduino main loop function.
void loop() {

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
    
    // TODO: read anolog values and update the characteristics
    pRemoteCharLeft->writeValue(0, 1, false);
    pRemoteCharRight->writeValue(0, 1, false);


  } else if (doScan) {
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  
  delay(1000); // Delay a second between loops.
} // End of loop
