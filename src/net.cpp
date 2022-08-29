#include <Arduino.h>
#include "SPIFFS.h" 
#include <HTTPClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <WirePacker.h>
#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_SLAVE_ADDR 0x04
#define DATA_STOP_TIME 1400
StaticJsonDocument<1024> config;
bool conn = false;
AsyncWebServer server(80);
int api_err_binance[5];
int api_err_gecko[5];
int api_err_dex[5];
String hostname = "coinLED";



void save() {
        File cfg = SPIFFS.open("/crypto.json", FILE_WRITE);
        String configStr;
        serializeJson(config, configStr);
        cfg.println(configStr);
        cfg.close();   
        ESP.restart();
        
}
void writeLCD(String name, double price, double percentage, int row){
    DynamicJsonDocument json(512);
    json["coin"+String(row)]["name"] = name;
    json["coin"+String(row)]["price"] = price;
    json["coin"+String(row)]["percentage"] = percentage;   
    WirePacker packer;
    char buffer[512];
    serializeJson(json, buffer);
    Serial.println(buffer);
    packer.print(buffer);
    packer.print("}");
    packer.end();
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    while (packer.available()) {    // write every packet byte
      Wire.write(packer.read());
    }
    Wire.endTransmission();  
}
void writeMeta(String ip, String vs){
    DynamicJsonDocument json(512);
    json["ip"] = ip;
    json["vs"] = vs;
    WirePacker packer;
    char buffer[512];
    serializeJson(json, buffer);
    Serial.println(buffer);
    packer.print(buffer);
    packer.print("}");
    packer.end();
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    while (packer.available()) {    // write every packet byte
      Wire.write(packer.read());
    }
    Wire.endTransmission();  
}
void writeBrightness(int brightness){
    DynamicJsonDocument json(128);
    json["brightness"] = brightness;
    WirePacker packer;
    char buffer[128];
    serializeJson(json, buffer);
    Serial.println(buffer);
    packer.print(buffer);
    packer.print("}");
    packer.end();
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    while (packer.available()) {    // write every packet byte
      Wire.write(packer.read());
    }
    Wire.endTransmission();  
}
void writeNoInternet(bool status){
//todo
}



void writeLCDBinance(String pair, double price, double percentage, int row){
    DynamicJsonDocument json(512);
    json["pair"+String(row)]["name"] = pair;
    json["pair"+String(row)]["price"] = price;
    json["pair"+String(row)]["percentage"] = percentage;   
    WirePacker packer;
    char buffer[512];
    serializeJson(json, buffer);
    Serial.println(buffer);
    packer.print(buffer);
    packer.print("}");
    packer.end();
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    while (packer.available()) {    // write every packet byte
      Wire.write(packer.read());
    }
    Wire.endTransmission();  
}
void writeLCDDEX(String name, double price, double percentage, int row){
    DynamicJsonDocument json(512);
    json["contract"+String(row)]["name"] = name;
    json["contract"+String(row)]["price"] = price;
    json["contract"+String(row)]["percentage"] = percentage;   
    WirePacker packer;
    char buffer[512];
    serializeJson(json, buffer);
    Serial.println(buffer);
    packer.print(buffer);
    packer.print("}");
    packer.end();
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    while (packer.available()) {    // write every packet byte
      Wire.write(packer.read());
    }
    Wire.endTransmission();  
}


