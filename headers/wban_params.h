/** PHY Layer constatns		**/
#define aMaxPHYPacketSize_Octet 	127					// size of PSDU
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

// constants related to Intra-Frame Spacing IFS 
#define aMaxSIFSFrameSize_Bits	(18*8)
#define aMinLIFSPeriod	40
#define aMinSIFSPeriod	12

#define aGTSDescPersistenceTime	4
#define aMinCAPLength	440	//Symbols


/** MAC Layer attributes		**/
#define macAckWaitDuration 54	// The max number of symbols to wait for an ACK



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

// WPAN bit rate [bps]
#define WPAN_DATA_RATE 250000.0

// broadcast address corresponds to 0xFFFF
#define BROADCAST_ADDRESS 65535

// temporary address of PAN coordinator (Traffic Destination MAC Adrress)
#define	PAN_COORDINATOR_ADDRESS	-1

#define Symbol2Bits 4

// Frame Control Field - Dest./Src. addressing mode = 10b (Address field contains a 16  bit short address) - [bits] MHR+MFR
#define MAC_HEADER_SIZE 104	

// Frame Control Field - Src. addressing mode = 10b (Address field contains a 16  bit short address) - [bits] MHR+MFR
#define BEACON_HEADER_SIZE 72

// Physical layer header (SHR+PHR) [bits]
#define PHY_HEADER_SIZE 48

// Beacon frame (MAC payload MSDU) size with length of beacon payload set to 0 [bits]
#define BEACON_MSDU_SIZE 32

// ACK frame (MPDU) size [bits]
#define ACK_FRAME_SIZE_BITS 40

