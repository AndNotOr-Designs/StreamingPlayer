    /////////////////////////////////////////////////////////////////
   //         ESP32 Internet Radio Project     v1.00              //
  //       Get the latest version of the code here:              //
 //          http://educ8s.tv/esp32-internet-radio              //
/////////////////////////////////////////////////////////////////


#include <VS1053.h>  //https://github.com/baldram/ESP_VS1053_Library
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wifi.h>

#define VS1053_CS    32 
#define VS1053_DCS   33  
#define VS1053_DREQ  35 

#define VOLUME  95 // volume level 0-100

int radioStation = 0;
int previousRadioStation = -1;
const int previousButton = 12;
const int nextButton = 13;

char ssid[] = "CPC Guest";            //  your network SSID (name) 
char pass[] = "Luke11:9";   // your network password

// Few Radio Stations
char *host[4] = {"149.255.59.162","radiostreaming.ert.gr","realfm.live24.gr", "secure1.live24.gr"};
      // http://dir.xiph.org/by_format/MP3
          // https://nprdmp-live01-mp3.akacast.akamaistream.net/7/998/364916/v1/npr.akacast.akamaistream.net/nprdmp_live01_mp3
          // http://broadcast.infomaniak.ch/jazz-wr03-128.mp3.m3u
          // http://swingfm.ice.infomaniak.ch/swingfm-128.mp3.m3u
          // http://www.radio.pionier.net.pl/stream.pls?radio=tuba5
          // http://dir.xiph.org/listen/1327342/listen.m3u
          // http://dir.xiph.org/listen/1097737/listen.m3u
    
char *path[4] = {"/1","/ert-kosmos","/realfm","/skai1003"};
int   port[4] = {8062,80,80,80};

int status = WL_IDLE_STATUS;
WiFiClient  client;
uint8_t mp3buff[32];   // vs1053 likes 32 bytes at a time

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

void IRAM_ATTR previousButtonInterrupt() {

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
 
 if (interrupt_time - last_interrupt_time > 200) 
 {
   if(radioStation>0)
    radioStation--;
    else
    radioStation = 3;
 }
 last_interrupt_time = interrupt_time;
}

void IRAM_ATTR nextButtonInterrupt() {

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
 
 if (interrupt_time - last_interrupt_time > 200) 
 {
   if(radioStation<4)
    radioStation++;
    else
    radioStation = 0;
 }
 last_interrupt_time = interrupt_time;
}

void setup () {

   Serial.begin(9600);
   delay(500);
   SPI.begin();

   pinMode(previousButton, INPUT_PULLUP);
   pinMode(nextButton, INPUT_PULLUP);

   attachInterrupt(digitalPinToInterrupt(previousButton), previousButtonInterrupt, FALLING);
   attachInterrupt(digitalPinToInterrupt(nextButton), nextButtonInterrupt, FALLING);

   initMP3Decoder();

   connectToWIFI();

}

void loop() {
     
      if(radioStation!=previousRadioStation)
      {
           station_connect(radioStation);
           previousRadioStation = radioStation; 
      }
      
      if (client.available() > 0)
      {
        uint8_t bytesread = client.read(mp3buff, 32);
        player.playChunk(mp3buff, bytesread);
      }
}    
  
void station_connect (int station_no ) {
    if (client.connect(host[station_no],port[station_no]) ) Serial.println("Connected now"); 
    client.print(String("GET ") + path[station_no] + " HTTP/1.1\r\n" +
               "Host: " + host[station_no] + "\r\n" + 
               "Connection: close\r\n\r\n");   
                 endNextionCommand(); 
                 drawRadioStationName(station_no);
                 endNextionCommand();
  }

  void connectToWIFI()
  {
    WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
   }
   Serial.println("WiFi connected");
  }

  void initMP3Decoder()
  {
    player.begin();
    player.switchToMp3Mode(); // optional, some boards require this
    player.setVolume(VOLUME);
  }

  void endNextionCommand()
{
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void drawRadioStationName(int id)
{
  String command;
  switch (id)  
  {
    case 0:  command = "p1.pic=2"; Serial.print(command); endNextionCommand(); break; //1940 UK Radio
    case 1:  command = "p1.pic=3"; Serial.print(command); endNextionCommand(); break; //KOSMOS     GREEK
    case 2:  command = "p1.pic=4"; Serial.print(command); endNextionCommand(); break; //REAL FM    GREEK
    case 3:  command = "p1.pic=5"; Serial.print(command); endNextionCommand(); break; //SKAI 100.3 GREEK
  }
}
