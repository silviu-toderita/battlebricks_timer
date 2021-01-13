#include "Arduino.h"
#include "ESP8266WebServer.h" //Web Server Library
#include "WebSocketsServer.h" //WebSockets Server Library
#include "fs.h" //SPIFFS Library
#include "ArduinoJson.h" //Arduino JavaScript Object Notation Library

//Callback function type (no args)
typedef void (*void_function_pointer)();

class Web_Interface{
    public:
    
        Web_Interface();
    
        void 
            set_callback(void_function_pointer offline),
            handle(),
            console_print(String output);

        bool
            begin();

        String
            load_setting(String setting);
        
};