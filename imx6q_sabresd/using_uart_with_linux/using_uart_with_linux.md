
# 在 Linux 中使用 UART 设备

本文基于 imx6q-sabresd 开发板介绍如何在 Linux 中启用 UART 设备。

## UART (Universal Asynchronous Receiver/Transmitter)

NXP i.MX6Q CPU 一共有5个 UART 外设，imx6q-sabresd 开发板中这5个 UART 均被占用（作为 UART 功能使用，或者 I/O 复用为其他功能）。本例将把 SD3 功能取消，将其 I/O 管脚作为 UART2 的功能管脚。

### 管脚定义

* 将 193 管脚的 SD3 DAT4 功能复用为 UART2 的 Rx 功能
* 将 192 管脚的 SD3 DAT5 功能复用为 UART2 的 Tx 功能

### 设备树配置示例

#### 总线定义

dtsi 文件 imx6q.dtsi 中：

```
aliases {

    // ... 省略

    serial1 = &uart2;

    // ... 省略
}

// ... 省略

soc {

    // ... 省略

	aips-bus@2100000 { /* AIPS2 */
		uart2: serial@21e8000 {
			compatible = "fsl,imx6q-uart", "fsl,imx21-uart";
			reg = <0x021e8000 0x4000>;
			interrupts = <0 27 IRQ_TYPE_LEVEL_HIGH>;
			clocks = <&clks IMX6QDL_CLK_UART_IPG>,
				 <&clks IMX6QDL_CLK_UART_SERIAL>;
			clock-names = "ipg", "per";
			dmas = <&sdma 27 4 0>, <&sdma 28 4 0>;
			dma-names = "rx", "tx";
			status = "disabled";
		};

        // ... 省略
	};

    // ... 省略
}

```

#### 关闭 SD3 功能

dtsi 文件 imx6qdl-sabresd.dtsi 中，将与 SD3 功能相关的定义注释掉：

```
//pinctrl_usdhc3: usdhc3grp {
//	fsl,pins = <
//		MX6QDL_PAD_SD3_CMD__SD3_CMD		0x17059
//		MX6QDL_PAD_SD3_CLK__SD3_CLK		0x10059
//		MX6QDL_PAD_SD3_DAT0__SD3_DATA0		0x17059
//		MX6QDL_PAD_SD3_DAT1__SD3_DATA1		0x17059
//		MX6QDL_PAD_SD3_DAT2__SD3_DATA2		0x17059
//		MX6QDL_PAD_SD3_DAT3__SD3_DATA3		0x17059
//		MX6QDL_PAD_SD3_DAT4__SD3_DATA4		0x17059
//		MX6QDL_PAD_SD3_DAT5__SD3_DATA5		0x17059
//		MX6QDL_PAD_SD3_DAT6__SD3_DATA6		0x17059
//		MX6QDL_PAD_SD3_DAT7__SD3_DATA7		0x17059
//	>;
//};

//&usdhc3 {
//	pinctrl-names = "default";
//	pinctrl-0 = <&pinctrl_usdhc3>;
//	bus-width = <8>;
//	cd-gpios = <&gpio2 0 GPIO_ACTIVE_LOW>;
//	wp-gpios = <&gpio2 1 GPIO_ACTIVE_HIGH>;
//	status = "okay";
//};
```

#### IOMUX 配置

dts 文件 imx6q-sabresd.dts 中，添加：

```
&iomuxc {
	imx6qdl-sabresd {
		pinctrl_uart2: uart2grp {
			fsl,pins = <
				MX6QDL_PAD_SD3_DAT4__UART2_RX_DATA	0x1b0b1
				MX6QDL_PAD_SD3_DAT5__UART2_TX_DATA	0x1b0b1
			>;
		};
	};
};
```

#### UART2 使能

dts 文件 imx6q-sabresd.dts 中，添加：

```
&uart2 {
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_uart2>;
	status = "okay";
};
```

### 在用户空间使用 UART2 设备

用户空间可以通过 `/dev` 设备文件接口访问 UART 设备，通过设备树申明，Linux 此时会创建一个形式为 `/dev/ttymxcX` 设备节点，其中 X 表示 UART 设备索引（从零开始计数，如本例中的 uart2 对应 X 为1）。本例中访问 UART2 可通过设备节点 `/dev/ttymxc1` 实现。

#### UART 设备使用

参考[此项目](https://github.com/ClarenceYk/wifi232b2_cli)
