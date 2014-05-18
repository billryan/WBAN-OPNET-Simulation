
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
	Objid conn_assign_attr_comp_id;
	Objid conn_assign_attr_id;
	Objid mac_attr_comp_id;
	Objid mac_attr_id;
	Objid traffic_source_up_id;
	
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
	mac_attr.wait_for_ack = OPC_FALSE;

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
		op_ima_obj_attr_get (beacon_attr_comp_id, "B2 Start", &beacon_attr.b2_start);
		op_ima_obj_attr_get (beacon_attr_comp_id, "Inactive Duration", &beacon_attr.inactive_duration);

		/* get the Connection Assignment for the Node */
		op_ima_obj_attr_get (node_attr.objid, "Connection Assignment", &conn_assign_attr_id);
		conn_assign_attr_comp_id = op_topo_child (conn_assign_attr_id, OPC_OBJTYPE_GENERIC, 0);
		op_ima_obj_attr_get (conn_assign_attr_comp_id, "EAP2 Start", &conn_assign_attr.eap2_start);
		SF.eap2_start = conn_assign_attr.eap2_start;
		SF.map1_end = SF.eap2_start - 1;
		SF.current_first_free_slot = SF.rap1_end + 1;

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
		
		SF.eap2_start = 0;
		beacon_attr.beacon_period_length = -1;
	}
	mac_state = MAC_SETUP;
	SF.SLEEP = OPC_TRUE;
	SF.ENABLE_TX_NEW = OPC_FALSE;
	
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
	//int source_address;
	//int dest_address;
	double packet_size;

	/* Stack tracing enrty point */
	FIN(wban_parse_incoming_frame);

	/* get the packet from the input stream	*/
	Stream_ID = op_intrpt_strm();
	rcv_frame = op_pk_get (Stream_ID);
	
	frame_type_fd = 0;
	/* check from what input stream the packet is received and do the right processing*/
	switch (Stream_ID) {
		case STRM_FROM_RADIO_TO_MAC: /*A PHY FRAME (PPDU) FROM THE RADIO RECIEVER*/
		{
			/* get MAC frame (MPDU=PSDU) from received PHY frame (PPDU)*/
			op_pk_nfd_get_pkt (rcv_frame, "PSDU", &frame_MPDU);

			/*update the battery*/
			packet_size = (double) op_pk_total_size_get(rcv_frame);
			wpan_battery_update_rx (packet_size, frame_type_fd);
			
			op_pk_nfd_get (frame_MPDU, "BAN ID", &ban_id);
			op_pk_nfd_get (frame_MPDU, "Recipient ID", &recipient_id);
			op_pk_nfd_get (frame_MPDU, "Sender ID", &sender_id);

			// filter the incoming BAN packet - not implemented entirely
    		if (!is_packet_for_me(frame_MPDU, ban_id, recipient_id, sender_id)) {
    			FOUT;
    		}

    		/* repalce the mac_attr.receipient_id with Sender ID */
    		mac_attr.recipient_id = sender_id;

			/*acquire "Frame Type" field*/
			op_pk_nfd_get (frame_MPDU, "Frame Type", &frame_type_fd);
			op_pk_nfd_get (frame_MPDU, "Frame Subtype", &frame_subtype_fd);
			op_pk_nfd_get (frame_MPDU, "Ack Policy", &ack_policy_fd);
			op_pk_nfd_get (frame_MPDU, "EAP Indicator", &eap_indicator_fd);
			op_pk_nfd_get (frame_MPDU, "B2", &beacon2_enabled_fd);
			op_pk_nfd_get (frame_MPDU, "Sequence Number", &sequence_number_fd);
			op_pk_nfd_get (frame_MPDU, "Inactive", &inactive_fd);

			op_pk_destroy (rcv_frame);
			
			switch (frame_type_fd) {
				case DATA: /* Handle data packets */
					if (enable_log) {
						fprintf (log,"t=%f  !!!!!!!!! Data Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), sender_id);
						printf (" [Node %s] t=%f  !!!!!!!!! Data Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
					}
					wban_extract_data_frame (frame_MPDU);
					/* send to higher layer for statistics */
					op_pk_send (frame_MPDU, STRM_FROM_MAC_TO_SINK);
					break;
				case MANAGEMENT: /* Handle management packets */
			 		if (enable_log) {
						fprintf (log,"t=%f  !!!!!!!!! Management Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), sender_id);
						printf (" [Node %s] t=%f  !!!!!!!!! Management Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
					}
					switch (frame_subtype_fd) {
						case BEACON: 
							if (enable_log) {
								fprintf (log,"t=%f  -> Beacon Frame Reception From @%d \n\n", op_sim_time(), sender_id);
								printf (" [Node %s] t=%f  -> Beacon Frame Reception From @%d \n\n", node_attr.name, op_sim_time(), sender_id);
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
							if (OPC_FALSE == SF.ENABLE_TX_NEW) {
								printf("Hub cannot rceive new connection request frame now.\n");
								op_pk_destroy(frame_MPDU);
								break;
							}
							SF.ENABLE_TX_NEW = OPC_FALSE;
							if (enable_log) {
								fprintf (log,"t=%f  !!!!!!!!! Connection request Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), sender_id);
								printf (" [Node %s] t=%f  !!!!!!!!! Connection request Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
							}
							wban_extract_conn_req_frame (frame_MPDU);
							break;
						case CONNECTION_ASSIGNMENT:
						{
							if (enable_log) {
								fprintf (log,"t=%f  !!!!!!!!! Connection assignment Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), sender_id);
								printf (" [Node %s] t=%f  !!!!!!!!! Connection assignment Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
							}
							wban_extract_conn_assign_frame (frame_MPDU);
							break;
						};
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
							wban_extract_i_ack_frame (frame_MPDU);
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
			break;
		};
		case STRM_FROM_TRAFFIC_UP_TO_MAC: /* INCOMMING PACKETS(MSDU) FROM THE TRAFFIC SOURCE */
		{
			wban_encapsulate_and_enqueue_data_frame (rcv_frame, I_ACK_POLICY, node_attr.traffic_dest_id);			
			break;
		};
	
		default :
		{
		};
	}

	/* Stack tracing exit point */
	FOUT;
}

/*------------------------------------------------------------------------------
 * Function:	is_packet_for_me
 *
 * Description:	filter the incoming BAN packet
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
	printf (" \t B2 Start               : %d \n", beacon_attr.b2_start);
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
	op_pk_nfd_set (beacon_MSDU, "B2 Start", beacon_attr.b2_start);
	op_pk_nfd_set (beacon_MSDU, "Inactive Duration", beacon_attr.inactive_duration);
	
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

	/* create PHY frame (PPDU) that encapsulates beacon MAC frame (MPDU) */
	beacon_PPDU = op_pk_create_fmt("wban_frame_PPDU_format");
	op_pk_nfd_set (beacon_PPDU, "RATE", node_attr.data_rate);
	/* wrap beacon MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (beacon_PPDU, "PSDU", beacon_MPDU);
	op_pk_nfd_set (beacon_PPDU, "LENGTH", ((double) op_pk_total_size_get(beacon_MPDU))/8); //[bytes]	
	
	SF.BI_Boundary = op_pk_creation_time_get (beacon_MPDU);
	beacon_frame_tx_time = TX_TIME(op_pk_total_size_get(beacon_PPDU), node_attr.data_rate);
	SF.eap1_start2sec = SF.BI_Boundary + beacon_frame_tx_time + pSIFS;

	// collect the number of beacon frame
	op_stat_write (beacon_frame_hndl, 1.0);

	wpan_battery_update_tx ((double) op_pk_total_size_get(beacon_PPDU));


	if (enable_log) {
		printf("sequence_num_beaconG = %d\n", sequence_num_beaconG - 1);
		printf("Size of beacon_PPDU=%d\n", op_pk_total_size_get(beacon_PPDU));
		printf("beacon_frame_tx_time = %f\n", beacon_frame_tx_time);
		fprintf(log,"t=%f  -> Beacon Frame transmission. \n\n", op_sim_time());
		printf(" [Node %s] t=%f  -> Beacon Frame transmission. \n\n", node_attr.name, op_sim_time());
	}

	//op_intrpt_priority_set (OPC_INTRPT_SELF, BEACON_INTERVAL_CODE, -2);
	op_pk_send (beacon_PPDU, STRM_FROM_MAC_TO_RADIO);
	wban_schedule_next_beacon (); // Maybe for updating the parameters of superframe
	/* Stack tracing exit point */
	FOUT;
}

/*-----------------------------------------------------------------------------
 * Function:	wban_extract_conn_req_frame
 *
 * Description:	extract the data frame from the MAC frame received from the network
 *              
 * Input :  frame_MPDU - the received MAC frame
 *-----------------------------------------------------------------------------*/
static void wban_extract_conn_req_frame(Packet* frame_MPDU) {
	// int ack_policy;
	int seq_num;
	int allocation_length;
	Packet* frame_MSDU;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_conn_req_frame);

	op_pk_nfd_get (frame_MPDU, "Sequence Number", &seq_num);
	op_pk_nfd_get (frame_MPDU, "Sender ID", &mac_attr.recipient_id);
	wban_send_i_ack_frame (seq_num);

	op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
	op_pk_nfd_get (frame_MSDU, "Allocation Length", &allocation_length);

	conn_assign_attr.allocation_length = allocation_length;
	// wban_send_conn_assign
 	// if (allocation_length > 0) {
 	// 	
 	// }
	op_intrpt_schedule_self(op_sim_time()+TX_TIME(I_ACK_PPDU_SIZE_BITS, node_attr.data_rate)+pSIFS, SEND_CONN_ASSIGN_CODE);
	/* Stack tracing exit point */
	FOUT;	
}

/*-----------------------------------------------------------------------------
 * Function:	wban_extract_conn_assign_frame
 *
 * Description:	extract the data frame from the MAC frame received from the network
 *              
 * Input :  frame_MPDU - the received MAC frame
 *-----------------------------------------------------------------------------*/
static void wban_extract_conn_assign_frame(Packet* frame_MPDU) {
	// int ack_policy;
	int seq_num;
	Packet* frame_MSDU;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_conn_assign_frame);

	op_pk_nfd_get (frame_MPDU, "Sequence Number", &seq_num);
	op_pk_nfd_get (frame_MPDU, "Sender ID", &mac_attr.recipient_id);
	wban_send_i_ack_frame (seq_num);

	op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
	op_pk_nfd_get (frame_MSDU, "EAP2 Start", &conn_assign_attr.eap2_start);
	op_pk_nfd_get (frame_MSDU, "Interval Start", &conn_assign_attr.interval_start);
	op_pk_nfd_get (frame_MSDU, "Interval End", &conn_assign_attr.interval_end);

	/* update the parameters of Superframe */
	SF.eap2_start = conn_assign_attr.eap2_start;
	SF.eap2_end = SF.rap2_start - 1;
	SF.map1_end = SF.eap2_start - 1;

	printf("Node %s assigned with Interval Start %d slot, Interval End %d slot.\n", node_attr.name, conn_assign_attr.interval_start, conn_assign_attr.interval_end);

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
	int beacon_PPDU_size;
	int rcv_sender_id;
	double beacon_frame_tx_time;
	int sequence_number_fd;
	int eap_indicator_fd;
	int beacon2_enabled_fd;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_beacon_frame);

	beacon_PPDU_size = op_pk_total_size_get(beacon_MPDU_rx) + PHY_HEADER_SIZE;

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
		op_pk_nfd_get (beacon_MSDU_rx, "B2 Start", &beacon_attr.b2_start);
		op_pk_nfd_get (beacon_MSDU_rx, "Inactive Duration", &beacon_attr.inactive_duration);

		// update with actual Hub id
		if (HUB_ID == node_attr.traffic_dest_id) {
			node_attr.traffic_dest_id = rcv_sender_id;
		}

		SF.BI_Boundary = op_pk_creation_time_get (beacon_MPDU_rx);
		beacon_frame_tx_time = TX_TIME(beacon_PPDU_size, node_attr.data_rate);
		SF.eap1_start2sec = SF.BI_Boundary + beacon_frame_tx_time + pSIFS;

		op_pk_destroy (beacon_MSDU_rx);
		op_pk_destroy (beacon_MPDU_rx);

		if (UNCONNECTED == mac_attr.sender_id) {
			/* We will try to connect to this BAN  if our scheduled access length
			 * is NOT set to unconnected (-1). If it is set to 0, it means we are
			 * establishing a sleeping pattern and waking up only to hear beacons
			 * and are only able to transmit in RAP periods.
			 */
			mac_attr.recipient_id = rcv_sender_id;
			mac_attr.sender_id = node_attr.objid; // we simply use objid as sender_id
			if (conn_req_attr.allocation_length > 0) {
				// we are unconnected, and we need to connect to obtain scheduled access
				// we will create and send a connection request
				// printf("Node %s start sending connection request frame at %f.\n", node_attr.name, op_sim_time());
				op_intrpt_schedule_self (SF.rap1_end2sec + node_attr.objid * beacon_frame_tx_time, SEND_CONN_REQ_CODE);
			}
		}
		wban_schedule_next_beacon();
	}
	/* Stack tracing exit point */
	FOUT;
}

/*------------------------------------------------------------------------------
 * Function:	wban_schedule_next_beacon
 *
 * Description:	generates the self interupt for the next beacon transmission
 *              and a self interrupts to indicate the start/end of the EAP/RAP and end of sleep period
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_schedule_next_beacon() {
	
	/* Stack tracing enrty point */
	FIN(wban_schedule_next_beacon);

	// update the superframe parameters
	SF.SD = beacon_attr.beacon_period_length; // the superframe duration(beacon preriod length) in slots
	SF.BI = beacon_attr.beacon_period_length * (1+ beacon_attr.inactive_duration); // active and inactive superframe
	SF.sleep_period = SF.SD * beacon_attr.inactive_duration;

	SF.slot_length2sec = allocationSlotLength2ms / 1000.0; // transfer allocation slot length from ms to sec.
	SF.rap1_start = beacon_attr.rap1_start;
	SF.rap1_end = beacon_attr.rap1_end;
	SF.rap2_start = beacon_attr.rap2_start;
	SF.rap2_end = beacon_attr.rap2_end;
	SF.b2_start = beacon_attr.b2_start;
	SF.current_slot = 0;
	SF.current_first_free_slot = beacon_attr.rap1_end + 1; // spec for hub assignment

	SF.rap1_start2sec = SF.BI_Boundary + SF.rap1_start * SF.slot_length2sec + pSIFS;
	SF.eap2_start2sec = SF.BI_Boundary + SF.eap2_start * SF.slot_length2sec + pSIFS; //for Hub at the first beacon
	SF.rap2_start2sec = SF.BI_Boundary + SF.rap2_start * SF.slot_length2sec + pSIFS;
	SF.eap1_end2sec = SF.rap1_start2sec - pSIFS;
	SF.rap1_end2sec = SF.BI_Boundary + (SF.rap1_end+1)*SF.slot_length2sec;
	SF.rap2_end2sec = SF.BI_Boundary + (SF.rap2_end+1)*SF.slot_length2sec;
	SF.b2_start2sec = SF.BI_Boundary + SF.b2_start * SF.slot_length2sec + pSIFS;

	// SF.eap1_length2sec = SF.rap1_start * SF.slot_length2sec - beacon_frame_tx_time;
	SF.rap1_length2sec = (SF.rap1_end - SF.rap1_start + 1) * SF.slot_length2sec;
	SF.rap2_length2sec = (SF.rap2_end - SF.rap2_start + 1) * SF.slot_length2sec;

	SF.current_first_free_slot = SF.rap1_end + 1; // spec for hub assignment

	/* for node we should calculate the slot boundary */
	SF.current_slot = (int)floor((op_sim_time()-SF.BI_Boundary)/SF.slot_length2sec);
	printf("SF.current_slot = %d when received beacon frame.\n", SF.current_slot);

	// SF_slot[BeaconPeriodLength] = {0}; // initialize array SF_slot

	/* INCREMENT_SLOT at slot boundary */
	op_intrpt_schedule_self (SF.BI_Boundary + (SF.current_slot+1)*SF.slot_length2sec, INCREMENT_SLOT);
	/* Use EAP1 Phase */
	if (SF.rap1_start > 0) {
		/* The SF.eap1_start2sec may behind current time op_sim_time() */
		printf("Node %s eap1_start2sec=%f, current time=%f.\n", node_attr.name, SF.eap1_start2sec, op_sim_time());
		SF.eap1_start2sec = max_double(op_sim_time(), SF.eap1_start2sec);
		op_intrpt_schedule_self (SF.eap1_start2sec, START_OF_EAP1_PERIOD_CODE);
	}
	if ((SF.rap1_start > 0) && (SF.rap1_length2sec > 0)) {
		op_intrpt_schedule_self (SF.eap1_end2sec, END_OF_EAP1_PERIOD_CODE);
		op_intrpt_schedule_self (SF.rap1_start2sec, START_OF_RAP1_PERIOD_CODE);
		op_intrpt_schedule_self (SF.rap1_end2sec, END_OF_RAP1_PERIOD_CODE);
		if (OPC_TRUE == node_attr.is_BANhub){
			SF.map1_start2sec = SF.rap1_end2sec + pSIFS;
			op_intrpt_schedule_self (SF.map1_start2sec, START_OF_MAP1_PERIOD_CODE);
		}
	}
	if (SF.eap2_start > 0) {
		SF.eap2_start2sec = SF.BI_Boundary + SF.eap2_start * SF.slot_length2sec + pSIFS;
		SF.eap2_end2sec = SF.rap2_start2sec - pSIFS;
		op_intrpt_schedule_self (SF.eap2_start2sec, START_OF_EAP2_PERIOD_CODE);
		op_intrpt_schedule_self (SF.eap2_end2sec, END_OF_EAP2_PERIOD_CODE);
		if (OPC_TRUE == node_attr.is_BANhub) {
			SF.map1_end2sec = SF.eap2_start2sec - pSIFS;
			op_intrpt_schedule_self (SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
		}
	}
	if ((SF.rap2_length2sec > 0) && (SF.rap2_start > 0)) {
		op_intrpt_schedule_self (SF.rap2_start2sec, START_OF_RAP2_PERIOD_CODE);
		op_intrpt_schedule_self (SF.rap2_end2sec, END_OF_RAP2_PERIOD_CODE);
		if (OPC_TRUE == node_attr.is_BANhub){
			SF.map2_start2sec = SF.rap2_end2sec + pSIFS;
			op_intrpt_schedule_self (SF.map2_start2sec, START_OF_MAP2_PERIOD_CODE);
		}
	}
	if ((SF.b2_start2sec > 0) && (OPC_TRUE == node_attr.is_BANhub)) {
		SF.map2_end2sec = SF.b2_start2sec - pSIFS;
		op_intrpt_schedule_self (SF.b2_start2sec - pSIFS, END_OF_MAP2_PERIOD_CODE);
		op_intrpt_schedule_self (SF.b2_start2sec, SEND_B2_FRAME);
	}
	op_intrpt_schedule_self (SF.BI_Boundary + SF.SD*SF.slot_length2sec, START_OF_SLEEP_PERIOD);
	op_intrpt_schedule_self (SF.BI_Boundary + SF.BI*SF.slot_length2sec, BEACON_INTERVAL_CODE);
	op_intrpt_schedule_remote (SF.BI_Boundary + SF.BI*SF.slot_length2sec, END_OF_SLEEP_PERIOD, node_attr.my_battery);
	//op_intrpt_priority_set (OPC_INTRPT_SELF, END_OF_CAP_PERIOD_CODE, -2);
	if (enable_log) {
		printf("Node %s Superframe parameters:\n", node_attr.name);
		printf("\tSF.eap1_start2sec=%f\n", SF.eap1_start2sec);
		printf("\tSF.eap1_end2sec=%f\n", SF.eap1_end2sec);
		printf("\tSF.rap1_start2sec=%f\n", SF.rap1_start2sec);
		printf("\tSF.rap1_end2sec=%f\n", SF.rap1_end2sec);
		printf("\tSF.map1_start2sec=%f\n", SF.map1_start2sec);
		printf("\tSF.map1_end2sec=%f\n", SF.map1_end2sec);
		printf("\tSF.eap2_start2sec=%f\n", SF.eap2_start2sec);
		printf("\tSF.eap2_end2sec=%f\n", SF.eap2_end2sec);
		printf("\tSF.rap2_start2sec=%f\n", SF.rap2_start2sec);
		printf("\tSF.rap2_end2sec=%f\n", SF.rap2_end2sec);
		printf("\tSF.map2_start2sec=%f\n", SF.map2_start2sec);
		printf("\tSF.map2_end2sec=%f\n", SF.map2_end2sec);

		fprintf (log,"t=%f  -> Schedule Next Beacon at %f\n\n", op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec);
		printf (" [Node %s] t=%f  -> Schedule Next Beacon at %f\n\n", node_attr.name, op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec);
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
	int random_num;
	// double conn_req_tx_time;

	/* Stack tracing enrty point */
	FIN(wban_send_connection_request_frame);

	random_num = wban_update_sequence_number();
	/* create a connection request frame */
	connection_request_MSDU = op_pk_create_fmt ("wban_connection_request_frame_format");
	
	/* set the fields of the connection_request frame */
	op_pk_nfd_set (connection_request_MSDU, "Allocation Length", conn_req_attr.allocation_length);

	/* create a MAC frame (MPDU) that encapsulates the connection_request payload (MSDU) */
	connection_request_MPDU = op_pk_create_fmt ("wban_frame_MPDU_format");

	op_pk_nfd_set (connection_request_MPDU, "Ack Policy", I_ACK_POLICY);
	op_pk_nfd_set (connection_request_MPDU, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (connection_request_MPDU, "Frame Subtype", CONNECTION_REQUEST);
	op_pk_nfd_set (connection_request_MPDU, "Frame Type", MANAGEMENT);
	op_pk_nfd_set (connection_request_MPDU, "B2", 1); // beacon2 enabled

	op_pk_nfd_set (connection_request_MPDU, "Sequence Number", random_num);
	op_pk_nfd_set (connection_request_MPDU, "Inactive", beacon_attr.inactive_duration); // connection_request and connection_request2 frame used

	op_pk_nfd_set (connection_request_MPDU, "Recipient ID", mac_attr.recipient_id);
	op_pk_nfd_set (connection_request_MPDU, "Sender ID", mac_attr.sender_id);
	op_pk_nfd_set (connection_request_MPDU, "BAN ID", mac_attr.ban_id);
	
	op_pk_nfd_set_pkt (connection_request_MPDU, "MAC Frame Payload", connection_request_MSDU); // wrap connection_request payload (MSDU) in MAC Frame (MPDU)

	/* create PHY frame (PPDU) that encapsulates connection_request MAC frame (MPDU) */
	connection_request_PPDU = op_pk_create_fmt("wban_frame_PPDU_format");

	op_pk_nfd_set (connection_request_PPDU, "RATE", node_attr.data_rate);
	/* wrap connection_request MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (connection_request_PPDU, "PSDU", connection_request_MPDU);
	op_pk_nfd_set (connection_request_PPDU, "LENGTH", ((double) op_pk_total_size_get(connection_request_MPDU))/8); //[bytes]	
	
	frame_PPDU_copy = op_pk_copy(connection_request_PPDU);
	// conn_req_tx_time = TX_TIME(op_pk_total_size_get(frame_PPDU_copy), node_attr.data_rate);
	// op_intrpt_schedule_self (SF.rap1_start2sec+SF.rap1_length2sec+random_num*conn_req_tx_time, SEND_CONN_REQ_CODE);
	op_pk_send(frame_PPDU_copy, STRM_FROM_MAC_TO_RADIO);

	mac_attr.wait_for_ack = OPC_TRUE;
	mac_attr.wait_ack_seq_num = random_num;

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_send_conn_assign_frame
 *
 * Description:	Create a connection request frame and send it to the Radio (wban_mac) 
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_send_conn_assign_frame ( int allocation_length) {
	Packet* conn_assign_MSDU;
	Packet* conn_assign_MPDU;
	Packet* conn_assign_PPDU;
	int random_num;
	double conn_assign_tx_time;
	int i;

	/* Stack tracing enrty point */
	FIN(wban_send_conn_assign_frame);

	random_num = wban_update_sequence_number();
	/* create a connection request frame */
	conn_assign_MSDU = op_pk_create_fmt ("wban_connection_assignment_frame_format");
	
	/* update the superframe parameters about MAP1 and EAP2 */
	if (conn_assign_attr.eap2_start - beacon_attr.rap1_end > 1) {
		SF.eap2_start = conn_assign_attr.eap2_start;
		SF.eap2_end = SF.rap2_start - 1;
		SF.map1_start = SF.rap1_end + 1;
		SF.map1_end = SF.eap2_start - 1;
	}

	/* set the fields of the conn_assign frame */
	op_pk_nfd_set (conn_assign_MSDU, "Connection Status", 0);

	if (SF.current_first_free_slot < SF.map1_end + 1) {
		/* allocation within MAP1 */
		if (SF.current_first_free_slot + allocation_length < SF.map1_end + 2) {
			conn_assign_attr.interval_start = SF.current_first_free_slot;
			conn_assign_attr.interval_end = conn_assign_attr.interval_start + allocation_length - 1;
			SF.current_first_free_slot = conn_assign_attr.interval_end + 1;

			/* map the allocation slot into array SF_slot */
			i = conn_assign_attr.interval_start;
			for (; i <= conn_assign_attr.interval_end; i++) {
				SF_slot[i] = mac_attr.recipient_id;
			}
		} else if (SF.rap2_end + allocation_length < SF.b2_start - 1) {
			conn_assign_attr.interval_start = SF.rap2_end + 1;
			conn_assign_attr.interval_end = conn_assign_attr.interval_start + allocation_length - 1;
			SF.current_first_free_slot = conn_assign_attr.interval_end + 1;

			/* map the allocation slot into array SF_slot */
			i = conn_assign_attr.interval_start;
			for (; i <= conn_assign_attr.interval_end; i++) {
				SF_slot[i] = mac_attr.recipient_id;
			}
		}

		if (SF.current_first_free_slot == SF.eap2_start) {
			SF.current_first_free_slot = SF.rap2_end + 1;
		}
	} else if (SF.current_first_free_slot < SF.b2_start) {
		/* allocation within MAP2 */
		if (SF.current_first_free_slot + allocation_length < SF.b2_start + 1) {
			conn_assign_attr.interval_start = SF.current_first_free_slot;
			conn_assign_attr.interval_end = conn_assign_attr.interval_start + allocation_length - 1;
			SF.current_first_free_slot = conn_assign_attr.interval_end + 1;

			/* map the allocation slot into array SF_slot */
			i = conn_assign_attr.interval_start;
			for (; i <= conn_assign_attr.interval_end; i++) {
				SF_slot[i] = mac_attr.recipient_id;
			}
		} else {
			conn_assign_attr.interval_start = 255;
			conn_assign_attr.interval_end = 0;
			printf("There is no enougth slots for scheduling.\n");
			// FOUT;
		}
	}

	/* set the fields of the conn_assign frame */
	op_pk_nfd_set (conn_assign_MSDU, "EAP2 Start", conn_assign_attr.eap2_start);
	op_pk_nfd_set (conn_assign_MSDU, "Interval Start", conn_assign_attr.interval_start);
	op_pk_nfd_set (conn_assign_MSDU, "Interval End", conn_assign_attr.interval_end);

	/* create a MAC frame (MPDU) that encapsulates the conn_assign payload (MSDU) */
	conn_assign_MPDU = op_pk_create_fmt ("wban_frame_MPDU_format");

	op_pk_nfd_set (conn_assign_MPDU, "Ack Policy", I_ACK_POLICY);
	op_pk_nfd_set (conn_assign_MPDU, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (conn_assign_MPDU, "Frame Subtype", CONNECTION_ASSIGNMENT);
	op_pk_nfd_set (conn_assign_MPDU, "Frame Type", MANAGEMENT);
	op_pk_nfd_set (conn_assign_MPDU, "B2", 1); // beacon2 enabled

	op_pk_nfd_set (conn_assign_MPDU, "Sequence Number", random_num);
	op_pk_nfd_set (conn_assign_MPDU, "Inactive", beacon_attr.inactive_duration); // conn_assign and conn_assign2 frame used

	op_pk_nfd_set (conn_assign_MPDU, "Recipient ID", mac_attr.recipient_id);
	op_pk_nfd_set (conn_assign_MPDU, "Sender ID", mac_attr.sender_id);
	op_pk_nfd_set (conn_assign_MPDU, "BAN ID", mac_attr.ban_id);
	
	op_pk_nfd_set_pkt (conn_assign_MPDU, "MAC Frame Payload", conn_assign_MSDU); // wrap conn_assign payload (MSDU) in MAC Frame (MPDU)

	/* create PHY frame (PPDU) that encapsulates conn_assign MAC frame (MPDU) */
	conn_assign_PPDU = op_pk_create_fmt("wban_frame_PPDU_format");

	op_pk_nfd_set (conn_assign_PPDU, "RATE", node_attr.data_rate);
	/* wrap conn_assign MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (conn_assign_PPDU, "PSDU", conn_assign_MPDU);
	op_pk_nfd_set (conn_assign_PPDU, "LENGTH", ((double) op_pk_total_size_get(conn_assign_MPDU))/8); //[bytes]	
	
	frame_PPDU_copy = op_pk_copy(conn_assign_PPDU);
	conn_assign_tx_time = TX_TIME(op_pk_total_size_get(frame_PPDU_copy), node_attr.data_rate);
	
	wpan_battery_update_tx ((double) op_pk_total_size_get(frame_PPDU_copy));
	if (op_stat_local_read(TX_BUSY_STAT) == 1.0)
			op_sim_end("ERROR : TRY TO SEND AN CONNECTION_ASSIGNMENT WHILE THE TX CHANNEL IS BUSY","SEND_CONN_ASSIGN_CODE","","");

	op_pk_send (frame_PPDU_copy, STRM_FROM_MAC_TO_RADIO);

	mac_attr.wait_for_ack = OPC_TRUE;
	mac_attr.wait_ack_seq_num = random_num;
	// op_intrpt_schedule_self (SF.rap1_start2sec+SF.rap1_length2sec+3*conn_assign_tx_time, WAIT_CONN_ASSIGN_END_CODE);
	/* Stack tracing exit point */
	FOUT;
}

/*-----------------------------------------------------------------------------
 * Function:	wban_send_i_ack_frame
 *
 * Description:	send back to the sender ACK frame
 *
 * Input :  seq_num - expected sequence number
 *-----------------------------------------------------------------------------*/
static void wban_send_i_ack_frame (int seq_num) {
	// double ack_tx_time;
	Packet* frame_MPDU;
	Packet* frame_PPDU;
	/* Stack tracing enrty point */
	FIN(wban_send_i_ack_frame);

	if (enable_log) {
		fprintf(log,"t=%f  -> Send ACK Frame [SEQ = %d] \n\n", op_sim_time(), ack_sequence_number);
		printf(" [Node %s] t=%f  -> Send ACK Frame [SEQ = %d] \n\n", node_attr.name, op_sim_time(), seq_num);
	}
	/* create a MAC frame (MPDU) that encapsulates the connection_request payload (MSDU) */
	frame_MPDU = op_pk_create_fmt ("wban_frame_MPDU_format");
	op_pk_nfd_set (frame_MPDU, "Ack Policy", N_ACK_POLICY);
	op_pk_nfd_set (frame_MPDU, "Frame Subtype", I_ACK);
	op_pk_nfd_set (frame_MPDU, "Frame Type", CONTROL);
	op_pk_nfd_set (frame_MPDU, "Sequence Number", seq_num);
	op_pk_nfd_set (frame_MPDU, "Recipient ID", mac_attr.recipient_id);
	op_pk_nfd_set (frame_MPDU, "Sender ID", mac_attr.sender_id);
	op_pk_nfd_set (frame_MPDU, "BAN ID", mac_attr.ban_id);
	// Hub may contain sync information
	/* create PHY frame (PPDU) that encapsulates connection_request MAC frame (MPDU) */
	frame_PPDU = op_pk_create_fmt("wban_frame_PPDU_format");

	op_pk_nfd_set (frame_PPDU, "RATE", node_attr.data_rate);
	/* wrap connection_request MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (frame_PPDU, "PSDU", frame_MPDU);
	op_pk_nfd_set (frame_PPDU, "LENGTH", ((double) op_pk_total_size_get(frame_MPDU))/8); //[bytes]	
	
	wpan_battery_update_tx ((double) op_pk_total_size_get(frame_PPDU));
	if (op_stat_local_read(TX_BUSY_STAT) == 1.0)
			op_sim_end("ERROR : TRY TO SEND AN ACK WHILE THE TX CHANNEL IS BUSY","ACK_SEND_CODE","","");

	op_pk_send (frame_PPDU, STRM_FROM_MAC_TO_RADIO);
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
static void wban_encapsulate_and_enqueue_data_frame (Packet* data_frame_up, enum AcknowledgementPolicy_type ack_policy, int dest_id) {
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

	op_pk_nfd_set (data_frame_mpdu, "Ack Policy", ack_policy);
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
			fprintf (log,"t=%f  -> Enqueuing of MAC DATA frame [SEQ = %d, ACK? = %d] and try to send \n\n", op_sim_time(), seq_num, ack_policy);
			printf (" [Node %s] t=%f  -> Enqueuing of MAC DATA frame [SEQ = %d, ACK? = %d] and try to send \n\n", node_attr.name, op_sim_time(), seq_num, ack_policy);
		}
	} else {
		if (enable_log) {
			fprintf (log,"t=%f  -> MAC DATA frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", op_sim_time());
			printf (" [Node %s] t=%f  -> MAC DATA frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", node_attr.name, op_sim_time());
		}
		
		/* destroy the packet */
		op_pk_destroy (data_frame_mpdu);
	}

	/* try to send the packet in SF active phase */
	if ((MAC_SLEEP != mac_state) && (MAC_SETUP != mac_state) && (OPC_TRUE == SF.ENABLE_TX_NEW)) {
		op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
	}
	
	/* Stack tracing exit point */
	FOUT;
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

/*--------------------------------------------------------------------------------
 * Function: wban_mac_interrupt_process
 *
 * Description:	processes all the interrupts that occurs in an unforced state      
 *             
 * No parameters  
 *--------------------------------------------------------------------------------*/
static void wban_mac_interrupt_process() {
	// Packet* frame_MPDU;
	// Packet* frame_MPDU_copy;
	//Packet* frame_MPDU_to_send;
	// Packet* frame_PPDU;
	//Packet* ack_MPDU;
	// double tx_time;
	//int ack_request;
	//int seq_num;
	//int dest_address;

	double map_start2sec;
	double map_end2sec;
	/* Stack tracing enrty point */
	FIN(wban_mac_interrupt_process);
	
	switch (op_intrpt_type()) {
	
		case OPC_INTRPT_STRM: // incomming packet
		{
			wban_parse_incoming_frame();	// parse the incoming packet
			break;
		};/*end of OPC_INTRPT_STRM */
	
		case OPC_INTRPT_SELF:
		{
			switch (op_intrpt_code()) { /* begin switch (op_intrpt_code()) */
				case BEACON_INTERVAL_CODE: /* Beacon Interval Expiration - end of the Beacon Interval and start of a new one */
				{
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF SLEEP PERIOD ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF SLEEP PERIOD ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					if (IAM_BAN_HUB) {
						/* value for the next superframe. End Device will obtain this value from beacon */
						wban_send_beacon_frame();
					}
					break;
				};/*end of BEACON_INTERVAL_CODE */

				case INCREMENT_SLOT: {
					SF.current_slot++;
					if (SF.SD > SF.current_slot + 1) {
						op_intrpt_schedule_self (op_sim_time() + SF.slot_length2sec, INCREMENT_SLOT);
					}

					if(!IAM_BAN_HUB) {
						map_start2sec = SF.BI_Boundary + conn_assign_attr.interval_start * SF.slot_length2sec + pSIFS;
						map_end2sec = SF.BI_Boundary + (conn_assign_attr.interval_end+1) * SF.slot_length2sec;
						if((SF.current_slot == conn_assign_attr.interval_start) && (SF.current_slot < SF.eap2_start)) {
							SF.map1_start2sec = map_start2sec;
							SF.map1_end2sec = map_end2sec;
							op_intrpt_schedule_self(max_double(SF.map1_start2sec, op_sim_time()) + pSIFS, START_OF_MAP1_PERIOD_CODE);
							op_intrpt_schedule_self(SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
						} else if ((SF.current_slot == conn_assign_attr.interval_start) && (SF.current_slot > SF.rap2_end)) {
							SF.map2_start2sec = map_start2sec;
							SF.map2_end2sec = map_end2sec;
							op_intrpt_schedule_self(max_double(SF.map2_start2sec, op_sim_time()) + pSIFS, START_OF_MAP2_PERIOD_CODE);
							op_intrpt_schedule_self(SF.map2_end2sec, END_OF_MAP2_PERIOD_CODE);
						}
					}
					break;
				};

				case START_OF_EAP1_PERIOD_CODE: /* start of EAP1 Period */
				{
					mac_state = MAC_EAP1;
					phase_start_timeG = SF.eap1_start2sec;
					phase_end_timeG = SF.rap1_start2sec;
					SF.IN_MAP_PHASE = OPC_FALSE;
					SF.IN_EAP_PHASE = OPC_TRUE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE EAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE EAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}

					if (OPC_FALSE == SF.ENABLE_TX_NEW) {
						op_intrpt_schedule_self (op_sim_time(), TRY_PROCESS_LAST_PACKET_CODE);
					}
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_EAP1_PERIOD_CODE */

				case END_OF_EAP1_PERIOD_CODE: /* end of EAP1 Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE EAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE EAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}			
					break;
				};/* end of END_OF_EAP1_PERIOD_CODE */

				case START_OF_RAP1_PERIOD_CODE: /* start of RAP1 Period */
				{
					mac_state = MAC_RAP1;
					phase_start_timeG = SF.rap1_start2sec;
					phase_end_timeG = SF.rap1_start2sec + SF.rap1_length2sec;
					SF.IN_MAP_PHASE = OPC_FALSE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE RAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE RAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}

					if (OPC_FALSE == SF.ENABLE_TX_NEW) {
						op_intrpt_schedule_self (op_sim_time(), TRY_PROCESS_LAST_PACKET_CODE);
					}
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_RAP1_PERIOD_CODE */

				case END_OF_RAP1_PERIOD_CODE: /* END of RAP1 Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE RAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE RAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}			
					break;
				};/* end of END_OF_RAP1_PERIOD_CODE */

				case START_OF_MAP1_PERIOD_CODE: /* start of RAP1 Period */
				{
					mac_state = MAC_MAP1;
					// mac_state = MAC_SLEEP;
					phase_start_timeG = SF.map1_start2sec;
					phase_end_timeG = SF.map1_end2sec;
					SF.IN_MAP_PHASE = OPC_TRUE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
					if(OPC_FALSE == node_attr.is_BANhub){
						map_attr.TX_state = OPC_TRUE;
					}
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE MAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE MAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					printf("Node %s Start MAP1 at %f, End MAP1 at %f.\n", node_attr.name, phase_start_timeG, phase_end_timeG);

					if (OPC_FALSE == SF.ENABLE_TX_NEW) {
						op_intrpt_schedule_self (op_sim_time(), TRY_PROCESS_LAST_PACKET_CODE);
					}
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_MAP1_PERIOD_CODE */

				case END_OF_MAP1_PERIOD_CODE: /* end of MAP1 Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE MAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE MAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}			
					break;
				};/* end of END_OF_MAP1_PERIOD_CODE */

				case START_OF_EAP2_PERIOD_CODE: /* start of EAP2 Period */
				{
					mac_state = MAC_EAP2;
					phase_start_timeG = SF.eap2_start2sec;
					phase_end_timeG = SF.rap2_start2sec - pSIFS;
					SF.IN_MAP_PHASE = OPC_FALSE;
					SF.IN_EAP_PHASE = OPC_TRUE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE EAP2 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE EAP2 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}

					if (OPC_FALSE == SF.ENABLE_TX_NEW) {
						op_intrpt_schedule_self (op_sim_time(), TRY_PROCESS_LAST_PACKET_CODE);
					}
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_EAP2_PERIOD_CODE */

				case END_OF_EAP2_PERIOD_CODE: /* end of EAP2 Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE EAP2 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE EAP2 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}			
					break;
				};/* end of END_OF_EAP2_PERIOD_CODE */

				case START_OF_RAP2_PERIOD_CODE: /* start of RAP2 Period */
				{
					mac_state = MAC_RAP2;
					phase_start_timeG = SF.rap2_start2sec;
					phase_end_timeG = SF.rap2_end2sec;
					SF.IN_MAP_PHASE = OPC_FALSE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;

					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE RAP2 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE RAP2 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}

					if (OPC_FALSE == SF.ENABLE_TX_NEW) {
						op_intrpt_schedule_self (op_sim_time(), TRY_PROCESS_LAST_PACKET_CODE);
					}
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_RAP1_PERIOD_CODE */

				case END_OF_RAP2_PERIOD_CODE: /* END of RAP2 Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE RAP2 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE RAP2 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}			
					break;
				};/* end of END_OF_RAP2_PERIOD_CODE */

				case START_OF_MAP2_PERIOD_CODE: /* start of MAP2 Period */
				{
					mac_state = MAC_MAP2;
					phase_start_timeG = SF.map2_start2sec;
					phase_end_timeG = SF.map2_end2sec;
					SF.IN_MAP_PHASE = OPC_TRUE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
					if(!IAM_BAN_HUB){
						map_attr.TX_state = OPC_TRUE;
					}
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE MAP2 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE MAP2 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					printf("Node %s Start MAP2 at %f, End MAP2 at %f.\n", node_attr.name, phase_start_timeG, phase_end_timeG);

					if (OPC_FALSE == SF.ENABLE_TX_NEW) {
						op_intrpt_schedule_self (op_sim_time(), TRY_PROCESS_LAST_PACKET_CODE);
					}
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_MAP2_PERIOD_CODE */

				case END_OF_MAP2_PERIOD_CODE: /* end of MAP2 Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE MAP2 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE MAP2 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}			
					break;
				};/* end of END_OF_MAP2_PERIOD_CODE */

				case START_OF_SLEEP_PERIOD: /* Start of Sleep Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF SLEEP PERIOD ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF SLEEP PERIOD ++++++++++ \n\n", node_attr.name, op_sim_time());
					}			
					break;
				};/* end of Start of Sleep Period */

				case TRY_PACKET_TRANSMISSION_CODE :
				{
					// SF.ENABLE_TX_NEW = OPC_TRUE;
					wban_attempt_TX();
					break;
				};
				
				case TRY_PROCESS_LAST_PACKET_CODE :
				{
					// wban_process_last_packet();
					break;
				};

				case CCA_START_CODE: /*At the start of the CCA */
				{
					/* do check if the channel is idle at the start of cca */
					/* at the start the channel is assumed idle, any change to busy, the CCA will report a busy channel */
				
					/* check at the beginning of CCA, if the channel is busy */
					csma.CCA_CHANNEL_IDLE = OPC_TRUE;
					if (op_stat_local_read (RX_BUSY_STAT) == 1.0) {
						csma.CCA_CHANNEL_IDLE = OPC_FALSE;
					}

					if (enable_log) {
						fprintf (log,"t=%f  -------- START CCA CW = %d\n",op_sim_time(),csma.CW);
						printf (" [Node %s] t=%f  -------- START CCA CW = %d\n",node_attr.name, op_sim_time(), csma.CW);
					}
					// csma.next_slot_start = op_sim_time() + pCSMASlotLength2Sec;
					op_intrpt_schedule_self (op_sim_time() + pCCATime, CCA_EXPIRATION_CODE);
					break;
				};/*end of CCA_START_CODE */
			
				case CCA_EXPIRATION_CODE :/*At the end of the CCA */
				{
					if(!can_fit_TX(&packet_to_be_sent)) {
						current_packet_CS_fails++;
						csma.backoff_counter_lock = OPC_TRUE;
						break;
					}

					/* bug with open-zigbee, for statwire interupt can sustain a duration */
					if ((OPC_FALSE == csma.CCA_CHANNEL_IDLE) || (op_stat_local_read (RX_BUSY_STAT) == 1.0)) {
						printf("t = %f, CCA with BUSY.\n", op_sim_time());
						csma.CCA_CHANNEL_IDLE = OPC_FALSE;
						csma.backoff_counter_lock = OPC_TRUE;
						// op_intrpt_schedule_self (csma.next_slot_start, CCA_START_CODE);
						op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
					} else {
						csma.backoff_counter--;
						printf("t = %f, CCA with IDLE, backoff_counter decrement to %d.\n", op_sim_time(), csma.backoff_counter);
						if (csma.backoff_counter != 0) {
							// printf("CCA at next available backoff boundary = %f sec.\n", csma.next_slot_start);
							// op_intrpt_schedule_self (csma.next_slot_start, CCA_START_CODE);
							printf("CCA at next available backoff boundary = %f sec.\n", wban_backoff_period_boundary_get());
							op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
						} else {
							printf("backoff_counter decrement to 0, %s start transmission at %f.\n", node_attr.name, wban_backoff_period_boundary_get());
							// op_intrpt_schedule_self (csma.next_slot_start, START_TRANSMISSION_CODE);
							op_intrpt_schedule_self (wban_backoff_period_boundary_get(), START_TRANSMISSION_CODE);
						}
					}
					break;
				};
			
				case START_TRANSMISSION_CODE: /* successful end of backoff and CCA or MAP period */
				{	
					/*backoff_start_time is initialized in the "init_backoff" state*/
					// op_stat_write(statistic_vector.mac_delay, op_sim_time()-backoff_start_time);
					// op_stat_write(statistic_global_vector.mac_delay, op_sim_time()-backoff_start_time);

					wban_send_packet();
					break;
				}; /*end of START_TRANSMISSION_CODE */
			
				case WAITING_ACK_END_CODE:	/* the timer for waiting an ACK has expired, the packet must be retransmitted */
				{
					// check if we reached the max number and if so delete the packet
					if (current_packet_txs + current_packet_CS_fails == max_packet_tries) {
						// collect statistics
						printf("Packet transmission exceeds max packet tries at time %f\n", op_sim_time());
						// remove MAC frame (MPDU) frame_MPDU_to_be_sent
						op_pk_destroy(frame_MPDU_to_be_sent);
						// packet_to_be_sent = NULL;
						mac_attr.wait_for_ack = OPC_FALSE;
						SF.ENABLE_TX_NEW = OPC_TRUE;

						// packetToBeSent = NULL;
						current_packet_txs = 0;
						current_packet_CS_fails = 0;
						op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					} else {
						mac_attr.wait_for_ack = OPC_TRUE;
						if (SF.IN_MAP_PHASE) {
							wban_send_packet();
						} else {
							// double the Contention Window, after every second fail.
							if (OPC_TRUE == csma.CW_double) {
								csma.CW *=2;
								if (csma.CW > CWmax[packet_to_be_sent.user_priority]) {
									csma.CW = CWmax[packet_to_be_sent.user_priority];
								}
								printf("CW doubled after %d tries.\n", current_packet_txs + current_packet_CS_fails);
							}
							(csma.CW_double == OPC_TRUE) ? (csma.CW_double=OPC_FALSE) : (csma.CW_double=OPC_TRUE);
							wban_backoff_delay_set(packet_to_be_sent.user_priority);
							op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
						}
						// op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);	// try to send the same packet once more
					}
					break;
				}; /*end of WAITING_ACK_END_CODE */

				case WAIT_CONN_ASSIGN_END_CODE:
				{
					printf("Hub did not receive the I-ACK of Conection Assignment Frame.\n");
					// if (op_sim_time() < SF.rap2_start2sec) {
					// op_intrpt_schedule_self(op_sim_time()+0.003, WAIT_CONN_ASSIGN_END_CODE);
					// }
					break;
				};

				case SEND_CONN_REQ_CODE:
				{
					printf("t = %f, Node %s start sending connection request frame to Hub.\n", op_sim_time(), node_attr.name);
					wban_send_connection_request_frame();
					break;
				};
				
				case SEND_CONN_ASSIGN_CODE:
				{
					printf("Node %s start sending connection assignment frame at %f.\n", node_attr.name, op_sim_time());
 					wban_send_conn_assign_frame(conn_assign_attr.allocation_length);
					break;
				};

				case N_ACK_PACKET_SENT:
					SF.ENABLE_TX_NEW = OPC_TRUE;
					break;

				default:
				{
				};	
				
			} /*end of switch (op_intrpt_code())*/ 
			
			break;
		};/*end of OPC_INTRPT_SELF */
		
		case OPC_INTRPT_ENDSIM:
		{
			if (enable_log) {
				fprintf (log, "t=%f  ***********   GAME OVER END - OF SIMULATION  ********************  \n\n",op_sim_time());
				printf (" [Node %s] t=%f  ***********   GAME OVER - END OF SIMULATION  *******************\n\n", node_attr.name, op_sim_time());
				
				fclose(log);
			}

			break;
		};	/*end of OPC_INTRPT_ENDSIM */
		
		case OPC_INTRPT_STAT: // statistic interrupt from PHY layer
		{
			switch (op_intrpt_stat()) {	/*begin switch (op_intrpt_stat())*/ 
				
				case RX_BUSY_STAT :	/* Case of the end of the BUSY RECEIVER STATISTIC */
				{
					/* if during the CCA the channel was busy for a while, then csma.CCA_CHANNEL_IDLE = OPC_FALSE*/
					if (op_stat_local_read(RX_BUSY_STAT) == 1.0) {
						csma.CCA_CHANNEL_IDLE = OPC_FALSE;
					}

					/*Try to send a packet if any*/
					if (op_stat_local_read (TX_BUSY_STAT) == 0.0) {
						op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					}
					
					break;
				}
				
				case TX_BUSY_STAT :
				{
					if (op_stat_local_read (TX_BUSY_STAT) == 0.0)
						op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					
					break;
				}
				
				case RX_COLLISION_STAT :
				{
					if (enable_log){
						fprintf(log,"t=%f  -> $$$$$ COLLISION $$$$$$$  \n\n",op_sim_time());
						printf(" [Node %s] t=%f  -> COLLISION TIME  \n",node_attr.name, op_sim_time());
					}
					
					break;
				}
			}/*end switch (op_intrpt_stat())*/
			
			break;
		};/*end of OPC_INTRPT_STAT */
		
		default :
		{
		};
		
	} /*end switch (op_intrpt_type())*/
	
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
	int ack_policy;
	int seq_num;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_data_frame);

	/* check if any ACK is requested */	
	op_pk_nfd_get (frame_MPDU, "Ack Policy", &ack_policy);
	switch (ack_policy) {
		case N_ACK_POLICY:
			break;
		case I_ACK_POLICY:
			op_pk_nfd_get (frame_MPDU, "Sequence Number", &seq_num);
			wban_send_i_ack_frame (seq_num);
			break;
		case L_ACK_POLICY:
			break;
		case B_ACK_POLICY:
			break;
		default:
			break;
	}
	/* Stack tracing exit point */
	FOUT;	
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
	op_ici_attr_set (iciptr, "WBAN DATA RATE", WBAN_DATA_RATE);
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

/*-----------------------------------------------------------------------------
 * Function:	wban_extract_i_ack_frame
 *
 * Description:	check if the sequence number of the received I ACK frame is the
 *				expected one.
 *
 * Input :  ack_frame - the MAC ACK frame
 *-----------------------------------------------------------------------------*/
static void wban_extract_i_ack_frame(Packet* ack_frame) {
	int seq_num;
	// Packet* mac_frame_dup;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_i_ack_frame);
	
	op_pk_nfd_get (ack_frame, "Sequence Number", &seq_num);
	
	/* if I'm waiting for an ACK */
	if (mac_attr.wait_for_ack == OPC_TRUE) {
		if (mac_attr.wait_ack_seq_num == seq_num) { /* yes, I have received my ACK */
			mac_attr.wait_for_ack = OPC_FALSE;
		
			/* disable the invocation of only the next interrupt of WAITING_ACK_END_CODE */
			op_intrpt_disable (OPC_INTRPT_SELF, WAITING_ACK_END_CODE, OPC_TRUE);
			
			if (enable_log) {
				fprintf(log,"t=%f  -> ACK Frame Reception [Requested SEQ = %d]\n\n", op_sim_time(), seq_num);
				printf(" [Node %s] t=%f  -> ACK Frame Reception [Requested SEQ = %d]\n\n", node_attr.name, op_sim_time(), seq_num);
			}

			//collect statistics
			SF.ENABLE_TX_NEW = OPC_TRUE;
			/* Try to send another packet after pSIFS */
			op_intrpt_schedule_self (op_sim_time() + pSIFS, TRY_PACKET_TRANSMISSION_CODE);
		} else	{	/* No, This is not my ACK, I'm Still Waiting */
			if (enable_log) {
				fprintf(log,"t=%f  -> WRONG ACK Frame Reception [RCV = %d], Still Waiting ACK [RQST = %d] \n\n", op_sim_time(), seq_num , mac_attr.wait_ack_seq_num );
				printf(" [Node %s] t=%f  -> WRONG ACK Frame Reception [RCV = %d], Still Waiting ACK [RQST = %d] \n\n", node_attr.name, op_sim_time(), seq_num , mac_attr.wait_ack_seq_num );
			}
		}
	} else {/* if (mac_attributes.wait_ack == OPC_FALSE) */ 
		if (enable_log) {
			fprintf (log,"t=%f  -> I'm not Waiting ACK Frame - ACK Frame Destroyed. \n\n", op_sim_time());	
			printf (" [Node %s] t=%f  -> I'm not Waiting ACK Frame - ACK Frame Destroyed. \n\n", node_attr.name, op_sim_time());
		}
	}
	
	/* destroy the ACK packet */
	op_pk_destroy (ack_frame);
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function: wban_attempt_TX
 *
 * Description:	attempt to TX in all TX access states (EAP, RAP, MAP, CAP)
 * It will check whether we need to retransmit the current packet, or prepare
 * a new packet from the MAC data buffer to be sent.
 * 
 * Input:	
 *--------------------------------------------------------------------------------*/
static void wban_attempt_TX() {
	Packet* frame_MPDU_temp;
	/* Stack tracing enrty point */
	FIN(wban_attempt_TX);

	// If we are not in an appropriate state, return
	switch (mac_state) {
		case MAC_EAP1: 
		case MAC_RAP1: 
		case MAC_EAP2: 
		case MAC_RAP2: 
		case MAC_CAP: 
			SF.IN_MAP_PHASE = OPC_FALSE;
			break;
		case MAC_MAP1: 
		case MAC_MAP2: 
			SF.IN_MAP_PHASE = OPC_TRUE;
			break;
		default: FOUT; // none of the valid state above
	}

	if (!((op_intrpt_type () == OPC_INTRPT_SELF) && (op_intrpt_code () == TRY_PACKET_TRANSMISSION_CODE))) {
		FOUT;
	}

	if ((op_stat_local_read(TX_BUSY_STAT) == 1.0) || (MAC_SLEEP == mac_state) || (MAC_SETUP == mac_state)) {
		FOUT;
	}

	if (op_subq_empty(SUBQ_DATA)) {
		SF.ENABLE_TX_NEW = OPC_TRUE;
		FOUT;
	} else {
		/* obtain the pointer to MAC frame (MPDU) stored in the adequate queue */
		frame_MPDU_to_be_sent = op_subq_pk_access (SUBQ_DATA, OPC_QPOS_HEAD);

		op_pk_nfd_get(frame_MPDU_to_be_sent, "Frame Subtype", &packet_to_be_sent.user_priority);
		if (OPC_TRUE == SF.IN_EAP_PHASE) {
			printf("Node %s enters into EAP phase.\n", node_attr.name);
			if (7 != packet_to_be_sent.user_priority) {
				printf("%s have no UP=7 traffic in the SUBQ_DATA subqueue currently.\n", node_attr.name);
				SF.ENABLE_TX_NEW = OPC_TRUE;
				FOUT;
			}
		}
	}

	op_pk_nfd_get(frame_MPDU_to_be_sent, "Sequence Number", &packet_to_be_sent.seq_num);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Ack Policy", &packet_to_be_sent.ack_policy);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Recipient ID", &packet_to_be_sent.recipient_id);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Frame Type", &packet_to_be_sent.frame_type);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Frame Subtype", &packet_to_be_sent.user_priority);
	
	/* for frame enqueued to subqueue before first beacon reception */
	if (packet_to_be_sent.recipient_id == HUB_ID) {
		op_pk_nfd_set(frame_MPDU_to_be_sent, "Recipient ID", mac_attr.recipient_id);
		op_pk_nfd_set(frame_MPDU_to_be_sent, "Sender ID", mac_attr.sender_id);
		op_pk_nfd_set(frame_MPDU_to_be_sent, "BAN ID", mac_attr.ban_id);
		packet_to_be_sent.recipient_id = mac_attr.recipient_id;
		packet_to_be_sent.sender_id = mac_attr.sender_id;
	}

	/* remove the packet in the head of the queue */
	frame_MPDU_temp = op_subq_pk_remove (SUBQ_DATA, OPC_QPOS_HEAD);

	current_packet_txs = 0;
	current_packet_CS_fails = 0;
	SF.ENABLE_TX_NEW = OPC_FALSE;

	if (SF.IN_MAP_PHASE) {
		if (OPC_TRUE == map_attr.TX_state) {
			wban_send_packet();
		}
	} else {
		csma.CW = CWmin[packet_to_be_sent.user_priority];
		csma.CW_double = OPC_FALSE;
		csma.backoff_counter = 0;
		wban_attempt_TX_CSMA(packet_to_be_sent.user_priority);
	}

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function: wban_attempt_TX_CSMA
 *
 * Description:	CSMA/CA for contention access period
 * 
 * Input:	
 *--------------------------------------------------------------------------------*/
static void wban_attempt_TX_CSMA(int user_priority) {
	//extern double backoff_start_time;
	/* Stack tracing enrty point */
	FIN(wban_attempt_TX_CSMA);

	if (enable_log) {
		fprintf(log," - BACKOFF INIT t=%f \n", op_sim_time());
		printf(" [Node %s] t=%f  - BACKOFF INIT  \n", node_attr.name, op_sim_time());
	}

	wban_backoff_delay_set(user_priority);
	csma.backoff_counter_lock = OPC_FALSE;

	if(!can_fit_TX(&packet_to_be_sent)) {
		current_packet_CS_fails++;
		csma.backoff_counter_lock = OPC_TRUE;

		FOUT;
	} else {
		csma.backoff_counter_lock = OPC_FALSE;
	}
	//CCA
	// op_intrpt_schedule_self (csma.next_slot_start, CCA_START_CODE);
	op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
	if (BACKOFF_EXPIRED && !SF.IN_MAP_PHASE) {

	}
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_backoff_period_boundary_get
 *
 * Description:	it is the absolue time that represents the backoff period boundary, 
 *             
 * No parameters  
 *--------------------------------------------------------------------------------*/
static double wban_backoff_period_boundary_get() {
	double backoff_period_index;
	double next_backoff_period_boundary;
	
	/* Stack tracing enrty point */
	FIN(wban_backoff_period_boundary_get);

	/* phase_start_timeG                      
	       |pSIFS|-----|-----|--t--|-----|-----|-s-|
	                               ^
	                               |
	                      next backoff period index
     */
	backoff_period_index = (int)ceil((op_sim_time() - phase_start_timeG - pSIFS)/pCSMASlotLength2Sec);
	next_backoff_period_boundary = phase_start_timeG + pSIFS + backoff_period_index * pCSMASlotLength2Sec;
	//return (next_backoff_period_boundary);

	/* Stack tracing exit point */
	FRET(next_backoff_period_boundary);
}

/*--------------------------------------------------------------------------------
 * Function:	wban_backoff_delay_set()
 *
 * Description:	set the backoff timer to a random value and generate interupt BACKOFF_EXPIRATION_CODE
 *             
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_backoff_delay_set( int user_priority) {
	double phase_remaining_time;
	
	/* Stack tracing enrty point */
	FIN(wban_backoff_delay_set);
	
	// /* Ignore the non-zero backoff_counter*/
	// if (0 != csma.backoff_counter) {
	// 	FOUT;
	// }
	csma.backoff_counter = floor (op_dist_uniform(csma.CW) + 1); // Randon number of backoffunits
	// op_stat_write(statistic_vector.backoff_units, backoff_unit);
	
	csma.backoff_time = (double) (csma.backoff_counter * pCSMASlotLength2Sec);
	// op_stat_write(statistic_vector.backoff_delay, csma.backoff_time);
	csma.backoff_expiration_time = wban_backoff_period_boundary_get() + csma.backoff_time;
	
	phase_remaining_time = phase_end_timeG - wban_backoff_period_boundary_get();
	// op_intrpt_schedule_self (csma.backoff_expiration_time, BACKOFF_EXPIRATION_CODE);

	if (enable_log) {	
		printf ("-------------------------- BACKOFF -----------------------------------\n");
		printf (" [Node %s] ENTERS BACKOFF STATUT AT %f\n", node_attr.name, op_sim_time());
		printf ("  Beacon Boundary = %f\n", SF.BI_Boundary);
		printf ("  CW = %d\n", csma.CW);
		printf ("  Random Backoff counter = %d\n", csma.backoff_counter);
		printf ("    + Random Backoff time  = %f sec \n", csma.backoff_time);
		printf ("    + Phase Remaining Length = %f sec \n", phase_remaining_time);
		printf ("  Current Time Slot = %d\n", SF.current_slot);
		printf ("  Backoff Boundary = %f sec \n", wban_backoff_period_boundary_get());
        printf ("  Phase Start Time     = %f sec \n", phase_start_timeG);
		printf ("  Phase End Time     = %f sec \n", phase_end_timeG);
		printf ("  Difference       = %f sec \n", phase_end_timeG- wban_backoff_period_boundary_get());
		printf ("  BackOff Expiration Time  = %f sec\n", csma.backoff_expiration_time);
		printf ("----------------------------------------------------------------------------\n\n");
	
		fprintf (log, "-------------------------- BACKOFF -----------------------------------\n");
		fprintf (log, " [Node %s] ENTERS BACKOFF STATUT AT %f\n", node_attr.name, op_sim_time());
		fprintf (log, "  Beacon Boundary = %f\n", SF.BI_Boundary);
		fprintf (log, "  CW = %d\n", csma.CW);
		fprintf (log, "  Random Backoff counter = %d\n", csma.backoff_counter);
		fprintf (log, "    + Random Backoff time  = %f sec \n", csma.backoff_time);
		fprintf (log, "    + Phase Remaining Length = %f sec \n", phase_remaining_time);
		fprintf (log, "  Current Time Slot = %d\n", SF.current_slot);
		fprintf (log, "  Backoff Boundary = %f sec \n", wban_backoff_period_boundary_get());
		fprintf (log, "  Phase Start Time     = %f sec \n", phase_start_timeG);
		fprintf (log, "  Phase End Time     = %f sec \n", phase_end_timeG);
		fprintf (log, "  Difference       = %f sec \n", phase_end_timeG- wban_backoff_period_boundary_get());
		fprintf (log, "  BackOff Expiration Time  = %f sec\n", csma.backoff_expiration_time);
		fprintf (log, "----------------------------------------------------------------------------\n\n");
	}
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function: can_fit_TX
 *
 * Description:	if a transmission fits in the time we have (scheduled or RAP)
 * 
 * Input: packet_to_be_sent_attributes frame_PPDU
 *--------------------------------------------------------------------------------*/
static Boolean can_fit_TX (packet_to_be_sent_attributes* packet_to_be_sent_local) {
	double phase_remaining_time;

	FIN(can_fit_TX);

	if ((!packet_to_be_sent_local)) {
		printf("packet_to_be_sent_local pointer is NULL.\n");
		FRET(OPC_FALSE);
	}

	if (!SF.IN_MAP_PHASE) {
		/*check if the backoff time will exceed the remaining time in the Current Phase */
		csma.backoff_expiration_time = wban_backoff_period_boundary_get() + csma.backoff_time;
		phase_remaining_time = phase_end_timeG - wban_backoff_period_boundary_get();
		/* the backoff is accepted if it expires at most pCSMASlotLength2Sec before the end of Current Phase */
		// if(compare_doubles((phase_remaining_time - pCSMASlotLength2Sec), csma.backoff_time) >=0) {// THERE IS A PROBLEM WITH EQUALITY IN DOUBLE
		// }
		if(packet_to_be_sent_local->ack_policy != N_ACK_POLICY) {
			if (compare_doubles(phase_remaining_time, (TX_TIME((packet_to_be_sent_local->total_bits + packet_to_be_sent_local->ack_bits), node_attr.data_rate)+pSIFS)) >=0) {
				FRET(OPC_TRUE);
			} else {
				printf("No enougth time for N_ACK_POLICY packet transmission in this phase.\n");
				FRET(OPC_FALSE);
			}
		
		} else {
			if (compare_doubles(phase_remaining_time, (TX_TIME((packet_to_be_sent_local->total_bits), node_attr.data_rate))+pSIFS* 0.000001) >=0) {
				FRET(OPC_TRUE);
			} else {
				printf("No enougth time for I_ACK_POLICY packet transmission in this phase.\n");
			}
		}
	}
	FRET(OPC_FALSE);
}

/*--------------------------------------------------------------------------------
 * Function:	wban_send_packet()
 *
 * Description:	it is the absolue time that represents the backoff period boundary, 
 *             
 * No parameters  
 *--------------------------------------------------------------------------------*/
static void wban_send_packet() {
	// double backoff_period_index;
	// double next_backoff_period_boundary;
	double PPDU_tx_time;
	double ack_tx_time;
	double ack_expire_time;

	/* Stack tracing enrty point */
	FIN(wban_send_packet);

	/* create PHY frame (PPDU) that encapsulates beacon MAC frame (MPDU) */
	frame_PPDU_copy = op_pk_create_fmt("wban_frame_PPDU_format");
	op_pk_nfd_set (frame_PPDU_copy, "RATE", node_attr.data_rate);
	/* wrap MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (frame_PPDU_copy, "PSDU", frame_MPDU_to_be_sent);
	op_pk_nfd_set (frame_PPDU_copy, "LENGTH", ((double) op_pk_total_size_get(frame_MPDU_to_be_sent))/8); //[bytes]	

	PPDU_tx_time = TX_TIME(op_pk_total_size_get(frame_PPDU_copy), node_attr.data_rate);
	ack_tx_time = TX_TIME(I_ACK_PPDU_SIZE_BITS, node_attr.data_rate);
	ack_expire_time = op_sim_time() + PPDU_tx_time + ack_tx_time + 2* pSIFS;
	wpan_battery_update_tx((double)op_pk_total_size_get(frame_PPDU_copy));

	switch (packet_to_be_sent.frame_type) {
		case DATA: 
			break;
		case MANAGEMENT: 
			break;
		case COMMAND:
			break;
		default: 
			break;
	}

	switch (packet_to_be_sent.ack_policy) {
		case N_ACK_POLICY:
			op_pk_send(frame_PPDU_copy, STRM_FROM_MAC_TO_RADIO);
			op_intrpt_schedule_self (op_sim_time() + PPDU_tx_time + pSIFS, N_ACK_PACKET_SENT);
			break;
		case I_ACK_POLICY:
			mac_attr.wait_for_ack = OPC_TRUE;
			mac_attr.wait_ack_seq_num = packet_to_be_sent.seq_num;

			current_packet_txs++;
			if (enable_log) {
				fprintf(log,"t=%f   ----------- START TX [DEST_ID = %d, SEQ = %d, with ACK expiring at %f] %d retries  \n\n", op_sim_time(), packet_to_be_sent.recipient_id, mac_attr.wait_ack_seq_num, ack_expire_time,current_packet_txs+current_packet_CS_fails);
				printf(" [Node %s] t=%f  ----------- START TX [DEST_ID = %d, SEQ = %d, with ACK expiring at %f] %d retries \n\n", node_attr.name, op_sim_time(), packet_to_be_sent.recipient_id, mac_attr.wait_ack_seq_num, ack_expire_time,current_packet_txs+current_packet_CS_fails);
			}
			op_pk_send(frame_PPDU_copy, STRM_FROM_MAC_TO_RADIO);
			op_intrpt_schedule_self (op_sim_time() + PPDU_tx_time + 2*pSIFS + ack_tx_time, WAITING_ACK_END_CODE);
			//PPDU_sent_bits = PPDU_sent_bits + ((double)(op_pk_total_size_get(frame_PPDU))/1000.0); // in kbits
			
			// op_stat_write(statistic_global_vector.sent_pkt, (double)(op_pk_total_size_get(frame_PPDU)));
			// op_stat_write(statistic_vector.sent_pkt, (double)(op_pk_total_size_get(frame_PPDU)));
			break;
		case L_ACK_POLICY:
			break;
		case B_ACK_POLL:
			break;
		default: 
			break;
	}
	
	/* Stack tracing exit point */
	FOUT;
}
