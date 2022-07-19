#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4

#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE 13

void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

byte readRom(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; ++pin) {
    pinMode(pin, INPUT);
  }

  setAddress(address, /*outputEnable*/ true);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
    data = (data << 1) + digitalRead(pin);
  }

  return data;
}

void writeRom(int address, byte data) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; ++pin) {
    pinMode(pin, OUTPUT);
  }

  setAddress(address, /*outputEnable*/ false);
  
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; ++pin) {
    digitalWrite(pin, data & 1);
    data >>= 1;
  }

  digitalWrite(WRITE_ENABLE, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_ENABLE, HIGH);
  delay(5);
}

void dumpRom(int range_start, int range_end) {
  for (int base = range_start; base < range_end; base += 16) {
    byte data[16];
    for (int offset = 0; offset < 16; ++offset) {
      data[offset] = readRom(base + offset);
    }

    char buf[54];
    sprintf(
      buf, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
      base,
      data[0], data[1], data[2], data[3],
      data[4], data[5], data[6], data[7],
      data[8], data[9], data[10], data[11],
      data[12], data[13], data[14], data[15]
    );
    Serial.println(buf);
  }
}

const byte digits[] = {
  0x7e,
  0x30,
  0x6d,
  0x79,
  0x33,
  0x5b,
  0x5f,
  0x70,
  0x7f,
  0x7b
  /*0x77,*/
  /*0x1f,*/
  /*0x4e,*/
  /*0x3d,*/
  /*0x4f,*/
  /*0x47*/
};

const byte neg_sign = 0x01;

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_ENABLE, HIGH);
  pinMode(WRITE_ENABLE, OUTPUT);

  Serial.begin(115200);

  Serial.print("Erasing EEPROM...\n");

  for (int i = 0; i < 2048; ++i) {
    writeRom(i, 0);
  }

  Serial.print("Programming EEPROM...\n");

  // program unsigned values
  const int tens_off = 0x01 << 8;
  const int hund_off = 0x02 << 8;

  for (int i = 0; i < 256; ++i) {
    writeRom(i, digits[i % 10]);
    if (i >= 10) {
      writeRom(i | tens_off, digits[(i / 10) % 10]);
      if (i >= 100) {
        writeRom(i | hund_off, digits[(i / 100) % 10]);
      }
    }
  }

  // program signed values
  const int thou_off = 0x03 << 8;
  const int two_comp_off = 0x04 << 8;

  for (int i = -128; i < 128; ++i) {
    int i_addr = (byte)i | two_comp_off;
    int i_abs = abs(i);

    writeRom(i_addr, digits[i_abs % 10]);
    if (i_abs >= 10) {
      writeRom(i_addr | tens_off, digits[(i_abs / 10) % 10]);
      if (i_abs >= 100) {
        writeRom(i_addr | hund_off, digits[(i_abs / 100) % 10]);
      }
    }

    // set neg sign
    if (i < 0) {
      if (i_abs < 10) {
        writeRom(i_addr | tens_off, neg_sign);
      } else if (i_abs < 100) {
        writeRom(i_addr | hund_off, neg_sign);
      } else {
        writeRom(i_addr | thou_off, neg_sign);
      }
    }
  }

  dumpRom(0, 2048);
}

void loop() {
  // put your main code here, to run repeatedly:
}
