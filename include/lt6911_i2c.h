#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* apis, Must check fun ret, Not thread safe, dep i2c-tools libi2c-dev */
#define LT6911_OK 0
#define LT6911_ERROR 1

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
