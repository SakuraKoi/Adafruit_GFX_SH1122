#include "Adafruit_SH1122.h"

Adafruit_SH1122::Adafruit_SH1122(int8_t cs, int8_t dc, int8_t rst, int8_t mosi, int8_t sclk)
    : Adafruit_GFX(SH1122_WIDTH, SH1122_HEIGHT)
    , _cs(cs), _dc(dc), _rst(rst), _mosi(mosi), _sclk(sclk)
    , _contrast(0x80), _invert(false)
{
    memset(_buffer, 0, SH1122_BUFSIZE);
}

Adafruit_SH1122::~Adafruit_SH1122() {}

bool Adafruit_SH1122::begin(bool reset) {
    pinMode(_cs, OUTPUT);
    pinMode(_dc, OUTPUT);

    digitalWrite(_cs, HIGH);
    digitalWrite(_dc, LOW);

#if defined(ESP32) || defined(ESP8266)
    SPI.begin(_sclk, -1, _mosi, -1);
#else
    SPI.begin();
#endif

    if (_rst >= 0) {
        pinMode(_rst, OUTPUT);
        if (reset) {
            hardwareReset();
        } else {
            digitalWrite(_rst, HIGH);
        }
    }

    sendInitSequence();
    setPowerSave(false);

    return true;
}

void Adafruit_SH1122::beginTransaction() {
#if defined(SPI_HAS_TRANSACTION)
    SPI.beginTransaction(SPISettings(SH1122_SPI_CLOCK, MSBFIRST, SPI_MODE0));
#endif
}

void Adafruit_SH1122::endTransaction() {
#if defined(SPI_HAS_TRANSACTION)
    SPI.endTransaction();
#endif
}

void Adafruit_SH1122::hardwareReset() {
    digitalWrite(_rst, HIGH);
    delay(10);
    digitalWrite(_rst, LOW);
    delay(10);
    digitalWrite(_rst, HIGH);
    delay(20);
}

void Adafruit_SH1122::sendInitSequence() {
    sendCommand(SH1122_CMD_DISPLAY_OFF);
    sendCommand(SH1122_CMD_START_LINE | 0x00);

    sendCommand(SH1122_CMD_SEG_REMAP_NORM);
    sendCommand(SH1122_CMD_COM_SCAN_NORM);

    sendCommand2(SH1122_CMD_CONTRAST, _contrast);
    sendCommand2(SH1122_CMD_MUX_RATIO, 0x3F);
    sendCommand2(SH1122_CMD_DCDC, 0x81);

    sendCommand2(SH1122_CMD_CLOCK_DIV, 0x50);
    sendCommand2(SH1122_CMD_DISPLAY_OFFSET, 0x00);
    sendCommand2(SH1122_CMD_PRECHARGE, 0x22);
    sendCommand2(SH1122_CMD_VCOM_DESELECT, 0x35);
    sendCommand2(SH1122_CMD_PRECHARGE_VOLT, 0x35);
    sendCommand(SH1122_CMD_DISCHARGE);
}

void Adafruit_SH1122::sendCommand(uint8_t cmd) {
    beginTransaction();
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    SPI.transfer(cmd);
    digitalWrite(_cs, HIGH);
    endTransaction();
}

void Adafruit_SH1122::sendCommand2(uint8_t cmd, uint8_t arg) {
    beginTransaction();
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    SPI.transfer(cmd);
    SPI.transfer(arg);
    digitalWrite(_cs, HIGH);
    endTransaction();
}

void Adafruit_SH1122::sendData(const uint8_t *data, uint16_t len) {
    beginTransaction();
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    while (len--) {
        SPI.transfer(*data++);
    }
    digitalWrite(_cs, HIGH);
    endTransaction();
}

void Adafruit_SH1122::sendDataByte(uint8_t data) {
    beginTransaction();
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    SPI.transfer(data);
    digitalWrite(_cs, HIGH);
    endTransaction();
}

uint8_t Adafruit_SH1122::colorToGray(uint16_t color) {
    return color & 0x0F;
}

void Adafruit_SH1122::writeRawPixel(int16_t x, int16_t y, uint8_t gray) {
    if (x < 0 || x >= SH1122_WIDTH || y < 0 || y >= SH1122_HEIGHT)
        return;

    uint16_t idx = (uint16_t)y * SH1122_BYTES_PER_ROW + ((uint16_t)x >> 1);

    if (x & 1)
        _buffer[idx] = (_buffer[idx] & 0x0F) | (gray << 4);
    else
        _buffer[idx] = (_buffer[idx] & 0xF0) | (gray & 0x0F);
}

void Adafruit_SH1122::drawPixel(int16_t x, int16_t y, uint16_t color) {
    writeRawPixel(x, y, colorToGray(color));
}

void Adafruit_SH1122::writePixel(int16_t x, int16_t y, uint16_t color) {
    writeRawPixel(x, y, colorToGray(color));
}

void Adafruit_SH1122::setRawPixel(int16_t x, int16_t y, uint8_t gray) {
    writeRawPixel(x, y, gray & 0x0F);
}

