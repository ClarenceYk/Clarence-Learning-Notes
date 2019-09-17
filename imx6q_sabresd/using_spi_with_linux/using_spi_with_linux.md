
# 在 Linux 中使用 SPI 设备

本文基于 imx6q-sabresd 开发板介绍如何在 Linux 中启用 SPI 设备。

## SPI (串行外设接口)

NXP i.MX6Q CPU 一共有5个 SPI 总线，imx6q-sabresd 开发板中这5个 SPI 接口均被占用（作为 SPI 功能使用，或者 I/O 复用为其他功能）。本例将把 SD2 功能取消，将其 I/O 管脚作为 SPI5 的功能管脚。

### 管脚定义

* 将 11 管脚的 SD2 CMD 功能复用为 SPI5 的 MOSI 功能
* 将 15 管脚的 SD2 DAT0 功能复用为 SPI5 的 MISO 功能
* 将 10 管脚的 SD2 CLK 功能复用为 SPI5 的 SCLK 功能
* 将 14 管脚的 SD2 DAT1 功能复用为 GPIO 功能
* 将 13 管脚的 SD2 DAT2 功能复用为 GPIO 功能

### Kernel 配置

在内核配置中开启 SPI 驱动支持：

```
Device Drivers ---> [*] SPI support
Device Drivers ---> SPI support ---> [*] Utilities for Bitbanging SPI masters
Device Drivers ---> SPI support ---> <*> User mode SPI device driver support
```

### 设备树配置示例

#### 总线定义

dtsi 文件 imx6q.dtsi 中：

```
aliases {

    // ... 省略

    spi4 = &ecspi5;

    // ... 省略
}

// ... 省略

soc {

    // ... 省略

	aips-bus@2000000 { /* AIPS1 */
		spba-bus@2000000 {
			ecspi5: ecspi@2018000 {
				#address-cells = <1>;
				#size-cells = <0>;
				compatible = "fsl,imx6q-ecspi", "fsl,imx51-ecspi";
				reg = <0x02018000 0x4000>;
				interrupts = <0 35 IRQ_TYPE_LEVEL_HIGH>;
				clocks = <&clks IMX6Q_CLK_ECSPI5>,
					 <&clks IMX6Q_CLK_ECSPI5>;
				clock-names = "ipg", "per";
				dmas = <&sdma 11 8 1>, <&sdma 12 8 2>;
				dma-names = "rx", "tx";
				status = "disabled";
			};
		};

        // ... 省略

	};

    // ... 省略

}

```

#### IOMUX 配置

dtsi 文件 imx6qdl-sabresd.dtsi 中：

```
pinctrl_ecspi5: ecspi5grp {
	fsl,pins = <
		MX6QDL_PAD_SD2_DAT0__ECSPI5_MISO 0x100b1
		MX6QDL_PAD_SD2_CMD__ECSPI5_MOSI	0x100b1
		MX6QDL_PAD_SD2_CLK__ECSPI5_SCLK 0x100b1
		MX6QDL_PAD_SD2_DAT1__GPIO1_IO14	0x1b0b0
		MX6QDL_PAD_SD2_DAT2__GPIO1_IO13	0x1b0b0
	>;
};
```

#### SPI 总线使能

dtsi 文件 imx6qdl-sabresd.dtsi 中：

```
&ecspi5 {
	fsl,spi-num-chipselects = <2>;
	cs-gpios = <&gpio1 14 0>,<&gpio1 13 0>;
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_ecspi5>;
	status = "okay";
};
```

### 在用户空间使用 SPI

在用户空间不能直接访问 SPI 总线。相反，可以通过 SPI 用户驱动来访问 SPI 设备。

#### SPI 设备接口

Linux 内核为 client 端提供了一个 `spidev` 驱动，使得用户空间可以通过 `/dev` 设备文件接口访问 SPI 总线来向 SPI 设备读取和写入数据。前面内核配置一节已经将此功能开启（User mode SPI device driver support）。

在设备树中添加一个 `spidev` 节点挂载到 SPI 总线下：

```
&ecspi5 {
	fsl,spi-num-chipselects = <2>;
	cs-gpios = <&gpio1 14 0>,<&gpio1 13 0>;
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_ecspi5>;
	status = "okay";
	spidev@0 {
		compatible = "spidev";
		reg = <0>;
		spi-max-frequency = <4000000>;
		status = "okay";
	};
};
```

Linux 此时会创建一个形式为 `/dev/spidevX.Y` 设备节点，其中 X 表示 SPI 总线索引从零开始计数（比如本例中的 ecspi5 对应 X 为4）,Y 对应 SPI 总线片选同样从零开始计数。本例中访问 SPI5 总线可通过设备节点 `/dev/spidev4.0` 实现。

#### SPI 设备测试

测试程序源码[链接](https://raw.githubusercontent.com/raspberrypi/linux/rpi-3.10.y/Documentation/spi/spidev_test.c)。

或者[此链接]()。

下载源码并且编译之后，运行：

```shell
./a.out --help
```

可看到测试程序的帮助信息。

##### 示例

将 SPI 外设的 MOSI 与 MISO 短接创建回环，使得 SPI 设备能自发自收便于测试，之后运行：

```shell
$ ./a.out -D /dev/spidev4.0
```

可看到终端输出：

```shell
spi mode: 0
bits per word: 8
max speed: 500000 Hz (500 KHz)
 
FF FF FF FF FF FF
40 00 00 00 00 95
FF FF FF FF FF FF
FF FF FF FF FF FF
FF FF FF FF FF FF
DE AD BE EF BA AD
F0 0D
```

#### 测试程序源码分析

头文件包含：

```c
#include <linux/spi/spidev.h>
```

打开 SPI 设备，配置 SPI 模式，配置 SPI 单字比特数，配置 SPI 传输时钟频率（Hz）：

```c
fd = open(device, O_RDWR);
ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
```

定义 SPI transfer：

```c
struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx, // 发送 buffer
	.rx_buf = (unsigned long)rx, // 接收 buffer
	.len = ARRAY_SIZE(tx),
	.delay_usecs = delay,
	.speed_hz = speed,
	.bits_per_word = bits,
};
```

发送数据：

```c
ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
```

收到的数据保存在接收 buffer 中：

```c
for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
	if (!(ret % 6))
		puts("");
	printf("%.2X ", rx[ret]);
}
```
