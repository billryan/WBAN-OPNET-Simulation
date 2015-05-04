# MAC 层 Scheduling 模块(非竞争阶段)

对于 Scheduling 的实现方式，在 IEEE 802.15.6 中的定义如下：

1. Node在MAP之前首先向Hub发送Connection Request帧请求一定的Slot时隙数。
2. Hub 收到 Node 发送到 Request 请求后向 Node 返回 ConnectionAssignment 帧以通知其在 MAP 阶段中可使用的 Slot 起止数。

详细的分配方案可用下图表示，MAP 中不同颜色的 slot 表示不同节点在MAP 阶段所获得的 slot。可以使用贪心算法实现。

![MAC Scheduling](./images/mac_scheduling.png)