#include <SPI.h>
#include <string.h>
#include "5x5_font.h"


// Pin outs
#define TFT_DC 8
#define TFT_CS 10
#define TFT_MOSI 16
#define TFT_CLK 15
#define TFT_RST 7
#define TFT_MISO 14
#define TFT_LED 18

#define ILI9341_TFTWIDTH 320  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 240 ///< ILI9341 max TFT height

#define BURST_MAX_SIZE 	500

#define BLACK       0x0000      
#define NAVY        0x000F      
#define DARKGREEN   0x03E0      
#define DARKCYAN    0x03EF      
#define MAROON      0x7800      
#define PURPLE      0x780F      
#define OLIVE       0x7BE0      
#define LIGHTGREY   0xC618      
#define DARKGREY    0x7BEF      
#define BLUE        0x001F      
#define GREEN       0x07E0      
#define CYAN        0x07FF      
#define RED         0xF800     
#define MAGENTA     0xF81F      
#define YELLOW      0xFFE0      
#define WHITE       0xFFFF      
#define ORANGE      0xFD20      
#define GREENYELLOW 0xAFE5     
#define PINK        0xF81F

// 4 different rotation options
#define SCREEN_VERTICAL_1			0
#define SCREEN_HORIZONTAL_1		1
#define SCREEN_VERTICAL_2			2
#define SCREEN_HORIZONTAL_2		3


//These need to be 32 bit for when we multiply them together
volatile uint16_t LCD_HEIGHT = ILI9341_TFTHEIGHT;
volatile uint16_t LCD_WIDTH	 = ILI9341_TFTWIDTH;
const uint32_t TOTAL_PIXELS = ((uint32_t)LCD_HEIGHT)*((uint32_t)LCD_WIDTH);


void ILI9341_send_data(uint8_t data) {
  // MSB is sent first
  // Arduino automatically uses best speed less than or equal to number in SPISettings

  digitalWrite(TFT_CS,LOW); //Low level enabled
  digitalWrite(TFT_DC,HIGH);
  SPI.transfer(data);
  // transmission_counter++;
  // if(transmission_counter%10 == 0){
  //   delay(100);
  // }
  digitalWrite(TFT_CS,HIGH);
  // SPI.endTransaction();

}

void ILI9341_send_command(uint8_t command) {
  // SPI.beginTransaction(SPISettings(8000000,MSBFIRST,SPI_MODE0)); 
  digitalWrite(TFT_CS,LOW); 
  digitalWrite(TFT_DC,LOW);
  SPI.transfer(command);
  // transmission_counter++;
  // if(transmission_counter%10 == 0){
  //   delay(100);
  // }
  digitalWrite(TFT_CS,HIGH);
  // SPI.endTransaction();
}

void ILI9341_set_address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2) {
  //X1 is start column, X2 is end column
  //Y1 is start page,   Y2 is end page
  //set column address
  ILI9341_send_command(0x2A);
  ILI9341_send_data(X1>>8); //first 8 MSB of X1
  ILI9341_send_data(X1);
  ILI9341_send_data(X2>>8);
  ILI9341_send_data(X2);

  //set page address
  ILI9341_send_command(0x2B);
  ILI9341_send_data(Y1>>8);
  ILI9341_send_data(Y1);
  ILI9341_send_data(Y2>>8);
  ILI9341_send_data(Y2);

  //memory write command; say we are going to transmit data
  //N bytes should follow?
  ILI9341_send_command(0x2C);
}

void ILI9341_hardware_reset(){
  digitalWrite(TFT_RST,LOW);
  delay(200);
  digitalWrite(TFT_CS,LOW);
  delay(200);
  digitalWrite(TFT_RST,HIGH);
}

