#include "lt6911uxe_i2c.h"

static unsigned char calculate_crc8(unsigned char* data, size_t length) {
  unsigned char crc = 0x00;   // Initial CRC value
  unsigned char poly = 0x07;  // CRC-8 polynomial

  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];  // XOR-in the next input byte

    for (unsigned char j = 0; j < 8; j++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ poly;
      } else {
        crc <<= 1;
      }
    }
  }
  log_info("crc ret 0x%X", crc);
  return crc;
}

unsigned char lt6911_read_firmware_from_file(unsigned char* data,
                                             unsigned char* filename) {
  FILE* fp;
  long file_size = 0;
  size_t bytesRead = 0;
  if (data == NULL || filename == NULL) {
    log_error("empty buffer or filename");
    return LT6911_ERROR;
  }

  fp = fopen((const char*)filename, "rb");
  if (fp == NULL) {
    log_error("open file failed, [%s]", filename);
    return LT6911_ERROR;
  }
  fseek(fp, 0L, SEEK_END);
  file_size = ftell(fp);
  fseek(fp, 0L, 0);
  if (file_size > MAX_FILE_LENGTH) {
    log_error("file %s too large", filename);
    return LT6911_ERROR;
  }

  bytesRead = fread(data, sizeof(unsigned char), file_size, fp);
  if (bytesRead != file_size) {
    log_error("read file failed, [%s]", filename);
    fclose(fp);
    return LT6911_ERROR;
  }

  fclose(fp);

  data[MAX_FILE_LENGTH - 1] = calculate_crc8(data, MAX_FILE_LENGTH - 1);
  return LT6911_OK;
}

unsigned char lt6911_write_firmware_to_flash(unsigned char* data,
                                             unsigned int length) {
  unsigned int offset = 0;
  unsigned int step = 32;
  unsigned int address = 0x000000;

  if(LT6911_ERROR == lt6911uxe_i2c_open()) {
    log_error("lt6911uxe_i2c_open error");
    return LT6911_ERROR;
  }
  // Configure Parameters
  LT6911_WRITE_AS(0xff, 0xe0, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0xee, 0x01, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5e, 0xc1, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x58, 0x00, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x59, 0x50, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x10, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x58, 0x21, lt6911uxe_i2c_close());

  // Block Erase
  LT6911_WRITE_AS(0x5a, 0x04, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5b, 0x00, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5c, 0x00, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5d, 0x00, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x01, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());

  usleep(500 * 1000);

  while (1) {
    log_info("write FIFO to flash 0x%X", address);
    // WREN
    LT6911_WRITE_AS(0xff, 0xe1, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x03, 0x3f, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x03, 0xff, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0xff, 0xe0, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x04, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());

    // I2C to FIFO
    LT6911_WRITE_AS(0x5e, 0xdf, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x20, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x58, 0x21, lt6911uxe_i2c_close());
    for (int i = 0; i < step; ++i) {
      LT6911_WRITE_AS(0x59, data[offset + i], lt6911uxe_i2c_close());
    }

    // FIFO to Flash
    LT6911_WRITE_AS(0x5b, ((address & 0xFF0000) >> 16), lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5c, ((address & 0x00FF00) >> 8), lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5d, (address & 0x0000FF), lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x10, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());
    usleep(10 * 1000);

    if (address + step < length) {
      // update offset and address
      offset += step;
      address += step;
      continue;
    }
    break;
  }

  // WRDI
  LT6911_WRITE_AS(0x5a, 0x08, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());
  lt6911uxe_i2c_close();
  log_info("lt6911_write_firmware_to_flash ok");

  return LT6911_OK;
}

