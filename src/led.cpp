
#include <PxMatrix.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <WireSlave.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_SLAVE_ADDR 0x04
#define IP_PIN 35
// Pins for LED MATRIX
#ifdef ESP32

#define P_LAT 32
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#endif


#define matrix_width 64
#define matrix_height 64

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time=10; //30-70 is usually fine
bool rotation=false; //od 0 do 3 po 90 stopni kazdy
uint8_t noInternet=0;
//PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
//PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
PxMATRIX display(64,64,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);

// Some standard colors
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);
uint16_t myCOLORS[8]={myRED,myGREEN,myBLUE,myWHITE,myYELLOW,myCYAN,myMAGENTA,myBLACK};
String jsonStr;
void receiveEvent(int howMany);
const unsigned char arrowdown[] PROGMEM = {
  B00110000,
  B00110000,
  B00110000,
  B00110000,
  B11111100,
  B11111100,
  B01111000,
  B00110000
};

const unsigned char arrowup[] PROGMEM = {
  B00110000,
  B01111000,
  B11111100,
  B11111100,
  B00110000,
  B00110000,
  B00110000,
  B00110000
};

const unsigned char noNetworkIcon[] PROGMEM = {
  B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
  B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
  B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
  B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000001,B00000000,
  B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000011,B10000000,
  B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000111,B11000000,
  B00000000,B00000000,B00000000,B00000111,B11100000,B00000000,B00001111,B11100000,
  B00000000,B00000000,B00000000,B11111111,B11111111,B00000000,B00011111,B11110000,
  B00000000,B00000000,B00000111,B11111111,B11111111,B11000000,B00111111,B11111000,
  B00000000,B00000000,B00011111,B11111111,B11111111,B11110000,B01111111,B11110000,
  B00000000,B00000000,B00111111,B11111111,B11111111,B11111100,B11111111,B11100000,
  B00000000,B00000000,B11111111,B11111110,B01111111,B11111111,B11111111,B11000000,
  B00000000,B00000001,B11111110,B01111100,B00111110,B01111111,B11111111,B10000000,
  B00000000,B00000011,B11111000,B11111000,B00011110,B00011111,B11111111,B00000000,
  B00000000,B00000111,B11100000,B11110000,B00011111,B00001111,B11111110,B00000000,
  B00000000,B00001111,B11000001,B11110000,B00001111,B00011111,B11111100,B00000000,
  B00000000,B00011111,B11111111,B11111111,B11111111,B11111111,B11111000,B00000000,
  B00000000,B00011111,B11111111,B11111111,B11111111,B11111111,B11111000,B00000000,
  B00000000,B00111111,B11111111,B11111111,B11111111,B11111111,B11111100,B00000000,
  B00000000,B01111111,B11111111,B11111111,B11111111,B11111111,B11111100,B00000000,
  B00000000,B01111111,B11111111,B11111111,B11111111,B11111111,B11111110,B00000000,
  B00000000,B11111000,B00000011,B11000000,B00000111,B11111111,B00111110,B00000000,
  B00000000,B11111000,B00000011,B11000000,B00001111,B11111110,B00011111,B00000000,
  B00000000,B11110000,B00000111,B11000000,B00011111,B11111100,B00011111,B00000000,
  B00000001,B11110000,B00000111,B11000000,B00111111,B11111000,B00001111,B00000000,
  B00000001,B11100000,B00000111,B11000000,B01111111,B11110000,B00001111,B10000000,
  B00000001,B11100000,B00000111,B10000000,B11111111,B11100000,B00000111,B10000000,
  B00000001,B11100000,B00000111,B10000001,B11111111,B11100000,B00000111,B10000000,
  B00000011,B11100000,B00000111,B10000011,B11111111,B11100000,B00000111,B10000000,
  B00000011,B11100000,B00000111,B10000111,B11111111,B11100000,B00000111,B10000000,
  B00000011,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B10000000,
  B00000011,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B10000000,
  B00000011,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B10000000,
  B00000011,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B10000000,
  B00000011,B11100000,B00000111,B11111111,B11100011,B11100000,B00000111,B10000000,
  B00000011,B11100000,B00000111,B11111111,B11000011,B11100000,B00000111,B10000000,
  B00000001,B11100000,B00000111,B11111111,B10000011,B11100000,B00000111,B10000000,
  B00000001,B11100000,B00000111,B11111111,B00000011,B11100000,B00000111,B10000000,
  B00000001,B11100000,B00001111,B11111110,B00000011,B11000000,B00001111,B10000000,
  B00000001,B11110000,B00011111,B11111100,B00000011,B11000000,B00001111,B00000000,
  B00000000,B11110000,B00111111,B11111000,B00000011,B11000000,B00011111,B00000000,
  B00000000,B11111000,B01111111,B11110000,B00000011,B11000000,B00011111,B00000000,
  B00000000,B11111000,B11111111,B11100000,B00000011,B11000000,B00111110,B00000000,
  B00000000,B01111101,B11111111,B11000000,B00000111,B11000000,B00111110,B00000000,
  B00000000,B01111111,B11111111,B11111111,B11111111,B11111111,B11111100,B00000000,
  B00000000,B00111111,B11111111,B11111111,B11111111,B11111111,B11111100,B00000000,
  B00000000,B00011111,B11111111,B11111111,B11111111,B11111111,B11111000,B00000000,
  B00000000,B00011111,B11111111,B11111111,B11111111,B11111111,B11110000,B00000000,
  B00000000,B00111111,B11111001,B11110000,B00001111,B00000111,B11100000,B00000000,
  B00000000,B01111111,B11110000,B11110000,B00011111,B00001111,B11100000,B00000000,
  B00000000,B11111111,B11111000,B11111000,B00011111,B00011111,B11000000,B00000000,
  B00000001,B11111111,B11111110,B01111000,B00111110,B01111111,B00000000,B00000000,
  B00000011,B11111111,B11111111,B11111100,B01111111,B11111110,B00000000,B00000000,
  B00000111,B11111111,B00111111,B11111111,B11111111,B11111100,B00000000,B00000000,
  B00001111,B11111110,B00001111,B11111111,B11111111,B11110000,B00000000,B00000000,
  B00011111,B11111100,B00000011,B11111111,B11111111,B11000000,B00000000,B00000000,
  B00001111,B11111000,B00000000,B11111111,B11111110,B00000000,B00000000,B00000000,
  B00000111,B11110000,B00000000,B00000111,B11100000,B00000000,B00000000,B00000000,
  B00000011,B11100000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
  B00000001,B11000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
  B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
  B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
  B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
  B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000
};

