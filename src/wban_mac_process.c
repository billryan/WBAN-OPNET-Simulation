/* the attributes of the node (Device Mode, BAN ID, ...) */
wban_node_attributes	\node_attr;

/* the Medium Access Attributes */
wban_mac_attributes	\mac_attr;

/* the MAP Attributes */
wban_map_attributes \map_attr;

/* the attributes of csma/ca (backoff, BE, ...) */
wban_csma_attributes	\csma;

/* the attributes of the beacon (EAP, RAP, MAP...) */
beacon_attributes	\beacon_attr;

/* connection request attributes of Node or Hub */
connection_request_attributes \conn_req_attr;

/* the parameters of the superframe structure */
wban_superframe_strucuture	\SF;

/* a log file to store the operations for each node */
FILE *	\log;

/* if enabled, all the operation will be saved in a log file */
Boolean	\enable_log;

/* the statistic vector */
wban_statistic_vector	\statistic_vector;

wban_global_statistic_vector	\statistic_global_vector;

/* Statistic handle for the total number of generated beacons. */
Stathandle	\beacon_frame_hndl;

/* sequence number of the received packet */
int	\ack_sequence_number;

/* the time when the packet enters the backoff */
double	\backoff_start_time;

/* MAP request was issued after reception of first beacon               */
/* then check if Hub accepts Connection request */
Boolean	\waiting_for_first_beacon;


/* Include files		*/
#include "headers\wban_struct.h"
#include "headers\wban_params.h"
#include "headers\wban_math.h"
#include <math.h>


/* Input and output streams		*/
#define STRM_FROM_RADIO_TO_MAC	0
#define STRM_FROM_MAC_TO_RADIO	0
#define STRM_FROM_TRAFFIC_UP_TO_MAC		1
#define STRM_FROM_MAC_TO_SINK	1


/* Statistics Input Index */
#define TX_BUSY_STAT 0
#define RX_BUSY_STAT 1
#define RX_COLLISION_STAT 2


/* Constants		*/
#define _Epsilon  0.000000000000
#define	INFINITE_TIME -1.0

// subqueues
#define SUBQ_DATA 0
#define SUBQ_MANAGE 1
#define SUBQ_COMMAND 2

#define TIME_SLOT_INDEX_MAX (aMaxBeaconPeriodLength_Slots-1) //0 for 256 slots

#define MAP_DIRECTION_UPLINK	0 // device->Hub
#define MAP_DIRECTION_DOWNLINK	1 // Hub->device


/* MAP characteristics type */
#define MAP_DEALLOCATION 0
#define MAP_ALLOCATION 1


/* Interrupt types		*/
#define DEFAULT_CODE 0
#define BEACON_INTERVAL_CODE 1
#define TIME_SLOT_CODE 2
#define TRY_PACKET_TRANSMISSION_CODE 23
#define BACKOFF_EXPIRATION_CODE 17
#define START_OF_CAP_PERIOD_CODE 25
#define START_OF_EAP1_PERIOD_CODE 50
#define START_OF_RAP1_PERIOD_CODE 51
#define START_OF_EAP2_PERIOD_CODE 52
#define START_OF_RAP2_PERIOD_CODE 53

// the same value should be in the Battery Module for remote process
#define PACKET_TX_CODE 101
#define PACKET_RX_CODE 102
#define END_OF_SLEEP_PERIOD 103
#define END_OF_ACTIVE_PERIOD_CODE 104
#define END_OF_CAP_PERIOD_CODE 105
#define END_OF_EAP1_PERIOD_CODE 60
#define END_OF_RAP1_PERIOD_CODE 61
#define END_OF_EAP2_PERIOD_CODE 62
#define END_OF_RAP2_PERIOD_CODE 63

#define END_OF_CFP_PERIOD_CODE 6
#define CCA_START_CODE 7
#define CCA_EXPIRATION_CODE 8
#define RETURN_TO_BACKOFF_CODE 19
#define FAILURE_CODE 10
#define SUCCESS_CODE 11
#define START_TRANSMISSION_CODE 30
#define WAITING_ACK_END_CODE 36
#define ACK_SUCCESS_CODE 43
#define ACK_FAILURE_CODE 44
#define ACK_SEND_CODE 47

// MAP
#define START_OF_MAP_USE 200
#define STOP_OF_MAP_USE 201
#define START_OF_MAP_PERIOD 210
#define END_OF_MAP_PERIOD 211
#define TRY_MAP_PACKET_TRANSMISSION_CODE 230


/* Global variables	*/
//extern double PPDU_sent_bits;	// Total number of bits (PPDU) dispatched to the network [kbits]
int current_free_connected_NID = 32; // start assigning connected NID from ID 32
int current_first_free_slot = -1;
int unconnectedNID;
int sequence_num_beaconG = 0; //sequence number for beacon frame, global variable
enum MAC_STATES mac_state = MAC_SETUP; //initialize MAC states with MAC_SETUP

int max_packet_tries;
int current_packet_txs;
int current_packet_CS_fails; // not enough time to tx in current phase(EAP,RAP,CAP,etc...)

static int CWmin[8] = { 16, 16, 8, 8, 4, 4, 2, 1 };
static int CWmax[8] = { 64, 32, 32, 16, 16, 8, 8, 4};
double beacon_frame_tx_time;
double phase_start_timeG; // start time in various Phase
double phase_end_timeG; // end time in various Phase
double backoff_start_time;

/* State machine conditions */
#define IAM_BAN_HUB (node_attr.is_BANhub == OPC_TRUE)

#define IAM_WAITING_ACK (mac_attr.wait_ack == OPC_TRUE)

#define BACKOFF_EXPIRED (op_intrpt_type () == OPC_INTRPT_SELF && op_intrpt_code () == BACKOFF_EXPIRATION_CODE) 
#define	DEFAULT_INTRPT	(op_intrpt_type () == OPC_INTRPT_SELF && op_intrpt_code () == DEFAULT_CODE) 

#define PACKET_READY_TO_SEND ((SF.CAP_ACTIVE==OPC_TRUE) && (SF.SLEEP==OPC_FALSE) &&\
                              !(op_subq_empty(SUBQ_DATA)) && \
                              (op_stat_local_read(TX_BUSY_STAT)==0.0) && \
                              (op_intrpt_type () == OPC_INTRPT_SELF && op_intrpt_code () == TRY_PACKET_TRANSMISSION_CODE))