uint8_t Adafruit_SH1122::getPixel(int16_t x, int16_t y) {
    if (x < 0 || x >= SH1122_WIDTH || y < 0 || y >= SH1122_HEIGHT)
        return 0;

    uint16_t idx = (uint16_t)y * SH1122_BYTES_PER_ROW + ((uint16_t)x >> 1);

    if (x & 1)
        return (_buffer[idx] >> 4) & 0x0F;
    else
        return _buffer[idx] & 0x0F;
}

void Adafruit_SH1122::fillScreen(uint16_t color) {
    uint8_t gray = colorToGray(color);
    uint8_t fillByte = gray | (gray << 4);
    memset(_buffer, fillByte, SH1122_BUFSIZE);
}

void Adafruit_SH1122::startWrite(void) {
}

void Adafruit_SH1122::endWrite(void) {
}

void Adafruit_SH1122::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (x < 0 || x >= SH1122_WIDTH)
        return;

    if (y < 0) {
        h += y;
        y = 0;
    }
    if (y + h > SH1122_HEIGHT)
        h = SH1122_HEIGHT - y;
    if (h <= 0)
        return;

    uint8_t gray = colorToGray(color);
    uint16_t idx = (uint16_t)y * SH1122_BYTES_PER_ROW + ((uint16_t)x >> 1);

    if (x & 1) {
        for (int16_t i = 0; i < h; i++) {
            _buffer[idx] = (_buffer[idx] & 0x0F) | (gray << 4);
            idx += SH1122_BYTES_PER_ROW;
        }
    } else {
        for (int16_t i = 0; i < h; i++) {
            _buffer[idx] = (_buffer[idx] & 0xF0) | (gray & 0x0F);
            idx += SH1122_BYTES_PER_ROW;
        }
    }
}

void Adafruit_SH1122::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (y < 0 || y >= SH1122_HEIGHT)
        return;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (x + w > SH1122_WIDTH)
        w = SH1122_WIDTH - x;
    if (w <= 0)
        return;

    uint8_t gray = colorToGray(color);
    uint16_t idx = (uint16_t)y * SH1122_BYTES_PER_ROW + ((uint16_t)x >> 1);
    uint8_t startOdd = x & 1;

    if (startOdd && w > 0) {
        _buffer[idx] = (_buffer[idx] & 0x0F) | (gray << 4);
        idx++;
        w--;
        x++;
    }

    while (w >= 2) {
        _buffer[idx] = gray | (gray << 4);
        idx++;
        w -= 2;
    }

    if (w > 0) {
        _buffer[idx] = (_buffer[idx] & 0xF0) | (gray & 0x0F);
    }
}

void Adafruit_SH1122::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SH1122_WIDTH)  w = SH1122_WIDTH - x;
    if (y + h > SH1122_HEIGHT) h = SH1122_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    uint8_t gray = colorToGray(color);
    uint8_t fillByte = gray | (gray << 4);

    uint16_t rowBase = (uint16_t)y * SH1122_BYTES_PER_ROW;
    uint16_t colStart = (uint16_t)x >> 1;

    for (int16_t row = 0; row < h; row++) {
        uint16_t idx = rowBase + (uint16_t)row * SH1122_BYTES_PER_ROW + colStart;
        int16_t remaining = w;
        uint8_t col = (uint8_t)(x & 1);

        if (col && remaining > 0) {
            _buffer[idx] = (_buffer[idx] & 0x0F) | (gray << 4);
            idx++;
            remaining--;
        }

        while (remaining >= 2) {
            _buffer[idx] = fillByte;
            idx++;
            remaining -= 2;
        }

        if (remaining > 0) {
            _buffer[idx] = (_buffer[idx] & 0xF0) | (gray & 0x0F);
        }
    }
}

void Adafruit_SH1122::display() {
    for (uint8_t row = 0; row < SH1122_HEIGHT; row++) {
        beginTransaction();

        digitalWrite(_dc, LOW);
        digitalWrite(_cs, LOW);

        SPI.transfer(SH1122_CMD_SET_ROW);
        SPI.transfer(row);
        SPI.transfer(SH1122_CMD_SET_COL_LOW | 0x00);
        SPI.transfer(SH1122_CMD_SET_COL_HIGH | 0x00);

        digitalWrite(_dc, HIGH);

        const uint8_t *rowPtr = _buffer + (uint16_t)row * SH1122_BYTES_PER_ROW;
        for (uint16_t col = 0; col < SH1122_BYTES_PER_ROW; col++) {
            SPI.transfer(rowPtr[col]);
        }

        digitalWrite(_cs, HIGH);
        endTransaction();
    }
}

void Adafruit_SH1122::clearDisplay() {
    memset(_buffer, 0, SH1122_BUFSIZE);
    display();
}

void Adafruit_SH1122::setContrast(uint8_t contrast) {
    _contrast = contrast;
    sendCommand2(SH1122_CMD_CONTRAST, _contrast);
}

void Adafruit_SH1122::setPowerSave(bool save) {
    sendCommand(save ? SH1122_CMD_DISPLAY_OFF : SH1122_CMD_DISPLAY_ON);
}

void Adafruit_SH1122::invertDisplay(bool i) {
    _invert = i;
    sendCommand(i ? SH1122_CMD_DISPLAY_INVERT : SH1122_CMD_DISPLAY_NORMAL);
}
