# 能量模块

为了能对整个通信系统中消耗的能量有更精细化的计算，尤其是对于能耗要求相对较高的体域网，能耗是个重要的指标。该仿真平台参考了 IEEE 802.15.4 开源的能量模块。该能量模块负责统计各节点在发送、接收、执行 CCA、空闲和睡眠阶段的能量消耗的情况。

对于通常的传感器节点来说，能耗主要集中在以下四个部分：

1. 发送模式
2. 接收模式
3. 空闲模式
4. 睡眠模式

能耗的具体计算公式则可以表示为 $$Energy ~Consumption(Joule) = \sum _{i} current_i \cdot time_i \cdot voltage$$.

四种模式下的电流值和电压值可以通过预设值给出，在节点模型的属性处可以设置。对于时间的计算则可以分成三类：

1. 发送/接收模式：time_1,2 = 包大小/数据传输速率
2. 空闲模式：time_3 = 当前仿真时刻 − 最近的上一次空闲时刻
3. 睡眠模式：time_4 = 当前仿真时刻 − 最近的上一次睡眠时刻

其中发送和接收模式下能量消耗最为明显， 空闲模式和睡眠模式功耗均很小。由于节点执行 CCA 过程时消耗的功率和接收功率相当，因此 CCA 过程的功耗可使用接收模式下的功耗来计算。

在 OPNET 仿真具体实现中，该平台设置了发送、接收、CCA 过程、活动阶段结束和睡眠阶段结束四种模式， 每当这五种事件中的任何一个发生时都会更新相应函数，从而计算出在每一种模式下的能耗，进而得到整体能耗。

### Battery(Energy) Structure

电池/能量模块的结构体信息如下所示：

首先是电池电压和四种模式下的电流信息，随后则是节点在不同模式下消耗能量的综合，最后是节点的初始化能量值/剩余能量/消耗能量。

```c
/* Battery */
typedef struct {
	/* Power Supply in Volt */
	double voltage;
	/* current draw in milli/micro Amper */
	double tx_mA;
	double rx_mA;
	double idle_uA;
	double sleep_uA;
	/* Energy consumed in TX, RX, CCA, IDLE, SlEEP, Joule */
	double engy_tx;
	double engy_rx;
	double engy_cca;
	double engy_idle;
	double engy_sleep;
	/* energy init, remainning, consumed. Joule */
	double engy_init;
	double engy_remainning;
	double engy_consumed;
} wban_battery_attributes;
```

### Energy module initialization

能量模块的初始化过程：

1. 获取当前电池的电压电流(四种不同模式)信息。
2. 初始化节点活动信息，如上一次睡眠/空闲的时刻。

```c
static void
wban_battery_init(Objid node_idL)
	{
	/** read Battery Attributes and set the activity var **/
	/* get the current draw in different mode */
	/* init the activity variable */
	/* Stack tracing exit point */
	}
```

### Transmission update

通过物理层统计线中断`TX_BUSY_STAT`反馈发送的起止时刻和目前的 MAC 层状态。在包被送到物理层时，相应统计线的值为1.0；包发送完毕时又会产生一次接收中断，但此时相应统计线的值为0。MAC 进程模型的 SV 状态变量中定义有`t_tx_start`和`t_tx_end`，便于计算 TX 的传输间隔，此传输间隔同时也作为`wban_battery_update_tx`的输入参数。

```c
case TX_BUSY_STAT :
	if (mac_state != MAC_SLEEP) {
		if (op_stat_local_read (TX_BUSY_STAT) == 1.0) {
			t_tx_start = op_sim_time();
		} else {
			t_tx_end = op_sim_time();
			if (t_tx_start > 0) {
				t_tx_interval = t_tx_end - t_tx_start;
				wban_battery_update_tx(t_tx_interval, mac_state);
			}
		}
	} else {
		t_tx_start = 0;
		t_tx_end = 0;
	}
```

```c
static void
wban_battery_update_tx (double tx_timeL, int mac_stateL)
	{
	/* compute the consumed energy when transmitting a packet */
	/* node can rx while tx at OPNET */
	/* compute the time spent by the node in idle state */
	/* update the consumed energy with the one of in idle state */
	/* update the current energy level */
	}
```

比较难懂的可能是在 TX 中计算了 RX 能量，而且 RX 的能量是作为负项加入到最终能量的。原因在于 OPNET 中节点在发送包的同时也能收到自己的包，而在物理层中是无法区分这一情况的，故采取的方法是在 TX 中减掉在 RX 中增加的能量，以达到最终的平衡。

### Receiption update

```c
case RX_BUSY_STAT :	/* Case of the end of the BUSY RECEIVER STATISTIC */
	if((mac_state != MAC_SLEEP) && ((waitForACK)||(IAM_BAN_HUB))){
		if (op_stat_local_read(RX_BUSY_STAT) == 1.0) {
			t_rx_start = op_sim_time();
			csma.CCA_CHANNEL_IDLE = OPC_FALSE;
		}else{
			t_rx_end = op_sim_time();
			if(t_rx_start > 0){
				t_rx_interval = t_rx_end - t_rx_start;
				wban_battery_update_rx(t_rx_interval, mac_state);
			}
		}
	}else{
		t_rx_start = 0;
		t_rx_end = 0;
	}
```

#### CCA

CSMA 中的 CCA 也会消耗可观的能量，这里近似以 RX 的功率计算，`wban_battery_cca(mac_state)`在中断`CCA_START_CODE`处开始执行。

### Sleep update

睡眠分拆为两个函数实现，睡眠开始和睡眠结束。睡眠开始在中断`END_OF_RAP1_PERIOD_CODE`和`END_OF_MAP1_PERIOD_CODE`之后；睡眠结束在中断`BEACON_INTERVAL_CODE`(超帧的结束，同时也是下一超帧的开始)和`START_OF_MAP1_PERIOD_CODE`之后，