#define CHANNEL_BUSY (op_intrpt_type () == OPC_INTRPT_SELF && op_intrpt_code () == RETURN_TO_BACKOFF_CODE)


#define CAP_IS_ACTIVE ((SF.CAP_ACTIVE==OPC_TRUE) && (SF.SLEEP==OPC_FALSE) && (SF.CFP_ACTIVE==OPC_FALSE))

#define TX_FAILURE (op_intrpt_type () == OPC_INTRPT_SELF && op_intrpt_code () == FAILURE_CODE)
#define TX_SUCCESS (op_intrpt_type () == OPC_INTRPT_SELF && op_intrpt_code () == SUCCESS_CODE)
#define RX_ACK_SUCCESS (op_intrpt_type () == OPC_INTRPT_SELF && op_intrpt_code () == ACK_SUCCESS_CODE)
#define CCA_END (TX_FAILURE || (TX_SUCCESS && !IAM_WAITING_ACK)|| RX_ACK_SUCCESS )


/* Structures		*/
/* statistic vectors */
typedef struct {
	Stathandle failure;
	Stathandle success;
	Stathandle ch_busy_cca1;
	Stathandle ch_busy_cca2;
	Stathandle ch_idle_cca1;
	Stathandle ch_idle_cca2;
	Stathandle deferred_cca;
	Stathandle dropped_i_ack_packets;
	Stathandle dropped_packets;
	Stathandle retransmission_attempts;
	Stathandle success_i_ack_packets;
	Stathandle backoff_delay;
	Stathandle mac_delay;
	Stathandle backoff_units;
	Stathandle mac_delay_up7_frame;
	Stathandle mac_delay_up5_frame;
	Stathandle sent_pkt;
	Stathandle deferred_cca_backoff;
} wban_statistic_vector; 

typedef struct {
	Stathandle failure;
	Stathandle success;
	Stathandle ch_busy_cca1;
	Stathandle ch_busy_cca2;
	Stathandle ch_idle_cca1;
	Stathandle ch_idle_cca2;
	Stathandle deferred_cca;
	Stathandle dropped_ack_packets;
	Stathandle dropped_packets;
	Stathandle retransmission_attempts;
	Stathandle success_ack_packets;
	Stathandle backoff_delay;
	Stathandle mac_delay;
	Stathandle backoff_units;
	Stathandle mac_delay_up7_frame;
	Stathandle mac_delay_up5_frame;
	Stathandle sent_pkt;
} wban_global_statistic_vector; 

typedef struct {
	Stathandle sent_frames;	// number of sent MAC frames
	Stathandle sent_framesG;
	Stathandle frame_delay;	// MAC frame delay (time between the MSDU generation and frame trasmission)
	Stathandle wasted_bandwidth_pc; // [%]
	Stathandle throughput_pc;	// [%]
	Stathandle buffer_saturation_pc;	// [%]
} wban_map1_statistic_vector;

typedef struct {
	double band_wast_sum; // [%] sum of the wasted bandwidth (IFS, waiting, ACK) during the using of MAP1
	double frame_delay_max;	// MAC frame delay (time between the MSDU generation and frame trasmission)
	double frame_delay_min;
	double frame_delay_sum;
	double sent_frames;	// total number of MAC frames sent during the using of MAP1
	double sent_bits;	// total number of data bits sent during the using of MAP1
	double generated_frames;	// total number of generated MAC frames
	double generated_bits;	// total number of generated data bits
	double dropped_frames;	// number of MAC frames which is dropped because of full buffer
	double dropped_bits;	// number of data bits which is dropped because of full buffer
	double map1_periods_count;	// number of MAP1 periods during the using of MAP1
} wban_map1_statistic_scalar;

/* GTS structures */
/* structure carried in the GTS request command frame */
typedef struct {
	int length;	// length of the GTS [# superframe slots]
	int direction; // direction of the transmission (device->PANCoord (transmit)=0, PANCoord->device(receive)=1)
	int characteristics_type;	// deallocation=0, allocation=1
} wban_gts_characteristics;

/* structure carried in the beacon frame - GTS_list_field */
typedef struct {
	int device_short_address;
	int start_slot;
	int length;
} wban_gts_descriptor;

/* Function prototypes.				*/
static void wban_mac_init (void);
static void wban_log_file_init (void);
static void fprint_mac_attributes (void);
static void print_mac_attributes (void);
static void wban_parse_incoming_frame (void);
static void wpan_encapsulate_and_enqueue_command_frame (Packet* cmd_frame, Boolean ack, int dest_address);
static void wban_encapsulate_and_enqueue_data_frame (Packet* data_frame, enum AcknowledgementPolicy_type ackPolicy, int dest_address);
static void wban_extract_beacon_frame (Packet* mac_frame);
static void wban_extract_command_frame (Packet* mac_frame);
static void wban_extract_ack_frame (Packet* mac_frame);
static void wban_extract_data_frame (Packet* mac_frame);
static void wban_schedule_next_beacon (void);
static void wban_send_beacon_frame (void);
static void wban_send_connection_request_frame (void);
static void wban_send_ack_frame (int sequence_number);
static void wban_mac_interrupt_process (void);

// backoff functions
static void   wpan_backoff_init (void);
static void   wpan_backoff_update (void);
static void   wpan_backoff_delay_set (void);
static int    wpan_backoff_period_index_get (void);
static double wpan_backoff_period_boundary_get (void);
static int 	  wpan_backoff_period_index_get_ (double Time);
static double wpan_backoff_period_boundary_get_ (double Time);

// cca functions
static Boolean  wpan_cca_defer (void);
static void 	wpan_cca_perform (int cca_index);
static void 	wpan_cca_expire (void);
static int wban_update_sequence_number (void);
static int wpan_current_time_slot_get (void);
static int wpan_ifs_symbol_get (Packet* frame_to_send);

// battery update functions
static void wpan_battery_update_tx (double pksize);
static void wpan_battery_update_rx (double pksize, int frame_type);

// utility functions
static void queue_status (void);
static void cap_is_not_active (void);
static Boolean is_packet_for_me(Packet* frame_MPDU, int ban_id, int recipient_id, int sender_id);


