#ifndef BAN_STRUCT_H
#define BAN_STRUCT_H

/** struct definition **/
/* node attributes */
typedef struct {
	Objid objid;
	Objid nodeid; // ID of the node
	Objid srcid; // Source ID
	Objid macid; // MAC module ID

	char name[20]; // node name
	// double	x; // X coordinate of the node
	// double	y; // Y coordinate of the node
	// double altitude; // The altitude of the node
	/* PHY Attributes */
	double data_rate; // WBAN Data Rate
	double tx_power;
	int	send_addr; // sender address of the node, 48 bits, for beacon frame
	int bid; // BAN ID
	int hid; // Hub ID
	int nid; // Node ID
	int unconnectedNID; // temporary unconnected NID
	int dev_mode; // Device Mode, Hub(0) or Node(1)
	int protocol_ver; //Protocol Version, 0 for 802.15.6, 1 for proposed protocol
	int map_schedule; //Schedule strategy for MAP
	int dest_id; // destination ID for Traffic
	/* node activity */
	Boolean is_idle;
	Boolean is_sleep;
	double last_idle_time;
	double sleeping_time;
} wban_node_attributes;

/* MAC module parameters */
typedef struct {
	Objid objid;
	int	sendid; // Sender ID of the node
	int	rcvid; // Recipient ID of the node
	int bid; // BAN ID

	int max_packet_tries;
	int MGMT_buffer_size;
	int DATA_buffer_size;
	Boolean wait_for_ack; //waiting for ack
	int wait_ack_seq_num;	// the sequence number of the waiting ACK	
} wban_mac_attributes;

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

/* define the beacon frame parameters */
typedef struct {
	int	send_addr; // 48 bits
	int beacon_period_length;
	int allocation_slot_length;
	int rap1_start;
	int rap1_end;
	int rap1_length;
	int inactive_duration;
} beacon_attributes;

/* define the connection request parameters */
typedef struct {
	int recipient_address;
	int	send_addr; // 48 bits
	int requested_wakeup_phase;
	int requested_wakeup_period;
	int allocation_id;
	int minimum_length;
	int allocation_length;
} connection_request_attributes;

/* define the MAP1 parameters of the node */
typedef struct {
	double start_time;	// start of the using of the MAP1
	double stop_time;	// end of the using of MAP1
	int length;	 // asked length of the MAP [superframe slots]
	int direction;	// direction of the transmission (node->Hub (transmit)=0, Hub->node(receive)=1)
	int start_slot;	// start slot given by Hub and received from the beacon frame
	Boolean MAP1_ACTIVE; //true if the MAP1 slot(s) is active
	Boolean TX_state; // true if in TX state
	int retries_nbr;	// actual number of retries (< aMaxFrameRetries)
} wban_map_attributes;

/* define the backoff parameters */
typedef struct {
	int CW; // Contention Window
	Boolean CW_double;
	int backoff_counter;
	Boolean backoff_counter_lock;
	double backoff_time;
	double next_slot_start;
	double backoff_timer; // for calculating next backoff_time
	double backoff_expiration_time;
	Boolean RESUME_BACKOFF_TIMER;
	Boolean CCA_DEFERRED;
	Boolean CCA_CHANNEL_IDLE; // if TRUE the Channel is assessed to be idle, otherwise busy
	int retries_nbr;	// actual number of retries (< aMaxFrameRetries)
} wban_csma_attributes;


/*
          |***********************************|.................................|

          <---------Superframe Duration-------><--------- Sleep Period----------->
          <------------------------Beacon Interval ------------------------------>

*/

/* define the superframe structure parameters */
typedef struct {
	int slot_duration; // the slot duration in slots
	double slot_length2sec; // one slot length in sec
	double slot_sec;
	int SD; // the superframe duration in slots
	int BI; // the beacon interval in slots
	double duration; // superfarm duration in second
	int sleep_period; // the inactive portion in slots
	int current_slot; // the current slot in the active portion of the superframe from 0 to beacon_period_length-1
	int free_slot;
	int first_free_slot;

	int beacon_period_length; // beacon period length in slots -all
	int rap1_start;
	int rap1_end;
	int map1_start;
	int map1_end;
	int map1_len;
	int inactive_duration;

	double beacon_frame_tx_time;
	double eap1_length2sec;
	double rap1_length2sec;
	double map1_length2sec;

	double BI_Boundary; // Specfiy the time at which the beacon frame has been created to synchronize all node to this time reference
	double eap1_start2sec;
	double eap1_end2sec;
	double rap1_start2sec;
	double rap1_end2sec;
	double map1_start2sec;
	double map1_end2sec;

	double backoff_timer; // remaining backoff time from last CAP
	Boolean CAP_ACTIVE;	// Contention Access Period (CAP) is active 
	Boolean CFP_ACTIVE;	// Contention Free Period (Scheduling) is active
	Boolean IN_MAP_PHASE; //true if in MAP phase
	Boolean IN_CAP_PHASE; //true if in Contention access Phase
	Boolean IN_EAP_PHASE; //true if in EAP1/EAP2 phase
	Boolean ENABLE_TX_NEW; // Enable new transmission
	Boolean IN_CONN_SETUP; // whether in connection setup
	Boolean SLEEP;	// Inactive portion
} wban_superframe_strucuture;