const unsigned char hourglass[] PROGMEM = {
  B00001111,B11111111,B11111111,B11110000,
  B00001111,B11111111,B11111111,B11110000,
  B00001111,B11111111,B11111111,B11110000,
  B00000011,B00000000,B00000000,B11000000,
  B00000011,B00000000,B00000000,B11000000,
  B00000011,B10000000,B00000001,B11000000,
  B00000011,B10000000,B00000001,B11000000,
  B00000001,B10000000,B00000001,B10000000,
  B00000001,B11011111,B11111011,B10000000,
  B00000001,B11011111,B11111011,B10000000,
  B00000000,B11101111,B11110111,B00000000,
  B00000000,B01110111,B11101110,B00000000,
  B00000000,B01111011,B11011110,B00000000,
  B00000000,B00111001,B10011100,B00000000,
  B00000000,B00011101,B10111000,B00000000,
  B00000000,B00001101,B10110000,B00000000,
  B00000000,B00001101,B10110000,B00000000,
  B00000000,B00011101,B10111000,B00000000,
  B00000000,B00111101,B10111100,B00000000,
  B00000000,B01111001,B10011110,B00000000,
  B00000000,B11110001,B10001110,B00000000,
  B00000000,B11100001,B10000111,B00000000,
  B00000001,B11000001,B10000011,B10000000,
  B00000001,B11000111,B11100011,B10000000,
  B00000011,B10011111,B11111001,B10000000,
  B00000011,B10111111,B11111101,B11000000,
  B00000011,B10111111,B11111101,B11000000,
  B00000011,B00111111,B11111100,B11000000,
  B00000011,B00000000,B00000000,B11000000,
  B00001111,B11111111,B11111111,B11110000,
  B00001111,B11111111,B11111111,B11110000,
  B00001111,B11111111,B11111111,B11110000
};



