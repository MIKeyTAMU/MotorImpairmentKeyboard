#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>

// Define some device constants
#define LCD_CHR  1 // Mode - Sending data
#define LCD_CMD  0 // Mode - Sending command

#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line

#define LCD_BACKLIGHT   0x08  // On
// LCD_BACKLIGHT = 0x00  # Off

#define ENABLE  0b00000100 // Enable bit

class LCD {
  public:

    int addr;
    int fd;

    LCD(int addr) {
      this->addr = addr;
      if (wiringPiSetup () == -1) exit (1);
      this->fd = wiringPiI2CSetup(addr);
      lcd_init(); // setup LCD
    }

    // clr lcd go home loc 0x80
    void clear(void)   {
      lcd_byte(0x01, LCD_CMD);
      lcd_byte(0x02, LCD_CMD);
    }

    // go to location on LCD
    void lcdLoc(int line)   {
      lcd_byte(line, LCD_CMD);
    }

    // out char to LCD at current position
    void typeChar(char val)   {
      lcd_byte(val, LCD_CHR);
    }

    // this allows use of any size string
    void suggest(const char *s)   {
      clear();
      while ( *s ) lcd_byte(*(s++), LCD_CHR);
    }

    void lcd_byte(int bits, int mode)   {

      //Send byte to data pins
      // bits = the data
      // mode = 1 for data, 0 for command
      int bits_high;
      int bits_low;
      // uses the two half byte writes to LCD
      bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT ;
      bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT ;

      // High bits
      wiringPiI2CReadReg8(fd, bits_high);
      lcd_toggle_enable(bits_high);

      // Low bits
      wiringPiI2CReadReg8(fd, bits_low);
      lcd_toggle_enable(bits_low);
    }

    void lcd_toggle_enable(int bits)   {
      // Toggle enable pin on LCD display
      delayMicroseconds(500);
      wiringPiI2CReadReg8(this->fd, (bits | ENABLE));
      delayMicroseconds(500);
      wiringPiI2CReadReg8(this->fd, (bits & ~ENABLE));
      delayMicroseconds(500);
    }

    void lcd_init()   {
      // Initialise display
      lcd_byte(0x33, LCD_CMD); // Initialise
      lcd_byte(0x32, LCD_CMD); // Initialise
      lcd_byte(0x06, LCD_CMD); // Cursor move direction
      lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
      lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
      lcd_byte(0x01, LCD_CMD); // Clear display
      delayMicroseconds(500);
    }
};

class LCDList {
  public:
    std::vector<LCD*> LCDs;

    LCDList() {
    }

    LCDList(std::vector<int> addrs) {
      for(long unsigned int i = 0; i < addrs.size(); i++) {
        LCD* l = new LCD(addrs[i]);
        LCDs.push_back(l);
      }
      std::cout << "Created" << std::endl;
    }

    //Take in vector of suggestions and display it on all LCDs
    void suggest(std::string* suggestions) {
      LCDs.at(0)->suggest(suggestions[0].c_str());
      LCDs.at(1)->suggest(suggestions[1].c_str());
      
      return;
      for(long unsigned int i = 0; i < LCDs.size(); i++) {
        std::cout << suggestions[i] << std::endl;
        LCDs.at(i)->suggest(suggestions[i].c_str());
      }
    } 

    void clear() {
      for(long unsigned int i = 0; i < LCDs.size(); i++) {
        LCDs[i]->clear();
      }
    }
};
