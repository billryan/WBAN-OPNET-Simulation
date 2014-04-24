/* the attributes of the node (Device Mode, BAN ID, ...) */
wban_node_attributes	\node_attributes;

/* the parameters of the superframe structure */
wban_superframe_strucuture	\SF;

/* connection request attributes of Node or Hub */
connection_request_attributes \connection_request_attributes;

/* the Medium Access Attributes */
wban_mac_attributes	\mac_attributes;

/* a log file to store the operations for each node */
FILE *	\log;

/* if enabled, all the operation will be saved in a log file */
Boolean	\enable_log;

/* the statistic vector */
wban_statistic_vector	\statistic_vector;

wban_global_statistic_vector	\statistic_global_vector;

/* sequence number of the received packet */
int	\ack_sequence_number;

/* the time when the packet enters the backoff */
double	\backoff_start_time;

/* MAP request was issued after reception of first beacon               */
/* then check if Hub accepts Connection request */
Boolean	\waiting_for_first_beacon;

/*--------------------------------------------------------------------------------
 * Function:	wban_mac_init
 *
 * Description:	- initialize the process
 *				- read the attributes and set the global variables
 *
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_mac_init() {

	Objid wban_attributes_comp_id;
	Objid wban_attributes_id;
	Objid connection_request_attributes_comp_id;
	Objid connection_request_attributes_id;
	Objid csma_attributes_comp_id;
	Objid csma_attributes_id;
	Objid mac_attributes_comp_id;
	Objid mac_attributes_id;
	Objid gts_attributes_comp_id;
	Objid gts_attributes_id;
	Objid traffic_source_up_id;
	Objid queue_objid;
	Objid subq_objid;
	
	/* Stack tracing enrty point */
	FIN(wban_mac_init);
	
	/* obtain self object ID of the surrounding processor or queue */
	node_attributes.objid = op_id_self ();
	
	/* obtain object ID of the parent object (node) */
	node_attributes.parent_id = op_topo_parent (node_attributes.objid);
	
	/* obtain object ID of the Traffic Source node */
	traffic_source_up_id = op_id_from_name(node_attributes.parent_id, OPC_OBJTYPE_PROC, "Traffic Source_UP");
	
	/* obtain destination ID for data transmission  */
	op_ima_obj_attr_get (traffic_source_up_id, "Destination ID", &node_attributes.traffic_destination_id);	
	
	/*get the battery attribute ID*/
	node_attributes.my_battery = op_id_from_name (node_attributes.parent_id, OPC_OBJTYPE_PROC, "Battery");

	if (node_attributes.my_battery == OPC_NIL) {
		op_sim_end("CANNOT FIND THE BATTERY ID","CHECK IF THE NAME OF THE BATTERY MODULE IS [Battery]","","");
	}
	
	/* get the Sender Address of the node */
	op_ima_obj_attr_get (node_attributes.parent_id, "Sender Address", &node_attributes.sender_address);

	/* Sender Address is not specified - Auto Assigned frome node objid */
	if (node_attributes.sender_address == -2) {
		node_attributes.sender_address = node_attributes.parent_id;
	}

	/* get the name of the node */
	op_ima_obj_attr_get (node_attributes.parent_id, "name", &node_attributes.name);
	
	/* get the geographic position of the node */
	op_ima_obj_attr_get (node_attributes.parent_id, "x position", &node_attributes.x);
	op_ima_obj_attr_get (node_attributes.parent_id, "y position", &node_attributes.y);
	op_ima_obj_attr_get (node_attributes.parent_id, "altitude", &node_attributes.altitude);	
	
	op_ima_obj_attr_get (node_attributes.parent_id, "WBAN DATA RATE", &node_attributes.data_rate)

	/* get the value of the BAN ID */
	op_ima_obj_attr_get (node_attributes.parent_id, "BAN ID", &node_attributes.ban_id);

	node_attributes.connectedHID = node_attributes.ban_id + 15; // set the value of HID=BAN ID + 15

	/* get the value to check if this node is Hub or not */
	op_ima_obj_attr_get (node_attributes.parent_id, "Device Mode", &node_attributes.Device_Mode);

	node_attributes.is_BANhub = OPC_FALSE;
	if (strcmp(node_attributes.Device_Mode, "Hub") == 0) {
		node_attributes.is_BANhub = OPC_TRUE;	
	}

	if (node_attributes.is_BANhub == OPC_TRUE) {
		node_attributes.sender_id = node_attributes.connectedHID;
		node_attributes.recipient_id = BROADCAST_NID; // default value, usually overwritten

		/* get the WBAN settings for the Hub */
		op_ima_obj_attr_get (node_attributes.parent_id, "WBAN Setting", &wban_attributes_id);
		wban_attributes_comp_id = op_topo_child (wban_attributes_id, OPC_OBJTYPE_GENERIC, 0);

		op_ima_obj_attr_get (wban_attributes_comp_id, "Beacon Period Length", &node_attributes.beacon_period_length);
		op_ima_obj_attr_get (wban_attributes_comp_id, "Allocation Slot Length", &node_attributes.allocation_slot_length);
		op_ima_obj_attr_get (wban_attributes_comp_id, "RAP1 Start", &node_attributes.rap1_start);
		op_ima_obj_attr_get (wban_attributes_comp_id, "RAP1 End", &node_attributes.rap1_end);
		op_ima_obj_attr_get (wban_attributes_comp_id, "RAP2 Start", &node_attributes.rap2_start);
		op_ima_obj_attr_get (wban_attributes_comp_id, "RAP2 End", &node_attributes.rap2_end);
		op_ima_obj_attr_get (wban_attributes_comp_id, "Inactive Duration", &node_attributes.inactive_duration);

		node_attributes.current_first_free_slot = node_attributes.rap1_end + 1;
	} else { /* if the node is not a Hub */
		node_attributes.sender_id = UNCONNECTED;
		node_attributes.recipient_id = UNCONNECTED;

		/* get the WBAN settings for the Hub */
		op_ima_obj_attr_get (node_attributes.parent_id, "Connection Request", &connection_request_attributes_id);
		connection_request_attributes_comp_id = op_topo_child (connection_request_attributes_id, OPC_OBJTYPE_GENERIC, 0);

		op_ima_obj_attr_get (connection_request_attributes_comp_id, "Beacon Period Length", &node_attributes.beacon_period_length);
		op_ima_obj_attr_get (connection_request_attributes_comp_id, "Allocation Slot Length", &node_attributes.allocation_slot_length);
		op_ima_obj_attr_get (connection_request_attributes_comp_id, "RAP1 Start", &node_attributes.rap1_start);
		op_ima_obj_attr_get (connection_request_attributes_comp_id, "RAP1 End", &node_attributes.rap1_end);
		op_ima_obj_attr_get (connection_request_attributes_comp_id, "RAP2 Start", &node_attributes.rap2_start);
		op_ima_obj_attr_get (connection_request_attributes_comp_id, "RAP2 End", &node_attributes.rap2_end);
		op_ima_obj_attr_get (connection_request_attributes_comp_id, "Inactive Duration", &node_attributes.inactive_duration);

		/* start assigning connected NID from ID 32 */
		// node_attributes.sender_id = 32 + ((current_free_connected_NID - 32) % 214);
		// current_free_connected_NID++;
		// node_attributes.recipient_id = node_attributes.connectedHID;

		node_attributes.beacon_period_length = -1;
	}

	SF.current_slot = 0;
	SF.Final_CAP_Slot = TIME_SLOT_INDEX_MAX; /* no CFP (GTSs) */
	SF.RESUME_BACKOFF_TIMER = OPC_FALSE;
	SF.backoff_timer= -1;
	SF.CCA_DEFERRED = OPC_FALSE;
	
	Final_CAP_slot_next = SF.Final_CAP_Slot;
	
	op_ima_obj_attr_get (mac_attributes_comp_id, "Batterie Life Extension", &mac_attributes.Battery_Life_Extension);
	mac_attributes.wait_ack = OPC_FALSE;
	mac_attributes.wait_ack_seq_num = 0;
	
	/* get the CSMA-CA settings */
	op_ima_obj_attr_get (node_attributes.objid, "CSMA-CA Parameters", &csma_attributes_id);
	csma_attributes_comp_id = op_topo_child (csma_attributes_id, OPC_OBJTYPE_GENERIC, 0);
	
	op_ima_obj_attr_get (csma_attributes_comp_id, "Maximum Backoff Number", &csma.MAX_CSMA_BACKOFF);
	op_ima_obj_attr_get (csma_attributes_comp_id, "Minimum Backoff Exponent", &csma.macMinBE);	
	
	/* CSMA initialization	*/
	csma.NB = 0;
	csma.CW = 2;
	csma.BE = csma.macMinBE;
	csma.retries_nbr = 0;	
	csma.CCA_CHANNEL_IDLE = OPC_TRUE;	
	if (mac_attributes.Battery_Life_Extension == OPC_TRUE)
		csma.BE = min_int(2, csma.macMinBE);
	

	op_ima_obj_attr_get (node_attributes.parent_id, "Enable Logging", &enable_log);

	print_mac_attributes ();
	
	wpan_log_file_init ();	
		
	fprint_mac_attributes();
	
	/* Stack tracing exit point */
	FOUT;
}


