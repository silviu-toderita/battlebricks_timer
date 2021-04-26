#include "Arduino.h"

#include "Soft_ISR.h"

// Graphics Libraries 
#include "Adafruit_GFX.h"
#include "Adafruit_NeoPixel.h"
#include "Adafruit_NeoMatrix.h"
#include "Picopixel.h"
#include "bitmaps.h"

// Pin Definitions
#define PIN_DISPLAY1    2
#define PIN_DISPLAY2    10
#define PIN_LED_RED     13
#define PIN_LED_BLUE    12

// Color Definitions
#define BLACK       0x0000
#define BLUE        0x001F
#define BLUE_DIM    0x0011
#define RED         0xF800
#define RED_DIM     0x8000
#define GREEN       0x07E0
#define GREEN_DIM   0x0300
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0 
#define WHITE       0xFFFF

typedef void (*void_function_pointer)();

class Graphics{
    public:
        void begin();
        void handle();

        void text_static(String,String);
        void text_dynamic(String,String);
        void text_dynamic(String,String,void_function_pointer);

        void set_brightness(String);
        uint8_t change_brightness();
        void show_wifi();

        void set_red_ready(bool);
        void set_blue_ready(bool);
        void set_green_ready(bool);
        void set_show_player_bar();

        void set_three_players(bool);
        void set_show_aux_lights(bool);
        void set_show_dim_lights(bool);
    
    private:
        // Matrix Displays
        Adafruit_NeoMatrix display_1 = Adafruit_NeoMatrix(16, 16, 2, 1, PIN_DISPLAY1, 
            NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + NEO_GRB + NEO_KHZ800);
        Adafruit_NeoMatrix display_2 = Adafruit_NeoMatrix(8, 8, 2, 1, PIN_DISPLAY2, 
            NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG + NEO_TILE_RIGHT + NEO_GRB + NEO_KHZ800);

        uint8_t brightness;

        // Current text on screen
        int16_t text_xpos = 0;
        bool text_scroll = false;
        uint16_t text_color = 0;
        String text_string = "";

        // Current graphics on screen
        bool show_player_bar;
        bool show_aux_lights;
        bool show_dim_lights;
        bool three_players;
        bool red_ready;
        bool blue_ready;
        bool green_ready;

        void draw_players_ready();
        void draw_two_players_ready();
        void draw_three_players_ready();
        void draw_text();
        void draw_brightness();

        void update_brightness();
        uint16_t parse_color(String);
};






