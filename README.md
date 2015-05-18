WBAN-OPNET-Simulation
=====================
If you want to use the IEEE 802.15.6 Simulation, please use the [ieee802156](https://github.com/billryan/WBAN-OPNET-Simulation/tree/ieee802156) branch. The master branch is for my own proposal.

WBAN(802.15.6) Simulation based on OPNET 14.5 Modeler for academical purpose.  

Please refer `documentation` folder for more details. If you want to use real IEEE 802.15.6 protocal, please use **ieee802156** branch.

## Coding conventions

源码说明，整个工程文件目录及各文件关系如下所示：


```
.
├── attribute
│   ├── connection_request.ad.m
│   ├── msdu_interarrival_time.ad.m
│   ├── msdu_size.ad.m
│   ├── row_count.ad.m
│   ├── traffic_parameter.ad.m
│   ├── traffic_start_time.ad.m
│   ├── traffic_stop_time.ad.m
│   └── wban_setting.ad.m
├── documentation
│   ├── ...
├── headers
│   ├── wban_math.h
│   ├── wban_params.h
│   └── wban_struct.h
├── ici
│   └── wban_battery_ici_format.ic.m
├── LICENSE.md
├── packets
│   ├── wban_app_traffic_format.pk.m
│   ├── wban_beacon_frame_proposal_format.pk.m
│   ├── wban_beacon_MSDU_format.pk.m
│   ├── wban_beacon2_MSDU_format.pk.m
│   ├── wban_connection_assignment_frame_format.pk.m
│   ├── wban_connection_request_frame_format.pk.m
│   ├── wban_frame_MPDU_format.pk.m
│   └── wban_frame_PPDU_format.pk.m
├── py_analysis
│   ├── load_data.py
│   ├── plot_stat.py
│   ├── raw_data.zip
│   └── read_large_file.py
├── README.md
├── Sensor_Icons.icons
├── src
│   ├── wban_mac_pr.hb.c
│   ├── wban_mac_pr.fb.c
│   ├── wban_mac_pr.sv.c
│   └── ...
├── wban_battery_process.pr.m
├── wban_gts_traffic_source.pr.m
├── wban_mac_process.pr.m
├── wban_packet_source_up_process.pr.m
├── wban_sensor_node.nd.m
```

- `attribute`目录下为一些属性的设置模板，如信号源的业务产生函数分布等，一般不需要关注。
- `documentation`为该项目的开发和使用文档。
- `headers`为头文件存放的地方，主要分为如下三块：
	1. `wban_math.h` - 一些数学运算辅助函数。
	2. `wban_params.h` - 一些参数的集中设置。
	3. `wban_struct.h` - 结构体信息集中定义，如节点信息，超帧信息等。
- `packets`目录为WBAN中所使用的一些包域信息，需要自定义包信息时修改。
- `py_analysis` - 使用 Python 进行数据分析与自动出图。
- `src` - 存放源码的目录，OPNET 中使用 FSM 和 进程模型进行协议编程，故不便于源码版本控制，这里抽取出 **HB**, **FB**, **SV** 单独成为一个文件。其中 **HB** 用作头文件，也可在这里定义伪全局变量(作用域为使用该进程模型的所有 process)，**SV** 用于定义部分变量，作用域为当前 process, **FB** 为存放函数块的地方，OPNET 推荐的做法是使用 FSM 有限状态机模型将这些函数块串起来，目前还在火热开发中，变化较剧烈，故几乎不怎么使用 FSM 来编写。
- `*.pr.m` - 进程模型。
- `*.nd.m` - 节点模型。

### Helper functions

use `hp` for abbreviation of `helper`.
