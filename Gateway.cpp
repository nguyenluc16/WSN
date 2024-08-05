#include <Arduino.h>
#include <WiFi.h>
#include "NimBLEDevice.h"
#include <ThingsBoard.h>
#include <string.h>

#define TOKEN               "GATEWAYTOCLOUD"  // cần để kết nối đến things qua mqtt
#define THINGSBOARD_SERVER  "thingsboard.cloud" 
#define THINGSBOARD_PORT    1883

// The remote service we wish to connect to.
static BLEUUID serviceUUID1("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID serviceUUID2("2583a4c7-4d51-464f-a252-9a222bc8306c");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID1("d3ed3642-a6fd-4b41-ad60-aaa2bd1362a0"); //heat
static BLEUUID    charUUID2("0d6a8657-47c7-4c5b-8bbc-b02cdfb93844"); //tl
static BLEUUID    charUUID3("74d7fcbe-76c2-4c0b-a71c-0d9015b476a2");//th
static BLEUUID    charUUID4("77bd3f14-5187-45c4-9b65-9af76140ab40"); //ota
static BLEUUID    charUUID5("caa72fc7-d821-42ca-97b4-c915b235a14a"); //heat
static BLEUUID    charUUID6("5b28a51d-620d-4025-a29a-b1be7206ef08"); //tl
static BLEUUID    charUUID7("38efc564-9f6a-4ad2-9e1f-4b6ff28168a3");//th
static BLEUUID    charUUID8("027c7bcf-067e-49ab-b8f7-fe5fc03d71e3"); //ota

static boolean doConnect1 = false;
static boolean connected1 = false;
static boolean doConnect2 = false;
static boolean connected2 = false;

bool subscribed = false;
static BLERemoteCharacteristic* pRemoteCharacteristic1;
static BLERemoteCharacteristic* pRemoteCharacteristic2;
static BLERemoteCharacteristic* pRemoteCharacteristic3;
static BLERemoteCharacteristic* pRemoteCharacteristic4;
static BLERemoteCharacteristic* pRemoteCharacteristic5;
static BLERemoteCharacteristic* pRemoteCharacteristic6;
static BLERemoteCharacteristic* pRemoteCharacteristic7;
static BLERemoteCharacteristic* pRemoteCharacteristic8;
static BLEAdvertisedDevice* myDevice1;
static BLEAdvertisedDevice* myDevice2;

const char* ssid = "ASUS_TUF"; // Setup wifi và passs
const char* password = "1357913579";

// Shared attributes we want to request from the server
constexpr std::array<const char*, 6U> SUBSCRIBED_SHARED_ATTRIBUTES = {
  "SetTempH1",
  "SetTempL1",
  "SetUdate1",
  "SetTempH2",
  "SetTempL2",
  "SetUdate2",
};

WiFiClient wifiClient; //Setup wifi
ThingsBoard tb(wifiClient); //setup Thingsboard

//setup WiFi
void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Reconnect Thingsboard if cant connected
void reconnect() {
  // Loop until we're reconnected
  while (!tb.connected()) {
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( tb.connect(THINGSBOARD_SERVER, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 2 seconds" );
      // Wait 5 seconds before retrying
      delay( 2000 );
    }
  }
}

//Hàm nhận dữ liệu từ Things nếu có thay đổi và gửi về Client
void processSharedAttributeUpdate(const Shared_Attribute_Data &data) {
  String key;
  String value;
  int jsonSize = JSON_STRING_SIZE(measureJson(data));
  char buffer[jsonSize];
  serializeJson(data, buffer, jsonSize);
  Serial.println(buffer);
  for(int i = 2; i<strlen(buffer); i++)
  {
    if (buffer[i]=='"'){break;}
    key += buffer[i];
  }
  for(int i = 14; i<strlen(buffer); i++)
  {
    if (buffer[i]=='}'){break;}
    value += buffer[i];
  }
  if (key == "SetTempL1" ){pRemoteCharacteristic2->writeValue(value);}
  if (key == "SetTempH1" ){pRemoteCharacteristic3->writeValue(value);}
  if (key == "SetUdate1" ){pRemoteCharacteristic4->writeValue(value);}
  if (key == "SetTempL2" ){pRemoteCharacteristic6->writeValue(value);}
  if (key == "SetTempH1" ){pRemoteCharacteristic7->writeValue(value);}
  if (key == "SetUdate2" ){pRemoteCharacteristic8->writeValue(value);}
}
//Thingsboard attribute callback
const Shared_Attribute_Callback callback(SUBSCRIBED_SHARED_ATTRIBUTES.cbegin(), SUBSCRIBED_SHARED_ATTRIBUTES.cend(), processSharedAttributeUpdate);

//Callback if connected or disconnected the sever
class MyClientCallback1 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected1 = false;
    Serial.println("onDisconnect");
  }
};

