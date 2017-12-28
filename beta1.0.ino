#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>                         //LED库
#include <Wire.h>
#include "ESP8266.h"                                   //无线模块
int Sensor = 8;                                     //传感器的pin
#define SSID           "lichunhuihaoshuai"                   // cannot be longer than 32 characters!
#define PASSWORD       "12345678"
#define IDLE_TIMEOUT_MS  500      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.
                                   //WEBSITE     
#define mic_pin A0
#define HOST_NAME   "api.heclouds.com"
#define DEVICEID   "20418528"//
#define PROJECTID "104775"//
#define HOST_PORT   (80)
#define INTERVAL_SENSOR   100
#define INTERVAL_NET      40000             //发送间隔
String jsonToSend; 
String apiKey="jd=B0sy5P3mA=TONH5Ej34KOTbo="; 
char buf[10];
SoftwareSerial mySerial(2, 3);//RX:D3, TX:D2 
ESP8266 wifi(mySerial);
String postString;                                           //用于存储发送数据的字符串

unsigned long sensor_time = millis();                        //传感器采样时间计时器
unsigned long net_time1 = millis();
int count=0;                                                //人数的统计
 
void sound();                                               //报警函数
void updateSensorData();                                  //上传函数
void setup() {
  Serial.begin(115200);
 Serial.begin(115200);

  pinMode(Sensor, INPUT); // 设置为输入
  pinMode(12,OUTPUT);
  pinMode(mic_pin,INPUT);

  Wire.begin();                        //加入无线网
  while(!Serial);
  Serial.print("setup begin\r\n");   
  Serial.print("FW Version:");
  Serial.println(wifi.getVersion().c_str());
  if (wifi.setOprToStationSoftAP()) {
    Serial.print("to station + softap ok\r\n");
  } else {
    Serial.print("to station + softap err\r\n");
  }
  if (wifi.joinAP(SSID, PASSWORD)) {                             
    Serial.print("Join AP success\r\n");  
    Serial.print("IP: ");
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.print("Join AP failure\r\n");
  }
  if (wifi.disableMUX()) {
    Serial.print("single ok\r\n");
  } else {
    Serial.print("single err\r\n");
  }
  Serial.print("setup end\r\n");
}

void loop() {
 
int val = digitalRead(Sensor); // 读取引脚电平
 
int val2 =analogRead(mic_pin);
  if (val==1||val2>600) { // 若检测到有人，则val值为1
      Serial.println("目前累计有");
      Serial.println(count+1);
      Serial.println("人经过。");
      digitalWrite(12,HIGH);
      sound();                        //调用声音函数
      digitalWrite(12,LOW);
      count++;

  }
  else{

      Serial.println("没人经过。");

  delay(100);}
   if (net_time1 > millis())  net_time1 = millis();
    if (millis() - net_time1 > INTERVAL_NET)          //发送数据时间间隔
    if(count!=0)
    {            
        Serial.println("sent data"); 
        Serial.println(count);  
      
        updateSensorData();                                     //将数据上传到服务器的函数
        net_time1 = millis();
       
        
    }
  delay(500);
}
  


  
  void sound() {                      //声音函数
  
int tune[]=
{495,393,495,393,495,393,495,393,495,393,495,393};
float durt[]=
{1,1,1,1,1,1,1,1,1,1,1,1,};
int length;
length=sizeof(tune)/sizeof(tune[0]);  
for(int x=0;x<length;x++)
  {
    tone(6,tune[x]);
    delay(450*durt[x]);   //这里用来根据节拍调节延时，500这个指数可以自己调整，在该音乐中，我发现用500比较合适。
    noTone(6);
   
  }
  delay(1000);
}
  
 
void updateSensorData() {           
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");
    
    jsonToSend="{\"Number\":";
    dtostrf(count,1,0,buf);                     
    jsonToSend+="\""+String(buf)+"\"";
   
    jsonToSend+="}";
    
    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  } else {
    Serial.print("create tcp err\r\n");
  }
}