#ifdef ESP8266
// ISR for display refresh
void display_updater()
{
  display.display(display_draw_time);
}
#endif

#ifdef ESP32
void IRAM_ATTR display_updater(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif
void receiveEvent(int howMany);


struct gecko {
  String name;
  double price;
  double percentage;
};
struct binance {
  String pair;
  double price;
  double percentage;
};
struct dex {
  String name;
  double price;
  double percentage;
};
struct binance Pair1;
struct binance Pair2;
struct binance Pair3;

struct gecko Coin1;
struct gecko Coin2;
struct gecko Coin3;

struct dex Contract1;
struct dex Contract2;
struct dex Contract3;

String IP = "1.1.1.1";
String vs;

void display_update_enable(bool is_enable)
{


  if (is_enable)
  {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 4000, true);
    timerAlarmEnable(timer);
  }
  else
  {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }

}
int brightnessLevel = 200;
void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB)
{
    uint16_t text_length = text.length();
    display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
    display.setTextSize(1);
    display.setTextColor(display.color565(colorR,colorG,colorB));

    // Asuming 5 pixel average character width
    for (int xpos=matrix_width; xpos>-(matrix_width+text_length*7); xpos--)
    {
      display.setTextColor(display.color565(colorR,colorG,colorB));
      display.clearDisplay();
      display.setCursor(xpos,ypos);
      display.println(text);
      delay(scroll_delay);
      yield();

      // This might smooth the transition a bit if we go slow
    //  display.setTextColor(display.color565(colorR/4,colorG/4,colorB/4));
    //   display.setCursor(xpos-1,ypos);
     //  display.println(text);

      delay(scroll_delay/5);
      yield();

    }
}

String priceParse(double price){
  String priceStr;
  if((price/1000000)<1 && (price/100000)>=1){
  	priceStr = String(price,1);
  }
    else if((price/100000)<1 && (price/10000)>=1){
  	priceStr = String(price,2);
  }
  else if((price/10000)<1 && (price/1000)>=1){
  	priceStr = String(price,3);
  }
  else if((price/1000)<1 && (price/100)>=1){
  	priceStr = String(price,4);
  }
  else if((price/100)<1 && (price/10)>=1){
  	priceStr = String(price,5);
  }
  else if((price/10)<1 && (price/1)>=1){
  	priceStr = String(price,6);
  }
  else if((price/1)<1){
  	priceStr = String(price,6);
  }
  return priceStr;
}
String percentageParse(double percentage){
  String percentageStr;
    if((percentage/100)>=1){
  	percentageStr = "99";
  }
  else if((percentage/100)<1 && (percentage/10)>=1){
  	percentageStr = String(percentage,0);
  }
  else if((percentage/10)<1 && (percentage/1)>=1){
  	percentageStr = String(percentage,1);
  }
  else if((percentage/1)<1){
  	percentageStr = String(percentage,1);
  }
  return percentageStr;
}

