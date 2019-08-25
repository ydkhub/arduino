
#include <dht11.h>
#include <aJSON.h>
dht11 DHT11;
// 设置 DHT 引脚 为 Pin 3
#define DHT11PIN 8//温度接口
int pinRelay = 7;
//粉尘
#define measurePin A1//输出引脚连接模拟口A0
#define ledPin 9 //LED引脚连接数字口9
unsigned int samplingTime = 280;//根据前面分析采样时间为280，所以这里为280
unsigned int deltaTime = 40;//测量完后脉冲需要继续保持，保持时间为320-280=40
unsigned int sleepTime = 9680;//LED脉冲周期为10毫秒，故此处为10000-320=9680
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
//粉尘

//=============  此处必须修改============火焰
String DEVICEID = "11505"; // 你的设备ID=======
String APIKEY = "58f6f4323"; // 设备密码==
String INPUTID = "10438"; //接口ID==============
String INPUTID1 = "10439"; //接口ID==============
String INPUTID2 = "10420"; //接口ID==============
String INPUTID3 = "10429"; //接口ID==============
String INPUTID4 = "10437"; //接口ID==============
//温度

const int FLAME = 0;//火焰
int irSensor = 4;//红外

unsigned long lastCheckStatusTime = 0; //记录上次报到时间
unsigned long lastUpdateTime = 0;//记录上次上传数据时间
const unsigned long postingInterval = 4000; // 每隔40秒向服务器报到一次
const unsigned long updateInterval = 2000; // 数据上传间隔时间5秒
unsigned long checkoutTime = 0;//登出时间

void setup() {
  pinMode(pinRelay, OUTPUT);
  pinMode(A0,INPUT);//模拟口输入模式火焰
 //红外
 pinMode(LED_BUILTIN, OUTPUT); //连接内置LED的引脚设置为输出模式
 pinMode(irSensor, INPUT);     //连接人体红外感应模块的OUT引脚设置为输入模式
  //红外
 
 
//粉尘
  pinMode(ledPin, OUTPUT);
  pinMode(A1, INPUT);
//粉尘
  //温度
   Serial.println(DHT11LIB_VERSION);
   Serial.println();
  //温度
  Serial.begin(115200);
  delay(3000);//等一会儿ESP8266
}
  
void loop() {
  //每一定时间查询一次设备在线状态，同时替代心跳
  if (millis() - lastCheckStatusTime > postingInterval) {
    checkStatus();
   
  }
  //checkout 50ms 后 checkin
  if ( checkoutTime != 0 && millis() - checkoutTime > 50 ) {
    checkIn();
  
    checkoutTime = 0;
    
  }
  //每隔一定时间上传一次数据

   //变量传递部分
  if (millis() - lastUpdateTime > updateInterval) {
   // float val;//定义变量

  //粉尘
    digitalWrite(ledPin, LOW);
 
    delayMicroseconds(samplingTime);
    voMeasured = analogRead(measurePin);
 
    delayMicroseconds(deltaTime);
    digitalWrite(ledPin, HIGH);
 
    delayMicroseconds(sleepTime);
    calcVoltage = voMeasured * (5.0 / 1024);
    dustDensity = 5000*calcVoltage/29 - 3000/29;
 
    if (dustDensity < 0) {
        dustDensity = 103;
    }
   
  //粉尘


   //红外
   bool sensorReading = digitalRead(irSensor);  //建立变量存储感应模块的输出信号
  
  if ( sensorReading ) {
    digitalWrite(LED_BUILTIN, HIGH);   // 模块感应到人.输出高电平.点亮LED
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);   //  无人状态保持LED关闭
  }
  //红外
    float dat;//定义变量
    DHT11.read(DHT11PIN);
    float dht11temp = DHT11.temperature;
    float dht11hum =  DHT11.humidity;
    dat = analogRead(FLAME); // 读取传感器的模拟值并赋值给dat
    update1(DEVICEID, INPUTID, dat, INPUTID1, sensorReading, INPUTID2, dht11temp, INPUTID3, dht11hum, INPUTID4, dustDensity);

   
    lastUpdateTime = millis();
  }



  //读取串口信息
  while (Serial.available()) {
    String inputString = Serial.readStringUntil('\n');
    //检测json数据是否完整
    int jsonBeginAt = inputString.indexOf("{");
    int jsonEndAt = inputString.lastIndexOf("}");
    if (jsonBeginAt != -1 && jsonEndAt != -1) {
      //净化json数据
      inputString = inputString.substring(jsonBeginAt, jsonEndAt + 1);
      int len = inputString.length() + 1;
      char jsonString[len];
      inputString.toCharArray(jsonString, len);
      aJsonObject *msg = aJson.parse(jsonString);
      processMessage(msg);
      aJson.deleteItem(msg);
    }
  }
}
//设备登录
//{"M":"checkin","ID":"xx1","K":"xx2"}\n
void checkIn() {
  Serial.print("{\"M\":\"checkin\",\"ID\":\"");
  Serial.print(DEVICEID);
  Serial.print("\",\"K\":\"");
  Serial.print(APIKEY);
  Serial.print("\"}\n");
}




//强制设备下线，用消除设备掉线延时
//{"M":"checkout","ID":"xx1","K":"xx2"}\n
void checkOut() {
  Serial.print("{\"M\":\"checkout\",\"ID\":\"");
  Serial.print(DEVICEID);
  Serial.print("\",\"K\":\"");
  Serial.print(APIKEY);
  Serial.print("\"}\n");
}

//查询设备在线状态
//{"M":"status"}\n
void checkStatus() {
  Serial.print("{\"M\":\"status\"}\n");
  lastCheckStatusTime = millis();
}


//处理来自ESP8266透传的数据
void processMessage(aJsonObject *msg) {
  aJsonObject* method = aJson.getObjectItem(msg, "M");
  aJsonObject* content = aJson.getObjectItem(msg, "C");
  if (!method) {
    return;
  }
  String M = method->valuestring;
 
  if (M == "WELCOME TO BIGIOT") {
    checkOut();
   
    checkoutTime = millis();
    return;
  }
  if (M == "connected") {
    checkIn();
    
  }
  
}

//上传一个接口数据
//{"M":"update","ID":"2","V":{"2":"120"}}\n
void update1(String did, String inputid, float value, String inputid1, bool value1, String inputid2, float value2, String inputid3, float value3, String inputid4, float value4) {
  Serial.print("{\"M\":\"update\",\"ID\":\"");
  Serial.print(did);
  Serial.print("\",\"V\":{\"");
  Serial.print(inputid);
  Serial.print("\":\"");
  Serial.print(value);
  Serial.print("\",\"");
   Serial.print(inputid1);
  Serial.print("\":\"");
  Serial.print(value1);
  Serial.print("\",\"");
   Serial.print(inputid2);
  Serial.print("\":\"");
  Serial.print(value2);
  Serial.print("\",\"");
   Serial.print(inputid3);
  Serial.print("\":\"");
  Serial.print(value3);
  Serial.print("\",\"");
  Serial.print(inputid4);
  Serial.print("\":\"");
  Serial.print(value4);
  Serial.println("\"}}");
}

//上传5个数据

//{"M":"update","ID":"112","V":{"6":"1","36":"116"}}\n

//同时上传两个接口数据调用此函数
/*
//{"M":"update","ID":"112","V":{"6":"1","36":"116"}}\n
void update2(String did, String inputid1, float value1, String inputid2, float value2) {
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
}
//上传更多数据，可以参考上面的写法，和通讯协议，自己添加。
//贝壳物联通讯协议：https://www.bigiot.net/help/1.html
*/
