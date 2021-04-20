/**
 * This example turns the ESP32 into a Bluetooth LE keyboard that writes the words, presses Enter, presses a media key and then Ctrl+Alt+Delete
 */
#include <BleKeyboard.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "WiFipass.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
unsigned char bluetooth_icon16x16[] =
{
  0b00000000, 0b00000000, //                 
  0b00000001, 0b10000000, //        ##       
  0b00000001, 0b11000000, //        ###      
  0b00000001, 0b01100000, //        # ##     
  0b00001001, 0b00110000, //     #  #  ##    
  0b00001101, 0b00110000, //     ## #  ##    
  0b00000111, 0b01100000, //      ### ##     
  0b00000011, 0b11000000, //       ####      
  0b00000001, 0b10000000, //        ##       
  0b00000011, 0b11000000, //       ####      
  0b00000111, 0b01100000, //      ### ##     
  0b00001101, 0b00110000, //     ## #  ##    
  0b00001001, 0b00110000, //     #  #  ##    
  0b00000001, 0b01100000, //        # ##     
  0b00000001, 0b11000000, //        ###      
  0b00000001, 0b10000000, //        ##       
};

 unsigned char cancel_icon16x16[] =
{
  0b00000000, 0b00000000, //                 
  0b00000000, 0b00000000, //                 
  0b00111000, 0b00001110, //   ###       ### 
  0b00111100, 0b00011110, //   ####     #### 
  0b00111110, 0b00111110, //   #####   ##### 
  0b00011111, 0b01111100, //    ##### #####  
  0b00001111, 0b11111000, //     #########   
  0b00000111, 0b11110000, //      #######    
  0b00000011, 0b11100000, //       #####     
  0b00000111, 0b11110000, //      #######    
  0b00001111, 0b11111000, //     #########   
  0b00011111, 0b01111100, //    ##### #####  
  0b00111110, 0b00111110, //   #####   ##### 
  0b00111100, 0b00011110, //   ####     #### 
  0b00111000, 0b00001110, //   ###       ### 
  0b00000000, 0b00000000, //                 
};

 unsigned char home_icon16x16[] = 
{
  0b00000111, 0b11100000, //      ######      
  0b00001111, 0b11110000, //     ########     
  0b00011111, 0b11111000, //    ##########   
  0b00111111, 0b11111100, //   ############  
  0b01111111, 0b11111110, //  ############## 
  0b11111111, 0b11111111, // ################
  0b11000000, 0b00000011, // ##            ##
  0b11000000, 0b00000011, // ##            ##
  0b11000000, 0b00000011, // ##            ##
  0b11001111, 0b11110011, // ##  ########  ##
  0b11001111, 0b11110011, // ##  ########  ##
  0b11001100, 0b00110011, // ##  ##    ##  ##
  0b11001100, 0b00110011, // ##  ##    ##  ##
  0b11001100, 0b00110011, // ##  ##    ##  ##
  0b11111100, 0b00111111, // ######    ######
  0b11111100, 0b00111111, // ######    ######
};

 unsigned char speak_icon16x16[] =
{
  0b00111111, 0b11111100, //   ############  
  0b01111111, 0b11111110, //  ############## 
  0b11111111, 0b11111111, // ################
  0b11110000, 0b00001111, // ####        ####
  0b11100000, 0b00000111, // ###          ###
  0b11100000, 0b00000111, // ###          ###
  0b11100000, 0b00000111, // ###          ###
  0b11100000, 0b00000111, // ###          ###
  0b11100000, 0b00000111, // ###          ###
  0b11100000, 0b00000111, // ###          ###
  0b11110000, 0b00001111, // ####        ####
  0b11110001, 0b11111110, // ####   ######## 
  0b01111011, 0b11111100, //  #### ########  
  0b00111111, 0b11111000, //   ###########   
  0b00011110, 0b00000000, //    ####         
  0b00001100, 0b00000000, //     ##          
};

BleKeyboard bleKeyboard("vldfr's CODEK", "vldfr Tech", 100);

#define NUM_BTN 10

