
#include <aJSON.h>
#include <DallasTemperature.h>
#include <OneWire.h>

//=============  设备号+ID号+设备密码============
String DEVICEID="3626"; // 你的设备ID=======
String APIKEY="8c5d635da"; // 设备密码==
String INPUTID="3314";//接口ID==============
//=======================================
//const int LM35 = 0;// LM35 Vout 接 A0
unsigned long lastCheckInTime = 0; //记录上次报到时间
unsigned long lastUpdateTime = 0;//记录上次上传数据时间
const unsigned long postingInterval = 40000; // 每隔40秒向服务器报到一次
const unsigned long updateInterval = 5000; // 数据上传间隔时间5秒
String inputString = "";//串口读取到的内容
boolean stringComplete = false;//串口是否读取完毕
boolean CONNECT = true; //连接状态
boolean isCheckIn = false; //是否已经登录服务器
char* parseJson(char *jsonString);//定义aJson字符串
#define TempSensor A0
OneWire oneWire(TempSensor);
DallasTemperature sensors(&oneWire);
void setup() {
  Serial.begin(115200);
  sensors.begin();
  delay(10000);
}
void loop() {
  if(millis() - lastCheckInTime > postingInterval || lastCheckInTime==0) {
    checkIn();
  }
  if(millis() - lastUpdateTime > updateInterval) {
    float val;//定义变量
    //get 18b20 data
    sensors.requestTemperatures();
    val = sensors.getTempCByIndex(0);
    update1(DEVICEID,INPUTID,val);
  }
  serialEvent();
    if (stringComplete) {
      inputString.trim();
      //Serial.println(inputString);
      if(inputString=="CLOSED"){
        Serial.println("connect closed!");
        CONNECT=false;   
        isCheckIn=false;     
      }else{
        int len = inputString.length()+1;    
        if(inputString.startsWith("{") && inputString.endsWith("}")){
          char jsonString[len];
          inputString.toCharArray(jsonString,len);
          aJsonObject *msg = aJson.parse(jsonString);
          processMessage(msg);//处理接收到的Json数据
          aJson.deleteItem(msg);          
        }
      }      
      inputString = "";
      stringComplete = false;
  }
}
void checkIn() {
  if (!CONNECT) {
    Serial.print("+++");
    delay(500);  
    Serial.print("\r\n"); 
    delay(1000);
    Serial.print("AT+RST\r\n"); 
    delay(6000);
    CONNECT=true;
    lastCheckInTime==0;
  }
  else{
    Serial.print("{\"M\":\"checkin\",\"ID\":\"");
    Serial.print(DEVICEID);
    Serial.print("\",\"K\":\"");
    Serial.print(APIKEY);
    Serial.print("\"}\r\n");
    lastCheckInTime = millis();
  }
}
void processMessage(aJsonObject *msg){
  aJsonObject* method = aJson.getObjectItem(msg, "M");
  aJsonObject* content = aJson.getObjectItem(msg, "C");     
  aJsonObject* client_id = aJson.getObjectItem(msg, "ID");  
  //char* st = aJson.print(msg);
  if (!method) {
    return;
  }
    //Serial.println(st); 
    //free(st);
    String M=method->valuestring;
    if(M=="checkinok"){
      isCheckIn = true;
    }
}
void update1(String did, String inputid, float value){
  Serial.print("{\"M\":\"update\",\"ID\":\"");
  Serial.print(did);
  Serial.print("\",\"V\":{\"");
  Serial.print(inputid);
  Serial.print("\":\"");
  Serial.print(value);
  Serial.println("\"}}");
  lastCheckInTime = millis();
  lastUpdateTime= millis(); 
}
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
//同时上传两个接口数据调用此函数
void update2(String did, String inputid1, float value1, String inputid2, float value2){
  Serial.print("{\"M\":\"update\",\"ID\":\"");
  Serial.print(did);
  Serial.print("\",\"V\":{\"");
  Serial.print(inputid1);
  Serial.print("\":\"");
  Serial.print(value1);
  Serial.print("\",\"");
  Serial.print(inputid2);
  Serial.print("\":\"");
  Serial.print(value2);
  Serial.println("\"}}");
  lastCheckInTime = millis();
  lastUpdateTime= millis(); 
}
