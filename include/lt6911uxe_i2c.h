#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"

/* apis, Must check fun ret, Not thread safe, dep i2c-tools libi2c-dev */
#define LT6911_OK 0
#define LT6911_ERROR 1

#define LT6911_WRITE_AS(__OFFSET_ADDR, __DATA, __ERROR_FUN)              \
  {                                                                      \
    if (LT6911_OK != lt6911uxe_write_byte(__OFFSET_ADDR, __DATA)) { \
      __ERROR_FUN;                                                       \
      return LT6911_ERROR;                                               \
    }                                                                    \
  }
#define LT6911_READ_AS(__OFFSET_ADDR, __READ_NUM, __DATA, __ERROR_FUN)  \
  {                                                                     \
    if (LT6911_OK !=                                                    \
        lt6911uxe_read_bytes(__OFFSET_ADDR, __READ_NUM, __DATA)) { \
      __ERROR_FUN;                                                      \
      return LT6911_ERROR;                                              \
    }                                                                   \
  }

#define MAX_FILE_LENGTH (1024 * 32)

/*
 * lt6911uxe i2c file close
 */
void lt6911uxe_i2c_close(void);

/*
 * lt6911uxe i2c file open
 */
unsigned char lt6911uxe_i2c_open(void);

/*
 * init I2C infomation
 */
unsigned char lt6911uxe_i2c_infomation_init(char* i2c_dev, unsigned char i2c_addr);

/*
 * check lt6911uxe CHIP ID
 */
unsigned char lt6911uxe_id_check(void);

/*
 * write a byte by w command
 */
unsigned char lt6911uxe_write_byte(unsigned char offset_addr,
                                        unsigned char data);

/*
 * write some bytes by r command
 */
unsigned char lt6911uxe_read_bytes(unsigned char offset_addr,
                                        unsigned char read_num,
                                        unsigned char* data);

/**
 * @brief write firmware to embedded flash and compare
 *
 * @param firmware_filename firmware file' s name
 * @return unsigned char ErrorCode
 */
unsigned char lt6911uxe_update_main_firmware(unsigned char* firmware_filename);

/**
 * @brief dump firmware to file
 *
 * @param filename file to save firmware
 * @return unsigned char ErrorCode
 */
unsigned char lt6911uxe_dump_firmware(unsigned char* filename);