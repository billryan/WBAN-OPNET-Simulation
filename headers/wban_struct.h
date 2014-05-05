#ifndef BAN_STRUCT_H
#define BAN_STRUCT_H

/* define the general node parameters */
typedef struct {
	Objid	parent_id; // ID of the node
	Objid	objid;	// ID of the module which received the packets
	Objid   my_battery; // Battery module ID

	char 	name [20];
	double	x; // X coordinate of the node
	double	y; // Y coordinate of the node
	double altitude; // The altitude of the node
	
	double data_rate; // WBAN Data Rate

	int	sender_address; // sender address of the node, 48 bits, for beacon frame

	int ban_id;	// ban ID
	int hub_id; //Hub ID
	int unconnectedNID; // temporary unconnected NID

	Boolean is_BANhub; // state if the node is a Hub or not
	char Device_Mode[20]; // Can be a Node or a Hub
	
	int traffic_destination_address;	// the destination MAC address for Traffic Source data transmission
	int traffic_dest_id;	// the destination ID for Traffic Source data transmission
} wban_node_attributes;

/* define the beacon frame parameters */
typedef struct {
	int	sender_address; // 48 bits

	int beacon_period_length;
	int allocation_slot_length;
	int rap1_start;
	int rap1_end;
	int rap2_start;
	int rap2_end;
	int inactive_duration;
} beacon_attributes;

/* define the connection request parameters */
typedef struct {
	int recipient_address;
	int	sender_address; // 48 bits
	int requested_wakeup_phase;
	int requested_wakeup_period;

	int allocation_id;
	int minimum_length;
	int allocation_length;
} connection_request_attributes;

/* define the redord storing in the wpan_node_attributes->GTS_list_PC */
typedef struct {
	int start_slot;	// the starting superframe slot of the appropriate GTS
	int length; // length of the GTS [# superframe slots]
	int direction; // direction of the transmission (device->PANCoord (transmit)=0, PANCoord->device(receive)=1)	
	int assoc_dev_address;	// MAC address of associated device
	int beacon_persistence_time;	// how long this GTS descriptor will appear in the beacon [# of superframes]
} wpan_gts_list_PC_record;

/* define the GTS parameters of the node */
typedef struct {
	double start_time;	// start of the using of the GTS
	double stop_time;	// end of the using of GTS
	int length;	 // asked length of the GTS [superframe slots]
	int direction;	// direction of the transmission (device->PANCoord (transmit)=0, PANCoord->device(receive)=1)
	int start_slot;	// start slot given by PANCoordinator and received from the beacon frame
	Boolean GTS_ACTIVE; //true if the GTS slot(s) is active
	int retries_nbr;	// actual number of retries (< aMaxFrameRetries)
} wpan_gts_attributes;

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
	int MAX_CSMA_BACKOFF; // maximum number of Backoff (macMaxCSMABackoffs)
	int macMinBE; // Minimum Backoff Exponent
	int NB; // current number of backoff
	int BE; // Backoff exponent
	int CW; // Contention Window
	Boolean CCA_CHANNEL_IDLE; // if TRUE the Channel is assessed to be idle, otherwise busy
	int retries_nbr;	// actual number of retries (< aMaxFrameRetries)
} wpan_csma_attributes;

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
	int SD; // the superframe duration in slots
	int BI; // the beacon interval in slots
	int sleep_period; // the inactive portion in slots
	int current_slot; // the current slot in the active portion of the superframe from 0 to beacon_period_length-1
	int current_first_free_slot;

	int beacon_period_length; // beacon period length in slots -all
	int rap1_start;
	int rap1_end;
	int rap2_start;
	int rap2_end;
	int inactive_duration;

	double beacon_frame_tx_time;
	double eap1_length2sec;
	double rap1_length2sec;
	double map1_length2sec;
	double eap2_length2sec;
	double rap2_length2sec;
	double map2_length2sec;

	double BI_Boundary; // Specfiy the time at which the beacon frame has been created to synchronize all node to this time reference
	double eap1_start2sec;
	double rap1_start2sec;
	double map1_start2sec;
	double rap2_start2sec;

	double backoff_timer; // remaining backoff time from last CAP
	Boolean CAP_ACTIVE;	// Contention Access Period (CAP) is active 
	Boolean CFP_ACTIVE;	// Contention Free Period (Scheduling) is active
	Boolean IN_MAP_PHASE; //true if in MAP phase
	Boolean IN_EAP_PHASE; //true if in EAP1/EAP2 phase
	Boolean ENABLE_TX_NEW; // Enable new transmission
	Boolean SLEEP;	// Inactive portion
	Boolean RESUME_BACKOFF_TIMER; // if TRUE the backoff is resumed in the new CAP
	Boolean CCA_DEFERRED; // if TRUE the CCA must start at the begining of the CAP of the next superframe
} wban_superframe_strucuture;

/* define the MAC parameters */
typedef struct {
	Objid	objid;	// ID of the module which received the packets
	Boolean Battery_Life_Extension; // if no BE = macMinBE, if yes BE = min(2,macMinBE);

	int	sender_id; // Sender ID of the node
	int	recipient_id; // Recipient ID of the node
	int ban_id;	// ban ID

	int max_packet_tries;
	int MGMT_buffer_size;
	Boolean wait_for_ack; //waiting for ack
	int wait_ack_seq_num;	// the sequence number of the waiting ACK	
} wban_mac_attributes;

/* define the packet to be sent parameters */
typedef struct 
{
	Objid objid;
	int	sender_id; // Sender ID of the node
	int	recipient_id; // Recipient ID of the node
	int ack_policy;
	int seq_num;
	int total_bits; // length of PPDU
	int ack_bits; // length of ack PPDU
	int user_priority;
	int frame_type;
	int frame_subtype;
} packet_to_be_sent_attributes;

#endif