class MyClientCallback2 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected2 = false;
    Serial.println("onDisconnect");
  }
};

//Callback if temp value changed and gửi to Thingsboard
static void notifyCallback1(
  BLERemoteCharacteristic* pBLERemoteCharacteristic1,uint8_t* pData,size_t length,bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic1->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
    char* temperatureChar;
    temperatureChar = (char*)pData;
    tb.sendTelemetryString("Temp1",temperatureChar);
}

static void notifyCallback2(
  BLERemoteCharacteristic* pBLERemoteCharacteristic5,uint8_t* pData,size_t length,bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic5->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
    char* temperatureChar;
    temperatureChar = (char*)pData;
    tb.sendTelemetryString("Temp2",temperatureChar);
}
//Hàm kết nối đến sever ble
bool connectToServer1() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice1->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback1());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice1);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID1);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID1.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    pRemoteCharacteristic1 = pRemoteService->getCharacteristic(charUUID1);
    if (pRemoteCharacteristic1 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID1.toString().c_str());
      pClient->disconnect();
      return false;
    }

    pRemoteCharacteristic2 = pRemoteService->getCharacteristic(charUUID2);
    if (pRemoteCharacteristic2 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID2.toString().c_str());
      pClient->disconnect();
      return false;
    }

    pRemoteCharacteristic4 = pRemoteService->getCharacteristic(charUUID4);
    if (pRemoteCharacteristic4 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID4.toString().c_str());
      pClient->disconnect();
      return false;
    }

    pRemoteCharacteristic3 = pRemoteService->getCharacteristic(charUUID3);
    if (pRemoteCharacteristic3 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID3.toString().c_str());
      pClient->disconnect();
      return false;
    }
    
    Serial.println(" - Found our characteristic");

    if(pRemoteCharacteristic1->canNotify())
      pRemoteCharacteristic1->registerForNotify(notifyCallback1);

    connected1 = true;
    return true;
}

bool connectToServer2() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice2->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback2());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice2);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID2);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID2.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    pRemoteCharacteristic5 = pRemoteService->getCharacteristic(charUUID5);
    if (pRemoteCharacteristic5 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID5.toString().c_str());
      pClient->disconnect();
      return false;
    }

    pRemoteCharacteristic6 = pRemoteService->getCharacteristic(charUUID6);
    if (pRemoteCharacteristic6 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID6.toString().c_str());
      pClient->disconnect();
      return false;
    }

    pRemoteCharacteristic7 = pRemoteService->getCharacteristic(charUUID7);
    if (pRemoteCharacteristic7 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID7.toString().c_str());
      pClient->disconnect();
      return false;
    }

    pRemoteCharacteristic8 = pRemoteService->getCharacteristic(charUUID8);
    if (pRemoteCharacteristic8 == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID8.toString().c_str());
      pClient->disconnect();
      return false;
    }
    
    Serial.println(" - Found our characteristic");

    if(pRemoteCharacteristic5->canNotify())
      pRemoteCharacteristic5->registerForNotify(notifyCallback1);

    connected2 = true;
    return true;
}
//Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks1: public BLEAdvertisedDeviceCallbacks {   
  void onResult(BLEAdvertisedDevice* advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice->toString().c_str());
// Found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID1)) {
      //BLEDevice::getScan()->stop();
      myDevice1 = advertisedDevice; /** Just save the reference now, no need to copy the object */
      doConnect1 = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

class MyAdvertisedDeviceCallbacks2: public BLEAdvertisedDeviceCallbacks {   
  void onResult(BLEAdvertisedDevice* advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice->toString().c_str());
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID2)) {
      //BLEDevice::getScan()->stop();
      myDevice2 = advertisedDevice;
      doConnect2 = true;

    } 
  } 
};

void setup() {
  Serial.begin(115200);
  setup_wifi();
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks1());
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks2());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);  // scan to run for 5 seconds.
} // End of setup.


// This is the Arduino main loop function.
void loop() {
if ( !tb.connected() ) {
    reconnect();
  }
  if (doConnect1 == true) {
    if (connectToServer1()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect1 = false;
  }
  if (doConnect2 == true) {
    if (connectToServer2()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect2 = false;
  }
  if (!subscribed) {
      Serial.println(F("Subscribing for shared attribute updates..."));
      if (!tb.Shared_Attributes_Subscribe(callback)) {
        Serial.println(F("Failed to subscribe for shared attribute updates"));
        return;
      }
      Serial.println(F("Subscribe done"));
      subscribed = true;
    }
    tb.loop();
    delay(1000);
} // End of loop
