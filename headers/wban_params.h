#ifndef BAN_PARAMS_H
#define BAN_PARAMS_H

/* Misc */
#define ESSENTIAL   0
#define VERBOSE     1
/** PHY Layer constatns		**/
#define aMaxPHYPacketSize_Octet 	127								// size of PSDU
#define aMaxPHYPacketSize_Bits 		(8*aMaxPHYPacketSize_Octet)		// 1016 bits
#define aMaxPHYPacketSize_Symbols 	(2*aMaxPHYPacketSize_Octet)		// 254 Symbols
#define aTurnaroundTime_Symbol 		12

/** MAC Layer constants		**/
#define aNumSuperframeSlots 16
#define aBaseSlotDuration	60 	// The number of symbols forming a superframe slot for a superframe order equal to 0
#define aMaxBE	5
#define aMaxBeaconOverhead	75
#define aMaxBeaconPayloadLength_Octet 	52
#define aBaseSuperframeDuration	(aNumSuperframeSlots*aBaseSlotDuration)
#define aUnitBackoffPeriod	20
#define aMaxFrameRetries	3
#define aMaxMACFrameSize_Bits  (aMaxPHYPacketSize_Bits-MAC_HEADER_SIZE)	//MAC Frame Payload (MSDU) size
// 802.15.6 related 
#define BeaconPeriodLength 64
#define pMaxFrameBodyLength_Bits (255*8) //0-255 octets
#define aMaxBeaconPeriodLength_Slots 256 //0-256 Slots
//the length of an allocation slot is equal to pAllocationSlotMin + L × pAllocationSlotResolution
#define pAllocationSlotMin 500 //500 μs for NB PHY
#define pAllocationSlotResolution 500 //500 μs for NB PHY
#define allocationSlotLength 0 //for NB PHY, L=3 means that 2 ms per slot
#define allocationSlotLength2ms ((pAllocationSlotMin + allocationSlotLength*pAllocationSlotResolution) * 0.001) //2 ms default

// 802.15.6 PHY-dependent MAC sublayer for narrowband PHY
// Symbol Rate
#define SYMBOL_RATE 600000.0 // 600 Ksps
#define pSIFS (75 * 0.000001) // μs
#define LOG_M 2 //QPSK Modulation
#define N_preamble 90 // 90 bits
#define N_header 31 //31 bits
#define S_header 4 // spreading factor for PLCP header
#define MILLI 0.001
#define MICRO 0.000001
#define BCH_CODE (51.0/63)
#define T_PHY ((N_preamble + S_header*N_header)/SYMBOL_RATE)
// CSMA/CA
// pCSMASlotLength = pCCATime + pCSMAMACPHYTime
// pCCATime = 63 / Symbol Rate
// pCSMAMACPHYTime = 40 μs
#define pCCATime (63.0/SYMBOL_RATE) // ms
#define pCSMAMACPHYTime (40 * 0.000001) // 40 μs
#define pCSMASlotLength2Sec (pCCATime + pCSMAMACPHYTime)
/* 802.15.6 related MAC parameters */

/** MAC Layer attributes		**/
#define macAckWaitDuration 54	// The max number of symbols to wait for an ACK
#define mTimeOut 30 // 30 μs

/** Frame Types Definitions according to the standard IEEE 802.15.4 2003 - p.112, Tab.65 **/
#define BEACON_FRAME_TYPE 0
#define DATA_FRAME_TYPE 1
#define ACK_FRAME_TYPE 2
#define COMMAND_FRAME_TYPE 3

/** Command frame identifiers **/
#define ASSOCIATION_REQ 1
#define ASSOCIATION_RES 2
#define DISASSOCIATION_NOT 3
#define DATA_REQ 4
#define PAN_ID_CONFLICT_NOT 5
#define ORPHAN_NOT 6
#define BEACON_REQ 7
#define COORDINATOR_REALIGMNET 8
#define GTS_REQ 9

// WBAN bit rate [bps]
#define WBAN_DATA_RATE 971400.0
#define WBAN_DATA_RATE_KBITS (WBAN_DATA_RATE/1000.0)

/* Node configuration constants     */
#define DELAY_INFINITY 1000.0

// broadcast address corresponds to 0xFFFF
#define BROADCAST_ADDRESS 65535

// temporary ID of HUB (Traffic Destination ID)
#define	HUB_ID	-1

// max nodes in a wban
#define NODE_MAX 10