void setup() {
    
    api_err_gecko[0] = 0;
    api_err_gecko[1] = 0;
    api_err_gecko[2] = 0;
    api_err_gecko[3] = 0;
    api_err_binance[0] = 0;
    api_err_binance[1] = 0;
    api_err_binance[2] = 0;
    api_err_binance[3] = 0;
    api_err_dex[0] = 0;
    api_err_dex[1] = 0;
    api_err_dex[2] = 0;
    api_err_dex[3] = 0;
    pinMode(23, INPUT);
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    Serial.begin(115200);

    // WiFi.mode(WiFi_STA); // it is a good practice to make sure your code sets wifi mode how you want it.
    Serial.println(F("Inizializing FS..."));
    if (SPIFFS.begin()){
        Serial.println(F("SPIFFS mounted correctly."));
    }else{
        Serial.println(F("!An error occurred during SPIFFS mounting"));
    }
    File configFile = SPIFFS.open("/crypto.json");
    String content = configFile.readString();
    Serial.println(content);
    deserializeJson(config, content);
    configFile.close();
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    wm.setHostname(hostname.c_str());
    wm.setWiFiAutoReconnect(true);
    //reset settings - wipe credentials for testing
    if(digitalRead(23)){
        Serial.println("ip sw while boot resetting wifi");
        wm.resetSettings();
    }
    

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result
    wm.setConnectTimeout(20);
    bool res;
     res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    //res = wm.autoConnect("CryptoLED","12345678"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        conn = true;
        delay(3000);
        Wire.begin(SDA_PIN, SCL_PIN);
        delay(2000);
        server.begin();
    }
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html;charset=UTF-8;charset=UTF-8");
    });
    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html;charset=UTF-8");
    });
    server.on("/gecko_select.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/gecko_select.html", "text/html;charset=UTF-8");
    }); 
    server.on("/gecko_list.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/gecko_list.html", "text/html;charset=UTF-8");
    }); 
    server.on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/favicon.png", "image/png");
    }); 
    server.on("/gecko_list.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/gecko_list.js", "text/javascript");
    }); 
    server.on("/binance_select.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/binance_select.html", "text/html;charset=UTF-8");
    }); 
  // Route to load style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });    
    server.on("/gecko", HTTP_GET, [](AsyncWebServerRequest *request){
        int paramsNr = request->params();
        JsonArray iter = config["gecko"];
        for (JsonArray::iterator it=iter.begin(); it!=iter.end(); ++it) { //wyczyszczenie tablicy z parami przed nadpisaniem z requesta
            iter.remove(it);
        }
        for(int i=0;i<paramsNr;i++){
            AsyncWebParameter* p = request->getParam(i);
            Serial.print("Param name: ");
            Serial.println(p->name());
            Serial.print("Param value: ");
            Serial.println(p->value());
            if(p->name()=="vs"){
                config["vs"] = p->value();
            }
            else {
                if(!(p->value()=="")){
                    config["gecko"][i] = p->value();
                }
                else{
                    config["gecko"][i] = "EMPTY";
                }
            }
            
            Serial.println("------");
        }
        request->send(SPIFFS, "/saved.html", "text/html;charset=UTF-8");
        serializeJson(config, Serial);
        save();

  });
    server.on("/binance", HTTP_GET, [](AsyncWebServerRequest *request){
        int paramsNr = request->params();
        JsonArray iter = config["binance"];
        for (JsonArray::iterator it=iter.begin(); it!=iter.end(); ++it) { //wyczyszczenie tablicy z parami przed nadpisaniem z requesta
            iter.remove(it);
        }
        for(int i=0;i<paramsNr;i++){
            AsyncWebParameter* p = request->getParam(i);
            Serial.print("Param name: ");
            Serial.println(p->name());
            Serial.print("Param value: ");
            Serial.println(p->value());
            if(!(p->value()=="")){
                config["binance"][i] = p->value();
            }
            else{
               config["binance"][i] = "EMPTY"; 
            }
            
            Serial.println("------");
        }
        request->send(SPIFFS, "/saved.html", "text/html;charset=UTF-8");
        serializeJson(config, Serial);
        save();

  });
  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request){
    int paramsNr = request->params();
    int b = config["brightness"];
    for(int i=0;i<paramsNr;i++){
            AsyncWebParameter* p = request->getParam(i);
            Serial.print("Param name: ");
            Serial.println(p->name());
            Serial.print("Param value: ");
            Serial.println(p->value());
            if(p->name()=="value"){
                b = (p->value()).toInt();
                if(!(b<15 || b>255)) config["brightness"] = b;
                
            }
    }
        request->send(SPIFFS, "/saved.html", "text/html;charset=UTF-8");
        serializeJson(config, Serial);
        save();
    
  });

}


