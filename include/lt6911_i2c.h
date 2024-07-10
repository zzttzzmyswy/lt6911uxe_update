#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"

#define LT6911_WRITE_AS(__OFFSET_ADDR, __DATA, __ERROR_FUN)              \
  {                                                                      \
    if (LT6911_OK != lt6911_write_command_byte(__OFFSET_ADDR, __DATA)) { \
      __ERROR_FUN;                                                       \
      return LT6911_ERROR;                                               \
    }                                                                    \
  }
#define LT6911_READ_AS(__OFFSET_ADDR, __READ_NUM, __DATA, __ERROR_FUN)  \
  {                                                                     \
    if (LT6911_OK !=                                                    \
        lt6911_read_command_bytes(__OFFSET_ADDR, __READ_NUM, __DATA)) { \
      __ERROR_FUN;                                                      \
      return LT6911_ERROR;                                              \
    }                                                                   \
  }

/* apis, Must check fun ret, Not thread safe, dep i2c-tools libi2c-dev */
#define LT6911_OK 0
#define LT6911_ERROR 1

/*
 * lt6911 i2c file close
 */
void lt6911_i2c_close(void);

/*
 * lt6911 i2c file open
 */
unsigned char lt6911_i2c_open(void);

/*
 * init I2C infomation
 */
unsigned char lt6911_i2c_infomation_init(char* i2c_dev, unsigned char i2c_addr);

/*
 * check LT6911 CHIP ID
 */
unsigned char lt6911_id_check(void);

/*
 * write a byte by w command
 */
unsigned char lt6911_write_command_byte(unsigned char offset_addr,
                                        unsigned char data);

/*
 * write some bytes by r command
 */
unsigned char lt6911_read_command_bytes(unsigned char offset_addr,
                                        unsigned char read_num,
                                        unsigned char* data);
