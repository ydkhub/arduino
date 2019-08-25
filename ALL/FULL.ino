#include <aJSON.h>
#include <dht11.h>

dht11 DHT11;
// 设置 DHT 引脚 为 Pin 3
#define DHT11PIN 8//温度接口
int irSensor = 2;//人体红外接口

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

//火焰传递
String DEVICEID = "11481"; // 你的设备ID=======
String APIKEY = "669bed0ce"; // 设备密码==
String INPUTID = "10405"; //接口ID==============
//=======================================

unsigned long lastCheckStatusTime = 0; //记录上次报到时间
unsigned long lastUpdateTime = 0;//记录上次上传数据时间
const unsigned long postingInterval = 40000; // 每隔40秒向服务器报到一次
const unsigned long updateInterval = 5000; // 数据上传间隔时间5秒
unsigned long checkoutTime = 0;//登出时间
//火焰传递

void setup() 
{
  Serial.begin(115200);//波特率115200
  Serial.println(DHT11LIB_VERSION);
  Serial.println();//dht11
  pinMode(A0,INPUT);//模拟口输入模式火焰
  pinMode(LED_BUILTIN, OUTPUT); //连接内置LED的引脚设置为输出模式
  pinMode(irSensor, INPUT);     //连接人体红外感应模块的OUT引脚设置为输入模式

  pinMode(ledPin, OUTPUT);
  pinMode(A1, INPUT);
  delay(5000);//等一会儿ESP8266
}
 
void loop() 
{  
  Serial.println("\n");
 int chk = DHT11.read(DHT11PIN);

  // 测试 DHT 是否正确连接
  Serial.print("Read sensor: ");
  switch (chk)
  {
    case DHTLIB_OK: 
    //Serial.println("OK"); 
    break;
    case DHTLIB_ERROR_CHECKSUM: 
    //Serial.println("Checksum error"); 
    break;
    case DHTLIB_ERROR_TIMEOUT: 
    //Serial.println("Time out error"); 
    break;
    default: 
    //Serial.println("Unknown error"); 
    break;
  }
    
  // 获取测量数据
  Serial.print("Humidity (%): ");
  Serial.println((float)DHT11.humidity, 2);

  Serial.print("Temperature °C): ");
  Serial.println((float)DHT11.temperature, 2);
  
  Serial.print("Flame: ");
  
  Serial.println(analogRead(A0));//火焰传感器输出
//火焰输出
 if (millis() - lastCheckStatusTime > postingInterval) {
    checkStatus();
  }
  //checkout 50ms 后 checkin
  if ( checkoutTime != 0 && millis() - checkoutTime > 50 ) {
    checkIn();
    checkoutTime = 0;
  }

  if (millis() - lastUpdateTime > updateInterval) {
    float val;//定义变量
    int dat;//定义变量
    dat = analogRead(A0); // 读取传感器的模拟值并赋值给dat
    
    update1(DEVICEID, INPUTID, A0);
    lastUpdateTime = millis();
  }


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
void update1(String did, String inputid, float value) {
  Serial.print("{\"M\":\"update\",\"ID\":\"");
  Serial.print(did);
  Serial.print("\",\"V\":{\"");
  Serial.print(inputid);
  Serial.print("\":\"");
  Serial.print(value);
  Serial.println("\"}}");
}
//同时上传两个接口数据调用此函数
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
//火焰输出




  
  
  bool sensorReading = digitalRead(irSensor);  //建立变量存储感应模块的输出信号
  
  if ( sensorReading ) {
    digitalWrite(LED_BUILTIN, HIGH);   // 模块感应到人.输出高电平.点亮LED
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);   //  无人状态保持LED关闭
  }
    Serial.print("Human body infrared: ");
    Serial.println(sensorReading);  //将模块输出信号通过串口监视器显示

//↓粉尘
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
    Serial.print("Dust Density: ");
    Serial.println(dustDensity);
  delay(1000);
                                                        
 }
