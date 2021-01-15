#include "Arduino.h"
#include "ESP8266WebServer.h" //Web Server Library
#include "fs.h" //SPIFFS Library
#include "ArduinoJson.h" //Arduino JavaScript Object Notation Library

class Web_Interface{
    public:
    
        Web_Interface();
    
        void 
            handle(),
            begin();
            
        String
            load_setting(String setting);
        
};