unsigned char lt6911_read_firmware_from_flash(unsigned char* data,
                                              unsigned int length) {
  unsigned int offset = 0;
  unsigned int step = 32;
  unsigned int address = 0x000000;
  if(LT6911_ERROR == lt6911uxe_i2c_open()) {
    log_error("lt6911uxe_i2c_open error");
    return LT6911_ERROR;
  }
  // Configure Parameters
  LT6911_WRITE_AS(0xff, 0xe0, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0xee, 0x01, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5e, 0xc1, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x58, 0x00, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x59, 0x50, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x10, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x58, 0x21, lt6911uxe_i2c_close());

  while (1) {
    log_info("read flash 0x%X to FIFO", address);
    // Flash to FIFO
    LT6911_WRITE_AS(0x5e, 0x5f, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x20, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());

    LT6911_WRITE_AS(0x5b, ((address & 0xFF0000) >> 16), lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5c, ((address & 0x00FF00) >> 8), lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5d, (address & 0x0000FF), lt6911uxe_i2c_close());

    LT6911_WRITE_AS(0x5a, 0x10, lt6911uxe_i2c_close());
    LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());

    //  FIFO to I2C
    LT6911_WRITE_AS(0x58, 0x21, lt6911uxe_i2c_close());
    LT6911_READ_AS(0x5f, 0x20, &data[offset], lt6911uxe_i2c_close());

    if (address + step < length) {
      address += step;
      offset += step;
      continue;
    }
    break;
  }
  // WRDI
  LT6911_WRITE_AS(0x5a, 0x08, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0x5a, 0x00, lt6911uxe_i2c_close());
  lt6911uxe_i2c_close();
  log_info("lt6911_read_firmware_from_flash ok");

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
    log_debug("compare [0x%X] 0x%X <-> 0x%X", i, firmware[i], data[i]);
    if (firmware[i] != data[i]) {
      log_error("compare failed");
      return LT6911_ERROR;
    }
  }
  log_info("lt6911_compare_firmware ok");
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

unsigned char lt6911uxe_update_main_firmware(unsigned char* firmware_filename) {
  unsigned char errorCode = LT6911_OK;
  do {
    unsigned char data[MAX_FILE_LENGTH];
    unsigned char read_data[MAX_FILE_LENGTH];
    memset(data, 0xFF, MAX_FILE_LENGTH);
    memset(read_data, 0xFF, MAX_FILE_LENGTH);
    errorCode = lt6911_read_firmware_from_file(data, firmware_filename);
    if (errorCode != LT6911_OK) break;
    errorCode = lt6911_write_firmware_to_flash(data, MAX_FILE_LENGTH);
    if (errorCode != LT6911_OK) break;
    errorCode = lt6911_read_firmware_from_flash(read_data, MAX_FILE_LENGTH);
    if (errorCode != LT6911_OK) break;
    errorCode = lt6911_compare_firmware(data, read_data, MAX_FILE_LENGTH);
    if (errorCode != LT6911_OK) break;
  } while (false);
  return errorCode;
}

unsigned char lt6911uxe_dump_firmware(unsigned char* filename) {
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

enum ctrlType { UNKNOWN = 0, WRITE, DUMP };

#if !ONLY_LT6911_I2C_TEST
int main(int argc, char* argv[]) {
  unsigned char i2c_addr_input = 0x00;
  char* debug_env = getenv("LT6911_UPDATE_DEBUG");
  log_info("lt6911_i2c_id_test, version: %s %s", GIT_COMMIT_HASH,
           GIT_COMMIT_DATE);
  if (debug_env != NULL) {
    log_info("LT6911_UPDATE_DEBUG value: %s", debug_env);
    log_set_level(LOG_TRACE);
  } else {
    log_info("LT6911_UPDATE_DEBUG environment variable not set.");
    log_set_level(LOG_INFO);
  }
  if (argc != 5) {
    log_error(
        "Usage: %s <i2c dev file> <i2c addr, hex> <update or dump> <firmware "
        "bin filename or dump filename>",
        argv[0]);
    return -1;
  }
  i2c_addr_input = strtol(argv[2], NULL, 16);
  if (i2c_addr_input >= 0x80) {
    log_error(
        "Usage: %s <i2c dev file> <i2c addr, hex> <update or dump> <firmware "
        "bin filename or dump filename>",
        argv[0]);
    log_error("Need: i2c addr(0x%X) < 0x80", i2c_addr_input);
    return -1;
  }
  if (LT6911_ERROR == lt6911uxe_i2c_infomation_init(argv[1], i2c_addr_input)) {
    log_error("lt6911uxe_i2c_infomation_init error");
    return -1;
  }

  enum ctrlType ctype = UNKNOWN;

  if (strncmp("update", argv[3], strlen("update")) == 0) {
    ctype = WRITE;
  } else if (strncmp("dump", argv[3], strlen("dump")) == 0) {
    ctype = DUMP;
  }

  if (ctype == WRITE) {
    unsigned char* filename = argv[4];
    unsigned char ret = lt6911uxe_update_main_firmware(filename);
    if (ret == LT6911_ERROR) {
      log_error("update main firmware failed");
      return -1;
    }
  } else if (ctype == DUMP) {
    unsigned char* filename = argv[4];
    unsigned char ret = lt6911uxe_dump_firmware(filename);
    if (ret == LT6911_ERROR) {
      log_error("dump main firmware failed");
      return -1;
    }
  } else {
    log_error("unknown parameters");
    return -1;
  }

  return 0;
}
#endif