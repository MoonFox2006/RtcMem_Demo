#include <Arduino.h>
#include "RtcMem.h"

struct __packed data_t {
  uint32_t timestamp;
  float temp;
  float hum;
  float press;
  bool sended : 1;
  bool stored : 1;
};

RtcList<data_t> list;

void printData() {
  data_t data;

  Serial.print(F("timestamp: "));
  for (uint16_t i = 0; i < list.count(); ++i) {
    list.get(i, data);
    if (i)
      Serial.print(',');
    Serial.print(data.timestamp);
  }
  Serial.println();
  Serial.print(F("temperature: "));
  for (uint16_t i = 0; i < list.count(); ++i) {
    list.get(i, data);
    if (i)
      Serial.print(',');
    Serial.print(data.temp);
  }
  Serial.println(F(" C"));
  Serial.print(F("humidity: "));
  for (uint16_t i = 0; i < list.count(); ++i) {
    list.get(i, data);
    if (i)
      Serial.print(',');
    Serial.print(data.hum);
  }
  Serial.println(F(" %"));
  Serial.print(F("pressure: "));
  for (uint16_t i = 0; i < list.count(); ++i) {
    list.get(i, data);
    if (i)
      Serial.print(',');
    Serial.print(data.press);
  }
  Serial.println(F(" Pa"));
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  uint16_t cnt;
  data_t data;

  cnt = list.count();
  if (! cnt) {
    Serial.println(F("First run"));
    data.timestamp = 1;
    data.temp = 20;
    data.hum = 50;
    data.press = 100000;
  } else {
    Serial.print(cnt);
    if (cnt == list.MAXSIZE)
      Serial.print('+');
    Serial.println(F(" time(s)"));
    if (! list.get(cnt - 1, data)) {
      Serial.println(F("Error getting last data item!"));
      return;
    }
    ++data.timestamp;
    if (data.temp < 25)
      data.temp += random(200) / 100.0;
    if (data.temp > 15)
      data.temp -= random(200) / 100.0;
    if (data.hum < 75)
      data.hum += random(500) / 100.0;
    if (data.hum > 25)
      data.hum -= random(500) / 100.0;
    if (data.press < 110000)
      data.press += random(1000);
    if (data.press > 90000)
      data.press -= random(1000);
  }
  data.sended = false;
  data.stored = false;
  if (list.add(data, true) == list.ERR_INDEX) {
    Serial.println(F("Error adding data item!"));
    return;
  }
  printData();

  Serial.flush();
  ESP.deepSleep(1000000); // 1 sec.
}

void loop() {
  Serial.flush();
  ESP.deepSleep(0); // halt
}