void printPrice(String name, double price, double percentage, int row){
  if(name=="EMPTY"){

  }
  else {
    bool loss = false;
    uint16_t myCOLOR = myGREEN;
    if(percentage<0){
      loss = true;
      myCOLOR = myRED;
      percentage = percentage*-1;
    }
    name.toUpperCase();
    display.drawRect(0,0+row*18, 64, 18, myBLACK);
    String priceStr = priceParse(price);
    int commaIndex = priceStr.indexOf('.');
    int secondCommaIndex = priceStr.indexOf(',', commaIndex + 1);
    String beforeComma = priceStr.substring(0,commaIndex);
    String afterComma = priceStr.substring(commaIndex +1, secondCommaIndex); //comma splicing
    String percentageStr = percentageParse(percentage);    
    int percentageCommaIndex = percentageStr.indexOf('.');
    int percentageSecondCommaIndex = percentageStr.indexOf('.', percentageCommaIndex + 1);
    String percentageBeforeComma = percentageStr.substring(0,percentageCommaIndex);
    String percentageAfterComma = percentageStr.substring(percentageCommaIndex +1, percentageSecondCommaIndex);
    if(percentageStr.length()==2){
      percentageAfterComma="";
    }
    display.setTextColor(myWHITE,myBLACK);
    
    if(name.length()<11){
      
    }
    else{
      name.remove(10,name.length()-10);
      //15
      //11 12 13 14 
    }
    display.setCursor(0,0+row*18);
    display.println(name);
    display.setCursor(0,9+row*18);
    display.setTextColor(myCOLOR, myBLACK);
    display.println(beforeComma); 
    int16_t  x1, y1;
    uint16_t w, h;
    display.getTextBounds(beforeComma,0,9+row*18,&x1,&y1,&w,&h); 
    display.setCursor(x1+w+1, 9+row*18); //after comma cursor
    display.println(afterComma); //decimal cena
    display.drawFastVLine(x1+w,y1+h-2,2,myWHITE); 
    if(loss){
      display.drawRect(x1+w+(afterComma.length()*6)+1,y1,18,8,myBLACK);
      display.drawBitmap(x1+w+(afterComma.length()*6)+1,y1,arrowdown,6,8,myCOLOR); //strzala 
    }
    else{
      display.drawRect(x1+w+(afterComma.length()*6)+1,y1,18,8,myBLACK);
      display.drawBitmap(x1+w+(afterComma.length()*6)+1,y1,arrowup,6,8,myCOLOR); //strzala 
    }

    display.setCursor(x1+w+(afterComma.length()*6)+1+7,y1); // odstep od strzaly na procentowe
    display.println(percentageBeforeComma); //procentowa
    if(!(percentageAfterComma=="")){
    display.setCursor(x1+w+(afterComma.length()*6)+1+7+6+2,9+row*18); // odstep od przecinka na procentowe
    display.println(percentageAfterComma); // dec procentowa
    display.drawFastVLine(x1+w+(afterComma.length()*6)+1+7+6,y1+h-2,2,myWHITE); //decimal procentowy

    }
  }
}
void printCurrency(String curr) {
  display.setCursor(0,56);
  display.setTextColor(myYELLOW);
  curr.toUpperCase();
  display.println(curr);
}
void apiErr(){
  display.clearDisplay();
  display.setCursor(5,24);
  display.setTextColor(myWHITE);
  display.println("API ERROR");
}
void printIP(String address){
  display.clearDisplay();
  display.setCursor(0,24);
  display.setTextColor(myWHITE);
  display.println(address);
  int i = 0;
  while(digitalRead(IP_PIN)==1){
    i++;
    delay(100);
    if(i>20){
      rotation=!rotation;
      display.clearDisplay();
      display.setRotate(rotation);
      display.println("OBROCONO");
      i=0;
    }
  }
}
void printPending(){

  display.clearDisplay();
  display.setRotation(3);
  display.setCursor(0,56);
  
  display.setTextColor(myWHITE);
  if(IP=="1.1.1.1"){
    display.drawBitmap(16,16,hourglass,32,32,myYELLOW);
    display.println("...");
  }
  else{
    display.drawBitmap(16,16,hourglass,32,32,myYELLOW);
    display.println("WiFi OK");
  }
}
void printNoInternet(){

  display.clearDisplay();
  display.setCursor(0,0); 
  display.setTextColor(myWHITE);
  display.drawBitmap(0,0,noNetworkIcon,64,64,myYELLOW);
}
void apMode(){
  Serial.println("AA");
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(myWHITE);
  display.println("Zresetowano ustawienia WiFi");
  delay(10000);
}
void setup() {

    Serial.begin(115200);
    display.begin(32);
    display.setBrightness(brightnessLevel);
    pinMode(IP_PIN, INPUT_PULLUP);

    bool success = WireSlave.begin(SDA_PIN, SCL_PIN, I2C_SLAVE_ADDR);
    if (!success) {
      Serial.println("I2C slave init failed");
      while(1) delay(100);
    }
    WireSlave.onReceive(receiveEvent);

  display_update_enable(true); 
  display.setRotate(rotation);
  if(digitalRead(IP_PIN)) apMode();
  else printPending();
}
union single_double{
  uint8_t two[2];
  uint16_t one;
} this_single_double;
int x = 0;
int btnwifitimestamp = 0;
int gecko_on = 10000;
int binance_on = 20000;
int dex_on = 30000;
int looptime = 20000;
int start = 0;
bool showingIP = false;
void loop(){ 
      x++;
      WireSlave.update();
      if(noInternet==0){
        if(Coin1.name!=NULL && Coin2.name!=NULL && Coin3.name!=NULL && vs!=NULL){
          if(Coin1.name!="EMPTY" || Coin2.name!="EMPTY" || Coin3.name!="EMPTY"){
            start = 0;
            if(x==gecko_on){
              showingIP =false;
              display.clearDisplay();
              display.setBrightness(brightnessLevel);
              printPrice(Coin1.name,Coin1.price,Coin1.percentage,0);
              printPrice(Coin2.name,Coin2.price,Coin2.percentage,1);
              printPrice(Coin3.name,Coin3.price,Coin3.percentage,2);
              printCurrency(vs);
              
            }
          }
        }
        else{
          if(x%100 == 0){
            printPending();
            display.setBrightness(brightnessLevel);
          }
        }
        if(Pair1.pair!=NULL && Pair2.pair!=NULL && Pair3.pair!=NULL){
          if(Pair1.pair!="EMPTY" || Pair2.pair!="EMPTY" || Pair3.pair!="EMPTY"){   
            if(dex_on!=30000) dex_on=30000;
            if(x==binance_on){
              showingIP = false;
              display.clearDisplay();
              display.setBrightness(brightnessLevel);
              printPrice(Pair1.pair,Pair1.price,Pair1.percentage,0);
              printPrice(Pair2.pair,Pair2.price,Pair2.percentage,1);
              printPrice(Pair3.pair,Pair3.price,Pair3.percentage,2);
              
            }
          }
          else{
            if(dex_on!=20000) dex_on=20000;
          }
        }
        if(Contract1.name!=NULL && Contract2.name!=NULL && Contract3.name!=NULL){
          if(Contract1.name!="EMPTY" || Contract2.name!="EMPTY" || Contract3.name!="EMPTY"){   
          
          if(Pair1.pair!=NULL && Pair2.pair!=NULL && Pair3.pair!=NULL){  //binance missing fix timing
            if(Pair1.pair!="EMPTY" || Pair2.pair!="EMPTY" || Pair3.pair!="EMPTY"){
                looptime=30000;
            }
            else {
              looptime=20000;
            }
          }
          else {
            looptime=20000;
          } 
          if(Coin1.name!=NULL && Coin2.name!=NULL && Coin3.name!=NULL && vs!=NULL){
            if(Coin1.name!="EMPTY" || Coin2.name!="EMPTY" || Coin3.name!="EMPTY"){
              start = 0;
            }
            else {
              start = 9999;
            }
          }
          else {
            start = 9999;
          }

            if(x==dex_on){
              showingIP = false;
              display.clearDisplay();
              display.setBrightness(brightnessLevel);
              printPrice(Contract1.name,Contract1.price,Contract1.percentage,0);
              printPrice(Contract2.name,Contract2.price,Contract2.percentage,1);
              printPrice(Contract3.name,Contract3.price,Contract3.percentage,2);
              
            }
          }
          else{
            looptime=20000;
          }
        }
      }
      else{
        printNoInternet();
      }
      if(digitalRead(IP_PIN)==1){
        btnwifitimestamp++;
      }
      else{
        btnwifitimestamp=0;
      }
      if(btnwifitimestamp>7){
        if(IP!="1.1.1.1"){
          printIP(IP);
          delay(20000);
          btnwifitimestamp=0;
        }
      }
      if(x>looptime) x=start;

      delay(1);
      }

