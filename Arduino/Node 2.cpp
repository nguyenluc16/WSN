#include <Arduino.h>
#include <NimBLEDevice.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <string.h>
#include <stdlib.h>
#include <HTTPUpdate.h>

#define LEDD 5 // LED đỏ
#define LEDX 4 // LED xanh
#define LEDV 3 // LED vàng

#define uS_TO_S_FACTOR 1000000  // biến chuyển từ micro giây sang giây
#define TIME_TO_SLEEP  7        //Thời gian thức dậy
RTC_DATA_ATTR int bootCount = 0; // biến này lưu tại bộ nhớ RTC

#define SERVICE_UUID         "2583a4c7-4d51-464f-a252-9a222bc8306c"   //Gán UUID cho service và chara
#define CHARACTERISTIC_UUID1 "caa72fc7-d821-42ca-97b4-c915b235a14a"
#define CHARACTERISTIC_UUID2 "5b28a51d-620d-4025-a29a-b1be7206ef08"
#define CHARACTERISTIC_UUID3 "38efc564-9f6a-4ad2-9e1f-4b6ff28168a3"
#define CHARACTERISTIC_UUID4 "027c7bcf-067e-49ab-b8f7-fe5fc03d71e3"


const char* ssid = "ASUS_TUF"; // Setup wifi và pass
const char* password = "1357913579";

const int oneWireBus = 2; //Setup chân truyền dữ liệu nhiệt

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic1 = NULL;
BLECharacteristic* pCharacteristic2 = NULL;
BLECharacteristic* pCharacteristic3 = NULL;
BLECharacteristic* pCharacteristic4 = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

uint32_t updateCounter = 0;
String version = "1.0";
String key = "88545d5d-0a5c-4bd0-8885-e26a3d96eb9a";

float temperatureC; //Biến đo nhiệt
float templs = 25; //Nhiệt độ low cài đặt 
float temphs = 27; //Nhiệt độ high cài đặt
float updates;
int i=0;
float tempsend = 0;
char tempString[8]; 

OneWire oneWire(oneWireBus); // Setup hàm truyền nhận và xử lý dữ liệu
DallasTemperature sensors(&oneWire);
WiFiClient wifiClient;

//Lấy ID của ESP32
String getChipId()
{
  String ChipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
  ChipIdHex += String((uint32_t)ESP.getEfuseMac(), HEX);
  return ChipIdHex;
}

//Hàm update OTA dùng OTADrive
void update_FOTA()
{
  String url = "http://otadrive.com/deviceapi/update?";
  url += "k=" + key;
  url += "&v=" + version;
  url += "&s=" + getChipId(); // định danh thiết bị trên Cloud
 
  httpUpdate.update(wifiClient, url, version);
}

//setup WiFi
void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
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
//callback xem kết nối dc ble chưa
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
//callback khi client viết gì đó, cụ thể là set nhiệt và ycau update ota
class MyControlCallbacks2: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic2) {
      std:: string tempL = pCharacteristic2 ->getValue();
      if (tempL.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < tempL.length(); i++)
          Serial.print(tempL[i]);
        Serial.println();
        Serial.println("*********");
        String yourValue;
        yourValue = tempL.c_str();
        templs = String(yourValue).toFloat();
      }
    }
};

class MyControlCallbacks3: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic3) {
      std:: string tempH = pCharacteristic3 ->getValue();
      if (tempH.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < tempH.length(); i++)
          Serial.print(tempH[i]);
        Serial.println();
        Serial.println("*********");
        String yourValue;
        yourValue = tempH.c_str();
        temphs = String(yourValue).toFloat();
      }
    }
};

class MyControlCallbacks4: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic4) {
      std:: string up = pCharacteristic4 ->getValue();
      if (up.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < up.length(); i++)
          Serial.print(up[i]);
        Serial.println();
        Serial.println("*********");
        String yourValue;
        yourValue = up.c_str();
        updates = String(yourValue).toFloat();
      }
    }
};


void setup() {
  Serial.begin(115200);
  pinMode(LEDX, OUTPUT);
  pinMode(LEDD, OUTPUT);
  pinMode(LEDV, OUTPUT);

  // Create the BLE Device
  BLEDevice::init("ESP32-1");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  sensors.begin();
  //mỗi lần thức dậy sẽ tăng biến này 1 lần và in ra
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  //gọi hàm thức dậy
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +" Seconds");

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic1 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID1,  
                      NIMBLE_PROPERTY::READ   |
                      NIMBLE_PROPERTY::WRITE  |
                      NIMBLE_PROPERTY::NOTIFY |
                      NIMBLE_PROPERTY::INDICATE
                    );
  pCharacteristic2 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID2,  
                      NIMBLE_PROPERTY::READ   |
                      NIMBLE_PROPERTY::WRITE  |
                      NIMBLE_PROPERTY::NOTIFY |
                      NIMBLE_PROPERTY::INDICATE
                    ); 
  pCharacteristic2->setCallbacks(new (MyControlCallbacks2));
  pCharacteristic3 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID3,  
                      NIMBLE_PROPERTY::READ   |
                      NIMBLE_PROPERTY::WRITE  |
                      NIMBLE_PROPERTY::NOTIFY |
                      NIMBLE_PROPERTY::INDICATE
                    ); 
  pCharacteristic3->setCallbacks(new (MyControlCallbacks3));
  pCharacteristic4 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID4,  
                      NIMBLE_PROPERTY::READ   |
                      NIMBLE_PROPERTY::WRITE  |
                      NIMBLE_PROPERTY::NOTIFY |
                      NIMBLE_PROPERTY::INDICATE
                    ); 
  pCharacteristic4->setCallbacks(new (MyControlCallbacks4));
  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  /** Note, this could be left out as that is the default value */
  //pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter

  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    sensors.requestTemperatures();   //đo nhiệt độ
    temperatureC = sensors.getTempCByIndex(0);
    tempsend = tempsend + temperatureC; //biến ghi lại dữ liệu nhiệt để tính trung bình
    Serial.print(temperatureC); 
    Serial.println("ºC");
    if(temperatureC<templs-0.5){digitalWrite(LEDV, LOW);}   //hiển thị led theo nhiệt đo dc
    else if(temperatureC>temphs+0.5){digitalWrite(LEDD, LOW);}
    else if(templs<=temperatureC<=temphs){digitalWrite(LEDX, LOW);}
    delay(1000);
    if(updates==1){setup_wifi();Serial.println("Check update");update_FOTA();} //Nếu có yêu cầu update thì bật wifi và update
    i++;
    if(i%5==0){ //đo đủ 5 lần
    tempsend = (round(tempsend/5 * 10))/10; //lấy trung bình 5 lần đo và làm tròn về 1 đơn vị sau dấu ,
    dtostrf(tempsend, 8, 1, tempString);
    if (deviceConnected) {
        pCharacteristic1->setValue(tempString);
        pCharacteristic1->notify();
        delay(1000);}}
    delay(1000);
    if(i%6==0){Serial.flush(); esp_deep_sleep_start();} //chạy thêm 1 chu kì rồi ngủ
}