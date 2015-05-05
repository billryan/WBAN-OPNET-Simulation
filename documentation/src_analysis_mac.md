# MAC 层源码分析

下图为 MAC 层模块的有限状态机实现，看似简单，其实是由于协议逻辑比较复杂，所以将大部分状态跳转都放进了源码里头，并没有直接体现在有限状态机转化图上。

![FSM-MAC](./images/fsm_mac.png)

如上图所示，MAC 层模块主要由三部分状态机实现，绿色的`init`模块为初始化过程，`wait_beacon`为等待beacon帧的状态机，最后的`idle`为核心状态跳转，主要在程序内部使用自中断和中断处理函数组成。

## init 初始化

`init`模块的入口代码为：

```c
wban_mac_init ();

//op_intrpt_priority_set (OPC_INTRPT_SELF, START_OF_GTS_PERIOD, -4);
op_intrpt_priority_set (OPC_INTRPT_SELF, END_OF_EAP1_PERIOD_CODE, -1);
op_intrpt_priority_set (OPC_INTRPT_SELF, START_OF_RAP1_PERIOD_CODE, -2);
op_intrpt_priority_set (OPC_INTRPT_SELF, END_OF_RAP1_PERIOD_CODE, -1);
op_intrpt_priority_set (OPC_INTRPT_SELF, START_OF_MAP1_PERIOD_CODE, -2);
op_intrpt_priority_set (OPC_INTRPT_SELF, END_OF_MAP1_PERIOD_CODE, -1);
op_intrpt_priority_set (OPC_INTRPT_SELF, SEND_B2_FRAME, -2);
op_intrpt_priority_set (OPC_INTRPT_SELF, START_OF_MAP2_PERIOD_CODE, -2);
op_intrpt_priority_set (OPC_INTRPT_SELF, END_OF_MAP2_PERIOD_CODE, -1);
op_intrpt_priority_set (OPC_INTRPT_SELF, START_OF_CAP_PERIOD_CODE, -2);
op_intrpt_priority_set (OPC_INTRPT_SELF, END_OF_CAP_PERIOD_CODE, -1);
op_intrpt_priority_set (OPC_INTRPT_SELF, BEACON_INTERVAL_CODE, -2);
op_intrpt_priority_set (OPC_INTRPT_SELF, BACKOFF_EXPIRATION_CODE, 15); // the highest priority
```

`wban_mac_init()`为 MAC 层模块初始化函数，主要用于读取在节点和 MAC 模块处设定的参数，区分当前节点是 Hub 还是 Node. 如果是 Hub 则读取超帧相关参数，若为 Node 则读取 `Connection Request` 参数。最后设置统计量参数及协议状态参数，较为重要的参数有：

1. `waitForACK` - 是否在等待对方的 ACK?
2. `TX_ING` - 是否正处于包传输过程中？如果是的话就不能再向物理广播模块发送包。
3. `attemptingToTX` - 一切都已就绪，等待将包传输至物理层。可以理解为同步锁，某个包已获得发送至物理层的权利，其他包再要发就得等资源释放出来。

`op_intrpt_priority_set`用于设置中断的优先级，因为如果多个自中断在同一时刻同时产生时，中断的处理是有一定顺序区分的，故需要妥善设置不同中断的优先级。

## `wait_beacon`等待beacon

`wait_beacon`状态机入口代码如下，主要用于区分当前节点是否为Hub, 若为 Hub 则周期性广播 beacon 帧。若为 普通节点 Node 则接收其他节点发送的包，接收包的入口函数为 `wban_parse_incoming_frame`, 接收包之后即转向下一个状态机`idle`。

```
/************************************************************************
In this state a node check if it is a Hub:
- Node is a Hub : it sends beacon frame interval
- Node is a Node : it waits until a beacon frame from the network. 
*/
    
/* get the packet from the stream */
switch (op_intrpt_type()) {
	case OPC_INTRPT_STRM : 
	{
		wban_parse_incoming_frame ();
		op_intrpt_schedule_self (op_sim_time (), DEFAULT_CODE); // -> to IDLE state
		break;
	};
	default :
	{
	};
}
```

## `idle`模块

`idle`模块仅有出口执行代码，代码也就一行。

```
/* Call the interrupt processing routing for each interrupt */
wban_mac_interrupt_process();
```

用于捕捉所有中断，整个状态机跳转自此全部由代码接管，不显式出现于状态机中。显示使用状态机相对好懂一些，但暂时精力有限，也就不打算继续做大的改动了。

## 具体函数解析

### `wban_parse_incoming_frame` - 解析接收到的包

以下代码有删减，只是把一些核心代码和逻辑贴出来了。

