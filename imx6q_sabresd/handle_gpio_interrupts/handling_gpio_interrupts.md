
# 在应用层处理 GPIO 中断

## Device tree 修改

- GPIO 管脚: 192, 193
- 管脚功能在头文件 `imx6dl-pinfunc.h` 中的定义：
    - 192: MX6QDL_PAD_SD3_DAT5__GPIO7_IO00
    - 193: MX6QDL_PAD_SD3_DAT4__GPIO7_IO01

在设备树文件 `imx6qdl-sabresd.dtsi` 中将与192和193管脚相关的节点定义注释掉：
```dts
//&usdhc3 {
//  pinctrl-names = "default";
//  pinctrl-0 = <&pinctrl_usdhc3>;
//  bus-width = <8>;
//  cd-gpios = <&gpio2 0 GPIO_ACTIVE_LOW>;
//  wp-gpios = <&gpio2 1 GPIO_ACTIVE_HIGH>;
//  status = "okay";
//};

*** 省略若干行 ***

//      pinctrl_usdhc3: usdhc3grp {
//          fsl,pins = <
//              MX6QDL_PAD_SD3_CMD__SD3_CMD     0x17059
//              MX6QDL_PAD_SD3_CLK__SD3_CLK     0x10059
//              MX6QDL_PAD_SD3_DAT0__SD3_DATA0      0x17059
//              MX6QDL_PAD_SD3_DAT1__SD3_DATA1      0x17059
//              MX6QDL_PAD_SD3_DAT2__SD3_DATA2      0x17059
//              MX6QDL_PAD_SD3_DAT3__SD3_DATA3      0x17059
//              MX6QDL_PAD_SD3_DAT4__SD3_DATA4      0x17059
//              MX6QDL_PAD_SD3_DAT5__SD3_DATA5      0x17059
//              MX6QDL_PAD_SD3_DAT6__SD3_DATA6      0x17059
//              MX6QDL_PAD_SD3_DAT7__SD3_DATA7      0x17059
//          >;
//      };
```

将管脚192和193定义为 GPIO 功能，在节点 `pinctrl_hog` 中添加2行：
```dts
fsl,pins = <
    MX6QDL_PAD_NANDF_D0__GPIO2_IO00 0x1b0b0
    MX6QDL_PAD_NANDF_D1__GPIO2_IO01 0x1b0b0
    MX6QDL_PAD_NANDF_D2__GPIO2_IO02 0x1b0b0
    MX6QDL_PAD_NANDF_D3__GPIO2_IO03 0x1b0b0
    MX6QDL_PAD_GPIO_0__CCM_CLKO1    0x130b0
    MX6QDL_PAD_NANDF_CLE__GPIO6_IO07 0x1b0b0
    MX6QDL_PAD_ENET_TXD1__GPIO1_IO29 0x1b0b0
    MX6QDL_PAD_EIM_D22__GPIO3_IO22  0x1b0b0
    MX6QDL_PAD_ENET_CRS_DV__GPIO1_IO25 0x1b0b0
    MX6QDL_PAD_SD3_DAT4__GPIO7_IO01 0x1b0b0 // 添加的第1行
    MX6QDL_PAD_SD3_DAT5__GPIO7_IO00 0x1b0b0 // 添加的第2行
>;
```

修改之后重新生成 dtb 并且替换系统中原有 dtb，重启系统。

## 添加 GPIO 管脚控制设备

```shell
$ cd /sys/class/gpio
$ echo 192 > export
$ echo 193 > export
```

设置管脚193为输出
```shell
$ cd /sys/class/gpio/gpio193
$ echo out > direction
```

设置管脚192为输入、开启中断（上升沿触发）
```shell
$ cd /sys/class/gpio/gpio192
$ echo in > direction
$ echo rising > edge
```

## 应用层获取192管脚 GPIO 中断请求

用 open 打开 `/sys/class/gpio/gpio192/value` 文件：

https://github.com/ClarenceYk/Clarence-Learning-Notes/blob/f0f5c6732c359e6f37a17632118b790bf608eeb1/imx6q_sabresd/handle_gpio_interrupts/handle_gpio_interrupts.c#L23

