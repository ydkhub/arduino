

float sinVal;            
int toneVal;
unsigned long tepTimer ;    

void setup(){ 
    pinMode(8, OUTPUT);        // 蜂鸣器引脚设置
    pinMode(13, OUTPUT);       // R灯引脚设置
    Serial.begin(9600);        //设置波特率为9600 bps
   
}

void loop(){ 
    int val;               // 用于存储LM35读到的值
    double data;          //用于存储已转换的温度值
    val=analogRead(0);   //LM35连到模拟口，并从模拟口读值
    data = (double) val * (5/10.24);  // 得到电压值，通过公式换成温度
   
    if(data>30){        //  如果温度大于30，蜂鸣器响   
          for(int x=0; x<180; x++){
            //将sin函数角度转化为弧度
            sinVal = (sin(x*(3.1412/180)));
            //用sin函数值产生声音的频率
            toneVal = 2000+(int(sinVal*1000));
            tone(8, toneVal);
            digitalWrite(13,HIGH);
            delay(2); 
     }   
    } else {    // 如果温度小于30，关闭蜂鸣器
           noTone(8);          //关闭蜂鸣器 
           digitalWrite(13,LOW);
    }
     
    Serial.println(data);
}
