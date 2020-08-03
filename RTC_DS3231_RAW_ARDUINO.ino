#include <Wire.h>
#define DEV_ADDR 0x68

byte mByte, mArr[7];

void setup() 
{
  Wire.begin();
  Serial.begin(115200);
}

void loop() 
{
  Wire.beginTransmission(DEV_ADDR);
  Wire.write(0);  // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DEV_ADDR, 7);  // seconds, minutes, hour, day, date, month, year

  for (int k = 0; k < 7; k++)
  {
    mByte = Wire.read();
    mByte = (mByte / 16 * 10) + (mByte % 16);
    mArr[k] = mByte;
  }

  Serial.println("Time:\t" + String(mArr[2] - 12) + ":" + String(mArr[1]) + ":" + String(mArr[0]));
  Serial.println("Date:\t" + String(mArr[4]) + "/" + String(mArr[5]) + "/" + String(2000 + mArr[6]));
  switch (mArr[3])
  {
    case 1:
    {
      Serial.println("Day:\tMonday\n");
      break;
    }
    case 2:
    {
      Serial.println("Day:\tTuesday\n");
      break;
    }
    case 3:
    {
      Serial.println("Day:\tWednesday\n");
      break;
    }
    case 4:
    {
      Serial.println("Day:\tThursday\n");
      break;
    }
    case 5:
    {
      Serial.println("Day:\tFriday\n");
      break;
    }
    case 6:
    {
      Serial.println("Day:\tSaturday\n");
      break;
    }
    case 7:
    {
      Serial.println("Day:\tSunday\n");
      break;
    }
  }
  delay(1000);
}
