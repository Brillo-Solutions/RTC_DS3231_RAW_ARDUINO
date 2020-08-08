#include <Wire.h>
#define DEV_ADDR_RTC 0x68 // I2C address of RTC - DS3231
#define DEV_ADDR_LCD 0x20 // I2C address of PCF8574A extender

byte mByte, mArr[7];
byte backLight, nArr[7] = {0, 30, 16, 7, 4, 8, 20}; // Set your time here (Seconds, Minutes, Hour, Day, Date, Month, Year)
const char *dayOfWeek[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"}; // Array of pointers containing days of week

void setup() 
{
  Wire.begin();
//  setRtc(); // Call this function if you want to set RTC date & time    
//  setAlarm(19, 23, 59); // Hour, Minutes and Seconds - call this function if you want to set alarm ON time
  setBackLight(false);
  initDisplay();
}

void loop() 
{
  readRtc();
  showTime();
  showDate();
  showDayOfWeek();
  showTemperature();
}

void readRtc()
{
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0);  // Set base register address to 0h (Seconds)
  Wire.endTransmission();
  Wire.requestFrom(DEV_ADDR_RTC, 7);  // Seconds, Minutes, Hour, Day, Date, Month, Year

  for (int k = 0; k < 7; k++) // Reading time, day and date from DS3231
  {
    mByte = Wire.read();
    mArr[k] = mByte;
  }
}

void showTemperature()
{
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x0E);
  Wire.endTransmission();
  Wire.requestFrom(DEV_ADDR_RTC, 1);
  
  mByte = Wire.read();
  mByte |= 0x20;  // Setting CONV bit of 0x0E register to convert the temperature

  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x0E);
  Wire.write(mByte);
  Wire.endTransmission();

  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x11); // Temperature value register
  Wire.endTransmission();
  Wire.requestFrom(DEV_ADDR_RTC, 1);

  mByte = Wire.read();
  mByte = (mByte / 10 * 16) + (mByte % 10); // Decimal to Hexadecimal (BCD)

  showData(((mByte >> 4) | 0x30), 1, 12);
  showData(((mByte & 0x0F) | 0x30), 1, 13);
  showData("\337", 1, 14);
}

void showTime()
{
  for (int m = 0, n = 10; m < 3; m++) // Showing time on 1st row of 16x2 LCD
  {
    showData(((mArr[m] & 0x0F) | 0x30), 1, n--);
    showData(((mArr[m] >> 4) | 0x30), 1, n--);
    if (n > 2)
      showData(':', 1, n--);
  }
}

void showDate()
{
  for (int m = 4, n = 3; m < 7; m++)  // Showing date on 2nd row of 16x2 LCD
  {
    showData(((mArr[m] >> 4) | 0x30), 2, n++);
    showData(((mArr[m] & 0x0F) | 0x30), 2, n++);
    if (n < 11)
      showData('/', 2, n++);
  }
}

void showDayOfWeek()
{
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

  showData("--:--:-- ---", 1, 3);
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

void setRtc()
{
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0);  // Set base register address to 0h (Seconds)
    for (int k = 0; k < 7; k++)
      Wire.write((nArr[k] / 10 * 16) + (nArr[k] % 10)); // Decimal to Hexadecimal (BCD)

  Wire.endTransmission();
}

void setAlarm(byte hByte, byte mByte, byte sByte)
{
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x07); // Alarm second setting
  Wire.write((sByte / 10 * 16) + (sByte % 10));
  Wire.endTransmission();
  
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x08); // Alarm minute setting
  Wire.write((mByte / 10 * 16) + (mByte % 10));
  Wire.endTransmission();
  
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x09); // Alarm hour setting
  Wire.write((hByte / 10 * 16) + (hByte % 10));
  Wire.endTransmission();
  
  onAlarm();
}

void onAlarm()
{
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x0A); // Alarm second, minute and hour mask bit setting
  Wire.write(0x80);
  Wire.endTransmission();
  
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x0E); // Reading control register content to set alarm - 1
  Wire.endTransmission();
  Wire.requestFrom(DEV_ADDR_RTC, 1);
  
  mByte = Wire.read();
  mByte |= 0x01;  // Setting A1IE bit of 0x0E register to enable alarm - 1

  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x0E);  // Writing A1IE bit setting back to control register
  Wire.write(mByte);
  Wire.endTransmission();
}

void offAlarm()
{
  Wire.beginTransmission(DEV_ADDR_RTC);
  Wire.write(0x0F); // Reading content of status register
  Wire.endTransmission();
  Wire.requestFrom(DEV_ADDR_RTC, 1);
  
  mByte = Wire.read();

  if (mByte & 0x01 == 0x01) // Monitoring A1F alarm - 1 flag to be set in 0x0F register
  {
    Wire.beginTransmission(DEV_ADDR_RTC);
    Wire.write(0x0F); // Reading control register content to set alarm - 1
    Wire.endTransmission();
    Wire.requestFrom(DEV_ADDR_RTC, 1);
    
    mByte = Wire.read();
    mByte &= 0xFE;
    
    Wire.beginTransmission(DEV_ADDR_RTC);
    Wire.write(0x0F);  
    Wire.write(mByte);  // Writing A1IE bit setting back to control register
    Wire.endTransmission();
  }
}