/*--------------------------------------------------------------------------------
 * Function:	wban_mac_init
 *
 * Description:	- initialize the process
 *				- read the attributes and set the global variables
 *
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_mac_init() {
	Objid beacon_attr_comp_id;
	Objid beacon_attr_id;
	Objid conn_req_attr_comp_id;
	Objid conn_req_attr_id;
	Objid csma_attr_comp_id;
	Objid csma_attr_id;
	Objid mac_attr_comp_id;
	Objid mac_attr_id;
	Objid traffic_source_up_id;
	//Objid queue_objid;
	//Objid subq_objid;
	
	/* Stack tracing enrty point */
	FIN(wban_mac_init);
	
	/* obtain self object ID of the surrounding processor or queue */
	mac_attr.objid = op_id_self ();
	
	/* obtain object ID of the parent object (node) */
	node_attr.objid = op_topo_parent (mac_attr.objid);

	/* get the name of the node */
	op_ima_obj_attr_get (node_attr.objid, "name", &node_attr.name);

	op_ima_obj_attr_get (node_attr.objid, "Enable Logging", &enable_log);

	/* get the geographic position of the node */
	op_ima_obj_attr_get (node_attr.objid, "x position", &node_attr.x);
	op_ima_obj_attr_get (node_attr.objid, "y position", &node_attr.y);
	op_ima_obj_attr_get (node_attr.objid, "altitude", &node_attr.altitude);	
	
	op_ima_obj_attr_get (node_attr.objid, "WBAN DATA RATE", &node_attr.data_rate);

	/* get the value of the BAN ID */
	op_ima_obj_attr_get (node_attr.objid, "BAN ID", &node_attr.ban_id);
	mac_attr.ban_id = node_attr.ban_id;

	/* get the Sender Address of the node */
	op_ima_obj_attr_get (node_attr.objid, "Sender Address", &node_attr.sender_address);

	/* Sender Address is not specified - Auto Assigned(-2) frome node objid */
	if (node_attr.sender_address == -2) {
		node_attr.sender_address = node_attr.objid;
	}
	
	/* obtain object ID of the Traffic Source node */
	traffic_source_up_id = op_id_from_name(node_attr.objid, OPC_OBJTYPE_PROC, "Traffic Source_UP");
	
	/* obtain destination ID for data transmission  */
	op_ima_obj_attr_get (traffic_source_up_id, "Destination ID", &node_attr.traffic_dest_id);	
	
	/* get the MAC settings */
	op_ima_obj_attr_get (mac_attr.objid, "MAC Attributes", &mac_attr_id);
	mac_attr_comp_id = op_topo_child (mac_attr_id, OPC_OBJTYPE_GENERIC, 0);

	op_ima_obj_attr_get (mac_attr_comp_id, "Batterie Life Extension", &mac_attr.Battery_Life_Extension);
	op_ima_obj_attr_get (mac_attr_comp_id, "Max Packet Tries", &max_packet_tries);
	op_ima_obj_attr_get (mac_attr_comp_id, "MGMT Buffer Size", &mac_attr.MGMT_buffer_size);
	
	mac_attr.wait_ack = OPC_FALSE;
	mac_attr.wait_ack_seq_num = 0;
	
	/*get the battery attribute ID*/
	node_attr.my_battery = op_id_from_name (node_attr.objid, OPC_OBJTYPE_PROC, "Battery");

	if (node_attr.my_battery == OPC_NIL) {
		op_sim_end("CANNOT FIND THE BATTERY ID","CHECK IF THE NAME OF THE BATTERY MODULE IS [Battery]","","");
	}
	
	wban_log_file_init();
	
	/* get the value to check if this node is Hub or not */
	op_ima_obj_attr_get (node_attr.objid, "Device Mode", &node_attr.Device_Mode);

	node_attr.is_BANhub = OPC_FALSE;
	if (strcmp(node_attr.Device_Mode, "Hub") == 0) {
		node_attr.is_BANhub = OPC_TRUE;	
	}

	if (node_attr.is_BANhub == OPC_TRUE) {
		mac_attr.sender_id = node_attr.ban_id + 15; // set the value of HID=BAN ID + 15
		mac_attr.recipient_id = BROADCAST_NID; // default value, usually overwritten

		/* get the beacon attributes for the Hub */
		beacon_attr.sender_address = node_attr.sender_address;

		op_ima_obj_attr_get (node_attr.objid, "Beacon", &beacon_attr_id);
		beacon_attr_comp_id = op_topo_child (beacon_attr_id, OPC_OBJTYPE_GENERIC, 0);

		op_ima_obj_attr_get (beacon_attr_comp_id, "Beacon Period Length", &beacon_attr.beacon_period_length);
		op_ima_obj_attr_get (beacon_attr_comp_id, "Allocation Slot Length", &beacon_attr.allocation_slot_length);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP1 Start", &beacon_attr.rap1_start);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP1 End", &beacon_attr.rap1_end);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP2 Start", &beacon_attr.rap2_start);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP2 End", &beacon_attr.rap2_end);
		op_ima_obj_attr_get (beacon_attr_comp_id, "Inactive Duration", &beacon_attr.inactive_duration);

		// register the beacon frame statistics
		beacon_frame_hndl = op_stat_reg ("MANAGEMENT.Number of Generated Beacon Frame", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
		wban_send_beacon_frame ();
	} else { /* if the node is not a Hub */
		mac_attr.sender_id = UNCONNECTED;
		mac_attr.recipient_id = UNCONNECTED;
		//node_attr.unconnectedNID = 1 + (unconnectedNID - 1) % 16;
		//unconnectedNID++;
		node_attr.unconnectedNID = node_attr.objid;
		
		if (enable_log) {
			fprintf (log," [Node %s] initialized with unconnectedNID %d \n\n", node_attr.name, node_attr.unconnectedNID);
			printf (" [Node %s] initialized with unconnectedNID %d \n\n", node_attr.name, node_attr.unconnectedNID);
		}

		/* get the Connection Request for the Node */
		op_ima_obj_attr_get (node_attr.objid, "Connection Request", &conn_req_attr_id);
		conn_req_attr_comp_id = op_topo_child (conn_req_attr_id, OPC_OBJTYPE_GENERIC, 0);

		op_ima_obj_attr_get (conn_req_attr_comp_id, "Requested Wakeup Phase", &conn_req_attr.requested_wakeup_phase);
		op_ima_obj_attr_get (conn_req_attr_comp_id, "Requested Wakeup Period", &conn_req_attr.requested_wakeup_period);
		op_ima_obj_attr_get (conn_req_attr_comp_id, "Minimum Length", &conn_req_attr.minimum_length);
		op_ima_obj_attr_get (conn_req_attr_comp_id, "Allocation Length", &conn_req_attr.allocation_length);
		/* start assigning connected NID from ID 32 */
		// node_attr.sender_id = 32 + ((current_free_connected_NID - 32) % 214);
		// current_free_connected_NID++;
		// node_attr.recipient_id = node_attr.connectedHID;
		
		mac_state = MAC_SETUP;

		beacon_attr.beacon_period_length = -1;
	}

	// SF.current_slot = 0;
	// SF.Final_CAP_Slot = TIME_SLOT_INDEX_MAX; /* no CFP (GTSs) */
	// SF.RESUME_BACKOFF_TIMER = OPC_FALSE;
	// SF.backoff_timer= -1;
	// SF.CCA_DEFERRED = OPC_FALSE;
	
	// /* CSMA initialization	*/
	// csma.NB = 0;
	// csma.CW = 2;
	// csma.BE = csma.macMinBE;
	// csma.retries_nbr = 0;
	// csma.CCA_CHANNEL_IDLE = OPC_TRUE;

	current_packet_txs = 0;
	current_packet_CS_fails = 0;
	//print_mac_attributes ();
	
	//wpan_log_file_init ();	
		
	//fprint_mac_attributes();
	
	/* Stack tracing exit point */
	FOUT;
}


