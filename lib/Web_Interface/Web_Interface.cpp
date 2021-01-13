/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ESP8266 library for hosting a simple web interface. Features:
    -Web console for an ESP that can't be plugged in to use the serial monitor. 
    -Dynamic settings page

    To use, initialize a Web_Interface setting and call handle() every loop or as
    often as possible. Call console_print() to output a line to the console. Place
    files for server in /www/ folder in SPIFFS. 

    To use the settings function, place a settings.txt file in the root 
    of the SPIFFS. Settings must be in the following JSON format ("advanced" is
    an optional category, while "basic" and "wifi" are required and can have any
    number of settings):
    {"basic":[
        {"id":"",
        "type":"",
        "name":"",
        "desc":"",
        "req":true,
        "val":""}
        ],
    "advanced":[
        {"id":"",
        "type":"",
        "name":"",
        "desc":"",
        "req":false,
        "val":""}
        ],
    "wifi":[
        {"id":"",
        "type":"",
        "name":"",
        "desc":"",
        "req":true,
        "val":""}
        ]
    }
    

    begin() will return false if there are any required settings that are missing.
    load_setting() allows you to load a setting based on its id. 

    Created by Silviu Toderita in 2020.
    silviu.toderita@gmail.com
    silviutoderita.com
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Web_Interface.h"

ESP8266WebServer server(80); //Create a web server listening on port 80
WebSocketsServer websockets_server = WebSocketsServer(81); //Create a websockets server listening on port 81

static void_function_pointer _offline; //Callback function when connected

int8_t websockets_client = -1; //Current websockets client number connected to (-1 is none)

const String settings_path = "/settings.txt"; //Path to settings file

File upload_file; //Holds file currently uploading

//settings
const bool settings_page = true;
const bool console_page = true;
const uint8_t number_custom_pages = 1;
const String custom_page_path[number_custom_pages] = "/contacts/";
const String custom_page_name[number_custom_pages] = "Contacts";

