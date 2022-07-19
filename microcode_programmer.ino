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
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
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
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    pinMode(pin, OUTPUT);
  }

  setAddress(address, /*outputEnable*/ false);
  
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
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
    for (int offset = 0; offset < 16; offset++) {
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

const uint16_t FI  = 0b0000000000000001;  // CPU flags in
const uint16_t J   = 0b0000000000000010;  // Jump
const uint16_t CO  = 0b0000000000000100;  // Counter out
const uint16_t CE  = 0b0000000000001000;  // Counter enable
const uint16_t OI  = 0b0000000000010000;  // Output in
const uint16_t BI  = 0b0000000000100000;  // B register in
const uint16_t SU  = 0b0000000001000000;  // ALU subtract
const uint16_t EO  = 0b0000000010000000;  // ALU out
const uint16_t AO  = 0b0000000100000000;  // A register out
const uint16_t AI  = 0b0000001000000000;  // A register in
const uint16_t II  = 0b0000010000000000;  // Instruction register in
const uint16_t IO  = 0b0000100000000000;  // Instruction register out
const uint16_t RO  = 0b0001000000000000;  // RAM out
const uint16_t RI  = 0b0010000000000000;  // RAM in
const uint16_t MI  = 0b0100000000000000;  // Address register in
const uint16_t HLT = 0b1000000000000000;  // Halt

const byte INST_COUNT = 16;
const byte MICRO_COUNT = 16;

const uint16_t INSTS[INST_COUNT][MICRO_COUNT] = {
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0000 - NOP
  {MI|CO, RO|II|CE, IO|MI, RO|AI, 0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0001 - LDA
  {MI|CO, RO|II|CE, IO|MI, RO|BI, EO|AI|FI,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0010 - ADD
  {MI|CO, RO|II|CE, IO|MI, RO|BI, EO|AI|SU|FI, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0011 - SUB
  {MI|CO, RO|II|CE, IO|MI, AO|RI, 0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0100 - STA
  {MI|CO, RO|II|CE, IO|AI, 0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0101 - LDI
  {MI|CO, RO|II|CE, IO|J,  0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0110 - JMP
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 0111 - JC
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 1000 - JZ
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {MI|CO, RO|II|CE, AO|OI, 0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 1110 - OUT
  {MI|CO, RO|II|CE, HLT,   0,     0,           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // 1111 - HLT
};

const byte INST_JMP = 6;
const byte INST_JC = 7;
const byte INST_JZ = 8;

const byte FLAG_COUNT = 4;

const byte FLAG_CF = 0b01;
const byte FLAG_ZF = 0b10;

byte getInst(byte inst, byte mc, bool hi_byte) {
  return hi_byte ? INSTS[inst][mc] >> 8 : INSTS[inst][mc];
}

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_ENABLE, HIGH);
  pinMode(WRITE_ENABLE, OUTPUT);

  Serial.begin(115200);

  Serial.print("Programming EEPROM...\n");
  int i = 0;
  for (byte bs = 0; bs < 2; ++bs) {
    for (byte fl = 0; fl < FLAG_COUNT; ++fl) {
      for (byte in = 0; in < INST_COUNT; ++in) {
        for (byte mc = 0; mc < MICRO_COUNT; ++mc) {
          if (mc == 2) {
            if (
              (in == INST_JC && fl & FLAG_CF) ||
              (in == INST_JZ && fl & FLAG_ZF)
            ) {
              writeRom(i, getInst(INST_JMP, 2, bs));
            } else {
              writeRom(i, getInst(in, mc, bs));
            }
          } else {
            writeRom(i, getInst(in, mc, bs));
          }

          ++i;
        }
      }
    }
  }

  dumpRom(0, 2048);
}

void loop() {
  // put your main code here, to run repeatedly:
}