void ILI9341_init(){


  // Initialise pin modes
  pinMode(TFT_DC,OUTPUT); //Low is command, High is data
  pinMode(TFT_CS,OUTPUT);
  pinMode(TFT_MOSI,OUTPUT);
  pinMode(TFT_CLK,OUTPUT);
  pinMode(TFT_RST,OUTPUT); 
  pinMode(TFT_MISO,INPUT); //not used by me
  pinMode(TFT_LED,OUTPUT);

  //Intialise SPI
  SPI.begin(); //Are there settings here
  SPI.beginTransaction(SPISettings(8000000,MSBFIRST,SPI_MODE0)); //What settings to use


  //Hardware reset
  ILI9341_hardware_reset(); //check timings


  //SOFTWARE RESET
  ILI9341_send_command(0x01);
  delay(1000); //Check timings

  //POWER CONTROL A
  ILI9341_send_command(0xCB);
  ILI9341_send_data(0x39);
  ILI9341_send_data(0x2C);
  ILI9341_send_data(0x00);
  ILI9341_send_data(0x34);
  ILI9341_send_data(0x02);

  //POWER CONTROL B
  ILI9341_send_command(0xCF);
  ILI9341_send_data(0x00);
  ILI9341_send_data(0xC1);
  ILI9341_send_data(0x30);

  //DRIVER TIMING CONTROL A
  ILI9341_send_command(0xE8);
  ILI9341_send_data(0x85);
  ILI9341_send_data(0x00);
  ILI9341_send_data(0x78);

  //DRIVER TIMING CONTROL B
  ILI9341_send_command(0xEA);
  ILI9341_send_data(0x00);
  ILI9341_send_data(0x00);

  //POWER ON SEQUENCE CONTROL
  ILI9341_send_command(0xED);
  ILI9341_send_data(0x64);
  ILI9341_send_data(0x03);
  ILI9341_send_data(0x12);
  ILI9341_send_data(0x81);

  //PUMP RATIO CONTROL
  ILI9341_send_command(0xF7);
  ILI9341_send_data(0x20);

  //POWER CONTROL,VRH[5:0]
  ILI9341_send_command(0xC0);
  ILI9341_send_data(0x23);

  //POWER CONTROL,SAP[2:0];BT[3:0]
  ILI9341_send_command(0xC1);
  ILI9341_send_data(0x10);

  //VCM CONTROL
  ILI9341_send_command(0xC5);
  ILI9341_send_data(0x3E);
  ILI9341_send_data(0x28);

  //VCM CONTROL 2
  ILI9341_send_command(0xC7);
  ILI9341_send_data(0x86);

  //MEMORY ACCESS CONTROL
  ILI9341_send_command(0x36);
  ILI9341_send_data(0x48);

  //PIXEL FORMAT
  ILI9341_send_command(0x3A);
  ILI9341_send_data(0x55);

  //FRAME RATIO CONTROL, STANDARD RGB COLOR
  ILI9341_send_command(0xB1);
  ILI9341_send_data(0x00);
  ILI9341_send_data(0x18);

  //DISPLAY FUNCTION CONTROL
  ILI9341_send_command(0xB6);
  ILI9341_send_data(0x08);
  ILI9341_send_data(0x82);
  ILI9341_send_data(0x27);

  // //GAMMA FUNCTION DISABLE
  // ILI9341_send_command(0xF2);
  // ILI9341_send_data(0x00);

  // //GAMMA CURVE SELECTED
  // ILI9341_send_command(0x26);
  // ILI9341_send_data(0x01);

  // //POSITIVE GAMMA CORRECTION
  // ILI9341_send_command(0xE0);
  // ILI9341_send_data(0x0F);
  // ILI9341_send_data(0x31);
  // ILI9341_send_data(0x2B);
  // ILI9341_send_data(0x0C);
  // ILI9341_send_data(0x0E);
  // ILI9341_send_data(0x08);
  // ILI9341_send_data(0x4E);
  // ILI9341_send_data(0xF1);
  // ILI9341_send_data(0x37);
  // ILI9341_send_data(0x07);
  // ILI9341_send_data(0x10);
  // ILI9341_send_data(0x03);
  // ILI9341_send_data(0x0E);
  // ILI9341_send_data(0x09);
  // ILI9341_send_data(0x00);

  // //NEGATIVE GAMMA CORRECTION
  // ILI9341_send_command(0xE1);
  // ILI9341_send_data(0x00);
  // ILI9341_send_data(0x0E);
  // ILI9341_send_data(0x14);
  // ILI9341_send_data(0x03);
  // ILI9341_send_data(0x11);
  // ILI9341_send_data(0x07);
  // ILI9341_send_data(0x31);
  // ILI9341_send_data(0xC1);
  // ILI9341_send_data(0x48);
  // ILI9341_send_data(0x08);
  // ILI9341_send_data(0x0F);
  // ILI9341_send_data(0x0C);
  // ILI9341_send_data(0x31);
  // ILI9341_send_data(0x36);
  // ILI9341_send_data(0x0F);

  // set brightness
  // ILI9341_send_command(0x51);
  // ILI9341_send_data(0x00);

  //exit sleep
  ILI9341_send_command(0x11);
  delay(120);

  //TURN ON DISPLAY
  ILI9341_send_command(0x29);

  //STARTING ROTATION
  ILI9341_set_rotation(SCREEN_HORIZONTAL_1);  
  
}