/*--------------------------------------------------------------------------------
 * Function:	 wban_log_file_init
 *
 * Description:	log file init
 *				
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_log_file_init() {
 
	char directory_path_name[120];
	char log_name[132];
	
	/* Stack tracing enrty point */
	FIN(wban_log_file_init);

	op_ima_obj_attr_get (node_attr.objid, "Log File Directory", directory_path_name);
	
	/* verification if the directory_path_name is a valid directory */
	if (prg_path_name_is_dir (directory_path_name) == PrgC_Path_Name_Is_Not_Dir) {
		char msg[128];
		sprintf (msg, " \"%s\" is not valid directory name. The output will not be logged.\n", directory_path_name);
		/* Display an appropriate warning.	*/
		op_prg_odb_print_major ("Warning from wban_mac process: ", msg, OPC_NIL);			
		enable_log = OPC_FALSE;
	}
	
	if (enable_log) {
		sprintf (log_name, "%s%s.ak", directory_path_name, node_attr.name);
		printf ("Log file name: %s \n\n", log_name);
		log = fopen(log_name,"w");
	}
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_send_beacon_frame
 *
 * Description:	Create a beacon frame and send it to the Radio  
 *
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_send_beacon_frame () {
	Packet* beacon_MSDU;
	Packet* beacon_MPDU;
	Packet* beacon_PPDU;
	double beacon_frame_creation_time;
	
	// double eap1_length2sec;
	// double rap1_length2sec;
	// double map1_length2sec;
	// double eap2_length2sec;
	// double rap2_length2sec;
	// double map2_length2sec;
	extern int sequence_num_beaconG;

	/* Stack tracing enrty point */
	FIN(wban_send_beacon_frame);
	
	printf (" \nNODE \"%s\" IS A HUB WITH A SUPERFRAME STRUCTURE AS FOLLOWS \n", node_attr.name);
	printf (" \t Beacon Period Length   : %d \n", beacon_attr.beacon_period_length);
	printf (" \t Allocation Slot Length : %d \n", beacon_attr.allocation_slot_length);
	printf (" \t RAP1 Start             : %d \n", beacon_attr.rap1_start);
	printf (" \t RAP1 End               : %d \n", beacon_attr.rap1_end);
	printf (" \t RAP2 Start             : %d \n", beacon_attr.rap2_start);
	printf (" \t RAP2 End               : %d \n", beacon_attr.rap2_end);
	printf (" \t Inactive Duration      : %d\n\n", beacon_attr.inactive_duration);
	
	/* create a beacon frame */
	beacon_MSDU = op_pk_create_fmt ("wban_beacon_MSDU_format");
	
	/* set the fields of the beacon frame */
	op_pk_nfd_set (beacon_MSDU, "Sender Address", beacon_attr.sender_address);
	op_pk_nfd_set (beacon_MSDU, "Beacon Period Length", beacon_attr.beacon_period_length);
	op_pk_nfd_set (beacon_MSDU, "Allocation Slot Length", beacon_attr.allocation_slot_length);
	op_pk_nfd_set (beacon_MSDU, "RAP1 End", beacon_attr.rap1_end);
	op_pk_nfd_set (beacon_MSDU, "RAP2 Start", beacon_attr.rap2_start);
	op_pk_nfd_set (beacon_MSDU, "RAP2 End", beacon_attr.rap2_end);
	op_pk_nfd_set (beacon_MSDU, "RAP1 Start", beacon_attr.rap1_start);
	op_pk_nfd_set (beacon_MSDU, "Inactive Duration", beacon_attr.inactive_duration);
	
	/* Battery Life Extension if not defined in this first beacon. By default, it is disabled. */

	beacon_frame_creation_time = op_pk_creation_time_get(beacon_MSDU);

	/* create a MAC frame (MPDU) that encapsulates the beacon payload (MSDU) */
	beacon_MPDU = op_pk_create_fmt ("wban_frame_MPDU_format");

	op_pk_nfd_set (beacon_MPDU, "Ack Policy", N_ACK_POLICY);
	op_pk_nfd_set (beacon_MPDU, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (beacon_MPDU, "Frame Subtype", BEACON);
	op_pk_nfd_set (beacon_MPDU, "Frame Type", MANAGEMENT);
	op_pk_nfd_set (beacon_MPDU, "B2", 1); // beacon2 enabled

	op_pk_nfd_set (beacon_MPDU, "Sequence Number", ((sequence_num_beaconG++) % 256));
	op_pk_nfd_set (beacon_MPDU, "Inactive", beacon_attr.inactive_duration); // beacon and beacon2 frame used

	op_pk_nfd_set (beacon_MPDU, "Recipient ID", mac_attr.recipient_id);
	op_pk_nfd_set (beacon_MPDU, "Sender ID", mac_attr.sender_id);
	op_pk_nfd_set (beacon_MPDU, "BAN ID", mac_attr.ban_id);
	
	op_pk_nfd_set_pkt (beacon_MPDU, "MAC Frame Payload", beacon_MSDU); // wrap beacon payload (MSDU) in MAC Frame (MPDU)

	//op_pk_print(beacon_MPDU); //print information about packet

	/* create PHY frame (PPDU) that encapsulates beacon MAC frame (MPDU) */
	beacon_PPDU = op_pk_create_fmt("wban_frame_PPDU_format");

	op_pk_nfd_set (beacon_PPDU, "RATE", node_attr.data_rate);
	/* wrap beacon MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (beacon_PPDU, "PSDU", beacon_MPDU);
	op_pk_nfd_set (beacon_PPDU, "LENGTH", ((double) op_pk_total_size_get(beacon_MPDU))/8); //[bytes]	
	
	// collect the number of beacon frame
	op_stat_write (beacon_frame_hndl, 1.0);

	wpan_battery_update_tx ((double) op_pk_total_size_get(beacon_PPDU));

	// update the superframe parameters
	SF.SD = beacon_attr.beacon_period_length; // the superframe duration(beacon preriod length) in slots
	SF.BI = beacon_attr.beacon_period_length * (1+ beacon_attr.inactive_duration); // active and inactive superframe
	SF.sleep_period = SF.SD * beacon_attr.inactive_duration;

	SF.slot_length2sec = allocationSlotLength2ms / 1000.0; // transfer allocation slot length from ms to sec.
	SF.rap1_start = beacon_attr.rap1_start;
	SF.rap1_end = beacon_attr.rap1_end;
	SF.rap2_start = beacon_attr.rap2_start;
	SF.rap2_end = beacon_attr.rap2_end;
	SF.current_slot = 0;
	SF.current_first_free_slot = beacon_attr.rap1_end + 1;

	SF.BI_Boundary = beacon_frame_creation_time;
	beacon_frame_tx_time = TX_TIME(op_pk_total_size_get(beacon_MPDU), node_attr.data_rate);
	SF.eap1_length2sec = SF.rap1_start * SF.slot_length2sec - beacon_frame_tx_time;
	SF.rap1_length2sec = (SF.rap1_end - SF.rap1_start + 1) * SF.slot_length2sec;
	SF.rap2_length2sec = (SF.rap2_end - SF.rap2_start + 1) * SF.slot_length2sec;

	if (enable_log) {
		printf("sequence_num_beaconG = %d\n", sequence_num_beaconG);
		printf("beacon_frame_tx_time = %d\n", beacon_frame_tx_time);
		fprintf(log,"t=%f  -> Beacon Frame transmission. \n\n", op_sim_time());
		printf(" [Node %s] t=%f  -> Beacon Frame transmission. \n\n", node_attr.name, op_sim_time());
	}


	//op_intrpt_priority_set (OPC_INTRPT_SELF, BEACON_INTERVAL_CODE, -2);
	
	op_intrpt_schedule_self (SF.BI_Boundary + beacon_frame_tx_time, START_OF_EAP1_PERIOD_CODE);
	op_intrpt_schedule_self (SF.BI_Boundary + SF.rap1_start*SF.slot_length2sec, START_OF_RAP1_PERIOD_CODE);
	op_intrpt_schedule_self (SF.BI_Boundary + (SF.rap1_end+1)*SF.slot_length2sec, END_OF_RAP1_PERIOD_CODE);
	//op_intrpt_schedule_self (SF.BI_Boundary + SF.rap2_start*SF.slot_length2sec, START_OF_RAP2_PERIOD_CODE);
	//op_intrpt_schedule_self (SF.BI_Boundary + SF.rap2_start*SF.slot_length2sec, END_OF_EAP2_PERIOD_CODE);
	//op_intrpt_schedule_self (SF.BI_Boundary + (SF.rap2_end+1)*SF.slot_length2sec, END_OF_RAP2_PERIOD_CODE);
	op_intrpt_schedule_self (SF.BI_Boundary + SF.BI*SF.slot_length2sec, BEACON_INTERVAL_CODE);
	
	op_intrpt_schedule_remote (SF.BI_Boundary + SF.BI*SF.slot_length2sec, END_OF_SLEEP_PERIOD, node_attr.my_battery);

	wban_schedule_next_beacon (); // Maybe for updating the parameters of superframe

	op_pk_send (beacon_PPDU, STRM_FROM_MAC_TO_RADIO);
	
	/* Stack tracing exit point */
	FOUT;
}

/*------------------------------------------------------------------------------
 * Function:	wban_schedule_next_beacon
 *
 * Description:	generates the self interupt for the next beacon transmission
 *              and a self interrupts to indicate the end of the CAP and end of sleep period
 *
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_schedule_next_beacon() {

	/* Stack tracing enrty point */
	FIN(wban_schedule_next_beacon);
	
	//printf("Sim Tim = %f -  [Node %s] : Next Beacon at time = %f    Next Time Slot = %f\n",op_sim_time(), my_attributes.name, SF.BI_Boundary+Symbols2Sec(SF.BI, WPAN_DATA_RATE), SF.BI_Boundary+Symbols2Sec(SF.slot_duration, WPAN_DATA_RATE));
	//op_intrpt_priority_set (OPC_INTRPT_SELF, END_OF_CAP_PERIOD_CODE, -2);
	
	if (enable_log) {
		fprintf (log,"t=%f  -> Schedule Next Beacon at %f - End of RAP1 : %f\n\n", op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec, SF.BI_Boundary+(SF.rap1_end+1)*SF.slot_length2sec);
		printf (" [Node %s] t=%f  -> Schedule Next Beacon at %f - End of RAP1 : %f\n\n", node_attr.name, op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec, SF.BI_Boundary+(SF.rap1_end+1)*SF.slot_length2sec);
	}

	/* Stack tracing exit point */
	FOUT;	
}

