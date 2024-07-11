# lt6911uxe_update

## 编译方式

* 直接编译 make
* 交叉编译 配置环境变量CROSS_COMPILE，例如 CROSS_COMPILE=aarch64-linux-gnu- make
* 编译debug模式 配置环境变量DEBUG_MODE=1
* 编译仅I2C ID验证最小CASE 配置环境变量ONLY_LT6911_I2C_TEST=1

> 注：编译出的程序是静态链接模式
