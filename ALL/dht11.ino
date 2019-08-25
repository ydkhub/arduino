#include <dht11.h>

dht11 DHT11;

// 设置 DHT 引脚 为 Pin 8
#define DHT11PIN 8

void setup() {
  Serial.begin(9600);
  
  Serial.println("DHT11 TEST PROGRAM");
  Serial.print("LIBRARY");
  // 输出 DHT 库的版本号
  Serial.println(DHT11LIB_VERSION);
  Serial.println();
}

void loop() {
  Serial.println("\n");

  int chk = DHT11.read(DHT11PIN);

  // 测试 DHT 是否正确连接
  Serial.print("Read sensor: ");
  switch (chk)
  {
    case DHTLIB_OK: 
    Serial.println("OK"); 
    break;
    case DHTLIB_ERROR_CHECKSUM: 
    Serial.println("Checksum error"); 
    break;
    case DHTLIB_ERROR_TIMEOUT: 
    Serial.println("Time out error"); 
    break;
    default: 
    Serial.println("Unknown error"); 
    break;
  }

  // 获取测量数据
  Serial.print("Humidity (%): ");
  Serial.println((float)DHT11.humidity, 2);

  Serial.print("Temperature °C): ");
  Serial.println((float)DHT11.temperature, 2);

  delay(2000);
}