/*------------------------------------------------------------------------------
 * Function:	is_packet_for_me
 *
 * Description:	filter the incoming BAN packet
 *              
 *
 * Input: frame_MPDU - the pointer to the frame (MPDU) which 
 * Output: TRUE if the packet is for me
 *--------------------------------------------------------------------------------*/
static Boolean is_packet_for_me(Packet* frame_MPDU, int ban_id, int recipient_id, int sender_id) {
	/* Stack tracing enrty point */
	FIN(is_packet_for_me);
	
	if (node_attr.ban_id != ban_id) {
		if (enable_log) {
			fprintf (log,"The packet from BAN ID %d is not the same with my BAN ID %d.\n", ban_id, node_attr.ban_id);
			printf (" [Node %s] The packet from BAN ID %d is not the same with my BAN ID %d.\n", node_attr.name, ban_id, node_attr.ban_id);
		}

		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}

	/*Check if the frame is loop*/
	if (mac_attr.sender_id == sender_id) {
		if (enable_log) {
			fprintf(log,"t=%f  -> Loop: DISCARD FRAME \n\n",op_sim_time());
			printf (" [Node %s] t=%f  -> Loop: DISCARD FRAME \n\n",node_attr.name, op_sim_time());
		}
		op_pk_destroy (frame_MPDU);

		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}

	if ((mac_attr.sender_id == recipient_id) || (BROADCAST_NID == recipient_id)) {
		/* Stack tracing exit point */
		FRET(OPC_TRUE);
	} else {
		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}
	/* Stack tracing exit point */
	FRET(OPC_FALSE);
}

