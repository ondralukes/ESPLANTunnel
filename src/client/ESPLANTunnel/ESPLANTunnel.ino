#include <Ticker.h>

#include <ESP8266Ping.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define ULONG_MAX 4294967295



#define MAXSEND 4096     //////////////////////Must be same as in swr.php
#define MAXREAD 4096

#define SIGLED LED_BUILTIN

#define LEDSHORTDELAY 125
#define LEDLONGDELAY 500
char ssid[256] __attribute__ ((section (".noinit")));
char password[100] __attribute__ ((section (".noinit")));

WiFiClient client;
uint8_t SIGLEDcountnext = 10;

Ticker wdt;
volatile int wdtcounter = 0;
volatile int wdttime = 10;
char dataout[MAXREAD*2+5];
char response[17100];
bool connected = false;
void ISRwdt(){
  wdtcounter++;
  Serial.print("WDT:");
  Serial.println(wdtcounter);
  if(wdtcounter >= wdttime){
    ESP.reset();
    }
   SIGLEDUpdate();
  }
void WDTFeed(){
  wdtcounter = 0;
  }
void config(){
  Serial.println("Scanning WiFi networks");
  byte num = WiFi.scanNetworks();
  Serial.print("Select network (");
  Serial.print(num);
  Serial.println(" networks available):");
  for(uint8_t i = 0;i<num;i++){
    Serial.print(i);
    Serial.print(") SSID: ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" RSSI: ");
    Serial.print(WiFi.RSSI(i));
    Serial.println(" dBm");
    }
    char buf[256];
    uint8_t i = 0;
  while(1){
    Serial.print("Enter number from 0 to ");
    Serial.print(num);
    Serial.println(" or c for other network");
    while(Serial.available()==0){WDTFeed();};
    i=0;
    while(1){
      WDTFeed();
      if(Serial.available()>0){
      char c = Serial.read();
      if(c=='\n') break;
      if(c!='\r'){
    buf[i] = c;
    i++;
      }
      }
    delay(10);
    }
    buf[i] = '\0';
    Serial.println(buf);
    if(buf[0]=='c'){
      Serial.println("Enter network SSID:");
      while(Serial.available()==0);
      i=0;
      while(1){
      if(Serial.available()>0){
      char c = Serial.read();
      if(c=='\n') break;
      if(c!='\r'){
    buf[i] = c;
    i++;
      }
      }
    delay(10);
    }
    buf[i] = '\0';
    strcpy(ssid,buf);
    break;
      } else{
        byte id = atoi(buf);
        if(id < num){
          WiFi.SSID(id).toCharArray(ssid,256);
          break;
          }
        }
    }
  Serial.println("Enter password: ");
  while(Serial.available()==0){WDTFeed();};
      i=0;
      while(1){
        WDTFeed();
      if(Serial.available()>0){
      char c = Serial.read();
      if(c=='\n') break;
      if(c!='\r'){
    buf[i] = c;
    i++;
      }
      }
    delay(10);
    }
    buf[i] = '\0';
    strcpy(password,buf);
}
void setup() {
  wdt.attach(1,ISRwdt);
  pinMode(SIGLED,OUTPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Reset after flashing!");
  while(!Serial);
  bool ok = true;
  do{
    ok=true;
    digitalWrite(SIGLED,LOW);
    Serial.println("Type anything to config");
    delay(3000);
    if(Serial.available() > 0){
      while(Serial.available() > 0){
        Serial.read();
        }
  config();
    }
    Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(". Type anything to cancel.");
     WiFi.mode(WIFI_STA);
     WiFi.begin(ssid,password);
     while (WiFi.status() != WL_CONNECTED) {
     digitalWrite(SIGLED,HIGH);
    delay(LEDSHORTDELAY);
    digitalWrite(SIGLED,LOW);
    delay(LEDSHORTDELAY);
    Serial.print(".");
    if(Serial.available()>0){
      WDTFeed();
        while(Serial.available()>0)Serial.read();
        Serial.println();
        Serial.println("Aborted");
        ok = false;
        break;
      }
  }
  } while(ok==false);
  Serial.println();
  Serial.println("Setup done!");
  strcpy(dataout,"CRESET");
}

void loop() {
  WDTFeed();
long rssi = WiFi.RSSI();
if(rssi < -70){
  SIGLEDcountnext = 2;
  } else if(rssi <-60){
    SIGLEDcountnext = 4;
    } else if(rssi <-50){
    SIGLEDcountnext = 6;
    } else {
    SIGLEDcountnext = 8;
    }
Serial.print("RSSI: ");
Serial.print(rssi);
Serial.println(" dBm");
WiFiClient httpclient;
HTTPClient http;
Serial.print("Request");
Serial.println(dataout);
if(http.begin(httpclient,"http://ondralukes.cz/wifi/swr.php?key=*************")){
  http.addHeader("Content-Type", "text/plain");
  char tmp[10];
   sprintf(tmp, "%d", strlen(dataout));
  http.addHeader("Content-Length", tmp);
  int httpCode = http.POST(dataout);
  dataout[0] = '\0';
  response[0] = '\0';
  response[1] = '\0';
  if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          http.getString().toCharArray(response,5100);
          Serial.println(response);
          WDTFeed();
          if(!client.connected()&&connected){
            strcpy(dataout,"CDisconnected");
            connected= false;
            } else if(response[1] == 'C'){
            CTLExec(response);
          } else {
          if(response[1] == 'D') SendData(response);
          ReadData();
          }
        }
      }
      http.end();
  }

SIGLEDUpdate();

}
volatile unsigned long SIGLEDlast = millis();
volatile unsigned long SIGLEDdelay = 500;
volatile bool SIGLEDstate = true;
volatile uint8_t SIGLEDcount = 10;
volatile uint8_t SIGLEDcountd = 0;
void SIGLEDUpdate(){
   if(timer(SIGLEDlast,millis()) > SIGLEDdelay){
     SIGLEDstate = !SIGLEDstate;
     digitalWrite(SIGLED,SIGLEDstate);
     SIGLEDlast = millis();
     SIGLEDcountd++;
     if(SIGLEDcountd==SIGLEDcount){
       SIGLEDcountd =0;
       SIGLEDcount = SIGLEDcountnext;
       SIGLEDdelay=LEDLONGDELAY;
      } else{
        SIGLEDdelay=LEDSHORTDELAY;
        }
    }
  }