/* define the packet to be sent parameters */
typedef struct 
{
	Objid objid;
	int	sendid; // Sender ID of the node
	int	rcvid; // Recipient ID of the node
	int ack_policy;
	int seq_num;
	int ppdu_bits; // length of PPDU
	int ack_bits; // length of ack PPDU
	int user_priority;
	int frame_type;
	int frame_subtype;
	Boolean enable; // indicate whether a packet is deleted
} packet_to_be_sent_attributes;

/* define the subqueue infomation parameters */
typedef struct {
	double pksize;
	double bitsize;
	double free_pksize;
	double free_bitsize;
	double pk_overflows;
	int up; // user priority
} subqueue_info;

/* define the data stat infomation parameters */
typedef struct {
	double number;
	double ppdu_kbits;
} data_stat_info;

/* pakcets received */
typedef struct {
	double number;
	double ppdu_kbits;
	double snr_db;
} st_pkt_rx;

/* slot allocated and packet received */
typedef struct {
	int slot;
	int thr;
} st_map_pkt;

/* statistic vectors */
typedef struct {
	Stathandle data_pkt_fail;
	Stathandle data_pkt_suc1;
	Stathandle data_pkt_suc2;
	Stathandle data_pkt_sent;
	Stathandle up0_sent;
	Stathandle up1_sent;
	Stathandle up2_sent;
	Stathandle up3_sent;
	Stathandle up4_sent;
	Stathandle up5_sent;
	Stathandle up6_sent;
	Stathandle up7_sent;
	Stathandle data_pkt_rec;
	Stathandle data_pkt_rec_map1;
	Stathandle dropped_packets;
	Stathandle mac_delay;
	Stathandle sent_pkt;

	// for log txt
	double ppdu_sent_kbits;
	double ppdu_sent_nbr;
	double ppdu_sent_kbits_rap;
	double ppdu_sent_nbr_rap;
	// for Hub only
	double ppdu_rcv_kbits;
	double ppdu_rcv_nbr;
	double ppdu_rcv_kbits_rap;
	double ppdu_rcv_nbr_rap;
	// ppdu sent kbits for RAP
	double ppdu_rap_sent_start;
	double ppdu_rap_sent_end;
	double ppdu_rap_rcv_start;
	double ppdu_rap_rcv_end;
	double traffic_G;
	double throughput_S;
} wban_statistic_vector;

typedef struct {
	Stathandle data_pkt_fail;
	Stathandle data_pkt_suc1;
	Stathandle data_pkt_suc2;
	Stathandle data_pkt_sent;
	Stathandle up0_sent;
	Stathandle up1_sent;
	Stathandle up2_sent;
	Stathandle up3_sent;
	Stathandle up4_sent;
	Stathandle up5_sent;
	Stathandle up6_sent;
	Stathandle up7_sent;
	Stathandle data_pkt_rec;
	Stathandle dropped_packets;
	Stathandle mac_delay;
	Stathandle sent_pkt;
} wban_global_statistic_vector;

typedef struct
{
	/* totel number of bits dispatched to the network (excepted beacons) [kbits] */
	double	rcv_bits;
	double	mean_delay;
	/* channel utility */
	double	Umax;
	/* total number of dispatched packets to the network (excepted beacons) */
	int	nbr_pkt_rcv;
	// int app_sent_msdu_nbr; // Number of non time-critical MSDU generated by the Application Layer
	// double app_sent_msdu_bits; // Number of non time-critical bits generated by the Application Layer [kbits]
	// int app_sent_gts_msdu_nbr; // Number of GTS MSDU generated by the Application Layer
	// double app_sent_gts_msdu_bits; // Number of GTS bits generated by the Application Layer [kbits]
	// double PPDU_sent_bits;	// Total number of bits (data and command PPDU) dispatched to the network [kbits]
	// rcv_bits = 0;
	// max_delay = 0.0;
	// mean_delay = 0.0;
	// nbr_pkt_rcv = 0;	
	// Umax = 1.0/((900.0+MAC_HEADER_SIZE)/WPAN_DATA_RATE);
} wban_anlyzer_vector;

typedef struct
{
	int nid;
	int slot_start;
	int slot_end;
	int map2_slot_start;
	int map2_slot_end;
	int slotnum;
	int up;
} assignment_map;

typedef struct
{
	int bid;
	int cid;
	int nid;
	int slot_start;
	int slot_end;
	int slotnum;
	int up;
	// average SNR received from Hub
	double snr;
	// average SNR received by Hub
	double eta;
	// priority factor
	double rho;
	// Utility function of Interference by Node
	double infer;
} map1_schedule_map;

typedef struct
{
	int nid;
	double rho;
	/* data */
} nid_rho;
#endif