void receiveEvent(int howMany)
{
    jsonStr = "";
    while (1 < WireSlave.available()) // loop through all but the last byte
    {
        char c = WireSlave.read();  // receive byte as a character
        jsonStr.concat(c);
    }
    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
    }
    JsonObject coin1 = doc["coin1"];
    JsonObject coin2 = doc["coin2"];
    JsonObject coin3 = doc["coin3"];
    JsonObject pair1 = doc["pair1"];
    JsonObject pair2 = doc["pair2"];
    JsonObject pair3 = doc["pair3"];
    JsonObject contract1 = doc["contract1"];
    JsonObject contract2 = doc["contract2"];
    JsonObject contract3 = doc["contract3"];
    JsonObject ipObj = doc["ip"];
    JsonObject vsObj = doc["vs"];   
    int brightness = doc["brightness"];
    String ipip = doc["ip"]; 
    String vsvs = doc["vs"];
    uint8_t internet = doc["internet"];
    JsonObject ap = doc["ap"];
    if(ipip==NULL || ipip=="null" || ipip == "" || ipip=="0"){
      
    }
    else{
      IP = ipip;
      Serial.println(IP);
    }
    if(internet==NULL){
    }
    else{
      if(internet==1) noInternet=1;
      if(internet==2) noInternet=0; 
      Serial.println(internet);
    }
    if(vsvs==NULL || vsvs=="null" || vsvs == "" || vsvs=="0"){
      
    }
    else{
      vs = vsvs;
      Serial.println(vs);
    }
    if(brightness!=NULL){
      brightnessLevel = brightness;
    }
    if(coin1!=NULL){
      String name1 = coin1["name"];
      Serial.println(name1);
      Coin1.name = name1;
      Coin1.price = coin1["price"];
      Coin1.percentage = coin1["percentage"];
    }
    if(coin2!=NULL){
      String name2 = coin2["name"];
      Coin2.name = name2;
      Coin2.price = coin2["price"];
      Coin2.percentage = coin2["percentage"];
    }
    if(coin3!=NULL){
      String name3 = coin3["name"];
      Coin3.name = name3;
      Coin3.price = coin3["price"];
      Coin3.percentage = coin3["percentage"];
    }
    if(pair1!=NULL){
      String pairname1 = pair1["name"];
      Serial.println(pairname1);
      Pair1.pair = pairname1;
      Pair1.price = pair1["price"];
      Pair1.percentage = pair1["percentage"];
    }
    if(pair2!=NULL){
      String pairname2 = pair2["name"];
      Serial.println(pairname2);
      Pair2.pair = pairname2;
      Pair2.price = pair2["price"];
      Pair2.percentage = pair2["percentage"];
    }
    if(pair3!=NULL){
      String pairname3 = pair3["name"];
      Serial.println(pairname3);
      Pair3.pair = pairname3;
      Pair3.price = pair3["price"];
      Pair3.percentage = pair3["percentage"];
    }

    if(contract1!=NULL){
      String contractname1 = contract1["name"];
      Serial.println(contractname1);
      Contract1.name = contractname1;
      Contract1.price = contract1["price"];
      Contract1.percentage = contract1["percentage"];
    }
    if(contract2!=NULL){
      String contractname2 = contract2["name"];
      Serial.println(contractname2);
      Contract2.name = contractname2;
      Contract2.price = contract2["price"];
      Contract2.percentage = contract2["percentage"];
    }
    if(contract3!=NULL){
      String contractname3 = contract3["name"];
      Serial.println(contractname3);
      Contract3.name = contractname3;
      Contract3.price = contract3["price"];
      Contract3.percentage = contract3["percentage"];
    }
    if(ap!=NULL){
      apMode();
    }
    


    Serial.println(jsonStr);

  
}