/*  (private) get_content_type: Returns the HTTP content type based on the extension
        filename: 
    RETURNS HTTP content type as a string
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String get_content_type(String filename){ 
    if(filename.endsWith(".htm")) return "text/html";
    else if(filename.endsWith(".html")) return "text/html";
    else if(filename.endsWith(".css")) return "text/css";
    else if(filename.endsWith(".js")) return "application/javascript";
    else if(filename.endsWith(".png")) return "image/png";
    else if(filename.endsWith(".gif")) return "image/gif";
    else if(filename.endsWith(".jpg")) return "image/jpeg";
    else if(filename.endsWith(".bmp")) return "image/bmp";
    else if(filename.endsWith(".ico")) return "image/x-icon";
    else if(filename.endsWith(".xml")) return "text/xml";
    else if(filename.endsWith(".pdf")) return "application/x-pdf"; //Compressed file
    else if(filename.endsWith(".zip")) return "application/x-zip"; //Compressed file
    else if(filename.endsWith(".gz")) return "application/x-gzip"; //Compressed file
    return "text/plain"; //If none of the above, assume file is plain text
}

/*  (private)handle_file_read: Read a file from SPIFFS and serve it when requested.
        path: The requested URI
    RETURNS true if the file exists, false if it does not exist
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
bool handle_file_read(String path){  
    //Append /www to the beginning of the path
    path = "/www" + path;

    //If only a folder is specified, attempt to return index.html
    if(path.endsWith("/")) path += "index.html"; 

    //Get the content type 
    String content_type = get_content_type(path); 

    bool cache = false;
    if(path.endsWith(".js") || path.endsWith(".css") || path.endsWith(".ico")){
        cache = true;
    } 

    //If the compressed file exists, stream it to the client
    if(SPIFFS.exists(path + ".gz")){
        File file = SPIFFS.open(path + ".gz", "r"); 
        if(cache) server.sendHeader("Cache-Control", "max-age=2592000");          
        server.streamFile(file, content_type);
        file.close();                                    
        return true;
    }

    //If the file exists, stream it to the client
    if(SPIFFS.exists(path)){
        File file = SPIFFS.open(path, "r");
        if(cache) server.sendHeader("Cache-Control", "max-age=2592000"); 
        server.streamFile(file, content_type);
        file.close();                                    
        return true;
    }

    //If the file exists in the root folder instead of the /www/ folder, stream it to the client (this is for debugging non-server files)
    if(SPIFFS.exists(path.substring(4))){
        File file = SPIFFS.open(path.substring(4), "r");                
        server.streamFile(file, content_type);
        file.close();                                    
        return true;
    }

    return false;                                         
}

/*  (private)websockets_event: Called when a new websockets event happens
        num: client number
        type: event type
        payload: message payload
        length: message length
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void websockets_event(uint8_t num, WStype_t type, uint8_t * payload, size_t length) { // When a WebSocket message is received

    //Take different action based on the type of event
    switch (type) {
        //When the websockets client disconnects...
        case WStype_DISCONNECTED:  
            //Erase the client number           
            websockets_client = -1;
            break;

        //When the websockets client connects...
        case WStype_CONNECTED:
            //Store the client number
            websockets_client = num;

            //If the console file exists...
            if(SPIFFS.exists("/www/console.txt")){
                //Open the console.txt file in read mode and send it to the client
                File console = SPIFFS.open("/www/console.txt", "r");
                String console_text = console.readString();
                websockets_server.sendTXT(num, console_text);
                //Close the file
                console.close();
            }

            break;

        //For all other cases, do nothing
        default:           
            break;
    }

}

/*  (private)text_input_HTML: Create the html for a text form input
        id: setting id
        val: current or default setting value
        type: Setting type, valid inputs are "num", "pass", or "text". Anything else defaults to "text"
    RETURNS complete HTML
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String text_input_HTML(String id, String val, String type, bool req){
    //Create the string that will hold the response
    String response = "<input type=\"";

    //Specify the type
    if(type == "num"){
        response += "number";
    }else if(type == "pass"){
        response += "password";
    }else{
        response += "text";
    }
    
    //Create the input field
    response += "\" class=\"form-control\" id=\"" + id + "\" name=\"" + id + "\" aria-describedby=\"" + id +"help\" value=\"" + val + "\"";
    //If this setting is required, make it a required field 
    if(req){
        response += "required";
    }

    response += ">";

    return response;
}

/*  (private)multi_input_HTML: Create the html for a multiple choice input
        id: setting id
        val: current or default setting value
        type: Setting type, valid inputs are "multi" or "bool"
        opt: a JsonArray of possible options, only required for "multi" type
    RETURNS complete HTML
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String multi_input_html(String id, String val, String type, JsonArray opt){

    //Create the string that will hold the response
    String response = "<select class=\"form-control\" id=\"" + id + "\" name=\"" + id + "\" aria-describedby=\"" + id +"help\">";

    //If this is a multiple-choice setting...
    if(type == "multi"){
        
        //For each option...
        for(int i = 0; i < opt.size(); i++){
            
            //This string holds the option name
            String this_option = opt[i];
            response += "<option value=\"" + this_option + "\"";
            //If the current option is the current value or default, pre-select it on the form 
            if(this_option == val) response += "selected";
            response +=">" + this_option + "</option>"; 
        }
    //Otherwise, this is a boolean setting...
    }else{
        //Create the On option
        response += "<option value=true";
        //If the current value is true, pre-select the Yes option
        if(val){
            response += "selected";
        }
        response +=">On</option>";

        //Create the Off option
        response += "<option value=false";
        //If the current value is false, pre-select the No option
        if(!val){
            response += "selected";
        }
        response +=">Off</option>";
    
    }

    response += "</select>";

    return response;
}

/*  (private)multi_input_HTML: Create the html for all inputs in a category
        settings: JsonArray of settings in this category
    RETURNS complete HTML
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String input_html(JsonArray settings){
    String response = "";
    //For each setting...
    for(int i = 0; i < settings.size(); i++){
        //Save the setting attributes
        String id = settings[i]["id"];
        String type = settings[i]["type"];
        String name = settings[i]["name"];
        String desc = "";
        if(settings[i].containsKey("desc")) desc = settings[i]["desc"].as<String>();
        String val = "";
        if(settings[i].containsKey("val")) val = settings[i]["val"].as<String>();
        bool req = settings[i]["req"];

        //Form the HTML response
        response += "<div class=\"form-group\">";
        response += "<label for=\"" + id + "\">" + name + "</label>";

        //Based on the type of setting, get the HTML
        if(type == "multi" || type == "bool"){
            response += multi_input_html(id, val, type, settings[i]["opt"]);
        }else{
            response += text_input_HTML(id, val, type, req);
        }
        
        //Add help text if the description is defined
        response +=     "<small id=\"" + id + "help\" class=\"form-text text-muted\">" + desc + "</small>";
        response += "</div>";

    }

    

    return response;
}

/*  (private)handle_settings_get: Send the settings to the browser as an HTML form
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void handle_settings_get(){
    //Open the settings file
    File file = SPIFFS.open(settings_path, "r");
    //Set aside enough memory for a JSON document
    DynamicJsonDocument doc(file.size() * 2);

    //Parse JSON from file
    DeserializationError error = deserializeJson(doc, file);

    //Close the file
    file.close();

    //If there is an error, send the response to the browser and exit the function
    if(error){
        server.send(200, "text/html", "<h3>Invalid settings file or no settings defined!</h3>");
        return;
    } 

    //Store whether the settings category exists or not
    bool advanced = doc.containsKey("advanced");

    //This will hold the HTML response to the browser
    String response;
    //Create the tabs
    response = "<ul class=\"nav nav-tabs\" id=\"settings_nav\" role=\"tablist\">";
    //Tab for general settings
    response +=     "<li class=\"nav-item\">";
    response +=         "<a class=\"nav-link active\" id=\"category-general-tab\" data-toggle=\"tab\" href=\"#category-general\" role\"tab\" aria-controls=\"category-general\" aria-selected=\"true;\">General</a>";
    response +=     "</li>";
    //Tab for advanced settings
    if(advanced){
        response += "<li class=\"nav-item\">";
        response +=     "<a class=\"nav-link\" id=\"category-advanced-tab\" data-toggle=\"tab\" href=\"#category-advanced\" role\"tab\" aria-controls=\"category-advanced\" aria-selected=\"false;\">Advanced</a>";
        response += "</li>";
    }
    //Tab for WiFi settings
    response +=     "<li class=\"nav-item\">";
    response +=         "<a class=\"nav-link\" id=\"category-wifi-tab\" data-toggle=\"tab\" href=\"#category-wifi\" role\"tab\" aria-controls=\"category-wifi\" aria-selected=\"false;\">Wi-Fi</a>";
    response +=     "</li>";
    //End the tabs
    response += "</ul>";
    response += "<br>";

    //Tab contents for general settings
    response += "<div class=\"tab-content\" id=\"settings_nav_content\">";
    response +=     "<div class=\"tab-pane fade show active\" id=\"category-general\" role=\"tabpanel\" aria-labelledby=\"category-general-tab\">";
    response +=         input_html(doc["general"]);
    response +=     "</div>";

    //Tab contents for advanced settings
    if(advanced){
        response += "<div class=\"tab-pane fade\" id=\"category-advanced\" role=\"tabpanel\" aria-labelledby=\"category-advanced-tab\">";
        response +=     input_html(doc["advanced"]);
        response += "</div>";
    }
    //Tab contents for wifi settings
    response +=     "<div class=\"tab-pane fade\" id=\"category-wifi\" role=\"tabpanel\" aria-labelledby=\"category-wifi-tab\">";
    response +=         input_html(doc["wifi"]);
    response +=     "</div>";

    //End the settings form
    response += "</div>";

    //Send the response to the browser
    server.send(200, "text/html", response);
}

/*  (private)handle_settings_post: Receive new settings from the browser
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void handle_settings_post(){
    //Confirm that the settings have been received
    server.send(200);

    //Open the file for reading
    File file = SPIFFS.open(settings_path, "r");
    //Set aside enough memory for a JSON document
    DynamicJsonDocument doc(file.size() * 2);

    //Parse JSON from file 
    deserializeJson(doc, file);
    //Close the file
    file.close();

    //Store whether the settings category exists or not
    bool advanced = doc.containsKey("advanced");

    //The current server argument being processed
    uint8_t current_server_arg = 0;

    //This array holds the settings for the current category
    JsonArray settings;

    //Cycle through each setting category
    for(int i = 0; i < doc.size(); i++){
        //Load the correct category depending on the current loop
        if(i == 0){
            settings = doc["general"];
        }else if(advanced && i == 1){
            settings = doc["advanced"];
        }else{
            settings = doc["wifi"];
        }

        //For each setting...
        for(int x = 0; x < settings.size(); x++){
            //Copy the server argument to the current value and increment the server argument
            settings[x]["val"] = server.arg(current_server_arg);
            current_server_arg++;
        }

    }


    //Open the file for writing
    file = SPIFFS.open(settings_path, "w");
    //Encode the JSON in the file
    serializeJson(doc, file);
    //Close the file
    file.close();

    //Take the tag machine offline and restart
    _offline();
    ESP.restart();
}

/*  (private)handle_nav: Send the navigation bar to the browser as a dynamically-generated navbar
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void handle_nav(){
    String response;
    response = "<nav class=\"navbar navbar-expand-md navbar-dark bg-dark mb-4\">";
    response +=     "<a class=\"navbar-brand\" href=\"/index.html\">TAG Machine</a>";
    response +=     "<button class=\"navbar-toggler\" type=\"button\" data-toggle=\"collapse\" data-target=\"#navbarCollapse\" aria-controls=\"navbarCollapse\" aria-expanded=\"false\" aria-label=\"Toggle navigation\">";
    response +=         "<span class=\"navbar-toggler-icon\"></span>";
    response +=     "</button>";
    response +=     "<div class=\"collapse navbar-collapse\" id=\"navbarCollapse\">";
    response +=         "<ul class=\"navbar-nav\">";

    for(int i = 0; i < number_custom_pages; i++){
        response +=         "<li class=\"nav-item\">";
        response +=             "<a class=\"nav-link\" href=\"" + custom_page_path[i] + "\">" + custom_page_name[i] + "</a>";
        response +=         "</li>";
    }

    if(settings_page){
        response +=         "<li class=\"nav-item\">";
        response +=             "<a class=\"nav-link\" href=\"/settings/\">Settings</a>";
        response +=         "</li>";
    }
    if(console_page){
        response +=         "<li class=\"nav-item\">";
        response +=             "<a class=\"nav-link\" href=\"/console/\">Console</a>";
        response +=         "</li>";
    }

    response +=         "</ul>";
    response +=     "</div>";
    response += "</nav>";

    server.sendHeader("Cache-Control", "max-age=2592000");    
    server.send(200, "text/html", response);
}

/*  Web_Interface Constructor (with defaults)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
Web_Interface::Web_Interface(){
    SPIFFS.begin();
    SPIFFS.gc();
}

/*  set_callback: Set the callback function to take tag machine offline safely before restarting
        offline: offline function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void Web_Interface::set_callback(void_function_pointer offline){
    _offline = offline;
}

/*  (private)check_settings_file: Check the settings file to ensure all the required parameters are present
    RETURNS true if there is no blank parameter that's required, false if there is a blank parameter that's required
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
bool check_settings_file(){

    // If the settings file does not exist, copy it from the default settings file
    if(!SPIFFS.exists(settings_path)){
        File settings_def = SPIFFS.open("/settings_def.txt", "r");
        File settings = SPIFFS.open(settings_path, "w");
        while(settings_def.available()){
            settings.write(settings_def.read());
        }
        settings_def.close();
        settings.close();
    }

    //Open the file
    File file = SPIFFS.open(settings_path, "r");
    //Set aside enough memory for a JSON document
    DynamicJsonDocument doc(file.size() * 2);

    //Parse JSON from file
    DeserializationError error = deserializeJson(doc, file);

    //Close the file
    file.close();

    //If there is an error, return false
    if(error){
        return false;
    } 
    
    //Store whether the settings category exists or not
    bool advanced = doc.containsKey("advanced");

    //This array holds the settings for the current category
    JsonArray settings;
    //Cycle through each setting category
    for(int i = 0; i < doc.size(); i++){
        //Load the correct category based on the current loop
        if(i == 0){
            settings = doc["general"];
        }else if(advanced && i == 1){
            settings = doc["advanced"];
        }else{
            settings = doc["wifi"];
        }

        //For each setting...
        for(int i = 0; i < settings.size(); i++){
            //If this setting is required...
            if(settings[i]["req"] == true){
                //If this setting has a value...
                if(settings[i].containsKey("val")){
                    //If this setting has a blank value, return false
                    if(settings[i]["val"] == "") return false;
                //If this setting doesn't have a value, return false
                }else{
                    return false;
                }
            }
        }
    }

    return true;
}

/*  (private)handle_file_upload: Processes file upload and saves it to SPIFFS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void handle_file_upload(){
    //Holds current upload
    HTTPUpload& upload = server.upload();
    //If the upload is starting...
    if(upload.status == UPLOAD_FILE_START){
        //Get the filename
        String filename = upload.filename;
        //Add a / prefix if it's not part of the filename already
        if(!filename.startsWith("/")) filename = "/" + filename;
        //Open the file for writing
        upload_file = SPIFFS.open(filename, "w");   
    //If the upload is in progress, write the buffer to the file        
    }else if(upload.status == UPLOAD_FILE_WRITE && upload_file){
        upload_file.write(upload.buf, upload.currentSize);
    
    //If the upload is over, send server status 201 and close the file
    }else if(upload.status == UPLOAD_FILE_END){
        server.send(201);
        if(upload_file) upload_file.close();
        //If the settings file was uploaded, restart the ESP
        if(upload.filename == "settings.txt"){
            _offline();
            ESP.restart();
        } 
    }

    String upload_status = String(upload.status);
    websockets_server.sendTXT(websockets_client, upload_status);
}

/*  begin: Start the web interface
    RETURNS True if the settings file is good, false if it's missing anything
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
bool Web_Interface::begin(){

    //When the settings file is requested or posted, call the corresponding function
    server.on("/settings_data", HTTP_POST, handle_settings_post);
    server.on("/settings_data", HTTP_GET, handle_settings_get);
    server.on("/nav", HTTP_GET, handle_nav);
    //When a POST is requested from /upload, send status 200 to initiate upload and call handle_file_upload function repeatedly
    server.on("/upload", HTTP_POST, [](){ server.send(200); }, handle_file_upload );


    //If any other file is requested, send it if it exists or send a generic 404 if it doesn't exist
    server.onNotFound([](){
        if(!handle_file_read(server.uri())){
            server.send(404, "text/plain", "404: Not Found");
        }
    });

    server.begin(); //Start the server

    if(console_page){
        //If a websockets message comes in, call this function
        websockets_server.onEvent(websockets_event);
        websockets_server.begin(); //Start the websockets server
        //If a console.txt file exists, delete it to start with a clean console upon init
        if(SPIFFS.exists("/www/console.txt")) SPIFFS.remove("/www/console.txt"); 
    } 

    //If the settings file is good, return true. Otherwise, return false. 
    if(check_settings_file() || !settings_page){
        return true;
    }
    return false;
}

/*  handle: Check for incoming requests to the server and to the websockets server
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void Web_Interface::handle(){
    server.handleClient();
    if(console_page) websockets_server.loop();
}

/*  console_print: Print to the web console
        output: Text to print
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void Web_Interface::console_print(String output){
    if(console_page){
        //If there is an active websockets connection, send the text to the client
        if(websockets_client != -1){
            websockets_server.sendTXT(websockets_client, output);
        }

        //Open/create the console.txt file in append mode
        File console = SPIFFS.open("/www/console.txt", "a");

        //If the console file is over 10kb, delete it and create a new one. 
        if(console.size() > 10000){
            console.close();
            SPIFFS.remove("/www/console.txt");
            console = SPIFFS.open("/www/console.txt", "w");
        }

        //output the current string to the end of the file
        console.print(output); 
        //Close the file
        console.close();
    }
}

/*  load_setting: 
    RETURNS the specified setting
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
String Web_Interface::load_setting(String setting){
    //Open the file for reading
    File file = SPIFFS.open(settings_path, "r");
    //Set aside enough memory for a JSON document
    DynamicJsonDocument doc(file.size() * 2);

    //Parse JSON from file
    DeserializationError error = deserializeJson(doc, file);
    //Close the file
    file.close();
    //If there is an error, return a blank response
    if(error){
        return "";
    } 

    //Store whether the settings category exists or not
    bool advanced = doc.containsKey("advanced");

    //This array holds the settings for the current category
    JsonArray settings;
    //Cycle through each setting category
    for(int i = 0; i < doc.size(); i++){
        //Load the correct category for the current loop
        if(i == 0){
            settings = doc["general"];
        }else if(advanced && i == 1){
            settings = doc["advanced"];
        }else{
            settings = doc["wifi"];
        }
        //For each setting...
        for(int i = 0; i < settings.size(); i++){
            //If the current setting is the one specified, return its value
            if(settings[i]["id"] == setting){
                return settings[i]["val"];
            }
        }
    }

    //If nothin has been returned so far, return an empty string
    return "";     

}