// Abbreviated addressing related to 802.15.6  
#define UNCONNECTED_BROADCAST_NID 0  // For broadcast to unconnected nodes
#define UNCONNECTED_NID 1 // For unicast from/to unconnected nodes in a BAN
#define BROADCAST_NID 255 //For broadcast to all nodes and hubs
//#define HID 31 				//Hub identifier for a BAN
//int unconnectedNID; // start assigning unconnectedNID from ID 1
#define AUTO_ASSIGNED_NID -2 //NID auto assignment from HUB
#define UNCONNECTED -1 //unconnected status

// Frame Control Field - Dest./Src. addressing mode = 10b (Address field contains a 16  bit short address) - [bits] MHR+MFR
#define MAC_HEADER_SIZE 104	

// Frame Control Field - Src. addressing mode = 10b (Address field contains a 16  bit short address) - [bits] MHR+MFR
#define BEACON_HEADER_SIZE 72

// Physical layer header [bits]
#define PHY_HEADER_SIZE 121

// Beacon frame (MAC payload MSDU) size with length of beacon payload set to 0 [bits]
#define BEACON_MSDU_SIZE 32

// ACK frame (MPDU) size [bits]
#define ACK_FRAME_SIZE_BITS 40
#define I_ACK_PPDU_SIZE_BITS (72+121)
#define BEACON_PPDU_BITS 321

#define MAC_STATE_ALL 10
// int i_test = 0;

/* USER_PRIORITY for DATA */
#define UP0     0
#define UP1     1
#define UP2     2
#define UP3     3
#define UP4     4
#define UP5     5
#define UP6     6
#define UP7     7
/* use the UP_ALL to replace constant 8 */
#define UP_ALL  8
/* Generate, Sent, Receive STAT for DATA */
#define GEN     0
#define QUEUE_SUCC 1
#define QUEUE_FAIL 2
#define SENT    3
#define RCV     4
#define SUBQ    5
/* use the DATA_STATE to replace constant 6 */
#define DATA_STATE 6

/** APP Layer constants		**/
// enum USER_PRIORITY {
// 	UP0 = 0,
// 	UP1 = 1,
// 	UP2 = 2,
// 	UP3 = 3,
// 	UP4 = 4,
// 	UP5 = 5,
// 	UP6 = 6,
// 	UP7 = 7
// };

enum MAC_STATES {
    MAC_SETUP = 1000,
    MAC_EAP1 = 1001,
    MAC_RAP1 = 1002,
    MAC_MAP1 = 1003,
    MAC_EAP2 = 1004,
    MAC_RAP2 = 1005,
    MAC_MAP2 = 1006,
    MAC_CAP = 1007,
    MAC_SLEEP = 1008,
    MAC_RAP = 1009,
    MAC_FREE_TX_ACCESS = 1012,
    MAC_FREE_RX_ACCESS = 1013,
    MAC_BEACON_WAIT = 1018,
    CONN_SETUP = 1019
};

enum SF_STATES {
    SF_SLEEP = 1000,
    SF_EAP1 = 1001,
    SF_RAP1 = 1002,
    SF_MAP1 = 1003,
    SF_EAP2 = 1004,
    SF_RAP2 = 1005,
    SF_MAP2 = 1006,
    SF_CAP = 1007,
    SF_RAP = 1017,
    SF_FREE_TX_ACCESS = 1012,
    SF_FREE_RX_ACCESS = 1013
};

enum AcknowledgementPolicy_type {
    N_ACK_POLICY = 1,
    I_ACK_POLICY = 2,
    L_ACK_POLICY = 3,
    B_ACK_POLICY = 4
};

enum Frame_type {
    MANAGEMENT = 1,
    CONTROL = 2,
    DATA = 3
};

enum Frame_subtype {
    RESERVED = 0,
    BEACON = 1,
    SECURITY_ASSOCIATION = 2,
    SECURITY_DISASSOCIATION = 3,
    PTK = 4,
    GTK = 5,
    CONNECTION_REQUEST = 6,
    CONNECTION_ASSIGNMENT = 7,
    DISCONNECTION = 8,
    COMMAND = 9,
    I_ACK = 10,
    B_ACK = 11,
    I_ACK_POLL = 12,
    B_ACK_POLL = 13,
    POLL = 14,
    T_POLL = 15,
    WAKEUP = 16,
    BEACON2 = 17
};

#endif