void ILI9341_set_rotation(uint8_t Rotation) 
{
  uint8_t screen_rotation = Rotation;

  ILI9341_send_command(0x36);
  delay(1);
    
  switch(screen_rotation) 
    {
      case SCREEN_VERTICAL_1:
        ILI9341_send_data(0x40|0x08);
        LCD_WIDTH = 240;
        LCD_HEIGHT = 320;
        break;
      case SCREEN_HORIZONTAL_1:
        ILI9341_send_data(0x20|0x08);
        LCD_WIDTH  = 320;
        LCD_HEIGHT = 240;
        break;
      case SCREEN_VERTICAL_2:
        ILI9341_send_data(0x80|0x08);
        LCD_WIDTH  = 240;
        LCD_HEIGHT = 320;
        break;
      case SCREEN_HORIZONTAL_2:
        ILI9341_send_data(0x40|0x80|0x20|0x08);
        LCD_WIDTH  = 320;
        LCD_HEIGHT = 240;
        break;
      default:
        //EXIT IF SCREEN ROTATION NOT VALID!
        break;
    }
}

void ILI9341_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Colour){
  //Starting from X,Y
  if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;

  if((X+Width-1)>=LCD_WIDTH){
      Width=LCD_WIDTH-X;
    }
  ILI9341_set_address(X, Y, X+Width-1, Y); //This command expects data to follow
  ILI9341_draw_colour_burst(Colour, Width);
}

void ILI9341_draw_rectangle(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Colour){
  if(X + Width  > LCD_WIDTH  ) {Width = LCD_HEIGHT - X;}
  if(Y + Height > LCD_HEIGHT ) {Height = LCD_WIDTH - Y;}
  ILI9341_set_address(X,Y,X+Width-1,Y+Height-1);
  ILI9341_draw_colour_burst(Colour, ((uint32_t)Width)*((uint32_t)Height));
}

void ILI9341_draw_colour_burst(uint16_t Colour, uint32_t Size) {
  // Outputs Size Colours values. 
  // Note that pixels each require two bytes
  // The address should be set before using draw colour burst with ILI9341_set_address

  digitalWrite(TFT_DC, HIGH);//High for data
  digitalWrite(TFT_CS, LOW);
  // uint8_t buffer[Size*2];
  for (uint32_t i = 0; i < Size*2; i+= 2)
  {
    // buffer[i] = Colour >> 8;
    // buffer[i+1] = Colour;
    SPI.transfer16(Colour);
  }
  

  // SPI.transfer(buffer, Size*2);
  digitalWrite(TFT_CS,HIGH);

}

void ILI9341_fill_screen(uint16_t Colour){
  //Start in the top corner and draw a block the size of the screen
  ILI9341_set_address(0,0,LCD_WIDTH,LCD_HEIGHT);
  ILI9341_draw_colour_burst(Colour,TOTAL_PIXELS);
}

void ILI9341_draw_char(char myChar, uint16_t font_colour, uint16_t background_colour){
  
  digitalWrite(TFT_DC, HIGH);//High for data
  digitalWrite(TFT_CS, LOW);


  for(int j=0; j < 8; j++){
    for(int i = 0; i < 6; i++){
      if( (font[myChar-32][i] >> j ) &  1){
        SPI.transfer16(font_colour);
      }else{
        SPI.transfer16(background_colour);
      }
    }
  }

  digitalWrite(TFT_CS,HIGH);
}



void ILI9341_write_word(char word[], uint16_t x, uint16_t y, uint16_t font_colour, uint16_t background_colour){
  // writes the word starting at pos x, y
  // wraps lines
  uint16_t initial_x = x;
  int len = strlen(word);

  for(int i =0; i < len; i++){
    ILI9341_set_address(x,y,x+6-1,y+8-1);
    ILI9341_draw_char(word[i], font_colour,background_colour);
    x+= 6;
    if(x+6>=LCD_WIDTH){ //wrap line
      x = initial_x;
      y += 8;
    }
  }

}

void setup() {

  ILI9341_init();
  digitalWrite(TFT_LED,HIGH);

 
  ILI9341_fill_screen(RED);
  ILI9341_draw_rectangle(100,100,20,20, BLUE);

  char str[] = "have you heard the tragedy of darth plagues the old the wise the long setnence";
  ILI9341_write_word(str, 10, 10, BLACK, GREENYELLOW);

  
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
