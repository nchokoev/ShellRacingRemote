#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>

#define SERVICE_UUID_FFF "0000fff0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_FFF1 "d44bc439-abfd-45a2-b575-925416129600" /*- Write, Write no response */ //directions

#define SERVICE_UUID_1800 "00001800-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_1800 "00002a00-0000-1000-8000-00805f9b34fb" /* - Write no response */ //name

uint8_t idle_data[16] = { 0x02, 0x5e, 0x69, 0x5a, 0x48, 0xff, 0x2a, 0x43, 0x8c, 0xa6, 0x80, 0xf8, 0x3e, 0x04, 0xe4, 0x5d };
uint8_t up_data[16] = { 0x29, 0x60, 0x9c, 0x66, 0x48, 0x52, 0xcf, 0xf1, 0xb0, 0xf0, 0xcb, 0xb9, 0x80, 0x14, 0xbd, 0x2c };
uint8_t down_data[16] = { 0x03, 0x20, 0x99, 0x09, 0xba, 0x9d, 0xa1, 0xc8, 0xb9, 0x86, 0x16, 0x3c, 0x6d, 0x48, 0x46, 0x55 };
uint8_t left_data[16] = { 0x51, 0x38, 0x21, 0x12, 0x13, 0x5c, 0xcc, 0xdb, 0x46, 0xcf, 0x89, 0x21, 0xb7, 0x05, 0x49, 0x9a };
uint8_t right_data[16] = { 0x1b, 0x57, 0x69, 0xcd, 0xf1, 0x3e, 0x8a, 0xb6, 0x27, 0x08, 0x0f, 0xf3, 0xce, 0xfc, 0x3b, 0xc0 };

uint8_t up_left_data[16] = { 0x99, 0x28, 0xe5, 0x90, 0xdf, 0xe8, 0x21, 0x48, 0x5f, 0x41, 0x4f, 0xbb, 0x63, 0x3d, 0x5c, 0x4e };
uint8_t up_right_data[16] = { 0x0f, 0x2c, 0xe5, 0x66, 0x62, 0xd4, 0xfd, 0x9d, 0x32, 0xa4, 0x4f, 0x10, 0x2b, 0xf2, 0x0a, 0xa7 };
uint8_t down_left_data[16] = { 0x98, 0xce, 0x98, 0x1d, 0x58, 0xd1, 0x15, 0xaf, 0xe1, 0x19, 0x60, 0xbf, 0x46, 0x13, 0x92, 0x5c };
uint8_t down_right_data[16] = { 0xf2, 0x52, 0x0f, 0xba, 0x31, 0x44, 0xfb, 0x11, 0x46, 0x8f, 0xe0, 0x80, 0xc6, 0xc2, 0xc2, 0x3c };


static BLEUUID serviceUUID(SERVICE_UUID_FFF);        //Service UUID
static BLEUUID charUUID(CHARACTERISTIC_UUID_FFF1);   //Characteristic  UUID
String connectto_car_address = "00:3c:a0:03:fc:5d";  //Hardware Bluetooth MAC of my shell racing, use "nRF Connect" to inspect

static BLERemoteCharacteristic* pRemoteCharacteristic;

BLEScan* pBLEScan;  //Name the scanning device as pBLEScan
BLEScanResults foundDevices;

static BLEAddress* Server_BLE_Address;
String scanned_car_address;

boolean paired = false;

#define PIN_UP 14
#define PIN_DOWN 33
#define PIN_LEFT 27
#define PIN_RIGHT 32

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {
    paired = true;
    Serial.println(" onConnect Connected to Server");
  }
  void onDisconnect(BLEClient* pClient) {
    paired = false;
    Server_BLE_Address = NULL;
    Serial.println("Disconnected from Server");
  }
};