/*--------------------------------------------------------------------------------
 * Function:	wban_parse_incoming_frame
 *
 * Description:	parse the incoming packet and make the adequate processing
 *
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_parse_incoming_frame() {

	Packet* frame_MPDU;
	Packet* rcv_frame;
	int Stream_ID;
	int ban_id;
	int recipient_id;
	int sender_id;
	int ack_policy_fd;
	int eap_indicator_fd;
	int frame_type_fd;
	int frame_subtype_fd;
	int beacon2_enabled_fd;
	int sequence_number_fd;
	int inactive_fd;
	int source_address;
	int dest_address;
	double packet_size;

	
	/* Stack tracing enrty point */
	FIN(wban_parse_incoming_frame);

	/* get the packet from the input stream	*/
	Stream_ID = op_intrpt_strm();
	rcv_frame = op_pk_get (Stream_ID);	
	
	/* check from what input stream the packet is received and do the right processing	*/		
	switch (Stream_ID) {
		case STRM_FROM_SYNCHRO_TO_MAC: /* A FIRST BEACON PAYLOAD (MSDU) FROM THE SYNCHRO MODULE (only for PANCoordinator) */
		{
			wban_encapsualte_beacon_frame (rcv_frame);
			break;
		};
	
		case STRM_FROM_UNACK_TO_MAC: /* INCOMMING UNACKNOWLEDGED PAYLOAD (MSDU) FROM THE TRAFFIC SOURCE */
		{			
			wpan_encapsulate_and_enqueue_data_frame (rcv_frame, OPC_FALSE, my_attributes.traffic_destination_address);			
			break;
		};
	
		case STRM_FROM_ACK_TO_MAC: /* INCOMMING ACKNOWLEDGED PAYLOAD (MSDU) FROM THE TRAFFIC SOURCE */
		{
			wpan_encapsulate_and_enqueue_data_frame (rcv_frame, OPC_TRUE, my_attributes.traffic_destination_address);			
			break;
		};

		case STRM_FROM_GTS_TO_MAC:	/* INCOMMING GTS PAYLOAD (MSDU) FROM THE GTS TRAFFIC SOURCE */
		{			
			wpan_encapsulate_and_enqueue_gts_frame (rcv_frame);
			break;
		};
	
		case STRM_FROM_RADIO_TO_MAC: /*A PHY FRAME (PPDU) FROM THE RADIO RECIEVER*/
		{
			/*update the battery*/
			packet_size = (double) op_pk_total_size_get(rcv_frame);
			wpan_battery_update_rx (packet_size, frame_type_fd);
			
			op_pk_destroy (rcv_frame);

			/* get MAC frame (MPDU=PSDU) from received PHY frame (PPDU)*/
			op_pk_nfd_get_pkt (rcv_frame, "PSDU", &frame_MPDU);

			op_pk_nfd_get (frame_MPDU, "BAN ID", &ban_id);
			op_pk_nfd_get (frame_MPDU, "Recipient ID", &recipient_id);
			op_pk_nfd_get (frame_MPDU, "Sender ID", &sender_id);

			// filter the incoming BAN packet - not implemented entirely
    		if (!is_packet_for_me(frame_MPDU, ban_id, recipient_id, sender_id)) {
    			FOUT;
    		}

			/*acquire "Frame Type" field*/
			op_pk_nfd_get (frame_MPDU, "Frame Type", &frame_type_fd);
			op_pk_nfd_get (frame_MPDU, "Frame Subtype", &frame_subtype_fd);
			op_pk_nfd_get (frame_MPDU, "Ack Policy", &ack_policy_fd);
			op_pk_nfd_get (frame_MPDU, "EAP Indicator", &eap_indicator_fd);
			op_pk_nfd_get (frame_MPDU, "B2", &beacon2_enabled_fd);
			op_pk_nfd_get (frame_MPDU, "Sequence Number", &sequence_number_fd);
			op_pk_nfd_get (frame_MPDU, "Inactive", &inactive_fd);

			switch (frame_type_fd) {
				case DATA: /* Handle data packets */
					if (enable_log) {
						fprintf (log,"t=%f  !!!!!!!!! Data Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), sender_id);
						printf (" [Node %s] t=%f  !!!!!!!!! Data Frame Reception From @%d !!!!!!!!! \n\n", node_attributes.name, op_sim_time(), sender_id);
					}
					wban_extract_data_frame (frame_MPDU);
					break;
				case MANAGEMENT: /* Handle management packets */
			 		if (enable_log) {
						fprintf (log,"t=%f  !!!!!!!!! Management Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), sender_id);
						printf (" [Node %s] t=%f  !!!!!!!!! Management Frame Reception From @%d !!!!!!!!! \n\n", node_attributes.name, op_sim_time(), sender_id);
					}
					switch (frame_subtype_fd) {
						case BEACON: 
							if (enable_log) {
								fprintf (log,"t=%f  -> Beacon Frame Reception From @%d \n\n", op_sim_time(), sender_id);
								printf (" [Node %s] t=%f  -> Beacon Frame Reception From @%d \n\n", node_attributes.name, op_sim_time(), sender_id);
							}
							wban_extract_beacon_frame (frame_MPDU);
							break;
						case SECURITY_ASSOCIATION: 
							// not implemented
							break;
						case SECURITY_DISASSOCIATION:
							// not implemented
							break;
						case PTK:
							// not implemented
							break;
						case GTK:
							// not implemented
							break;
						case CONNECTION_REQUEST:
							// not implemented
							break;
						case CONNECTION_ASSIGNMENT:
							// not implemented
							break;
						case DISCONNECTION:
							// not implemented
							break;
						case COMMAND:
							// not implemented
							break;
					}
					break;
				case  CONTROL: /* Handle control packets */
					switch (frame_subtype_fd) {
						case I_ACK:
							// not implemented
							break;
						case B_ACK:
							// not implemented
							break;
						case I_ACK_POLL:
							// not implemented
							break;
						case B_ACK_POLL:
							// not implemented
							break;
						case POLL:
							// not implemented
							break;
						case T_POLL:
							// not implemented
							break;
					}
				default:	/*OTHER FRAME TYPES*/
					break;
			}
			/*Check "Frame Type" field*/
			if (frame_type_fd == ACK_FRAME_TYPE) {	/* acknowledgment frame */			
				if (enable_log) {
					fprintf (log,"t=%f  -> ACK Frame Reception - WAITING for ACK? = %s\n\n",op_sim_time(), boolean2string(mac_attributes.wait_ack));
					printf (" [Node %s] t=%f  -> ACK Frame Reception - WAITING for ACK? = %s\n\n",my_attributes.name,op_sim_time(), boolean2string(mac_attributes.wait_ack));
				}
				wpan_extract_ack_frame (frame_MPDU);
			}
			else if (frame_type_fd == BEACON_FRAME_TYPE) { /* beacon frame */
				op_pk_nfd_get (frame_MPDU, "Source Address", &source_address);
				
				if (enable_log) {
					fprintf (log,"t=%f  -> Beacon Frame Reception From @%d \n\n", op_sim_time(), source_address);
					printf (" [Node %s] t=%f  -> Beacon Frame Reception From @%d \n\n", my_attributes.name, op_sim_time(), source_address);
				}
						
				wpan_extract_beacon_frame (frame_MPDU);
			}
			else {   /*if it is not an ACK Frame*/			
				op_pk_nfd_get (frame_MPDU, "Source Address", &source_address);
				op_pk_nfd_get (frame_MPDU, "Destination Address", &dest_address);
			
				/*Check if the frame is loop*/
				if (source_address == my_attributes.mac_address) {
					if (enable_log) {
						fprintf(log,"t=%f  -> Loop: DISCARD FRAME \n\n",op_sim_time());
						printf (" [Node %s] t=%f  -> Loop: DISCARD FRAME \n\n",my_attributes.name, op_sim_time());
					}
					op_pk_destroy (frame_MPDU);
				} 
				else if ((dest_address != my_attributes.mac_address) && (dest_address!= BROADCAST_ADDRESS)) {
					if (enable_log) {
						fprintf (log,"t=%f  -> Frame Reception From @%d \n\n",op_sim_time(), source_address);
						printf (" [Node %s] t=%f  -> Frame Reception From @%d \n\n",my_attributes.name, op_sim_time(), source_address);

						fprintf (log,"t=%f  -> Wrong Destination Address (my_address: %d <-> dest_address: %d) : DISCARD FRAME \n\n",op_sim_time(), my_attributes.mac_address, dest_address);
						printf (" [Node %s] t=%f  -> Wrong Destination Address (my_address: %d <-> dest_address: %d) : DISCARD FRAME \n\n",my_attributes.name, op_sim_time(), my_attributes.mac_address, dest_address);
					}
					op_pk_destroy (frame_MPDU);
				}
				else {	// dest_address = my_attributes.mac_address or dest_address = BROADCAST_ADDRESS 
					switch(frame_type_fd) {						
						case DATA_FRAME_TYPE: 	/* For a data frame */
							if (enable_log) {
								fprintf (log,"t=%f  !!!!!!!!! Data Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), source_address);
								printf (" [Node %s] t=%f  !!!!!!!!! Data Frame Reception From @%d !!!!!!!!! \n\n", my_attributes.name, op_sim_time(), source_address);
							}
							wpan_extract_data_frame (frame_MPDU);
							break;
						case COMMAND_FRAME_TYPE:	/* For a Command frame */
						 	if (enable_log) {
								fprintf (log,"t=%f  !!!!!!!!! Command Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), source_address);
								printf (" [Node %s] t=%f  !!!!!!!!! Command Frame Reception From @%d !!!!!!!!! \n\n", my_attributes.name, op_sim_time(), source_address);
							}
							wpan_extract_command_frame (frame_MPDU);
							break;
						default:	/*OTHER FRAME TYPES*/
							break;					
					}
				}
			}
			break;
		};
	
		default :
		{
		};
	}

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_encapsualte_beacon_frame
 *
 * Description:	encapsulate the first beacon payload (MSDU) received from the sychro module 
 *              into a MAC frame (MPDU) and PHY frame (PPDU), and sends it to the network and schedule the next beacon frame 
 *
 *  Input :  beacon_MSDU - the beacon payload (MSDU) for encapsulation
 *--------------------------------------------------------------------------------*/

static void wban_encapsualte_beacon_frame(Packet* beacon_MSDU) {

	Packet* beacon_MPDU;
	Packet* beacon_PPDU;
	double beacon_frame_creation_time;
	extern int sequence_num_beaconG;

	/* Stack tracing enrty point */
	FIN(wban_encapsualte_beacon_frame);

	/* create a MAC frame (MPDU) that encapsulates the beacon payload (MSDU) */
	beacon_MPDU = op_pk_create_fmt ("wban_mac_pkt_format");

	op_pk_nfd_set (beacon_MPDU, "Ack Policy", N_ACK_POLICY);
	op_pk_nfd_set (beacon_MPDU, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (beacon_MPDU, "Frame Subtype", BEACON);
	op_pk_nfd_set (beacon_MPDU, "Frame Type", MANAGEMENT);
	op_pk_nfd_set (beacon_MPDU, "B2", 1); // beacon2 enabled

	op_pk_nfd_set (beacon_MPDU, "Sequence Number", ((sequence_num_beaconG++) % 256));
	op_pk_nfd_set (beacon_MPDU, "Inactive", node_attributes.inactive_duration); // beacon and beacon2 frame used

	op_pk_nfd_set (beacon_MPDU, "Recipient ID", node_attributes.recipient_id);
	op_pk_nfd_set (beacon_MPDU, "Sender ID", node_attributes.sender_id);
	op_pk_nfd_set (beacon_MPDU, "BAN ID", node_attributes.ban_id);
	
	op_pk_nfd_set_pkt (beacon_MPDU, "MSDU", beacon_MSDU); // wrap beacon payload (MSDU) in MAC Frame (MPDU)

	//op_pk_print(beacon_MPDU); //print information about packet

	beacon_frame_creation_time = op_pk_creation_time_get(beacon_MSDU);

	// update the superframe parameters
	SF.SD = node_attributes.beacon_period_length; // the superframe duration(beacon preriod length) in slots
	SF.BI = node_attributes.beacon_period_length * (1+ node_attributes.inactive_duration); // active and inactive superframe
	SF.sleep_period = SF.SD * node_attributes.inactive_duration;

	SF.slot_length2sec = allocationSlotLength2ms / 1000.0; // transfer allocation slot length from ms to sec.
	SF.rap1_start = node_attributes.rap1_start;
	SF.rap1_end = node_attributes.rap1_end;
	SF.rap2_start = node_attributes.rap2_start;
	SF.rap2_end = node_attributes.rap2_end;

	SF.BI_Boundary = beacon_frame_creation_time;

	if (enable_log) {
		fprintf(log,"t=%f  -> Beacon Frame transmission. \n\n", op_sim_time());
		printf(" [Node %s] t=%f  -> Beacon Frame transmission. \n\n", node_attributes.name, op_sim_time());
	}
	
	op_intrpt_schedule_self (SF.BI_Boundary+Bits2Sec(op_pk_total_size_get(beacon_MPDU), node_attributes.data_rate), START_OF_EAP1_PERIOD_CODE);
	wban_schedule_next_beacon ();
	
	/* create PHY frame (PPDU) that encapsulates beacon MAC frame (MPDU) */
	beacon_PPDU = op_pk_create_fmt("wban_frame_PPDU_format");

	op_pk_nfd_set (beacon_PPDU, "RATE", node_attributes.data_rate);
	/* wrap beacon MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (beacon_PPDU, "PSDU", beacon_MPDU);
	op_pk_nfd_set (beacon_PPDU, "LENGTH", ((double) op_pk_total_size_get(beacon_MPDU))/8); //[bytes]	
	
	wpan_battery_update_tx ((double) op_pk_total_size_get(beacon_PPDU));
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
	
	op_intrpt_schedule_self (SF.BI_Boundary + SF.rap1_start*SF.slot_length2sec, END_OF_EAP1_PERIOD_CODE);
	op_intrpt_schedule_self (SF.BI_Boundary + (SF.rap1_end+1)*SF.slot_length2sec, END_OF_RAP1_PERIOD_CODE);
	op_intrpt_schedule_self (SF.BI_Boundary + SF.rap2_start*SF.slot_length2sec, END_OF_EAP2_PERIOD_CODE);
	op_intrpt_schedule_self (SF.BI_Boundary + (SF.rap2_end+1)*SF.slot_length2sec, END_OF_RAP2_PERIOD_CODE);
	
	//op_intrpt_priority_set (OPC_INTRPT_SELF, BEACON_INTERVAL_CODE, -2);
	
	op_intrpt_schedule_self (SF.BI_Boundary + SF.BI*SF.slot_length2sec, BEACON_INTERVAL_CODE);
	
	op_intrpt_schedule_remote (SF.BI_Boundary + SF.BI*SF.slot_length2sec, END_OF_SLEEP_PERIOD, node_attributes.my_battery);

	if (enable_log) {
		fprintf (log,"t=%f  -> Schedule Next Beacon at %f - End of RAP1 : %f\n\n", op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec, SF.BI_Boundary+(SF.rap1_end+1)*SF.slot_length2sec);
		printf (" [Node %s] t=%f  -> Schedule Next Beacon at %f - End of RAP1 : %f\n\n", node_attributes.name, op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec, SF.BI_Boundary+(SF.rap1_end+1)*SF.slot_length2sec);
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

static Boolean is_packet_for_me(Packet* frame_MPDU, int ban_id, int recipient_id, int sender_id)
{
	/* Stack tracing enrty point */
	FIN(is_packet_for_me);
	

	if (node_attributes.ban_id != ban_id) {
		if (enable_log) {
			fprintf (log,"The packet from BAN ID %d is not the same with my BAN ID %d.\n", ban_id, node_attributes.ban_id);
			printf (" [Node %s] The packet from BAN ID %d is not the same with my BAN ID %d.\n", node_attributes.name, ban_id, node_attributes.ban_id);
		}

		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}

	/*Check if the frame is loop*/
	if (node_attributes.sender_id == sender_id) {
		if (enable_log) {
			fprintf(log,"t=%f  -> Loop: DISCARD FRAME \n\n",op_sim_time());
			printf (" [Node %s] t=%f  -> Loop: DISCARD FRAME \n\n",node_attributes.name, op_sim_time());
		}
		op_pk_destroy (frame_MPDU);
	}

	if ((node_attributes.sender_id == recipient_id) || (BROADCAST_NID == recipient_id)) {
		/* Stack tracing exit point */
		FRET(OPC_TRUE);
	} else {
		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}

}

 
/*--------------------------------------------------------------------------------
 * Function:	wban_extract_beacon_frame
 *
 * Description:	extract the beacon frame from the MAC frame received from the network
 *              and schedule the next beacon frame
 *
 * Input :  mac_frame - the received MAC frame
 *--------------------------------------------------------------------------------*/

static void wban_extract_beacon_frame(Packet* mac_frame){

	Packet* beacon_frame;
	PrgT_List* gts_list;
	wpan_gts_descriptor* gts_descriptor_ptr;
	Boolean is_it_from_PAN_Coord;
	int rcv_sender_id;
	double beacon_frame_creation_time;
	double beacon_TX_time;
	int gts_directions_mask;
	int gts_descriptor_count;
	int gts_length;
	int i;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_beacon_frame);

	op_pk_nfd_get_pkt (mac_frame, "MSDU", &beacon_frame);

	op_pk_nfd_set (beacon_MPDU, "Ack Policy", N_ACK_POLICY);
	op_pk_nfd_set (beacon_MPDU, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (beacon_MPDU, "Frame Subtype", BEACON);
	op_pk_nfd_set (beacon_MPDU, "Frame Type", MANAGEMENT);
	op_pk_nfd_set (beacon_MPDU, "B2", 1); // beacon2 enabled

	op_pk_nfd_set (beacon_MPDU, "Sequence Number", ((sequence_num_beaconG++) % 256));
	op_pk_nfd_set (beacon_MPDU, "Inactive", node_attributes.inactive_duration); // beacon and beacon2 frame used

	op_pk_nfd_set (beacon_MPDU, "Recipient ID", node_attributes.recipient_id);
	op_pk_nfd_set (beacon_MPDU, "Sender ID", node_attributes.sender_id);
	op_pk_nfd_set (beacon_MPDU, "BAN ID", node_attributes.ban_id);
	
	// if I'm a End Device, I get the information and sychronize myself
	if (!node_attributes.is_BANhub) {
		op_pk_nfd_get (mac_frame, "Sender ID", &rcv_sender_id);
		
		if (node_attributes.ban_id + 15 == rcv_sender_id) {
			op_pk_nfd_get (beacon_frame, "Sender Address", &node_attributes.sender_address);
			op_pk_nfd_get (beacon_frame, "Beacon Period Length", &node_attributes.beacon_period_length);
			op_pk_nfd_get (beacon_frame, "Allocation Slot Length", &node_attributes.allocation_slot_length);
			op_pk_nfd_get (beacon_frame, "RAP1 End", &node_attributes.rap1_end);
			op_pk_nfd_get (beacon_frame, "RAP2 Start", &node_attributes.rap2_start);
			op_pk_nfd_get (beacon_frame, "RAP2 End", &node_attributes.rap2_end);
			op_pk_nfd_get (beacon_frame, "RAP1 Start", &node_attributes.rap1_start);
			op_pk_nfd_get (beacon_frame, "Inactive Duration", &node_attributes.inactive_duration);

			// update with actual Hub address - since we fixed it on init
			// if (my_attributes.traffic_destination_address == PAN_COORDINATOR_ADDRESS)
			// 	my_attributes.traffic_destination_address = my_attributes.PANcoordinator_mac_address;
			
			beacon_frame_creation_time = op_pk_creation_time_get (beacon_frame);
			
			if (waiting_for_first_beacon) {
				op_intrpt_schedule_self (op_sim_time(), START_OF_GTS_USE);
			}

			// update the superframe parameters
			SF.BI_Boundary = beacon_frame_creation_time;
			SF.SD = node_attributes.beacon_period_length; // the superframe duration(beacon preriod length) in slots
			SF.BI = node_attributes.beacon_period_length * (1+ node_attributes.inactive_duration); // active and inactive superframe
			SF.sleep_period = SF.SD * node_attributes.inactive_duration;

			SF.slot_length2sec = allocationSlotLength2ms / 1000.0; // transfer allocation slot length from ms to sec.
			SF.rap1_start = node_attributes.rap1_start;
			SF.rap1_end = node_attributes.rap1_end;
			SF.rap2_start = node_attributes.rap2_start;
			SF.rap2_end = node_attributes.rap2_end;
			
			beacon_TX_time = Bits2Sec(op_pk_total_size_get(mac_frame), node_attributes.data_rate);

			if (enable_log) {
				printf (" [Node %s] t=%f  -> Beacon Frame Reception - synchronization. \n", my_attributes.name, op_sim_time());
				printf ("   -> BEACON ORDER (BO)     : %d \n", my_attributes.beacon_order);
				printf ("   -> SUPERFRAME ORDER (SO) : %d \n", my_attributes.superframe_order);
				printf ("   -> PAN ID                : %d \n", my_attributes.pan_id);
				printf ("   -> PAN COORDINATOR MAC ADDRESS  : %d \n", my_attributes.PANcoordinator_mac_address);
				printf ("   -> BEACON INTERVAL BOUNDARY     : %f Sec \n", SF.BI_Boundary);
				printf ("   -> SUPERFRAME DURATION          : %d Symbols = %f Sec \n", SF.SD, Symbols2Sec(SF.SD, WPAN_DATA_RATE));
				printf ("   -> BEACON INTERVAL              : %d Symbols = %f Sec\n", SF.BI, Symbols2Sec(SF.BI, WPAN_DATA_RATE));
				printf ("   -> TIME SLOT DURATION           : %d Symbols = %f Sec \n", SF.slot_duration, Symbols2Sec(SF.slot_duration, WPAN_DATA_RATE));
				printf ("   -> FINAL CAP SLOT (0-15)        : %d \n", SF.Final_CAP_Slot);
				printf ("   -> BATT LIFE EXTENSION          : %s \n\n", boolean2string(mac_attributes.Battery_Life_Extension));	
				printf ("   -> GTS PERMIT by PANCoordinator : %s \n", boolean2string(my_attributes.is_GTS_Permit));
				if (GTS.start_slot > 0) {
					printf ("   -> ALLOCATED GTS START SLOT     : %d \n", GTS.start_slot);
					printf ("   -> ALLOCATED LENGTH OF THE GTS  : %d slots = %f sec \n", GTS.length, Symbols2Sec(GTS.length*SF.slot_duration, WPAN_DATA_RATE));
				} else {
					printf ("   -> NONE ALLOCATED GTS.\n");
				}
				printf ("\n");
			
				fprintf (log,"t=%f  -> Beacon Frame Reception - synchronization. \n",op_sim_time());
				fprintf (log,"   -> BEACON ORDER (BO)     : %d \n", my_attributes.beacon_order);
				fprintf (log,"   -> SUPERFRAME ORDER (SO) : %d \n", my_attributes.superframe_order);
				fprintf (log,"   -> PAN ID                : %d \n", my_attributes.pan_id);
				fprintf (log,"   -> PAN COORDINATOR MAC ADDRESS  : %d \n", my_attributes.PANcoordinator_mac_address);
				fprintf (log,"   -> BEACON INTERVAL BOUNDARY     : %f Sec \n", SF.BI_Boundary);
				fprintf (log,"   -> SUPERFRAME DURATION          : %d Symbols = %f Sec \n", SF.SD, Symbols2Sec(SF.SD, WPAN_DATA_RATE));
				fprintf (log,"   -> BEACON INTERVAL              : %d Symbols = %f Sec\n", SF.BI, Symbols2Sec(SF.BI, WPAN_DATA_RATE));
				fprintf (log,"   -> TIME SLOT DURATION           : %d Symbols = %f Sec \n", SF.slot_duration, Symbols2Sec(SF.slot_duration, WPAN_DATA_RATE));
				fprintf (log,"   -> FINAL CAP SLOT (0-15)        : %d \n", SF.Final_CAP_Slot);
				fprintf (log,"   -> BATT LIFE EXTENSION          : %s \n\n", boolean2string(mac_attributes.Battery_Life_Extension));
				fprintf (log,"   -> GTS PERMIT by PANCoordinator : %s \n", boolean2string(my_attributes.is_GTS_Permit));
				if (GTS.start_slot > 0) {
					fprintf (log, "   -> ALLOCATED GTS START SLOT     : %d \n", GTS.start_slot);
					fprintf (log, "   -> ALLOCATED LENGTH OF THE GTS  : %d slots = %f sec \n", GTS.length, Symbols2Sec(GTS.length*SF.slot_duration, WPAN_DATA_RATE));
				} else {
					fprintf (log, "   -> NONE ALLOCATED GTS.\n");
				}
				fprintf (log, "\n");
			}
			
			/* schdedule the MAP data period - allocated MAP slot(s) in the End Device node*/
			if (GTS.start_slot > 0) {
				op_intrpt_schedule_self (SF.BI_Boundary+Symbols2Sec(((GTS.start_slot)*SF.slot_duration), WPAN_DATA_RATE), START_OF_GTS_PERIOD);
				op_intrpt_schedule_self (SF.BI_Boundary+Symbols2Sec(((GTS.start_slot+GTS.length)*SF.slot_duration), WPAN_DATA_RATE), END_OF_GTS_PERIOD);

				if (enable_log) {
					fprintf (log, "t=%f  -> Schedule the GTS data period from %f sec to %f sec. \n\n", op_sim_time(), SF.BI_Boundary+Symbols2Sec(((GTS.start_slot)*SF.slot_duration), WPAN_DATA_RATE), SF.BI_Boundary+Symbols2Sec(((GTS.start_slot+GTS.length)*SF.slot_duration), WPAN_DATA_RATE));
					printf (" [Node %s] t=%f  -> Schedule the GTS data period from %f sec to %f sec. \n\n", my_attributes.name, op_sim_time(), SF.BI_Boundary+Symbols2Sec(((GTS.start_slot)*SF.slot_duration), WPAN_DATA_RATE), SF.BI_Boundary+Symbols2Sec(((GTS.start_slot+GTS.length)*SF.slot_duration), WPAN_DATA_RATE));
				}
			}
			
			op_intrpt_schedule_self (op_sim_time(), DEFAULT_CODE);
			op_intrpt_schedule_self (op_sim_time(), START_OF_CAP_PERIOD_CODE);
	
			wban_schedule_next_beacon();
			
		} else { /*  == OPC_FALSE */
			if (enable_log) {
				fprintf(log,"t=%f  -> Beacon Frame Reception  - but not from Hub. \n",op_sim_time());	
				printf(" [Node %s] t=%f  -> Beacon Frame Reception - but not from Hub. \n", node_attributes.name, op_sim_time());
			}
		}
	} 
	
	op_pk_destroy (beacon_frame);
	op_pk_destroy (mac_frame);
	
	/* Stack tracing exit point */
	FOUT;	
}