/*--------------------------------------------------------------------------------
 * Function:	wban_update_sequence_number()
 *
 * Description:	generate a random sequence number to not interfere with others
 *
 * No parameters  
 *--------------------------------------------------------------------------------*/
static int wban_update_sequence_number() {
	
	int seq_num;
	int max_seq;
	
	/* Stack tracing enrty point */
	FIN(wban_update_sequence_number);
	
	max_seq = 255;
		
	seq_num = floor (op_dist_uniform (max_seq));
	
	//return(seq_num);
	
	/* Stack tracing exit point */
	FRET(seq_num);
}

/*-----------------------------------------------------------------------------
 * Function:	queue_status
 *
 * Description:	print the status of each subqueue
 *              
 * No parameters
 *-----------------------------------------------------------------------------*/
static void queue_status() {
	
	Objid queue_objid;
	Objid subq_objid;
	double bit_capacity;
	double pk_capacity;
	int subq_count;
	int i;
	
	/* Stack tracing enrty point */
	FIN(queue_status);

	/* get the subqueues object ID */
	op_ima_obj_attr_get (mac_attr.objid, "subqueue", &queue_objid);
	
	/* obtain how many subqueues exist */
	subq_count = op_topo_child_count (queue_objid, OPC_OBJMTYPE_ALL);
	
	/* get the object IDs of each subqueue and get subqueue attributes  */
	for (i = 0; i < subq_count; i++)
	{
		/* Obtain object ID of the ith subqueue */
		subq_objid = op_topo_child (queue_objid, OPC_OBJMTYPE_ALL, i);
		
		/* Get current subqueue attribute settings */
		op_ima_obj_attr_get (subq_objid, "bit capacity", &bit_capacity);
		op_ima_obj_attr_get (subq_objid, "pk capacity", &pk_capacity);
		
		if (op_subq_empty(i)) {
			if (enable_log) {	
				fprintf(log,"t=%f  -> Subqueue #%d is empty, wait for MAC frames \n\t -> capacity [%#e frames, %#e bits]. \n\n", op_sim_time(), i, pk_capacity, bit_capacity);
				printf(" [Node %s] t=%f  -> Subqueue #%d is empty, wait for MAC frames \n\t -> capacity [%#e frames, %#e bits]. \n\n", node_attr.name, op_sim_time(), i, pk_capacity, bit_capacity);
			}
		} else {
			if (enable_log) {	 
				fprintf(log,"t=%f  -> Subqueue #%d is non empty, \n\t -> occupied space [%#e frames, %#e bits] - empty space [%#e frames, %#e bits] \n\n", op_sim_time(), i, op_subq_stat (i, OPC_QSTAT_PKSIZE), op_subq_stat (i, OPC_QSTAT_BITSIZE), op_subq_stat (i, OPC_QSTAT_FREE_PKSIZE), op_subq_stat (i, OPC_QSTAT_FREE_BITSIZE));
				printf(" [Node %s] t=%f  -> Subqueue #%d is non empty,\n\t -> occupied space [%#e frames, %#e bits] - empty space [%#e frames, %#e bits] \n\n", node_attr.name, op_sim_time(), i, op_subq_stat (i, OPC_QSTAT_PKSIZE), op_subq_stat (i, OPC_QSTAT_BITSIZE), op_subq_stat (i, OPC_QSTAT_FREE_PKSIZE), op_subq_stat (i, OPC_QSTAT_FREE_BITSIZE));
			}
		}	
	}
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wpan_battery_update_tx
 *
 * Description:	send information about the operation to do to the battery module 
 *
 * Input : 	pksize - size of sent packet [bits]
 *--------------------------------------------------------------------------------*/
static void wpan_battery_update_tx(double pksize) {
	
	Ici * iciptr;
	
	/* Stack tracing enrty point */
	FIN(wpan_battery_update_tx);
	
	iciptr = op_ici_create ("wpan_battery_ici_format");
	op_ici_attr_set (iciptr, "Packet Size", pksize);
	op_ici_attr_set (iciptr, "WPAN DATA RATE", WPAN_DATA_RATE);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), PACKET_TX_CODE, node_attr.my_battery); 
	op_ici_install (OPC_NIL);
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wpan_battery_update_rx
 *
 * Description:	send information about the operation to do to the battery module 
 *
 * Input :	pksize - size of received packet
 *			frame_type - type of received packet
 *--------------------------------------------------------------------------------*/
static void wpan_battery_update_rx(double pksize, int frame_type) {
	
	Ici * iciptr;
	
	/* Stack tracing enrty point */
	FIN(wpan_battery_update_rx);
	
	iciptr = op_ici_create ("wpan_battery_ici_format");
	op_ici_attr_set (iciptr, "Packet Size", pksize);
	op_ici_attr_set (iciptr, "WPAN DATA RATE", WPAN_DATA_RATE);
	op_ici_attr_set (iciptr, "Frame Type", frame_type);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), PACKET_RX_CODE, node_attr.my_battery); 
	op_ici_install (OPC_NIL);
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function: wban_encapsulate_and_enqueue_data_frame
 *
 * Description:	encapsulates the MSDU into a MAC frame and enqueues it.      
 *             
 * Input:	msdu - MSDU (MAC Frame Payload)
 *			ack - 
 *			dest_id - the destionation ID for packet
 *--------------------------------------------------------------------------------*/
