#include "lt6911_i2c.h"

unsigned char lt6911_read_firmware_from_file(unsigned char* data,
                                             unsigned int* length,
                                             unsigned char* filename) {
  if (data == NULL || filename == NULL) {
    log_error("empty buffer or filename");
    return LT6911_ERROR;
  }

  FILE* fp = fopen((const char*)filename, "rb");
  if (fp == NULL) {
    log_error("open file failed, [%s]", filename);
    return LT6911_ERROR;
  }

  size_t bytesRead = fread(data, sizeof(unsigned char), *length, fp);
  if (bytesRead < length) {
    log_error("read file failed, [%s]", filename);
    fclose(fp);
    return LT6911_ERROR;
  }

  if (length > MAX_FILE_LENGTH) {
    log_error(
        "buffer too small, please check firmware file [%s] and MAX_FILE_LENGTH",
        filename);
    fclose(fp);
    return LT6911_ERROR;
  }

  fclose(fp);
  return LT6911_OK;
}

unsigned char lt6911_write_firmware_to_flash(unsigned char* data,
                                             unsigned int length) {
  unsigned int offset = 0;
  unsigned int step = 32;
  unsigned int address = 0x000000;

  // Configure Parameters
  LT6911_WRITE_AS(0xff, 0xe0, lt6911_i2c_close());
  LT6911_WRITE_AS(0xee, 0x01, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5e, 0xc1, lt6911_i2c_close());
  LT6911_WRITE_AS(0x58, 0x00, lt6911_i2c_close());
  LT6911_WRITE_AS(0x59, 0x50, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x10, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());
  LT6911_WRITE_AS(0x58, 0x21, lt6911_i2c_close());

  // Block Erase
  LT6911_WRITE_AS(0x5a, 0x04, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5b, 0x00, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5c, 0x00, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5d, 0x00, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x01, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());

  usleep(500);

  while (1) {
    // WREN
    LT6911_WRITE_AS(0xff, 0xe1, lt6911_i2c_close());
    LT6911_WRITE_AS(0x03, 0x3f, lt6911_i2c_close());
    LT6911_WRITE_AS(0x03, 0xff, lt6911_i2c_close());
    LT6911_WRITE_AS(0xff, 0xe0, lt6911_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x04, lt6911_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());

    // I2C to FIFO
    LT6911_WRITE_AS(0x5e, 0xdf, lt6911_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x20, lt6911_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());
    LT6911_WRITE_AS(0x58, 0x21, lt6911_i2c_close());
    for (int i = 0; i < step; ++i) {
      LT6911_WRITE_AS(0x59, data[offset + i], lt6911_i2c_close());
    }

    // FIFO to Flash
    LT6911_WRITE_AS(0x5b, ((address & 0x110000) >> 4), lt6911_i2c_close());
    LT6911_WRITE_AS(0x5c, ((address & 0x001100) >> 2), lt6911_i2c_close());
    LT6911_WRITE_AS(0x5d, (address & 0x000011), lt6911_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x10, lt6911_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());

    if (address + step < length) {
      // update offset and address
      offset += step;
      address += step;
      continue;
    }
    break;
  }

  // WRDI
  LT6911_WRITE_AS(0x5a, 0x08, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());

  return LT6911_OK;
}

unsigned char lt6911_read_firmware_from_flash(unsigned char* data,
                                              unsigned int length) {
  unsigned int offset = 0;
  unsigned int step = 32;
  unsigned int address = 0x000000;

  // Configure Parameters
  LT6911_WRITE_AS(0xff, 0xe0, lt6911_i2c_close());
  LT6911_WRITE_AS(0xee, 0x01, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5e, 0xc1, lt6911_i2c_close());
  LT6911_WRITE_AS(0x58, 0x00, lt6911_i2c_close());
  LT6911_WRITE_AS(0x59, 0x50, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x10, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());
  LT6911_WRITE_AS(0x58, 0x21, lt6911_i2c_close());

  while (1) {
    // Flash to FIFO
    LT6911_WRITE_AS(0x5e, 0x5f, lt6911_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x20, lt6911_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());

    LT6911_WRITE_AS(0x5b, ((address & 0x110000) >> 4), lt6911_i2c_close());
    LT6911_WRITE_AS(0x5c, ((address & 0x001100) >> 2), lt6911_i2c_close());
    LT6911_WRITE_AS(0x5d, (address & 0x000011), lt6911_i2c_close());

    LT6911_WRITE_AS(0x5a, 0x10, lt6911_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());

    //  FIFO to I2C
    LT6911_WRITE_AS(0x58, 0x21, lt6911_i2c_close());
    LT6911_READ_AS(0x5f, 0x20, &data[offset], lt6911_i2c_close());

    if (address + step < length) {
      address += step;
      offset += step;
      continue;
    }
    break;
  }
  // WRDI
  LT6911_WRITE_AS(0x5a, 0x08, lt6911_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911_i2c_close());

  return LT6911_OK;
}

unsigned char lt6911_compare_firmware(unsigned char* firmware,
                                      unsigned char* data,
                                      unsigned int length) {
  if (firmware == NULL || data == NULL) {
    log_error("input to be compared is empty");
    return LT6911_ERROR;
  }

  for (unsigned int i = 0; i < length; ++i) {
    if (firmware[i] != data[i]) {
      log_error("compare failed");
      return LT6911_ERROR;
    }
  }
  return LT6911_OK;
}

unsigned char lt6911_write_firmware_to_file(unsigned char* data,
                                            unsigned int length,
                                            unsigned char* filename) {
  if (data == NULL || filename == NULL) {
    log_error("empty buffer or filename, [%s]", filename);
    return LT6911_ERROR;
  }

  FILE* file = fopen((const char*)filename, "wb");
  if (file == NULL) {
    log_error("open file failed, [%s]", filename);
    return LT6911_ERROR;
  }

  size_t bytesWritten = fwrite(data, sizeof(unsigned char), length, file);
  if (bytesWritten < length) {
    fclose(file);
    log_error("write file failed, [%s]", filename);
    return LT6911_ERROR;
  }

  fclose(file);

  return LT6911_OK;
}

unsigned char lt6911_update_main_firmware(unsigned char* firmware_filename) {
  unsigned char errorCode = LT6911_OK;
  do {
    unsigned char data[MAX_FILE_LENGTH];
    unsigned int length = 0;
    errorCode =
        lt6911_read_firmware_from_file(data, &length, firmware_filename);
    if (errorCode != LT6911_OK) break;
    errorCode = lt6911_write_firmware_to_flash(data, length);
    if (errorCode != LT6911_OK) break;
    unsigned char read_data[MAX_FILE_LENGTH];
    errorCode = lt6911_read_firmware_from_flash(read_data, length);
    if (errorCode != LT6911_OK) break;
    errorCode = lt6911_compare_firmware(data, read_data, length);
    if (errorCode != LT6911_OK) break;
  } while (false);
  return errorCode;
}

unsigned char lt6911_dump_firmware(unsigned char* filename) {
  unsigned char errorCode = LT6911_OK;
  do {
    unsigned char data[MAX_FILE_LENGTH];
    unsigned int length = MAX_FILE_LENGTH;
    errorCode = lt6911_read_firmware_from_flash(data, length);
    if (errorCode != LT6911_OK) break;
    errorCode = lt6911_write_firmware_to_file(data, length, filename);
    if (errorCode != LT6911_OK) break;
  } while (false);
  return errorCode;
}