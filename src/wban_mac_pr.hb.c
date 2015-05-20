/* Include files		*/
#include "headers\wban_struct.h"
#include "headers\wban_params.h"
#include "headers\wban_math.h"
#include <math.h>
#include <stdlib.h>
#include <io.h>
#include <stdio.h>


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
#define SUBQ_MAN 1
#define SUBQ_COM 2

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
#define INCREMENT_SLOT 3
#define MAP1_SCHEDULE  4
#define TRY_PACKET_TRANSMISSION_CODE 23
#define TRY_PROCESS_LAST_PACKET_CODE 25
#define SEND_I_ACK 26
#define N_ACK_PACKET_SENT 24
#define BACKOFF_EXPIRATION_CODE 17

#define START_OF_EAP1_PERIOD_CODE 50
#define START_OF_RAP1_PERIOD_CODE 51
#define START_OF_MAP1_PERIOD_CODE 52
#define START_OF_EAP2_PERIOD_CODE 53
#define START_OF_RAP2_PERIOD_CODE 54
#define START_OF_MAP2_PERIOD_CODE 55
#define START_OF_CAP_PERIOD_CODE 56
#define START_OF_SLEEP_PERIOD 57
#define UPDATE_OF_CONN_ASSGIN 58
#define SEND_B2_FRAME 27
#define SEND_CONN_REQ_CODE 28
#define SEND_CONN_ASSIGN_CODE 29

// the same value should be in the Battery Module for remote process
#define PACKET_TX_CODE 101
#define PACKET_RX_CODE 102
#define CCA_CODE 107
#define END_OF_SLEEP_PERIOD_CODE 103
#define START_OF_SLEEP_PERIOD_CODE 104
#define END_OF_CAP_PERIOD_CODE 105
#define END_OF_SIM 106
#define END_OF_EAP1_PERIOD_CODE 60
#define END_OF_RAP1_PERIOD_CODE 61
#define END_OF_MAP1_PERIOD_CODE 62
#define END_OF_EAP2_PERIOD_CODE 63
#define END_OF_RAP2_PERIOD_CODE 64
#define END_OF_MAP2_PERIOD_CODE 65

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
#define WAIT_CONN_ASSIGN_END_CODE 212
#define END_PK_IN_MAP 213

#define TRY_MAP_PACKET_TRANSMISSION_CODE 230


/** Global variables	**/

/* attributes of the node (Device Mode, BAN ID, NID, ...) */
wban_node_attributes nd_attrG[NODE_ALL_MAX];
/* attributes of the Battery(Energy) module */
wban_battery_attributes bat_attrG[NODE_ALL_MAX];

//extern double PPDU_sent_bits;	// Total number of bits (PPDU) dispatched to the network [kbits]
int current_free_connected_NID = 32; // start assigning connected NID from ID 32
int current_first_free_slot = -1;
int unconnectedNID;
//int sequence_num_beaconG = 0; //sequence number for beacon frame, global variable
int SF_slot[BeaconPeriodLength];

/* SINR received by Node(Hub -> Node) */
double	snr_node[NODE_ALL_MAX][SF_NUM];

// enum MAC_STATES mac_state = MAC_SETUP; //initialize MAC states with MAC_SETUP
enum SF_STATES sf_state = SF_SLEEP; //initialize Superframe states with SF_SLEEP
/* IEEE 802.15.6 Std. */
// static int CWmin[8] = {16, 16, 8,  8,  4,  4, 2, 1};
// static int CWmax[8] = {64, 32, 32, 16, 16, 8, 8, 4};
static int CWmin[8] = {16, 16, 8,  8,  4,  16, 2, 1};
static int CWmax[8] = {64, 32, 32, 16, 16, 64, 8, 4};

/* assignment slot map, 50 node max */
map1_schedule_map map1_sche_map[NODE_ALL_MAX];

/* Global variables*/
data_stat_info data_stat_all[UP_ALL][DATA_STATE];

/* State machine conditions */
#define IAM_BAN_HUB (nd_attrG[nodeid].dev_mode == HUB)

#define IAM_WAITING_ACK (mac_attr.wait_ack==OPC_TRUE)

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
static void wban_extract_i_ack_frame(Packet* ack_frame);
static void wban_extract_data_frame (Packet* mac_frame);
static void wban_schedule_next_beacon (void);
static void wban_send_beacon_frame (void);
void map1_scheduling(void);
static void wban_send_ack_frame (int sequence_number);
static void wban_send_i_ack_frame (int seq_num);
static void wban_send_mac_pk_to_phy(Packet* frame_MPDU);
static void wban_mac_interrupt_process (void);
static void wban_attempt_TX(void);
static void wban_attempt_TX_CSMA(void);

// backoff functions
static void wban_backoff_delay_set( int user_priority);
static double wban_backoff_period_boundary_get(void);

// battery(energy) module
static void wban_battery_init();
static void wban_battery_update_tx (double tx_timeL, int mac_state);
static void wban_battery_update_rx (double rx_timeL, int mac_state);
static void wban_battery_cca(int mac_state);
static void wban_battery_sleep_start(int mac_state);
static void wban_battery_sleep_end(int mac_state);

// utility functions
static void queue_status (void);
static void cap_is_not_active (void);
static Boolean is_packet_for_me(Packet* frame_MPDU, int ban_id, int recipient_id, int sender_id);
static Boolean can_fit_TX(packet_to_be_sent_attributes* pkt_to_be_sentL);
static void subq_info_get (int subq_index);
static void subq_data_info_get(void);
static int wban_norm_phy_bits(Packet* frame_MPDU);
static void phy_to_radio(Packet* frame_MPDU);
void calc_prio_node(void);
void calc_prio_hub(void);
static int hp_rfind_nodeid (int nid);
static double avg_snr_hub(int node_id_l);
static void reset_map1_scheduling(int seq);

static double hp_tx_time (int ppdu_bits);