static void wban_encapsulate_and_enqueue_data_frame (Packet* data_frame_up, enum AcknowledgementPolicy_type ackPolicy, int dest_id) {
	Packet* data_frame_msdu;
	Packet* data_frame_mpdu;	
	int seq_num;
	int user_priority;

	/* Stack tracing enrty point */
	FIN(wban_encapsulate_and_enqueue_data_frame);

	op_pk_nfd_get_pkt (data_frame_up, "MSDU Payload", &data_frame_msdu);
	op_pk_nfd_get (data_frame_up, "User Priority", &user_priority);
	op_pk_nfd_get (data_frame_up, "App Sequence Number", &seq_num);

	/* create a MAC frame (MPDU) that encapsulates the data_frame payload (MSDU) */
	data_frame_mpdu = op_pk_create_fmt ("wban_frame_MPDU_format");

	/* generate the sequence number for the packet */
	// seq_num = wban_update_sequence_number();

	op_pk_nfd_set (data_frame_mpdu, "Ack Policy", ackPolicy);
	op_pk_nfd_set (data_frame_mpdu, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (data_frame_mpdu, "Frame Subtype", user_priority);
	op_pk_nfd_set (data_frame_mpdu, "Frame Type", DATA);
	op_pk_nfd_set (data_frame_mpdu, "B2", 1); // beacon2 enabled

	op_pk_nfd_set (data_frame_mpdu, "Sequence Number", seq_num);
	op_pk_nfd_set (data_frame_mpdu, "Inactive", beacon_attr.inactive_duration); // beacon and beacon2 frame used

	op_pk_nfd_set (data_frame_mpdu, "Recipient ID", dest_id);
	op_pk_nfd_set (data_frame_mpdu, "Sender ID", mac_attr.sender_id);
	op_pk_nfd_set (data_frame_mpdu, "BAN ID", mac_attr.ban_id);
	
	op_pk_nfd_set_pkt (data_frame_mpdu, "MAC Frame Payload", data_frame_msdu); // wrap data_frame_msdu (MSDU) in MAC Frame (MPDU)

	/* put it into the queue with priority waiting for transmission */
	op_pk_priority_set (data_frame_mpdu, (double)user_priority);
	if (op_subq_pk_insert(SUBQ_DATA, data_frame_mpdu, OPC_QPOS_PRIO) == OPC_QINS_OK) {
		if (enable_log) {
			fprintf (log,"t=%f  -> Enqueuing of MAC DATA frame [SEQ = %d, ACK? = %d] and try to send \n\n", op_sim_time(), seq_num, ackPolicy);
			printf (" [Node %s] t=%f  -> Enqueuing of MAC DATA frame [SEQ = %d, ACK? = %s] and try to send \n\n", node_attr.name, op_sim_time(), seq_num, ackPolicy);
		}
	} else {
		if (enable_log) {
			fprintf (log,"t=%f  -> MAC DATA frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", op_sim_time());
			printf (" [Node %s] t=%f  -> MAC DATA frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", node_attr.name, op_sim_time());
		}
		
		/* destroy the packet */
		op_pk_destroy (data_frame_mpdu);
	}

	/* try to send the packet */
	op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_extract_beacon_frame
 *
 * Description:	extract the beacon frame from the MAC frame received from the network
 *              and schedule the next beacon frame
 *
 * Input :  mac_frame - the received MAC frame
 *--------------------------------------------------------------------------------*/

static void wban_extract_beacon_frame(Packet* beacon_MPDU_rx){

	Packet* beacon_MSDU_rx;
	int rcv_sender_id;
	double beacon_frame_creation_time;
	double beacon_TX_time;
	int sequence_number_fd;
	int eap_indicator_fd;
	int beacon2_enabled_fd;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_beacon_frame);

	op_pk_nfd_get_pkt (beacon_MPDU_rx, "MAC Frame Payload", &beacon_MSDU_rx);
	
	// if I'm a End Device, I get the information and synchronize myself
	if (!node_attr.is_BANhub) {
		op_pk_nfd_get (beacon_MPDU_rx, "Sender ID", &rcv_sender_id);
		op_pk_nfd_get (beacon_MPDU_rx, "Sequence Number", &sequence_number_fd);
		op_pk_nfd_get (beacon_MPDU_rx, "EAP Indicator", &eap_indicator_fd);
		op_pk_nfd_get (beacon_MPDU_rx, "B2", &beacon2_enabled_fd);

		if (node_attr.ban_id + 15 != rcv_sender_id) {
			if (enable_log) {
				fprintf(log,"t=%f  -> Beacon Frame Reception  - but not from Hub. \n",op_sim_time());	
				printf(" [Node %s] t=%f  -> Beacon Frame Reception - but not from Hub. \n", node_attr.name, op_sim_time());
			}

			/* Stack tracing exit point */
			FOUT;
		} else {
			if (enable_log) {
				printf (" [Node %s] t=%f  -> Beacon Frame Reception - synchronization. \n", node_attr.name, op_sim_time());
				printf ("   -> Sequence Number              : %d \n", sequence_number_fd);
			}
		}
		op_pk_nfd_get (beacon_MSDU_rx, "Sender Address", &beacon_attr.sender_address);
		op_pk_nfd_get (beacon_MSDU_rx, "Beacon Period Length", &beacon_attr.beacon_period_length);
		op_pk_nfd_get (beacon_MSDU_rx, "Allocation Slot Length", &beacon_attr.allocation_slot_length);
		op_pk_nfd_get (beacon_MSDU_rx, "RAP1 End", &beacon_attr.rap1_end);
		op_pk_nfd_get (beacon_MSDU_rx, "RAP2 Start", &beacon_attr.rap2_start);
		op_pk_nfd_get (beacon_MSDU_rx, "RAP2 End", &beacon_attr.rap2_end);
		op_pk_nfd_get (beacon_MSDU_rx, "RAP1 Start", &beacon_attr.rap1_start);
		op_pk_nfd_get (beacon_MSDU_rx, "Inactive Duration", &beacon_attr.inactive_duration);

		// update with actual Hub address - since we fixed it on init
		// if (my_attributes.traffic_destination_address == PAN_COORDINATOR_ADDRESS)
		// 	my_attributes.traffic_destination_address = my_attributes.PANcoordinator_mac_address;
		
		beacon_frame_creation_time = op_pk_creation_time_get (beacon_MSDU_rx);

		beacon_TX_time = Bits2Sec(op_pk_total_size_get(beacon_MPDU_rx), node_attr.data_rate);
		
		// update the superframe parameters
		SF.SD = beacon_attr.beacon_period_length; // the superframe duration(beacon preriod length) in slots
		SF.BI = beacon_attr.beacon_period_length * (1+ beacon_attr.inactive_duration); // active and inactive superframe
		SF.sleep_period = SF.SD * beacon_attr.inactive_duration;

		SF.slot_length2sec = allocationSlotLength2ms / 1000.0; // transfer allocation slot length from ms to sec.
		SF.rap1_start = beacon_attr.rap1_start;
		SF.rap1_end = beacon_attr.rap1_end;
		SF.rap2_start = beacon_attr.rap2_start;
		SF.rap2_end = beacon_attr.rap2_end;
		SF.current_slot = 0;
		SF.current_first_free_slot = beacon_attr.rap1_end + 1;

		SF.BI_Boundary = beacon_frame_creation_time;
		beacon_frame_tx_time = TX_TIME(op_pk_total_size_get(beacon_MPDU_rx), node_attr.data_rate);
		SF.eap1_length2sec = SF.rap1_start * SF.slot_length2sec - beacon_frame_tx_time;
		SF.rap1_length2sec = (SF.rap1_end - SF.rap1_start + 1) * SF.slot_length2sec;
		SF.rap2_length2sec = (SF.rap2_end - SF.rap2_start + 1) * SF.slot_length2sec;

		op_pk_destroy (beacon_MSDU_rx);
		op_pk_destroy (beacon_MPDU_rx);

		if (UNCONNECTED == mac_attr.sender_id) {
			/* We will try to connect to this BAN  if our scheduled access length
			 * is NOT set to unconnected (-1). If it is set to 0, it means we are
			 * establishing a sleeping pattern and waking up only to hear beacons
			 * and are only able to transmit in RAP periods.
			 */
			if (conn_req_attr.allocation_length > 0) {
				// we are unconnected, and we need to connect to obtain scheduled access
				// we will create and send a connection request
			}
		}
		if (SF.rap1_start > 0) {

		}
		op_intrpt_schedule_self (op_sim_time(), DEFAULT_CODE);
		op_intrpt_schedule_self (op_sim_time(), START_OF_EAP1_PERIOD_CODE);
	
		wban_schedule_next_beacon();
	}
	/* Stack tracing exit point */
	FOUT;	
}

/*--------------------------------------------------------------------------------
 * Function:	wban_send_connection_request_frame
 *
 * Description:	Create a connection request frame and send it to the Radio (wban_mac) 
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_send_connection_request_frame () {
	Packet* connection_request_MSDU;
	Packet* connection_request_MPDU;
	Packet* connection_request_PPDU;

	/* Stack tracing enrty point */
	FIN(wban_send_connection_request_frame);

	/* create a connection request frame */
	connection_request_MSDU = op_pk_create_fmt ("wban_connection_request_frame_format");
	
	/* set the fields of the connection_request frame */
	op_pk_nfd_set (connection_request_MSDU, "Allocation Length", conn_req_attr.allocation_length);

	/* create a MAC frame (MPDU) that encapsulates the connection_request payload (MSDU) */
	connection_request_MPDU = op_pk_create_fmt ("wban_mac_pkt_format");

	op_pk_nfd_set (connection_request_MPDU, "Ack Policy", I_ACK_POLICY);
	op_pk_nfd_set (connection_request_MPDU, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (connection_request_MPDU, "Frame Subtype", CONNECTION_REQUEST);
	op_pk_nfd_set (connection_request_MPDU, "Frame Type", MANAGEMENT);
	op_pk_nfd_set (connection_request_MPDU, "B2", 1); // beacon2 enabled

	op_pk_nfd_set (connection_request_MPDU, "Sequence Number", 0);
	op_pk_nfd_set (connection_request_MPDU, "Inactive", beacon_attr.inactive_duration); // connection_request and connection_request2 frame used

	op_pk_nfd_set (connection_request_MPDU, "Recipient ID", mac_attr.recipient_id);
	op_pk_nfd_set (connection_request_MPDU, "Sender ID", mac_attr.sender_id);
	op_pk_nfd_set (connection_request_MPDU, "BAN ID", mac_attr.ban_id);
	
	op_pk_nfd_set_pkt (connection_request_MPDU, "MSDU", connection_request_MSDU); // wrap connection_request payload (MSDU) in MAC Frame (MPDU)

	/* create PHY frame (PPDU) that encapsulates connection_request MAC frame (MPDU) */
	connection_request_PPDU = op_pk_create_fmt("wban_frame_PPDU_format");

	op_pk_nfd_set (connection_request_PPDU, "RATE", node_attr.data_rate);
	/* wrap connection_request MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (connection_request_PPDU, "PSDU", connection_request_MPDU);
	op_pk_nfd_set (connection_request_PPDU, "LENGTH", ((double) op_pk_total_size_get(connection_request_MPDU))/8); //[bytes]	
	
	wpan_battery_update_tx ((double) op_pk_total_size_get(connection_request_PPDU));
	//op_pk_send (connection_request_PPDU, STRM_FROM_MAC_TO_RADIO);
	
	/* Stack tracing exit point */
	FOUT;
}

/*-----------------------------------------------------------------------------
 * Function:	wban_extract_data_frame
 *
 * Description:	extract the data frame from the MAC frame received from the network
 *              
 * Input :  frame_MPDU - the received MAC frame
 *-----------------------------------------------------------------------------*/

static void wban_extract_data_frame(Packet* frame_MPDU) {

	//int Ack_Request;
	//int seq_num;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_data_frame);
	
	/* check if any ACK is requested */
	
	/* Stack tracing exit point */
	FOUT;	
}