```c
/*--------------------------------------------------------------------------------
 * Function:	wban_parse_incoming_frame
 *
 * Description:	parse the incoming packet and make the adequate processing
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_parse_incoming_frame() {
	/* get the packet from the input stream	*/
	Stream_ID = op_intrpt_strm();
	rcv_frame = op_pk_get (Stream_ID);
	
	frame_type_fd = 0;
	/* check from what input stream the packet is received and do the right processing*/
	switch (Stream_ID) {
		case STRM_FROM_RADIO_TO_MAC: /*A PHY FRAME (PPDU) FROM THE RADIO RECIEVER*/
			ppdu_bits = op_pk_total_size_get(rcv_frame);
			/* get MAC frame (MPDU=PSDU) from received PHY frame (PPDU)*/
			op_pk_nfd_get_pkt (rcv_frame, "PSDU", &frame_MPDU);
			ete_delay = op_sim_time() - op_pk_creation_time_get(frame_MPDU);
			if (MAC_SLEEP == mac_state){
				FOUT;
			}
			op_pk_nfd_get (frame_MPDU, "BAN ID", &ban_id);
			op_pk_nfd_get (frame_MPDU, "Recipient ID", &recipient_id);
			op_pk_nfd_get (frame_MPDU, "Sender ID", &sender_id);

			// filter the incoming BAN packet - not implemented entirely
			/*update the battery module*/
			if (!is_packet_for_me(frame_MPDU, ban_id, recipient_id, sender_id)) {
				FOUT;
			}

			/* repalce the mac_attr.receipient_id with Sender ID */
			mac_attr.recipient_id = sender_id;
			if(I_ACK_POLICY == ack_policy_fd){
				ack_seq_num = sequence_number_fd;
				op_intrpt_schedule_self(op_sim_time()+pSIFS, SEND_I_ACK);
				if(ack_seq_nid[sender_id%NODE_MAX] != ack_seq_num){
					ack_seq_nid[sender_id%NODE_MAX] = ack_seq_num;
				}else{
					// printf("\t  Duplicate packet received\n");
					op_pk_destroy (rcv_frame);
					op_pk_destroy (frame_MPDU);
					FOUT;
				}
			}

			switch (frame_type_fd) {
				case DATA: /* Handle data packets */
					// printf ("\t  Data Packet reception from sender_id=%d\n", sender_id);
					/* collect statistics */
					op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
					app_latency = op_sim_time() - op_pk_creation_time_get(frame_MSDU);
					latency_avg[frame_subtype_fd] = (latency_avg[frame_subtype_fd] * data_stat_local[frame_subtype_fd][RCV].number + ete_delay)/(data_stat_local[frame_subtype_fd][RCV].number + 1);
					data_stat_local[frame_subtype_fd][RCV].number += 1;
					data_stat_local[frame_subtype_fd][RCV].ppdu_kbits += 0.001*ppdu_bits;
					wban_extract_data_frame (frame_MPDU);
					/* send to higher layer for statistics */
					op_pk_send (frame_MPDU, STRM_FROM_MAC_TO_SINK);
					break;
				case MANAGEMENT: /* Handle management packets */
					// op_stat_write(stat_vec.data_pkt_rec, 0.0);
					// printf ("\t  Management Packet reception\n");
					switch (frame_subtype_fd) {
						case BEACON: 
							// printf ("\t    Beacon Packet reception\n");
							wban_extract_beacon_frame (frame_MPDU);
							break;
						case CONNECTION_REQUEST:
							// printf ("\t    Connection Request Packet reception\n");
							wban_extract_conn_req_frame (frame_MPDU);
							break;
						case CONNECTION_ASSIGNMENT:
							// printf ("\t    Connection Assignment Packet reception\n");
							wban_extract_conn_assign_frame (frame_MPDU);
							break;
					}
					break;
				case CONTROL: /* Handle control packets */
					// printf ("\t  Control Packet reception\n");
					switch (frame_subtype_fd) {
						case I_ACK:
							// printf ("\t    I-ACK Packet reception\n");
							wban_extract_i_ack_frame (frame_MPDU);
							break;
						case BEACON2: 
							// printf ("\t    BEACON2 Packet reception\n");
							wban_battery_sleep_start(mac_state);
							mac_state = MAC_SLEEP;
							wban_extract_beacon2_frame(frame_MPDU);
							break;
					}
					break;
				default:	/*OTHER FRAME TYPES*/
					// printf("\t  Other Packet reception\n");
					break;
			}
			break;
		case STRM_FROM_TRAFFIC_UP_TO_MAC: /* INCOMMING PACKETS(MSDU) FROM THE TRAFFIC SOURCE */
			wban_encapsulate_and_enqueue_data_frame (rcv_frame, I_ACK_POLICY, node_attr.traffic_dest_id);			
			break;
		default : break;
	}
	op_pk_destroy (rcv_frame);
	/* Stack tracing exit point */
	FOUT;
}
```

首先从流中断中获取流ID，并从流ID中获取接收到的包。
```c
Stream_ID = op_intrpt_strm();
rcv_frame = op_pk_get (Stream_ID);
```

