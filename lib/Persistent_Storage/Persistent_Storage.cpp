/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    A persistent storage library for ESP8266. Allows storage of key:value pairs of
    Strings. 

    To use, initialize an object with the path you'd like to use. Use set() to
    store or change a key:value pair, and get() to retrieve a value based on the
    key. Use remove() to delete a key:value pair. 

    Created by Silviu Toderita in 2020.
    silviu.toderita@gmail.com
    silviutoderita.com
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Persistent_Storage.h"

/*  Persistent_Storage Constructor
        name: The name of this storage object
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
Persistent_Storage::Persistent_Storage(String name){
    path = "/" + name + ".txt";
    SPIFFS.begin();
}

/*  put: Add a new key:value pair to storage, or modify the value of an existing key
        key:
        value:
    RETURNS True if successful, false if not
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
bool Persistent_Storage::set(String key, String value){ 
    //If the key is blank, it can't be stored
    if(key == "") return false;

    //Open the file for reading
    File file = SPIFFS.open(path, "r");
    //Set aside enough memory for a JSON document
    DynamicJsonDocument doc(file.size() * 2 + 256);

    //Parse JSON from file
    deserializeJson(doc, file);
    
    //Close the file for reading
    file.close();
    
    //Add/change the value in the file
    doc[key] = value;

    //Open the file for writing
    file = SPIFFS.open(path, "w");

    bool status = false;
    //Export the JSON to the file
    if(serializeJson(doc, file)){
        status = true;
    } 

    file.close();
    return status;
}

/*  get_value: Get the value of a specific key 
        key:
    RETURNS Value of the key. If the key is not found, returns "".
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String Persistent_Storage::get(String key){
    //Open the file for reading
    File file = SPIFFS.open(path, "r");
    //Set aside enough memory for a JSON document
    DynamicJsonDocument doc(file.size() * 2);

    //Parse JSON from file
    DeserializationError error = deserializeJson(doc, file);

    //Close the file
    file.close();

    //If there are any issues, return a blank string
    if(error){
        return "";
    } 

    //Return the value corresponding to the key. If the key doesn't exist, return a blank string
    return doc[key] | "";
}

/*  remove: Delete a key:value pair
        key: Key to delete
    RETURNS True if successful, false if not
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
bool Persistent_Storage::remove(String key){ 
    //If the key is blank, there is nothing to remove
    if(key == "") return false;

    //Open the file for reading
    File file = SPIFFS.open(path, "r");
    //Set aside enough memory for a JSON document
    DynamicJsonDocument doc(file.size() * 2);

    //Parse JSON from file
    deserializeJson(doc, file);
    
    //Close the file for reading
    file.close();
    
    //Add/change the value in the file
    doc.remove(key);

    //Open the file for writing
    file = SPIFFS.open(path, "w");

    bool status = false;
    //Export the JSON to the file
    if(serializeJson(doc, file)){
        status = true;
    } 

    file.close();
    return status;
}