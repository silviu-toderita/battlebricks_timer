#include "Arduino.h"
#include "FS.h" //SPI Flash File System (SPIFFS) Library
#include "ArduinoJson.h" //Arduino JavaScript Object Notation Library

class Persistent_Storage{
    
    public:

        Persistent_Storage(String name);
    
        bool
            set(String key, String value),
            remove(String key);

        String 
            get(String key);

    private:

        String path; //The path for this object

};