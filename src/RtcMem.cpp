extern "C" {
#include "user_interface.h"
}
#include "pgmspace.h"
#include "RtcMem.h"

inline bool rtcUserMemoryRead(uint16_t offset, uint32_t *data, uint16_t size) {
  return system_rtc_mem_read(64 + offset, data, size);
}

inline bool rtcUserMemoryWrite(uint16_t offset, uint32_t *data, uint16_t size) {
  return system_rtc_mem_write(64 + offset, data, size);
}

uint8_t RtcMem::bytes(uint16_t index) {
  if (index < SIZE) {
    uint32_t data;

    rtcUserMemoryRead(index / 4, &data, 4);
    return (data >> ((index % 4) * 8));
  }
}

uint16_t RtcMem::words(uint16_t index) {
  if (index < SIZE / 2) {
    uint32_t data;

    rtcUserMemoryRead(index / 2, &data, 4);
    return (data >> ((index % 2) * 16));
  }
}

uint32_t RtcMem::dwords(uint16_t index) {
  if (index < SIZE / 4) {
    uint32_t data;

    rtcUserMemoryRead(index, &data, 4);
    return data;
  }
}

bool RtcMem::set(uint16_t index, uint8_t value) {
  if (index < SIZE) {
    uint32_t data;

    rtcUserMemoryRead(index / 4, &data, 4);
    data &= ~(0xFF << ((index % 4) * 8));
    data |= value << ((index % 4) * 8);
    rtcUserMemoryWrite(index / 4, &data, 4);
    return true;
  }
  return false;
}

bool RtcMem::set(uint16_t index, uint16_t value) {
  if (index < SIZE / 2) {
    uint32_t data;

    rtcUserMemoryRead(index / 2, &data, 4);
    data &= ~(0xFFFF << ((index % 2) * 16));
    data |= value << ((index % 2) * 16);
    rtcUserMemoryWrite(index / 2, &data, 4);
    return true;
  }
  return false;
}

bool RtcMem::set(uint16_t index, uint32_t value) {
  if (index < SIZE / 4) {
    rtcUserMemoryWrite(index, &value, 4);
    return true;
  }
  return false;
}

bool RtcMem::fill(uint16_t offset, uint16_t length, uint8_t pattern) {
  if (length && (offset + length <= SIZE)) {
    uint32_t data;

    if (offset % 4) {
      rtcUserMemoryRead(offset / 4, &data, 4);
      for (uint8_t i = offset % 4; i < 4; ++i) {
        data &= ~(0xFF << (i * 8));
        data |= pattern << (i * 8);
        if (! --length)
          break;
      }
      rtcUserMemoryWrite(offset / 4, &data, 4);
      offset = ((offset + 3) / 4) * 4;
    }
    data = (pattern << 24) | (pattern << 16) | (pattern << 8) | pattern;
    while (length >= 4) {
      rtcUserMemoryWrite(offset / 4, &data, 4);
      offset += 4;
      length -= 4;
    }
    if (length) {
      rtcUserMemoryRead(offset / 4, &data, 4);
      for (uint8_t i = 0; i < length; ++i) {
        data &= ~(0xFF << (i * 8));
        data |= pattern << (i * 8);
      }
      rtcUserMemoryWrite(offset / 4, &data, 4);
    }
    return true;
  }
  return false;
}

bool RtcMem::load(uint8_t *buf, uint16_t offset, uint16_t length) {
  if (length && (offset + length <= SIZE)) {
    uint32_t data;

    if (offset % 4) {
      rtcUserMemoryRead(offset / 4, &data, 4);
      for (uint8_t i = offset % 4; i < 4; ++i) {
        *buf++ = data >> (i * 8);
        if (! --length)
          break;
      }
      offset = ((offset + 3) / 4) * 4;
    }
    if (length >= 4) {
      if ((uint32_t)buf % 4) { // Address is not aligned!
        while (length >= 4) {
          rtcUserMemoryRead(offset / 4, &data, 4);
          *buf++ = data;
          *buf++ = data >> 8;
          *buf++ = data >> 16;
          *buf++ = data >> 24;
          offset += 4;
          length -= 4;
        }
      } else {
        rtcUserMemoryRead(offset / 4, (uint32_t*)buf, (length / 4) * 4);
        offset += (length / 4) * 4;
        buf += (length / 4) * 4;
        length -= (length / 4) * 4;
      }
    }
    if (length) {
      rtcUserMemoryRead(offset / 4, &data, 4);
      for (uint8_t i = 0; i < length; ++i) {
        *buf++ = data >> (i * 8);
      }
    }
    return true;
  }
  return false;
}