void req_gecko(String coin, String vs, int row){
     HTTPClient http;
     http.begin("https://api.coingecko.com/api/v3/simple/price?ids=" + coin + "&vs_currencies=" + vs + "&include_24hr_change=true");
     int httpCode = http.GET();
     String payload = "";
     if (httpCode == 200) { //Check the returning code
        payload = http.getString();   //Get the request response payload
        Serial.println(payload);             //Print the response payload 
        http.end();   //Close connection
        StaticJsonDocument<768> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }
        JsonObject selectcoin = doc[coin];
        if(selectcoin==NULL) {
            Serial.println("Blad API");
            api_err_gecko[row]++;
        }
        else {
            api_err_gecko[row] = 0;
            double price = selectcoin[vs];
            double change = selectcoin[vs+"_24h_change"];
            writeLCD(coin,price,change,row);
            writeNoInternet(false);

        }

     }
     else if (httpCode == -1){
          writeNoInternet(true);
     }
     else {
          Serial.println("HTTP ERROR");
          api_err_gecko[row]++;
     }
      if(api_err_gecko[row]>4){
          writeLCD("ERROR",0,0,row);
        Serial.println("Wrote Error");

      }

}
void req_binance(String pair, int row){
     HTTPClient http;
     http.begin("https://api.binance.com/api/v3/ticker/24hr?symbol=" + pair);
     int httpCode = http.GET();
     String payload = "";
     if (httpCode == 200) { //Check the returning code
        payload = http.getString();   //Get the request response payload toFloat()
        //Serial.println(payload);             //Print the response payload 
        http.end();   //Close connection
        StaticJsonDocument<768> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            
        }
        const char* apierror = doc["msg"];
        if(apierror){
            Serial.println("Blad API");
            api_err_binance[row]++;
        }
        else {
            api_err_binance[row]=0;
            float price = float(doc["lastPrice"]);
            float change = doc["priceChangePercent"];
            writeLCDBinance(pair, price, change, row);
            writeNoInternet(false);
            
        }

      }
      else if (httpCode == 400){
          Serial.println("Blad API");
          api_err_binance[row]++;
      }
      else if (httpCode == -1){
          writeNoInternet(true);
      }
      else {
          Serial.println("HTTP ERROR");
          Serial.println(httpCode);

      }
    if(api_err_binance[row]>4){
        writeLCDBinance("ERROR",0,0,row);
        Serial.println("Wrote Error");
    }
}
void req_dex(String contract, int row){
     HTTPClient http;
     http.begin("https://api.dex.guru/v1/tokens/" + contract);
     int httpCode = http.GET();
     String payload = "";
     if (httpCode == 200) { //Check the returning code
        payload = http.getString();   //Get the request response payload toFloat()
        //Serial.println(payload);             //Print the response payload 
        http.end();   //Close connection
        StaticJsonDocument<768> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            
        }
        const char* apierror = doc["msg"];
        if(apierror){
            Serial.println("Blad API");
            api_err_dex[row]++;
        }
        else {
            api_err_dex[row]=0;
            String name = doc["name"];
            float price = float(doc["priceUSD"]);
            float change = doc["priceChange24h"];
            Serial.println(name);
            Serial.println(price);
            writeLCDDEX(name, price, change*100, row);
            writeNoInternet(false);
            
        }

      }
      else if (httpCode == 400){
          Serial.println("Blad API");
          api_err_dex[row]++;
      }
      else if (httpCode == -1){
          writeNoInternet(true);
      }
      else {
          Serial.println("HTTP ERROR");

      }
    if(api_err_dex[row]>4){
        writeLCDDEX("ERROR",0,0,row);
        Serial.println("Wrote Error");
    }
}
void loop() {
    delay(2500);
    if(conn){
    String ip = WiFi.localIP().toString();
    writeMeta(ip, config["vs"]);
    JsonArray gecko = config["gecko"];
    String gecko_1 = gecko[0];
    String gecko_2 = gecko[1];
    String gecko_3 = gecko[2];
    Serial.println(gecko_1);
     Serial.println(gecko_2);
     Serial.println(gecko_3);
    JsonArray binance = config["binance"];
    String binance_1 = binance[0];
    String binance_2 = binance[1];
    String binance_3 = binance[2];
    Serial.println(binance_1);
    Serial.println(binance_2);
    Serial.println(binance_3);
    writeBrightness(config["brightness"]);
    delay(DATA_STOP_TIME);
    if(gecko_1!=NULL && gecko_1 != "EMPTY") req_gecko(config["gecko"][0], config["vs"],1); else if(gecko_1 == "EMPTY") writeLCD("EMPTY", 0, 0, 1);
    delay(DATA_STOP_TIME);
    if(gecko_2!=NULL && gecko_2 != "EMPTY" ) req_gecko(config["gecko"][1], config["vs"],2); else if(gecko_2 == "EMPTY") writeLCD("EMPTY", 0, 0, 2);
    delay(DATA_STOP_TIME);
    if(gecko_3!=NULL && gecko_3 != "EMPTY") req_gecko(config["gecko"][2], config["vs"],3); else if(gecko_3 == "EMPTY") writeLCD("EMPTY", 0, 0, 3);
    delay(DATA_STOP_TIME);
    //if(dex_1!=NULL && dex_1 != "EMPTY") req_dex(config["dex"][0],1); else if(dex_1 == "EMPTY") writeLCDDEX("EMPTY", 0, 0, 1);
    //if(dex_2!=NULL && dex_2 != "EMPTY") req_dex(config["dex"][1],2); else if(dex_2 == "EMPTY") writeLCDDEX("EMPTY", 0, 0, 2);
    //if(dex_3!=NULL && dex_3 != "EMPTY") req_dex(config["dex"][2],3); else if(dex_3 == "EMPTY") writeLCDDEX("EMPTY", 0, 0, 3);
    if(binance_1!=NULL && binance_1 != "EMPTY") req_binance(config["binance"][0],1); else if(binance_1 == "EMPTY") writeLCDBinance("EMPTY", 0, 0, 1);
    delay(DATA_STOP_TIME);
    if(binance_2!=NULL && binance_2 != "EMPTY") req_binance(config["binance"][1],2); else if(binance_2 == "EMPTY") writeLCDBinance("EMPTY", 0, 0, 2);
    delay(DATA_STOP_TIME);
    if(binance_3!=NULL && binance_3 != "EMPTY") req_binance(config["binance"][2],3); else if(binance_3 == "EMPTY") writeLCDBinance("EMPTY", 0, 0, 3);
    delay(DATA_STOP_TIME);
    writeMeta(ip, config["vs"]);
    }

}

