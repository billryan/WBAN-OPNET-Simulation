
/* define the general node parameters */
typedef struct {
	Objid	parent_id; // ID of the node
	Objid	objid;	// ID of the module which received the packets
	Objid   my_battery; // Battery module ID
	Objid   gts_traffic_source; //GTS Traffic Source module ID
	int		mac_address; // mac address of the node
	char name [20];
	double	x; // X coordinate of the node 
	double	y; // Y coordinate of the node
	double altitude; // The altitude of the node
	Boolean is_PANcoordinator; // state if the node is a PAN Coordinator or not
	char Device_Mode[20]; // Can be a End Device or a PAN Coordinator
	int PANcoordinator_mac_address; // the MAC address of the WPAN coordinator
	int traffic_destination_address;	// the destination MAC address for Traffic Source data transmission
	
	Boolean is_GTS_Permit;	// if PANCoordinator accepts GTS request	
	PrgT_List* GTS_list_PC;	// list of allocated GTS slots on the PANCoordinator node
	
	int beacon_order;
	int superframe_order;
	int pan_id;		
} wpan_node_attributes;

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


/*
          |***********************************|.................................|

          <---------Superframe Duration-------><--------- Sleep Period----------->
          <------------------------Beacon Interval ------------------------------>

*/

/* define the superframe structure parameters */
typedef struct {
	int slot_duration; // the slot duration in symbol
	int SD; // the superframe duration in symbol
	int BI; // the beacon interval in symbol
	int sleep_period; // the inactive portion in symbol
	int current_slot; // the current slot in the active portion of the superframe from 0 to 15
	double BI_Boundary; // Specfiy the time at which the beacon frame has been created to synchronize all node to this time reference
	int Final_CAP_Slot; // Final slot in the CAP
	double backoff_timer; // remaining backoff time from last CAP
	Boolean CAP_ACTIVE;	// Contention Access Period (CAP) is active 
	Boolean CFP_ACTIVE;	// Contention Free Period (GTS) is active
	Boolean SLEEP;	// Inactive portion
	Boolean RESUME_BACKOFF_TIMER; // if TRUE the backoff is resumed in the new CAP
	Boolean CCA_DEFERRED; // if TRUE the CCA must start at the begining of the CAP of the next superframe
} wpan_superframe_strucuture;



/* define the MAC parameters */
typedef struct {
	Boolean Battery_Life_Extension; // if no BE = macMinBE, if yes BE = min(2,macMinBE);
	Boolean wait_ack;	// acknowledged packet?
	int wait_ack_seq_num;	// the sequence number of the waiting ACK	
} wpan_mac_attributes;