bool RtcMem::store(const uint8_t *buf, uint16_t offset, uint16_t length) {
  if (length && (offset + length <= SIZE)) {
    uint32_t data;

    if (offset % 4) {
      rtcUserMemoryRead(offset / 4, &data, 4);
      for (uint8_t i = offset % 4; i < 4; ++i) {
        data &= ~(0xFF << (i * 8));
        data |= *buf++ << (i * 8);
        if (! --length)
          break;
      }
      rtcUserMemoryWrite(offset / 4, &data, 4);
      offset = ((offset + 3) / 4) * 4;
    }
    if (length >= 4) {
      if ((uint32_t)buf % 4) {
        while (length >= 4) {
          data = *buf++;
          data |= *buf++ << 8;
          data |= *buf++ << 16;
          data |= *buf++ << 24;
          rtcUserMemoryWrite(offset / 4, &data, 4);
          offset += 4;
          length -= 4;
        }
      } else {
        rtcUserMemoryWrite(offset / 4, (uint32_t*)buf, (length / 4) * 4);
        offset += (length / 4) * 4;
        buf += (length / 4) * 4;
        length -= (length / 4) * 4;
      }
    }
    if (length) {
      rtcUserMemoryRead(offset / 4, &data, 4);
      for (uint8_t i = 0; i < length; ++i) {
        data &= ~(0xFF << (i * 8));
        data |= *buf++ << (i * 8);
      }
      rtcUserMemoryWrite(offset / 4, &data, 4);
    }
    return true;
  }
  return false;
}

bool RtcMem::store_P(const uint8_t *buf, uint16_t offset, uint16_t length) {
  if (length && (offset + length <= SIZE)) {
    uint32_t data;

    if (offset % 4) {
      rtcUserMemoryRead(offset / 4, &data, 4);
      for (uint8_t i = offset % 4; i < 4; ++i) {
        data &= ~(0xFF << (i * 8));
        data |= pgm_read_byte(buf++) << (i * 8);
        if (! --length)
          break;
      }
      rtcUserMemoryWrite(offset / 4, &data, 4);
      offset = ((offset + 3) / 4) * 4;
    }
    while (length >= 4) {
      data = pgm_read_dword(buf);
      rtcUserMemoryWrite(offset / 4, &data, 4);
      offset += 4;
      buf += 4;
      length -= 4;
    }
    if (length) {
      rtcUserMemoryRead(offset / 4, &data, 4);
      for (uint8_t i = 0; i < length; ++i) {
        data &= ~(0xFF << (i * 8));
        data |= pgm_read_byte(buf++) << (i * 8);
      }
      rtcUserMemoryWrite(offset / 4, &data, 4);
    }
    return true;
  }
  return false;
}

bool RtcMem::move(uint16_t dest, uint16_t src, uint16_t length) {
  if (length && (dest != src) && (dest + length <= SIZE) && (src + length <= SIZE)) {
    uint32_t d, s;

    if (dest < src) {
      rtcUserMemoryRead(src / 4, &s, 4);
      rtcUserMemoryRead(dest / 4, &d, 4);
      while (length--) {
        d &= ~(0xFF << ((dest % 4) * 8));
        d |= ((s >> ((src % 4) * 8)) & 0xFF) << ((dest % 4) * 8);
        if (++dest % 4 == 0) {
          rtcUserMemoryWrite((dest - 1) / 4, &d, 4);
          rtcUserMemoryRead(dest / 4, &d, 4);
        }
        if (++src % 4 == 0) {
          rtcUserMemoryRead(src / 4, &s, 4);
        }
      }
      if (dest % 4) {
        rtcUserMemoryWrite(dest / 4, &d, 4);
      }
    } else {
      src += length;
      dest += length;
      rtcUserMemoryRead(--src / 4, &s, 4);
      rtcUserMemoryRead(--dest / 4, &d, 4);
      while (length--) {
        d &= ~(0xFF << ((dest % 4) * 8));
        d |= ((s >> ((src % 4) * 8)) & 0xFF) << ((dest % 4) * 8);
        if (--dest % 4 == 3) {
          rtcUserMemoryWrite((dest + 1) / 4, &d, 4);
          rtcUserMemoryRead(dest / 4, &d, 4);
        }
        if (--src % 4 == 3) {
          rtcUserMemoryRead(src / 4, &s, 4);
        }
      }
      if (dest % 4 < 3) {
        rtcUserMemoryWrite(dest / 4, &d, 4);
      }
    }
#if 0
    if (dest < src) {
      while (length--) {
        set(dest++, bytes(src++));
      }
    } else {
      dest += length;
      src += length;
      while (length--) {
        set(--dest, bytes(--src));
      }
    }
#endif
    return true;
  }
  return false;
}

#ifndef NO_GLOBAL_INSTANCES
RtcMem RTCMEM;
#endif
