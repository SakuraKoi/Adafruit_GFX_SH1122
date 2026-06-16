#ifndef _ADAFRUIT_SH1122_H_
#define _ADAFRUIT_SH1122_H_

#include <Adafruit_GFX.h>
#include <SPI.h>

#define SH1122_WIDTH  256
#define SH1122_HEIGHT 64

#define SH1122_BYTES_PER_ROW  (SH1122_WIDTH / 2)
#define SH1122_BUFSIZE        (SH1122_HEIGHT * SH1122_BYTES_PER_ROW)
#define SH1122_SPI_CLOCK      40000000UL
#define SH1122_BLACK          0x0
#define SH1122_WHITE          0xF

#ifndef BLACK
#define BLACK SH1122_BLACK
#endif
#ifndef WHITE
#define WHITE SH1122_WHITE
#endif

#define SH1122_CMD_DISPLAY_OFF    0xAE
#define SH1122_CMD_DISPLAY_ON     0xAF
#define SH1122_CMD_SET_ROW        0xB0
#define SH1122_CMD_SET_COL_LOW    0x00
#define SH1122_CMD_SET_COL_HIGH   0x10
#define SH1122_CMD_CONTRAST       0x81
#define SH1122_CMD_SEG_REMAP_NORM 0xA0
#define SH1122_CMD_SEG_REMAP_FLIP 0xA1
#define SH1122_CMD_COM_SCAN_NORM  0xC0
#define SH1122_CMD_COM_SCAN_FLIP  0xC8
#define SH1122_CMD_DISPLAY_NORMAL 0xA6
#define SH1122_CMD_DISPLAY_INVERT 0xA7
#define SH1122_CMD_START_LINE     0x40
#define SH1122_CMD_MUX_RATIO      0xA8
#define SH1122_CMD_DCDC           0xAD
#define SH1122_CMD_CLOCK_DIV      0xD5
#define SH1122_CMD_DISPLAY_OFFSET 0xD3
#define SH1122_CMD_PRECHARGE      0xD9
#define SH1122_CMD_VCOM_DESELECT  0xDB
#define SH1122_CMD_PRECHARGE_VOLT 0xDC
#define SH1122_CMD_DISCHARGE      0x30
#define SH1122_CMD_NOOP           0xE3

class Adafruit_SH1122 : public Adafruit_GFX {
public:
  Adafruit_SH1122(int8_t cs, int8_t dc, int8_t rst = -1, int8_t mosi = -1, int8_t sclk = -1);
  ~Adafruit_SH1122();

  bool begin(bool reset = true);
  void display();

  void setContrast(uint8_t contrast);
  void setPowerSave(bool save);

  void clearDisplay();

  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void fillScreen(uint16_t color) override;

  void startWrite(void) override;
  void endWrite(void) override;
  void writePixel(int16_t x, int16_t y, uint16_t color) override;
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

  void invertDisplay(bool i) override;

  uint8_t getPixel(int16_t x, int16_t y);

  void setRawPixel(int16_t x, int16_t y, uint8_t gray);

private:
  int8_t _cs, _dc, _rst, _mosi, _sclk;
  uint8_t _buffer[SH1122_BUFSIZE];
  uint8_t _contrast;
  bool    _invert;

  void sendCommand(uint8_t cmd);
  void sendCommand2(uint8_t cmd, uint8_t arg);
  void sendData(const uint8_t *data, uint16_t len);
  void sendDataByte(uint8_t data);
  void beginTransaction();
  void endTransaction();

  void writeRawPixel(int16_t x, int16_t y, uint8_t gray);
  static uint8_t colorToGray(uint16_t color);

  void hardwareReset();
  void sendInitSequence();
};

#endif