bool connectToserver(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient();
  Serial.println(" - Created client");
  pClient->setClientCallbacks(new MyClientCallback());
  // Connect to the BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to Shell Race Car");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService != nullptr) {
    Serial.println(" - Found our service");
  } else
    return false;

  std::map<std::string, BLERemoteCharacteristic*>* cm = pRemoteService->getCharacteristics();
  std::map<std::string, BLERemoteCharacteristic*>::iterator it;
  for (it = cm->begin(); it != cm->end(); it++) {
    Serial.print(it->first.c_str());
    Serial.print(":");
    Serial.print(it->second->toString().c_str());
    Serial.println("");
  }
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic != nullptr)
    Serial.println(" - Found our characteristic");

  return true;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    scanned_car_address = advertisedDevice.getAddress().toString().c_str();
    Serial.println(scanned_car_address);
    if (scanned_car_address.equals(connectto_car_address)) {
      Server_BLE_Address = new BLEAddress(advertisedDevice.getAddress());
    }
  }
};

void setup() {
  Serial.begin(115200);                                          //Start serial monitor
  Serial.println("ESP32 BLE Client - Shell Racing Car Remote");  //Intro message

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();                                            //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());  //Call the class that is defined above
  pBLEScan->setActiveScan(true);                                              //active scan uses more power, but get results faster

  // Make them with interl pullups
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
}

void loop() {
  if (paired == false) {
    foundDevices = pBLEScan->start(3);  //Scan for 3 seconds to find the Shell Car
    while (foundDevices.getCount() >= 1) {
      if (Server_BLE_Address != NULL &&paired == false) {
        Serial.println("Found Device :-)... connecting to Server as client");
        if (connectToserver(*Server_BLE_Address)) {
          paired = true;
          Serial.println("********************LED turned ON************************");
          //digitalWrite (PIN_LED, LOW);
          break;
        } else {
          Serial.println("Pairing failed");
          break;
        }
      }

      if (paired == true) {
        Serial.println("Our device went out of range");
        paired = false;
        Serial.println("********************LED OOOFFFFF************************");
        //digitalWrite (PIN_LED, HIGH);
        ESP.restart();
        break;
      } else {
        Serial.println("We have some other BLe device in range");
        break;
      }
    }
  } else {
    //
    bool response = false;
    if (!digitalRead(PIN_UP) && !digitalRead(PIN_LEFT)) {
      Serial.println("DIR_UP_LEFT");
      if (pRemoteCharacteristic != nullptr)
        pRemoteCharacteristic->writeValue(up_left_data, 16, response);
    } else if (!digitalRead(PIN_UP) && !digitalRead(PIN_RIGHT)) {
      Serial.println("DIR_UP_RIGHT");
      if (pRemoteCharacteristic != nullptr)
        pRemoteCharacteristic->writeValue(up_right_data, 16, response);
    } else if (!digitalRead(PIN_DOWN) && !digitalRead(PIN_LEFT)) {
      Serial.println("DIR_DOWN_LEFT");
      if (pRemoteCharacteristic != nullptr)
        pRemoteCharacteristic->writeValue(down_left_data, 16, response);
    } else if (!digitalRead(PIN_DOWN) && !digitalRead(PIN_RIGHT)) {
      Serial.println("DIR_DOWN_RIGHT");
      if (pRemoteCharacteristic != nullptr)
        pRemoteCharacteristic->writeValue(down_right_data, 16, response);
    } else if (!digitalRead(PIN_UP)) {
      Serial.println("DIR_UP");
      if (pRemoteCharacteristic != nullptr)
        pRemoteCharacteristic->writeValue(up_data, 16, response);
    } else if (!digitalRead(PIN_DOWN)) {
      Serial.println("DIR_DOWN");
      if (pRemoteCharacteristic != nullptr)
        pRemoteCharacteristic->writeValue(down_data, 16, response);
    } else if (!digitalRead(PIN_LEFT)) {
      Serial.println("DIR_LEFT");
      if (pRemoteCharacteristic != nullptr)
        pRemoteCharacteristic->writeValue(left_data, 16, response);
    } else if (!digitalRead(PIN_RIGHT)) {
      Serial.println("DIR_RIGHT");
      if (pRemoteCharacteristic != nullptr)
        pRemoteCharacteristic->writeValue(right_data, 16, response);
    } else {
      Serial.println("idle");
      if (pRemoteCharacteristic != nullptr)
        pRemoteCharacteristic->writeValue(idle_data, 16, response);
    }
    delay(100);
  }
}
