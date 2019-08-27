#ifndef __RTCMEM_H
#define __RTCMEM_H

#include <inttypes.h>

class RtcMem {
public:
  static const uint16_t SIZE = 512;

  static uint8_t bytes(uint16_t index);
  uint8_t operator[](uint16_t index) const {
    return bytes(index);
  }
  static uint16_t words(uint16_t index);
  static uint32_t dwords(uint16_t index);
  static bool set(uint16_t index, uint8_t value);
  static bool set(uint16_t index, uint16_t value);
  static bool set(uint16_t index, uint32_t value);
  static bool fill(uint16_t offset = 0, uint16_t length = SIZE, uint8_t pattern = 0);
  static bool load(uint8_t *buf, uint16_t offset, uint16_t length);
  static bool store(const uint8_t *buf, uint16_t offset, uint16_t length);
  static bool store_P(const uint8_t *buf, uint16_t offset, uint16_t length);
  static bool move(uint16_t dest, uint16_t src, uint16_t length);
};

template <class T>
class RtcList {
public:
  static const uint16_t MAXSIZE = (RtcMem::SIZE - 4) / sizeof(T);
  static const uint16_t ERR_INDEX = 0xFFFF;

  RtcList<T>() {
    read();
  }

  uint16_t count() const {
    return _count;
  }
  void clear();
  uint16_t add(const T &value, bool overwrite = false);
  bool remove(uint16_t index);
  bool get(uint16_t index, T &value);
  bool set(uint16_t index, const T &value);

protected:
  uint16_t crc16();
  void read();
  void write();

  uint16_t _count;
};

template <class T>
void RtcList<T>::clear() {
  _count = 0;
  write();
}

template <class T>
uint16_t RtcList<T>::add(const T &value, bool overwrite) {
  if (_count >= MAXSIZE) {
    if (! overwrite)
      return ERR_INDEX;
    RtcMem::move(0, sizeof(T), sizeof(T) * (_count - 1));
  } else
    ++_count;
  RtcMem::store((uint8_t*)&value, sizeof(T) * (_count - 1), sizeof(T));
  write();
  return (_count - 1);
}

template <class T>
bool RtcList<T>::remove(uint16_t index) {
  if (index < _count) {
    if (index < _count - 1) {
      RtcMem::move(sizeof(T) * index, sizeof(T) * (index + 1), sizeof(T) * (_count - index - 1));
    }
    --_count;
    write();
    return true;
  }
  return false;
}

template <class T>
bool RtcList<T>::get(uint16_t index, T &value) {
  if (index < _count) {
    return RtcMem::load((uint8_t*)&value, sizeof(T) * index, sizeof(T));
  }
  return false;
}

template <class T>
bool RtcList<T>::set(uint16_t index, const T &value) {
  if (index < _count) {
    if (RtcMem::store((uint8_t*)&value, sizeof(T) * index, sizeof(T))) {
      write();
      return true;
    }
  }
  return false;
}

template <class T>
uint16_t RtcList<T>::crc16() {
  uint16_t crc = 0xFFFF;

  for (uint16_t offset = 0; offset < (RtcMem::SIZE - 4) / 4; ++offset) { // data without footer
    uint32_t data = RtcMem::dwords(offset);

    for (uint8_t i = 0; i < 4; ++i) {
      crc ^= (uint8_t)(data >> (i * 8)) << 8;
      for (uint8_t j = 0; j < 8; ++j)
        crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
  }
  {
    uint16_t data = RtcMem::words((RtcMem::SIZE - 2) / 2 - 1); // count in footer

    for (uint8_t i = 0; i < 2; ++i) {
      crc ^= (uint8_t)(data >> (i * 8)) << 8;
      for (uint8_t j = 0; j < 8; ++j)
        crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
  }
  return crc;
}

template <class T>
void RtcList<T>::read() {
  _count = RtcMem::words((RtcMem::SIZE - 2) / 2 - 1);
  if ((RtcMem::words((RtcMem::SIZE - 2) / 2) != crc16()) || (_count > MAXSIZE))
    clear();
}

template <class T>
void RtcList<T>::write() {
  RtcMem::set((RtcMem::SIZE - 2) / 2 - 1, _count);
  RtcMem::set((RtcMem::SIZE - 2) / 2, crc16());
}

#ifndef NO_GLOBAL_INSTANCES
extern RtcMem RTCMEM;
#endif

#endif