从流ID中区分其是哪一种中断类型，对于 MAC 层模块来说，其接收到的包要么来自上层应用层，要么来自底层物理层，如果都不是，则删除此流中断中携带的包。
```
switch (Stream_ID)
	case STRM_FROM_RADIO_TO_MAC:
	case STRM_FROM_TRAFFIC_UP_TO_MAC:
	op_pk_destroy (rcv_frame);
```

解包来自物理层发送过来的包，PPDU ==> PSDU(对应于 MAC 层中的MPDU)，计算端到端时延，并提取出 BAN ID, Recipient ID, Sender ID用于函数`is_packet_for_me`入口参数。若不满足节点当前接收状态或者接收到的包不是发给自己的就丢弃之。若要自定义包过滤规则可以更改函数`is_packet_for_me`.
```c
ete_delay = op_sim_time() - op_pk_creation_time_get(frame_MPDU);
if (MAC_SLEEP == mac_state){
	FOUT;
}
op_pk_nfd_get (frame_MPDU, "BAN ID", &ban_id);
op_pk_nfd_get (frame_MPDU, "Recipient ID", &recipient_id);
op_pk_nfd_get (frame_MPDU, "Sender ID", &sender_id);

// filter the incoming BAN packet - not implemented entirely
/*update the battery module*/
if (!is_packet_for_me(frame_MPDU, ban_id, recipient_id, sender_id)) {
	FOUT;
}
```

检查包的 I_ACK_POLICY, 如果是要求返回 I-ACK，则应在 pSIFS 之后发送 I-ACK, 同时检查当前包是否为重发的包，若为已经接收过多包则丢弃并返回。
```c
/* repalce the mac_attr.receipient_id with Sender ID */
mac_attr.recipient_id = sender_id;
if(I_ACK_POLICY == ack_policy_fd){
	ack_seq_num = sequence_number_fd;
	op_intrpt_schedule_self(op_sim_time()+pSIFS, SEND_I_ACK);
	if(ack_seq_nid[sender_id%NODE_MAX] != ack_seq_num){
		ack_seq_nid[sender_id%NODE_MAX] = ack_seq_num;
	}else{
		// printf("\t  Duplicate packet received\n");
		op_pk_destroy (rcv_frame);
		op_pk_destroy (frame_MPDU);
		FOUT;
	}
}
```

处理完 I-ACK 的包之后即对 MAC 层包进行过滤分类处理，主要分为三大类—— DATA, MANAGEMENT, CONTROL. 

1. 对于数据包主要做一些统计工作，如时延等。
2. 对于管理帧则需要进一步区分到底是哪一类，主要分为 BEACON, CONNECTION_REQUEST 和 CONNECTION_ASSIGNMENT 三类。
    - 对于 BEACON 帧，则调用`wban_extract_beacon_frame`对其进行进一步解析。
    - 对于 CONNECTION_REQUEST 帧，则调用`wban_extract_conn_req_frame`对其进行进一步解析。
    - 对于 CONNECTION_ASSIGNMENT 帧，则调用`wban_extract_conn_assign_frame`对其进行进一步解析。
3. 对于控制帧主要分为两类，I-ACK 和 BEACON2.
    - 对于 I-ACK 则调用`wban_extract_i_ack_frame`.
    - 对于 BEACON2 则调用`wban_extract_beacon2_frame`.
```c
switch (frame_type_fd) {
	case DATA: /* Handle data packets */
		/* collect statistics */
		wban_extract_data_frame (frame_MPDU);
		/* send to higher layer for statistics */
		op_pk_send (frame_MPDU, STRM_FROM_MAC_TO_SINK);
	case MANAGEMENT: /* Handle management packets */
		switch (frame_subtype_fd) {
			case BEACON: 
				wban_extract_beacon_frame (frame_MPDU);
			case CONNECTION_REQUEST:
				wban_extract_conn_req_frame (frame_MPDU);
			case CONNECTION_ASSIGNMENT:
				wban_extract_conn_assign_frame (frame_MPDU);
		}
	case CONTROL: /* Handle control packets */
		switch (frame_subtype_fd) {
			case I_ACK:
				wban_extract_i_ack_frame (frame_MPDU);
			case BEACON2:
				wban_battery_sleep_start(mac_state);
				mac_state = MAC_SLEEP;
				wban_extract_beacon2_frame(frame_MPDU);
		}
```

接收来自应用层模块发送的包，主要由`case STRM_FROM_TRAFFIC_UP_TO_MAC`负责，具体封装函数为`wban_encapsulate_and_enqueue_data_frame`, 目前的策略为均使用 I-ACK 策略，如果要调整 ACK 策略可从这里更改，也可在发送到物理层之前更改，不过在发送至物理层之前更改需要注意修改包后的端到端时延计算。

```c
case STRM_FROM_TRAFFIC_UP_TO_MAC: /* INCOMMING PACKETS(MSDU) FROM THE TRAFFIC SOURCE */
	wban_encapsulate_and_enqueue_data_frame (rcv_frame, I_ACK_POLICY, node_attr.traffic_dest_id);
```