unsigned long timer(unsigned long start,unsigned long now){
    if(now > start){
       return now - start;
      } else{
        return now + (ULONG_MAX - start);
        }
  }
void CTLExec(char * msg){
  if(strcmp(msg,"^Ctest")==0){
     strcpy(dataout,"COK");
   } else if(strcmp(msg,"^Cipconfig")==0){
    dataout[0] = '\0';
    IPAddress ip = WiFi.localIP();
    IPAddress gateway = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();
    sprintf(dataout, "CIP:%d.%d.%d.%d\nSubnet:%d.%d.%d.%d\nGateway:%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3],subnet[0],subnet[1],subnet[2],subnet[3],gateway[0],gateway[1],gateway[2],gateway[3]);
    } else if(strcmp(msg,"^Cipscan")==0){
      dataout[0] = '\0';
      strcpy(dataout,"C");
      unsigned long start = millis();
      IPAddress ip = WiFi.localIP();
      char tmp[20];
      for(uint8_t i = 0;i!=255;i++){
       ip[3] = i;
        bool ret = Ping.ping(ip,1);
        Serial.println(ip);
     if(ret){
      tmp[0] ='\0';
      sprintf(tmp,"%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
      strcat(dataout,tmp);
      }
  }
      tmp[0] ='\0';
      sprintf(tmp,"Scan complete in %ums",timer(start,millis()));
      strcat(dataout,tmp);
      } else if(strncmp(msg,"^Cportscan",10) == 0) {
        strcpy(dataout,"C");
        unsigned long start = millis();
        IPAddress ip;
        char *whs0 = strchr(msg+10,' ');
        char* whs1 = strchr(whs0+1,' ');
        char* whs2 = strchr(whs1 + 1,' ');
        *whs1 = '\0';
        *whs2 = '\0';
        char * olddot = whs0;
        char * dot = whs0+1;
        for(uint8_t i = 0;i<4;i++){
          if(i<3)dot=strchr(olddot+1,'.');
          *dot = '\0';
          ip[i] = atoi(olddot+1);
          olddot = dot;
          }
        uint16_t from = atoi(whs1 +1);
        uint16_t to = atoi(whs2+1);
        Serial.print("IP ");
        Serial.print(ip);
        Serial.print(" From ");
        Serial.print(from);
        Serial.print(" To ");
        Serial.println(to);
        char tmp[20];
        for(uint16_t i = from;i<=to;i++){
          Serial.println(i);
          if(client.connect(ip,i)){
            tmp[0] ='\0';
            sprintf(tmp,"%d\n",i);
            strcat(dataout,tmp);
            }
            client.stop();
            if(i==to&&to==65535)break;
          }
          tmp[0] ='\0';
          sprintf(tmp,"Scan complete in %ums",timer(start,millis()));
          strcat(dataout,tmp);
        } else if(strncmp(msg,"^Csetwdt",8)==0){
          dataout[0] = '\0';
          char *whs0 = strchr(msg+8,' ');
          *whs0 = '\0';
          wdttime = atoi(whs0+1);

          sprintf(dataout,"CWatchdog set to %ds",wdttime);
      } else if(strncmp(msg,"^Csettimeout",12)==0){
          dataout[0] = '\0';
          char *whs0 = strchr(msg+8,' ');
          *whs0 = '\0';
          client.setTimeout(atoi(whs0+1));

          sprintf(dataout,"CTimeout set to %dms",atoi(whs0+1));
      } else if(strncmp(msg,"^Cconnect",9)==0){
          dataout[0] = '\0';
           IPAddress ip;
        char *whs0 = strchr(msg+9,' ');
        char* whs1 = strchr(whs0+1,' ');
        *whs1 = '\0';
        char * olddot = whs0;
        char * dot = whs0+1;
        for(uint8_t i = 0;i<4;i++){
          if(i<3)dot=strchr(olddot+1,'.');
          *dot = '\0';
          ip[i] = atoi(olddot+1);
          olddot = dot;
          }
        uint16_t port = atoi(whs1 +1);
        Serial.print("IP ");
        Serial.print(ip);
        Serial.print(" Port ");
        Serial.print(port);
        if(client.connect(ip,port)){
          strcpy(dataout,"CConnected");
          connected = true;
        } else{
          strcpy(dataout,"CFailed");
          }
      } else if(strncmp(msg,"^Cdisconnect",12)==0){
          dataout[0] = '\0';
          client.stop();
          strcpy(dataout,"CDisconnected");
          connected = false;
      } else if(strncmp(msg,"^Chostip",8)==0){
         dataout[0] = '\0';
        char *whs0 = strchr(msg+8,' ');
        *whs0 = '\0';
        IPAddress ip;
        WiFi.hostByName(whs0+1,ip);
        sprintf(dataout,"C%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
      } else {
          strcpy(dataout,"CUnknown command");
          }
  }
void SendData(char * msg){
  Serial.println("Writing to client");
  for(int i = 0;i<MAXSEND;i++){
    if(*(msg+i+2) == '\0') break;
    if(*(msg+i+2) == '\\'){
      if(*(msg+i+3) == '\\')client.write((uint8_t)'\\');
      if(*(msg+i+3) == '0')client.write((uint8_t)'\0');
      i++;
      } else {
  client.write(*(msg+i+2));
      }
  }
  client.flush();
}
void ReadData(){

  uint16_t count = 0;
  if(client.available()>0){
    Serial.println("Reading from client");
    strcpy(dataout,"D");
    char * ptr = &dataout[1];
    while(client.available() >0 && count < MAXREAD){
      *ptr = client.read();
      if(*ptr == '\\'){
        ptr++;
        count++;
        *ptr = '\\';
        } else if(*ptr == '\0'){
          *ptr = '\\';
          ptr++;
          count++;
          *ptr = '0';
          }
      ptr++;
      count++;
      }
      *ptr='\0';
       Serial.print("Read ");
       Serial.print(count);
       Serial.println(" bytes");
    }
  }
