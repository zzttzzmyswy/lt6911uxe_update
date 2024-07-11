# lt6911uxe_update

## 编译方式

* 直接编译 make
* 交叉编译 配置环境变量CROSS_COMPILE，例如 CROSS_COMPILE=aarch64-linux-gnu- make
* 编译debug模式 配置环境变量DEBUG_MODE=1
* 编译仅I2C ID验证最小CASE 配置环境变量ONLY_LT6911_I2C_TEST=1

> 注：编译出的程序是静态链接模式

### hex2bin程序编译方式

* cd tools/Hex2bin-2.5
* 直接编译 make
* 交叉编译 配置环境变量CROSS_COMPILE，例如 CROSS_COMPILE=aarch64-linux-gnu- make
* 在同目录下生成hex2bin文件


## 使用方法

* 完成编译后，将编译结果 `lt6911uxe_update` 二进制文件scp到开发板上
* 参数说明：

```bash
Usage: ./lt6911uxe_update <i2c dev file> <i2c addr, hex> <update or dump> <firmware bin filename or dump filename>

<i2c dev file> : i2c设备名
<i2c addr> : i2c 地址，16进制
<update or dump> : 操作标识，更新固件或储存固件
<firmware bin filename or dump filename> : 文件名。若操作为update，需要输入固件文件名；若操作为dump，输入将保存的文件名
```