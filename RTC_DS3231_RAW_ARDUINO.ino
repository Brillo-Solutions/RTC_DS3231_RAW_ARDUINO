#include <Wire.h>
#define DEV_ADDR_RTC 0x68 // I2C address of RTC - DS3231
#define DEV_ADDR_LCD 0x3F // I2C address of PCF8574A extender

byte mByte, mArr[7];
byte backLight;
const char *dayOfWeek[7] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"}; // Array of pointers containing days of week

void setup() 
{
  Wire.begin();
  setBackLight(true);
  initDisplay();
}

void loop() 
{
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0);  // Set base resiter address to 0h (Seconds)
  Wire.endTransmission();
  Wire.requestFrom(DEV_ADDR_RTC, 7);  // Seconds, Minutes, Hour, Day, Date, Month, Year

  for (int k = 0; k < 7; k++) // Reading time, day and date from DS3231
  {
    mByte = Wire.read();
    mArr[k] = mByte;
  }

  for (int m = 0, n = 12; m < 3; m++) // Showing time on 1st row of 16x2 LCD
  {
    showData(((mArr[m] & 0x0F) | 0x30), 1, n--);
    showData(((mArr[m] >> 4) | 0x30), 1, n--);
    if (n > 4)
      showData(':', 1, n--);
  }

  for (int m = 4, n = 3; m < 7; m++)  // Showing date on 2nd row of 16x2 LCD
  {
    showData(((mArr[m] >> 4) | 0x30), 2, n++);
    showData(((mArr[m] & 0x0F) | 0x30), 2, n++);
    if (n < 11)
      showData('/', 2, n++);
  }

  showData(dayOfWeek[mArr[3] - 1], 2, 12);
}

void sendToDisplay(byte mByte)  // Methods sending 8-bit data to PCF8574A I2C extender
{
  Wire.beginTransmission(DEV_ADDR_LCD);
  Wire.write(mByte);
  Wire.endTransmission();
}

void brkInstByte(byte mByte)
{
  byte nByte;
  nByte = mByte & 0xF0;
  sendToDisplay(nByte | (backLight | 0x04));
  delay(1);
  sendToDisplay(nByte | (backLight | 0x00));
  nByte = ((mByte << 4) & 0xF0);
  sendToDisplay(nByte | (backLight | 0x04));
  delay(1);
  sendToDisplay(nByte | (backLight | 0x00));
}

void brkDataByte(byte mByte)
{
  byte nByte;
  nByte = mByte & 0xF0;
  sendToDisplay(nByte | (backLight | 0x05));
  delay(1);
  sendToDisplay(nByte | (backLight | 0x01));
  nByte = ((mByte << 4) & 0xF0);
  sendToDisplay(nByte | (backLight | 0x05));
  delay(1);
  sendToDisplay(nByte | (backLight | 0x01));
}

void initDisplay()
{
  byte mArr[4] = {0x02, 0x28, 0x01, 0x0C};  // LCD 4-bit mode initialization commands
  for (int k = 0; k < 4; k++)
    brkInstByte(mArr[k]);  // Send 8-bit LCD commands in 4-bit mode

  showData("--:--:--", 1, 5);
  showData("--/--/-- ---", 2, 3);
}

void showData(const char *p, byte rn, byte cp)
{
  byte rowAddr;
  
  if (rn == 1)
    rowAddr = 0x80; // First row of 16x2 LCD
  else
    rowAddr = 0xC0; // Second row of 16x2 LCD

  brkInstByte(rowAddr + (cp - 1));  // Send 8-bit LCD commands in 4-bit mode
    
  while (*p != '\0')
  {
    brkDataByte(*p);  // Send 8-bit LCD data in 4-bit mode
    p++;
  }
}

void showData(byte mByte, byte rn, byte cp)
{
  byte rowAddr;
  
  if (rn == 1)
    rowAddr = 0x80; // First row of 16x2 LCD
  else
    rowAddr = 0xC0; // Second row of 16x2 LCD

  brkInstByte(rowAddr + (cp - 1));  // Send 8-bit LCD commands in 4-bit mode 
  brkDataByte(mByte);  // Send 8-bit LCD data in 4-bit mode
}

void setBackLight(boolean mBool)
{
  if (mBool)
    backLight = 0x08;  // Turn ON backlight of LCD
  else
    backLight = 0x00;  // Turn OFF backlight of LCD
}
