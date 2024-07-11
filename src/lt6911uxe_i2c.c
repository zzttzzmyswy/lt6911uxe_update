#include "lt6911uxe_i2c.h"

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

/* init this infomation before use there api */
static char lt6911uxe_i2c_dev[50];
static unsigned char lt6911uxe_i2c_addr;
static int lt6911uxe_i2c_dev_file;
static int lt6911uxe_i2c_dev_debug_flag;
static int lt6911uxe_i2c_fun_cmd_enable = 0;

void lt6911uxe_i2c_close(void) { close(lt6911uxe_i2c_dev_file); }

unsigned char lt6911uxe_i2c_open(void) {
  log_debug("lt6911uxe i2c open start");
  if ((lt6911uxe_i2c_dev_file = open(lt6911uxe_i2c_dev, O_RDWR)) < 0) {
    log_error("Failed to open the i2c bus");
    return LT6911_ERROR;
  }
  return LT6911_OK;
}

unsigned char lt6911uxe_i2c_infomation_init(char* i2c_dev,
                                         unsigned char i2c_addr) {
  log_debug("start i2c info init, [%s:0x%X]", i2c_dev, i2c_addr);
  if (strlen(i2c_dev) > 50) {
    log_error("i2c dev name to long");
    return LT6911_ERROR;
  }
  memset(lt6911uxe_i2c_dev, 0, sizeof(lt6911uxe_i2c_dev));
  strcpy(lt6911uxe_i2c_dev, i2c_dev);
  lt6911uxe_i2c_addr = i2c_addr;
  if (LT6911_ERROR == lt6911uxe_i2c_open()) return LT6911_ERROR;
  lt6911uxe_i2c_close();
  log_info("lt6911uxe i2c info: [%s:0x%X]", lt6911uxe_i2c_dev, lt6911uxe_i2c_addr);
  return lt6911uxe_id_check();
}

unsigned char lt6911uxe_id_check(void) {
  unsigned char id[2] = {0x00, 0x00};
  log_debug("start lt6911uxe id check");
  lt6911uxe_i2c_open();
  LT6911_WRITE_AS(0xff, 0xe0, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0xee, 0x01, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0xff, 0xe1, lt6911uxe_i2c_close());
  LT6911_READ_AS(0x00, 0x02, id, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0xff, 0xe0, lt6911uxe_i2c_close());
  LT6911_WRITE_AS(0xee, 0x00, lt6911uxe_i2c_close());
  if ((id[0] != 0x21) || (id[1] != 0x02)) {
    log_error("id check error, read id is [0x%X:0x%X]", id[0], id[1]);
    lt6911uxe_i2c_close();
    return LT6911_ERROR;
  }
  lt6911uxe_i2c_close();
  log_info("lt6911uxe id check ok, chip id: 0x%X", id[0] << 8 | id[1]);
  return LT6911_OK;
}

unsigned char lt6911uxe_write_byte(unsigned char offset_addr,
                                        unsigned char data) {
  log_debug("start lt6911uxe write command, [0x%X:0x%X]", offset_addr, data);
  struct i2c_rdwr_ioctl_data ioctl_data;
  struct i2c_msg msg;
  unsigned char buf[3];
  int ret = 0;

  buf[0] = offset_addr;
  buf[1] = data;
  buf[2] = 0x00;

  msg.addr = lt6911uxe_i2c_addr;
  msg.flags = 0;
  if (lt6911uxe_i2c_fun_cmd_enable == 1)
    msg.len = 3;
  else
    msg.len = 2;
  msg.buf = (unsigned char*)buf;

  ioctl_data.msgs = &msg;
  ioctl_data.nmsgs = 1;

  ret = ioctl(lt6911uxe_i2c_dev_file, I2C_RDWR, &ioctl_data);
  if (ret < 0) {
    log_error("i2c ioctl error, ret: %d", ret);
    return LT6911_ERROR;
  }
  if (lt6911uxe_i2c_dev_debug_flag == 1) {
    log_debug("i2c write [0x%X] 0x%X", offset_addr, data);
  }
  return LT6911_OK;
}

unsigned char lt6911uxe_read_bytes(unsigned char offset_addr,
                                        unsigned char read_num,
                                        unsigned char* data) {
  log_debug("start lt6911uxe read command, [0x%X:0x%X:0x%X]", offset_addr,
            read_num, data);
  if (read_num > 50) {
    log_error("read num too long: %d", read_num);
  }
  memset(data, 0x00, read_num);
  struct i2c_rdwr_ioctl_data ioctl_data;
  struct i2c_msg msgs[2];
  unsigned char buf[3];
  int ret = 0;

  buf[0] = offset_addr;
  buf[1] = read_num;
  buf[2] = 0xFF;

  /* write read info */
  msgs[0].addr = lt6911uxe_i2c_addr;
  msgs[0].flags = 0;
  if (lt6911uxe_i2c_fun_cmd_enable == 1)
    msgs[0].len = 3;
  else
    msgs[0].len = 1;
  msgs[0].buf = (unsigned char*)buf;

  /* read info */
  msgs[1].addr = lt6911uxe_i2c_addr;
  msgs[1].flags = I2C_M_RD;
  msgs[1].len = read_num;
  msgs[1].buf = (unsigned char*)data;

  ioctl_data.msgs = msgs;
  ioctl_data.nmsgs = 2;

  ret = ioctl(lt6911uxe_i2c_dev_file, I2C_RDWR, &ioctl_data);
  if (ret < 0) {
    log_error("i2c ioctl error, ret: %d", ret);
    return LT6911_ERROR;
  }
  if (lt6911uxe_i2c_dev_debug_flag == 1) {
    log_debug("i2c read info:");
    for (int i = 0; i < read_num; i++) {
      log_debug("[0x%X] 0x%X", offset_addr + i, data[i]);
    }
  }
  return LT6911_OK;
}

#if ONLY_LT6911_I2C_TEST
int main(int argc, char* argv[]) {
  unsigned char i2c_addr_input = 0x00;
  char* debug_env = getenv("LT6911_UPDATE_DEBUG");
  log_info("lt6911_i2c_id_test, version: %s %s", GIT_COMMIT_HASH,
           GIT_COMMIT_DATE);
  if (debug_env != NULL) {
    log_info("LT6911_UPDATE_DEBUG value: %s", debug_env);
    log_set_level(LOG_TRACE);
    lt6911uxe_i2c_dev_debug_flag = 1;
  } else {
    log_info("LT6911_UPDATE_DEBUG environment variable not set.");
    log_set_level(LOG_INFO);
    lt6911uxe_i2c_dev_debug_flag = 0;
  }
  if (argc != 3) {
    log_error("Usage: %s <i2c dev file> <i2c addr, hex>", argv[0]);
    return -1;
  }
  i2c_addr_input = strtol(argv[2], NULL, 16);
  if (i2c_addr_input >= 0x80) {
    log_error("Usage: %s <i2c dev file> <i2c addr, hex>", argv[0]);
    log_error("Need: i2c addr(0x%X) < 0x80", i2c_addr_input);
    return -1;
  }
  if (LT6911_ERROR == lt6911uxe_i2c_infomation_init(argv[1], i2c_addr_input)) {
    log_error("lt6911uxe_i2c_infomation_init error");
    return -1;
  }
  return 0;
}
#endif