const int button_pins[10] = {
  27,26,25,33,32,
  16,17, 5,18,19
};

int button_states[10];
int last_button_states[10]={LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
unsigned long lastDebounceTime[10] = {0};
unsigned long debounceDelay = 40;

unsigned long maxEl(unsigned long arr[], int nr){
  unsigned long m = 0;
  for(int i=0;i<nr;i++)
    if(arr[i]>m)
      m = arr[i];
  return m;
}

const uint8_t *media[][10] = {{
  KEY_MEDIA_PLAY_PAUSE,
  KEY_MEDIA_MUTE,
  KEY_MEDIA_VOLUME_DOWN,
  0,
  KEY_MEDIA_VOLUME_UP,
  0,
  KEY_MEDIA_NEXT_TRACK,
  0,
  0,
  0
}};

uint8_t normal[][10] = {{
  0,
  0,
  0,
  KEY_UP_ARROW,
  0,
  'f',
  0,
  KEY_LEFT_ARROW,
  KEY_DOWN_ARROW,
  KEY_RIGHT_ARROW
}};

bool is_mediaKey[][10] = {{
  1,1,1,0,1,0,1,0,0,0
}};

unsigned char *modeIcon[] = {
  home_icon16x16
//  ,
//  speak_icon16x16
};

String modeName[] = {
  "Home"
};

int current_mode=0;

bool first_display = false;

bool displaysLogs = false;


void setup() {
//  Serial.begin(115200);
//  Serial.println("Starting BLE work!");
  
  pinMode(2, OUTPUT);
  for(int i = 0;i<NUM_BTN;i++){
    pinMode(button_pins[i], INPUT_PULLUP);  
  }
  

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
//    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.println("Starting BLE..");
  display.display();
  
  bleKeyboard.begin();
  display.println("Started BLE..");
  display.display();
  
}

void loop() {
  displayInterface();
  
  
  for(int i = 0;i<NUM_BTN;i++){
      int reading = !digitalRead(button_pins[i]);
      if (reading != last_button_states[i]) {
        lastDebounceTime[i] = millis();
      }

      if ((millis() - lastDebounceTime[i]) > debounceDelay) {
          if (reading != button_states[i]) {
          button_states[i] = reading;
    
          if (button_states[i] == HIGH) {
            
            if(bleKeyboard.isConnected()){
              if(is_mediaKey[current_mode][i])
                bleKeyboard.write(media[current_mode][i]);
              else
                bleKeyboard.write(normal[current_mode][i]);
            }
          }
        }
      }
      
    last_button_states[i] = reading;
  }
  
  delay(2);
}

void displayConnectionState(){
  if(bleKeyboard.isConnected()){
    display.drawBitmap(112, 0, bluetooth_icon16x16, 16,16, 1);
    digitalWrite(2, HIGH);
  }
  else{
    display.drawBitmap(112, 0, cancel_icon16x16, 16,16,1);
    digitalWrite(2, LOW);
  }
//  if(WiFi.status() == WL_CONNECTED)
//    display.drawBitmap(94, 0, wifi1_icon16x16, 16,16, 1);
//  else{
//    display.drawBitmap(94, 0, wifi1_icon16x16, 16,16, 1);
//    display.drawBitmap(94, 0, cancel_icon16x16, 16,16,1);
//  }
}

void displayMode(){
  display.drawBitmap(0, 0, modeIcon[current_mode], 16,16, 1);
  display.setTextSize(1);
  display.setCursor(24, 4);
  display.setTextColor(WHITE, BLACK);
  display.print(modeName[current_mode]);
}

void displayKbd(){
  display.setTextSize(1);
  for(int i=0;i<10;i++){
    display.setCursor((128/5 + 1)*(i%5), 32+18*(i/5));
    if(!button_states[i])
      display.setTextColor(BLACK, WHITE);
    else
      display.setTextColor(WHITE, BLACK);
    display.print(" "+(String)i+" ");
  }
  
}

void displayInterface(){
  if(!displaysLogs){
    display.clearDisplay();
    
    displayMode();
    displayConnectionState();
    displayKbd();
    
    display.display();
  }
}
