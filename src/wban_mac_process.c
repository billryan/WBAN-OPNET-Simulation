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
	Objid b2_attr_comp_id;
	Objid b2_attr_id;
	Objid mac_attr_comp_id;
	Objid mac_attr_id;
	Objid traffic_source_up_id;
	Packet* ack_pk_temp;

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
	// op_ima_obj_attr_get (node_attr.objid, "x position", &node_attr.x);
	// op_ima_obj_attr_get (node_attr.objid, "y position", &node_attr.y);
	// op_ima_obj_attr_get (node_attr.objid, "altitude", &node_attr.altitude);
	op_ima_obj_attr_get (node_attr.objid, "WBAN DATA RATE", &node_attr.data_rate);
	/* get the value of the BAN ID */
	op_ima_obj_attr_get (node_attr.objid, "BAN ID", &node_attr.ban_id);
	mac_attr.ban_id = node_attr.ban_id;
	/* get the Sender Address of the node */
	op_ima_obj_attr_get (node_attr.objid, "Sender Address", &node_attr.sender_address);
	/* Sender Address is not specified - Auto Assigned(-2) frome node objid */
	if (node_attr.sender_address == AUTO_ASSIGNED_NID) {
		node_attr.sender_address = node_attr.objid;
	}
	/* get the value of protocol version */
	op_ima_obj_attr_get (node_attr.objid, "Protocol Version", &node_attr.protocol_ver);
	op_ima_obj_attr_get (node_attr.objid, "MAP Schedule", &node_attr.map_schedule);
	
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
	op_ima_obj_attr_get (mac_attr_comp_id, "DATA Buffer Size", &mac_attr.DATA_buffer_size);
	mac_attr.wait_for_ack = OPC_FALSE;

	/*get the battery attribute ID*/
	node_attr.my_battery = op_id_from_name (node_attr.objid, OPC_OBJTYPE_PROC, "Battery");
	if (node_attr.my_battery == OPC_NIL) {
		op_sim_end("CANNOT FIND THE BATTERY ID","CHECK IF THE NAME OF THE BATTERY MODULE IS [Battery]","","");
	}
	
	wban_log_file_init();

	ack_pk_temp = op_pk_create_fmt ("wban_frame_MPDU_format");
	I_ACK_TX_TIME = TX_TIME(wban_norm_phy_bits(ack_pk_temp), node_attr.data_rate);
	op_pk_destroy(ack_pk_temp);

	/* get the value to check if this node is Hub or not */
	op_ima_obj_attr_get (node_attr.objid, "Device Mode", &node_attr.Device_Mode);
	node_attr.is_BANhub = OPC_FALSE;
	if (strcmp(node_attr.Device_Mode, "Hub") == 0) {
		node_attr.is_BANhub = OPC_TRUE;	
	}

	if (IAM_BAN_HUB) {
		mac_attr.sender_id = node_attr.ban_id + 15; // set the value of HID=BAN ID + 15
		mac_attr.recipient_id = BROADCAST_NID; // default value, usually overwritten

		/* get the beacon attributes for the Hub */
		beacon_attr.sender_address = node_attr.sender_address;
		op_ima_obj_attr_get (node_attr.objid, "Beacon", &beacon_attr_id);
		beacon_attr_comp_id = op_topo_child (beacon_attr_id, OPC_OBJTYPE_GENERIC, 0);

		op_ima_obj_attr_get (beacon_attr_comp_id, "Beacon Period Length", &beacon_attr.beacon_period_length);
		op_ima_obj_attr_get (beacon_attr_comp_id, "Allocation Slot Length", &beacon_attr.allocation_slot_length);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP1 Start", &beacon_attr.rap1_start);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP1 Length", &beacon_attr.rap1_length);
		// op_ima_obj_attr_get (beacon_attr_comp_id, "RAP1 End", &beacon_attr.rap1_end);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP2 Start", &beacon_attr.rap2_start);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP2 End", &beacon_attr.rap2_end);
		op_ima_obj_attr_get (beacon_attr_comp_id, "B2 Start", &beacon_attr.b2_start);
		op_ima_obj_attr_get (beacon_attr_comp_id, "Inactive Duration", &beacon_attr.inactive_duration);
		/* update rap1_end with rap1_start + rap1_length */
		beacon_attr.rap1_end = beacon_attr.rap1_start + beacon_attr.rap1_length - 1;

		/* get the Connection Assignment for the Hub */
		op_ima_obj_attr_get (node_attr.objid, "Connection Assignment", &conn_assign_attr_id);
		conn_assign_attr_comp_id = op_topo_child (conn_assign_attr_id, OPC_OBJTYPE_GENERIC, 0);
		op_ima_obj_attr_get (conn_assign_attr_comp_id, "EAP2 Start", &conn_assign_attr.eap2_start);
		SF.eap2_start = conn_assign_attr.eap2_start;
		if (SF.eap2_start > 0){
			SF.map1_end = SF.eap2_start - 1;
		}

		/* get the beacon2 frame for the Hub*/
		op_ima_obj_attr_get (node_attr.objid, "Beacon2", &b2_attr_id);
		b2_attr_comp_id = op_topo_child (b2_attr_id, OPC_OBJTYPE_GENERIC, 0);
		op_ima_obj_attr_get (b2_attr_comp_id, "CAP End", &b2_attr.cap_end);
		op_ima_obj_attr_get (b2_attr_comp_id, "MAP2 End", &b2_attr.map2_end);
		SF.cap_end = b2_attr.cap_end;
		SF.map2_end = b2_attr.map2_end;

		SF.current_first_free_slot = SF.rap1_end + 1;

		// register the beacon frame statistics
		beacon_frame_hndl = op_stat_reg ("MANAGEMENT.Number of Generated Beacon Frame", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
		wban_send_beacon_frame ();
	} else { /* if the node is not a Hub */
		mac_attr.sender_id = UNCONNECTED_NID;
		mac_attr.recipient_id = UNCONNECTED;
		//node_attr.unconnectedNID = 1 + (unconnectedNID - 1) % 16;
		//unconnectedNID++;
		node_attr.unconnectedNID = node_attr.objid;
		
		if (enable_log) {
			// fprintf (log," [Node %s] initialized with unconnectedNID %d \n\n", node_attr.name, node_attr.unconnectedNID);
			printf (" [Node %s] initialized with unconnectedNID %d \n\n", node_attr.name, node_attr.unconnectedNID);
		}
		/* get the Connection Request for the Node */
		op_ima_obj_attr_get (node_attr.objid, "Connection Request", &conn_req_attr_id);
		conn_req_attr_comp_id = op_topo_child (conn_req_attr_id, OPC_OBJTYPE_GENERIC, 0);
		// op_ima_obj_attr_get (conn_req_attr_comp_id, "Requested Wakeup Phase", &conn_req_attr.requested_wakeup_phase);
		// op_ima_obj_attr_get (conn_req_attr_comp_id, "Requested Wakeup Period", &conn_req_attr.requested_wakeup_period);
		// op_ima_obj_attr_get (conn_req_attr_comp_id, "Minimum Length", &conn_req_attr.minimum_length);
		op_ima_obj_attr_get (conn_req_attr_comp_id, "Allocation Length", &conn_req_attr.allocation_length);
		/* start assigning connected NID from ID 32 */
		// node_attr.sender_id = 32 + ((current_free_connected_NID - 32) % 214);
		// current_free_connected_NID++;
		// node_attr.recipient_id = node_attr.connectedHID;
		
		// SF.eap2_start = 0;
		// beacon_attr.beacon_period_length = -1;
	}
	mac_state = MAC_SETUP;
	SF.SLEEP = OPC_TRUE;
	SF.ENABLE_TX_NEW = OPC_FALSE;

	/* register the statistics */
	stat_vec.data_pkt_fail = op_stat_reg("DATA.Data Packet failed", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	stat_vec.data_pkt_suc1 = op_stat_reg("DATA.Data Packet Succed 1", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	stat_vec.data_pkt_suc2 = op_stat_reg("DATA.Data Packet Succed 2", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	stat_vec.data_pkt_sent = op_stat_reg("DATA.Data Packet Sent", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	stat_vec.up7_sent = op_stat_reg("DATA.UP7 Sent", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	stat_vec.up5_sent = op_stat_reg("DATA.UP5 Sent", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	stat_vec.data_pkt_rec = op_stat_reg("DATA.Data Packet Received", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	stat_vec.data_pkt_rec_map1 = op_stat_reg("DATA.Data Packet Received MAP1", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	/* register the GLOBAL statistics */
	stat_vecG.data_pkt_fail = op_stat_reg("DATA.Data Packet failed", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	stat_vecG.data_pkt_suc1 = op_stat_reg("DATA.Data Packet Succed 1", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	stat_vecG.data_pkt_suc2 = op_stat_reg("DATA.Data Packet Succed 2", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	stat_vecG.data_pkt_sent = op_stat_reg("DATA.Data Packet Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	stat_vecG.up7_sent = op_stat_reg("DATA.UP7 Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	stat_vecG.up5_sent = op_stat_reg("DATA.UP5 Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	stat_vecG.data_pkt_rec = op_stat_reg("DATA.Data Packet Received", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);


	stat_vec.ppdu_sent_kbits = 0;
	stat_vec.ppdu_sent_nbr = 0;
	stat_vec.ppdu_sent_kbits_rap = 0;
	stat_vec.ppdu_sent_nbr_rap = 0;
	// for Hub only
	stat_vec.ppdu_rcv_kbits = 0;
	stat_vec.ppdu_rcv_nbr = 0;
	stat_vec.ppdu_rcv_kbits_rap = 0;
	stat_vec.ppdu_rcv_nbr_rap = 0;
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
	time_t timep;
	struct tm *p;

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

	time(&timep);
	p=localtime(&timep);
	printf("%d%d%d",(1900+p->tm_year), (1+p->tm_mon),p->tm_mday);
	printf("%d;%d;%d\n", p->tm_hour, p->tm_min, p->tm_sec);

	if (enable_log) {
		sprintf (log_name, "%s%s-%d%d-%d%d%d-ver%d-map%d.trace", directory_path_name, node_attr.name, (1+p->tm_mon), p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec, node_attr.protocol_ver, node_attr.map_schedule);
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
	int packet_size;
	int ppdu_bits;

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
			ppdu_bits = op_pk_total_size_get(rcv_frame);
    		PPDU_rcv_nbr = PPDU_rcv_nbr + 1;
    		PPDU_rcv_kbits = PPDU_rcv_kbits + 1.0*ppdu_bits/1000.0;
			/* get MAC frame (MPDU=PSDU) from received PHY frame (PPDU)*/
			op_pk_nfd_get_pkt (rcv_frame, "PSDU", &frame_MPDU);
			/*update the battery*/
			packet_size = op_pk_total_size_get(frame_MPDU);
			printf("Received packet size=%d.\n", packet_size);
			if (MAC_SLEEP == mac_state){
				FOUT;
			}
			
			// wban_battery_update_rx (frame_MPDU);
			op_pk_nfd_get (frame_MPDU, "BAN ID", &ban_id);
			op_pk_nfd_get (frame_MPDU, "Recipient ID", &recipient_id);
			op_pk_nfd_get (frame_MPDU, "Sender ID", &sender_id);
			// filter the incoming BAN packet - not implemented entirely
    		if (!is_packet_for_me(frame_MPDU, ban_id, recipient_id, sender_id)) {
    			FOUT;
    		} else {
				wban_battery_update_rx (frame_MPDU);
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
			
			if(I_ACK_POLICY == ack_policy_fd){
				ack_seq_num = sequence_number_fd;
				op_intrpt_schedule_self(op_sim_time()+pSIFS, SEND_I_ACK);
			}
			switch (frame_type_fd) {
				case DATA: /* Handle data packets */
					if (enable_log) {
						printf (" [Node %s] t=%f  !!!!!!!!! Data Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
					}
					stat_vec.ppdu_rcv_nbr = stat_vec.ppdu_rcv_nbr + 1;
					stat_vec.ppdu_rcv_kbits = stat_vec.ppdu_rcv_kbits + 1.0*ppdu_bits/1000.0;
					if((mac_state == MAC_RAP1) && (node_attr.is_BANhub)){
						stat_vec.ppdu_rcv_kbits_rap = stat_vec.ppdu_rcv_kbits_rap + 1.0*ppdu_bits/1000.0;
						stat_vec.ppdu_rcv_nbr_rap = stat_vec.ppdu_rcv_nbr_rap + 1;
					}
					op_stat_write(stat_vec.data_pkt_rec, op_pk_total_size_get(frame_MPDU));
					op_stat_write(stat_vecG.data_pkt_rec, op_pk_total_size_get(frame_MPDU));

					wban_extract_data_frame (frame_MPDU);
					/* send to higher layer for statistics */
					op_pk_send (frame_MPDU, STRM_FROM_MAC_TO_SINK);
					break;
				case MANAGEMENT: /* Handle management packets */
					fprintf(log,"RX,MAN_ALL,t=%f,pk size of MAN_MPDU=%d,received from ID=%d\n", op_sim_time(), packet_size, sender_id);
					// op_stat_write(stat_vec.data_pkt_rec, 0.0);
			 		if (enable_log) {
						// fprintf (log,"t=%f  !!!!!!!!! Management Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), sender_id);
						printf (" [Node %s] t=%f  !!!!!!!!! Management Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
					}
					switch (frame_subtype_fd) {
						case BEACON: 
							fprintf(log,"RX,MAN_BEACON,t=%f,pk size of MAN_MPDU=%d,received from ID=%d\n", op_sim_time(), packet_size, sender_id);
							if (enable_log) {
								// fprintf (log,"t=%f  -> Beacon Frame Reception From @%d \n\n", op_sim_time(), sender_id);
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
							if (enable_log) {
								fprintf(log,"RX,MAN_CONN_REQ,t=%f,pk size of MAN_MPDU=%d,received from ID=%d\n", op_sim_time(), packet_size, sender_id);
								printf (" [Node %s] t=%f  !!!!!!!!! Connection request Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
							}
							wban_extract_conn_req_frame (frame_MPDU);
							break;
						case CONNECTION_ASSIGNMENT:
						{
							fprintf(log,"RX,MAN_CONN_ASSIGN,t=%f,pk size of MAN_MPDU=%d,received from ID=%d\n", op_sim_time(), packet_size, sender_id);
							if (enable_log) {
								// fprintf (log,"t=%f  !!!!!!!!! Connection assignment Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), sender_id);
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
					op_prg_odb_bkpt("rcv_con");
			 		if (enable_log) {
						// fprintf (log,"t=%f  !!!!!!!!! Control Frame Reception From @%d !!!!!!!!! \n\n", op_sim_time(), sender_id);
						printf (" [Node %s] t=%f  !!!!!!!!! Control Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
					}
					switch (frame_subtype_fd) {
						case I_ACK:
							wban_extract_i_ack_frame (frame_MPDU);
							break;
						case B_ACK:
							// not implemented
							break;
						case BEACON2: 
						{	
							op_prg_odb_bkpt("rcv_b2");
							wban_extract_beacon2_frame(frame_MPDU);
							break;
						}
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
					break;
				default:	/*OTHER FRAME TYPES*/
					printf("Node %s received none of the frame above.\n", node_attr.name);
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
	op_pk_destroy (rcv_frame);
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
	
	/*Check if the frame is loop*/
	if (mac_attr.sender_id == sender_id) {
		// if (enable_log) {
		// 	fprintf(log,"t=%f  -> Loop: DISCARD FRAME \n\n",op_sim_time());
			printf (" [Node %s] t=%f  -> Loop: DISCARD FRAME \n\n",node_attr.name, op_sim_time());
		// }
		op_pk_destroy (frame_MPDU);

		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}

	if (node_attr.ban_id != ban_id) {
		if (enable_log) {
			// fprintf (log,"The packet from BAN ID %d is not the same with my BAN ID %d.\n", ban_id, node_attr.ban_id);
			printf (" [Node %s] The packet from BAN ID %d is not the same with my BAN ID %d.\n", node_attr.name, ban_id, node_attr.ban_id);
		}

		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}

	if ((mac_attr.sender_id == recipient_id) || (BROADCAST_NID == recipient_id)) {
		/* Stack tracing exit point */
		FRET(OPC_TRUE);
	} else {
		printf (" [Node %s] t=%f  -> Not the frame for me: DISCARD FRAME \n\n",node_attr.name, op_sim_time());
		op_pk_destroy (frame_MPDU);
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
	double beacon_frame_tx_time;
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
	op_pk_nfd_set (beacon_MPDU, "Recipient ID", BROADCAST_NID);
	op_pk_nfd_set (beacon_MPDU, "Sender ID", mac_attr.sender_id);
	op_pk_nfd_set (beacon_MPDU, "BAN ID", mac_attr.ban_id);
	
	op_pk_nfd_set_pkt (beacon_MPDU, "MAC Frame Payload", beacon_MSDU); // wrap beacon payload (MSDU) in MAC Frame (MPDU)

	SF.BI_Boundary = op_pk_creation_time_get (beacon_MPDU);
	beacon_frame_tx_time = TX_TIME(wban_norm_phy_bits(beacon_MPDU), node_attr.data_rate);
	op_prg_odb_bkpt("send_beacon");
	SF.eap1_start2sec = SF.BI_Boundary + beacon_frame_tx_time + pSIFS;

	/* send the MPDU to PHY and calculate the energer consuming */
	wban_send_mac_pk_to_phy(beacon_MPDU);
	
	wban_schedule_next_beacon (); // Maybe for updating the parameters of superframe
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_send_beacon2_frame
 *
 * Description:	Create a beacon frame and send it to the Radio  
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_send_beacon2_frame () {
	Packet* beacon2_MSDU;
	Packet* beacon2_MPDU;
	double beacon2_frame_tx_time;
	double update_conn_assgin_start;

	/* Stack tracing enrty point */
	FIN(wban_send_beacon2_frame);
	
	/* create a beacon2 frame */
	beacon2_MSDU = op_pk_create_fmt ("wban_beacon2_MSDU_format");
	
	/* set the fields of the beacon2 frame */
	op_pk_nfd_set (beacon2_MSDU, "Beacon Period Length", beacon_attr.beacon_period_length);
	op_pk_nfd_set (beacon2_MSDU, "Allocation Slot Length", beacon_attr.allocation_slot_length);
	op_pk_nfd_set (beacon2_MSDU, "CAP End", b2_attr.cap_end);
	op_pk_nfd_set (beacon2_MSDU, "MAP2 End", b2_attr.map2_end);
	
	/* create a MAC frame (MPDU) that encapsulates the beacon2 payload (MSDU) */
	beacon2_MPDU = op_pk_create_fmt ("wban_frame_MPDU_format");

	op_pk_nfd_set (beacon2_MPDU, "Ack Policy", N_ACK_POLICY);
	op_pk_nfd_set (beacon2_MPDU, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (beacon2_MPDU, "Frame Subtype", BEACON2);
	op_pk_nfd_set (beacon2_MPDU, "Frame Type", CONTROL);
	op_pk_nfd_set (beacon2_MPDU, "B2", 1); // beacon2 enabled
	op_pk_nfd_set (beacon2_MPDU, "Sequence Number", rand_int(256));
	op_pk_nfd_set (beacon2_MPDU, "Inactive", beacon_attr.inactive_duration); // beacon and beacon2 frame used
	op_pk_nfd_set (beacon2_MPDU, "Recipient ID", BROADCAST_NID);
	op_pk_nfd_set (beacon2_MPDU, "Sender ID", mac_attr.sender_id);
	op_pk_nfd_set (beacon2_MPDU, "BAN ID", mac_attr.ban_id);
	
	op_pk_nfd_set_pkt (beacon2_MPDU, "MAC Frame Payload", beacon2_MSDU); // wrap beacon payload (MSDU) in MAC Frame (MPDU)

	/* send the MPDU to PHY and calculate the energer consuming */
	wban_send_mac_pk_to_phy(beacon2_MPDU);
	// op_prg_odb_bkpt("send_b2");

	beacon2_frame_tx_time = TX_TIME(wban_norm_phy_bits(beacon2_MPDU), node_attr.data_rate);
	SF.cap_start2sec = SF.b2_start2sec + beacon2_frame_tx_time + pSIFS;
	// SF.map2_end = b2_attr.map2_end;
	/* Additional processing for proposed protocol */
	if (1 == node_attr.protocol_ver) {
		SF.cap_start2sec = SF.BI_Boundary + (SF.map2_end+1) * SF.slot_length2sec;
		update_conn_assgin_start = SF.b2_start2sec + pSIFS;
		op_intrpt_schedule_self(update_conn_assgin_start, UPDATE_OF_CONN_ASSGIN);
	}

	if (enable_log) {
		// fprintf(log,"t=%f  -> Beacon2 Frame transmission. \n\n", op_sim_time());
		printf(" [Node %s] t=%f  -> Beacon2 Frame transmission. \n\n", node_attr.name, op_sim_time());
	}

	op_intrpt_schedule_self(SF.cap_start2sec, START_OF_CAP_PERIOD_CODE);
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
	int allocation_length;
	int user_priority;
	Packet* frame_MSDU;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_conn_req_frame);

	op_pk_nfd_get (frame_MPDU, "Sequence Number", &ack_seq_num);
	op_pk_nfd_get (frame_MPDU, "Sender ID", &mac_attr.recipient_id);
	// op_intrpt_schedule_self(op_sim_time()+pSIFS, SEND_I_ACK);
	op_intrpt_schedule_self(op_sim_time()+2*pSIFS+I_ACK_TX_TIME, TRY_PACKET_TRANSMISSION_CODE);

	op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
	op_pk_nfd_get (frame_MSDU, "Allocation Length", &allocation_length);
	op_pk_nfd_get (frame_MSDU, "User Priority", &user_priority);

	conn_assign_attr.allocation_length = allocation_length;
	printf("NID=%d Allocation Length=%d.\n", mac_attr.recipient_id, allocation_length);
	op_prg_odb_bkpt("rcv_req");
	// wban_send_conn_assign
	wban_send_conn_assign_frame(allocation_length);
	// For sure that the allocation length > 0
 	// if (allocation_length > 0) {
 	// 	op_intrpt_schedule_self(op_sim_time()+pSIFS+I_ACK_TX_TIME, SEND_CONN_ASSIGN_CODE);
 	// }
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

	op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
	// op_pk_nfd_get (frame_MSDU, "EAP2 Start", &conn_assign_attr.eap2_start);
	op_pk_nfd_get (frame_MSDU, "Interval Start", &conn_assign_attr.interval_start);
	op_pk_nfd_get (frame_MSDU, "Interval End", &conn_assign_attr.interval_end);

	/* update the parameters of Superframe */
	// SF.eap2_start = conn_assign_attr.eap2_start;
	// if(0 < SF.eap2_start){
	// 	SF.eap2_end = SF.rap2_start - 1;
	// } else {
	// 	SF.eap2_end = 0;
	// 	if(0 < SF.b2_start){
	// 		SF.map1_end = SF.b2_start - 1;
	// 	}
	// }

	printf("Node %s assigned with Interval Start %d slot, Interval End %d slot.\n", node_attr.name, conn_assign_attr.interval_start, conn_assign_attr.interval_end);
	op_prg_odb_bkpt("rcv_assign");
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
	int sequence_number_fd;
	int eap_indicator_fd;
	int beacon2_enabled_fd;
	double beacon_frame_tx_time;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_beacon_frame);

	// printf("size of beacon_MPDU_rx=%d\n", op_pk_total_size_get(beacon_MPDU_rx));
	// printf("size of normalized beacon_MPDU_rx=%d\n", wban_norm_phy_bits(beacon_MPDU_rx));
	beacon_frame_tx_time = TX_TIME(wban_norm_phy_bits(beacon_MPDU_rx), node_attr.data_rate);
	// printf("beacon frame tx time=%f\n", beacon_frame_tx_time);
	op_pk_nfd_get_pkt (beacon_MPDU_rx, "MAC Frame Payload", &beacon_MSDU_rx);
	// if I'm a End Device, I get the information and synchronize myself
	if (!node_attr.is_BANhub) {
		op_pk_nfd_get (beacon_MPDU_rx, "Sender ID", &rcv_sender_id);
		op_pk_nfd_get (beacon_MPDU_rx, "Sequence Number", &sequence_number_fd);
		op_pk_nfd_get (beacon_MPDU_rx, "EAP Indicator", &eap_indicator_fd);
		op_pk_nfd_get (beacon_MPDU_rx, "B2", &beacon2_enabled_fd);

		if (node_attr.ban_id + 15 != rcv_sender_id) {
			if (enable_log) {
				// fprintf(log,"t=%f  -> Beacon Frame Reception  - but not from Hub. \n",op_sim_time());	
				printf(" [Node %s] t=%f  -> Beacon Frame Reception - but not from Hub. \n", node_attr.name, op_sim_time());
			}
			/* Stack tracing exit point */
			FOUT;
		} else {
			if (enable_log) {
				// printf (" [Node %s] t=%f  -> Beacon Frame Reception - synchronization. \n", node_attr.name, op_sim_time());
				printf ("   -> Sequence Number              : %d \n", sequence_number_fd);
			}
		}
		op_pk_nfd_get (beacon_MSDU_rx, "Sender Address", &beacon_attr.sender_address);
		op_pk_nfd_get (beacon_MSDU_rx, "Beacon Period Length", &beacon_attr.beacon_period_length);
		op_pk_nfd_get (beacon_MSDU_rx, "Allocation Slot Length", &beacon_attr.allocation_slot_length);
		op_pk_nfd_get (beacon_MSDU_rx, "RAP1 End", &beacon_attr.rap1_end);
		// op_pk_nfd_get (beacon_MSDU_rx, "RAP2 Start", &beacon_attr.rap2_start);
		// op_pk_nfd_get (beacon_MSDU_rx, "RAP2 End", &beacon_attr.rap2_end);
		op_pk_nfd_get (beacon_MSDU_rx, "RAP1 Start", &beacon_attr.rap1_start);
		op_pk_nfd_get (beacon_MSDU_rx, "B2 Start", &beacon_attr.b2_start);
		op_pk_nfd_get (beacon_MSDU_rx, "Inactive Duration", &beacon_attr.inactive_duration);

		// update with actual Hub id
		if (HUB_ID == node_attr.traffic_dest_id) {
			node_attr.traffic_dest_id = rcv_sender_id;
		}
		/*update rap length*/
		beacon_attr.rap1_length = beacon_attr.rap1_end - beacon_attr.rap1_start + 1;
		SF.BI_Boundary = op_pk_creation_time_get (beacon_MPDU_rx);
		SF.eap1_start2sec = SF.BI_Boundary + beacon_frame_tx_time + pSIFS;
		op_prg_odb_bkpt("rcv_beacon");
		op_pk_destroy (beacon_MSDU_rx);
		op_pk_destroy (beacon_MPDU_rx);

		mac_attr.recipient_id = rcv_sender_id;
		if (UNCONNECTED_NID == mac_attr.sender_id) {
			/* We will try to connect to this BAN  if our scheduled access length
			 * is NOT set to unconnected (-1). If it is set to 0, it means we are
			 * establishing a sleeping pattern and waking up only to hear beacons
			 * and are only able to transmit in RAP periods.
			 */
			/* initialize the NID from 32 */
			mac_attr.sender_id = current_free_connected_NID;
			current_free_connected_NID++;
			// printf("Node %s get the NID=%d\n", node_attr.name, mac_attr.sender_id);
			fprintf(log,"NID=%d\n", mac_attr.sender_id);
			op_prg_odb_bkpt("get_nid");
			// mac_attr.sender_id = node_attr.objid; // we simply use objid as sender_id
		}
		wban_schedule_next_beacon();
	}
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_extract_beacon2_frame
 *
 * Description:	extract the beacon2 frame from the MAC frame received from the network
 *              and schedule the next beacon frame
 *
 * Input :  mac_frame - the received MAC frame
 *--------------------------------------------------------------------------------*/
static void wban_extract_beacon2_frame(Packet* beacon2_MPDU_rx) {
	Packet* beacon2_MSDU_rx;
	int beacon2_PPDU_size;
	int rcv_sender_id;
	int sequence_number_fd;
	int eap_indicator_fd;
	int beacon2_enabled_fd;
	double beacon2_frame_tx_time;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_beacon2_frame);

	op_prg_odb_bkpt("extract_b2");
	beacon2_PPDU_size = op_pk_total_size_get(beacon2_MPDU_rx) + PHY_HEADER_SIZE;

	op_pk_nfd_get_pkt (beacon2_MPDU_rx, "MAC Frame Payload", &beacon2_MSDU_rx);
	// if I'm a Node, I get the information and synchronize myself
	if (!node_attr.is_BANhub) {
		op_pk_nfd_get (beacon2_MPDU_rx, "Sender ID", &rcv_sender_id);
		op_pk_nfd_get (beacon2_MPDU_rx, "Sequence Number", &sequence_number_fd);
		op_pk_nfd_get (beacon2_MPDU_rx, "EAP Indicator", &eap_indicator_fd);
		op_pk_nfd_get (beacon2_MPDU_rx, "B2", &beacon2_enabled_fd);

		if (node_attr.ban_id + 15 != rcv_sender_id) {
			if (enable_log) {
				// fprintf(log,"t=%f  -> Beacon2 Frame Reception  - but not from Hub. \n",op_sim_time());	
				printf(" [Node %s] t=%f  -> Beacon2 Frame Reception - but not from Hub. \n", node_attr.name, op_sim_time());
			}
			/* Stack tracing exit point */
			FOUT;
		} else {
			if (enable_log) {
				printf (" [Node %s] t=%f  -> Beacon2 Frame Reception - synchronization. \n", node_attr.name, op_sim_time());
				printf ("   -> Sequence Number              : %d \n", sequence_number_fd);
			}
		}
		// op_pk_nfd_get (beacon2_MSDU_rx, "Beacon Period Length", &beacon_attr.beacon_period_length);
		// op_pk_nfd_get (beacon2_MSDU_rx, "Allocation Slot Length", &beacon_attr.allocation_slot_length);
		op_pk_nfd_get (beacon2_MSDU_rx, "CAP End", &b2_attr.cap_end);
		op_pk_nfd_get (beacon2_MSDU_rx, "MAP2 End", &b2_attr.map2_end);
		SF.cap_end = b2_attr.cap_end;

		beacon2_frame_tx_time = TX_TIME(beacon2_PPDU_size, node_attr.data_rate);
		printf("SF.BI_Boundary=%f\n", SF.BI_Boundary);
		printf("SF.b2_start2sec=%f\n", SF.b2_start2sec);
		printf("beacon2_frame_tx_time=%f\n", beacon2_frame_tx_time);
		printf("SF.b2_start2sec + beacon2_frame_tx_time + pSIFS=");
		printf("%f\n", SF.b2_start2sec + beacon2_frame_tx_time + pSIFS);
		SF.cap_start2sec = max_double(SF.b2_start2sec + beacon2_frame_tx_time + pSIFS, op_sim_time());
		if(0 == SF.cap_end){
			SF.cap_end2sec = SF.BI_Boundary + SF.SD*SF.slot_length2sec;
		} else {
			SF.cap_end2sec = SF.BI_Boundary + SF.cap_end * SF.slot_length2sec;
		}
		/* Additional processing for proposed protocol */
		if (1 == node_attr.protocol_ver) {
			SF.map2_end = b2_attr.map2_end;
			SF.cap_start2sec = SF.BI_Boundary + (SF.map2_end+1) * SF.slot_length2sec;
			printf("SF.cap_start2sec replaced with %f for protocol version 1.\n", SF.cap_start2sec);
		}
		op_intrpt_schedule_self(SF.cap_start2sec, START_OF_CAP_PERIOD_CODE);
		op_intrpt_schedule_self(SF.cap_end2sec, END_OF_CAP_PERIOD_CODE);

		op_pk_destroy (beacon2_MSDU_rx);
		op_pk_destroy (beacon2_MPDU_rx);
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
	// SF.rap2_start = beacon_attr.rap2_start;
	// SF.rap2_end = beacon_attr.rap2_end;
	SF.b2_start = beacon_attr.b2_start;
	SF.current_slot = 0;
	// SF.current_first_free_slot = beacon_attr.rap1_end + 1; // spec for hub assignment

	SF.rap1_start2sec = SF.BI_Boundary + SF.rap1_start * SF.slot_length2sec + pSIFS;
	SF.eap1_end2sec = SF.rap1_start2sec - pSIFS;
	SF.rap1_end2sec = SF.BI_Boundary + (SF.rap1_end+1)*SF.slot_length2sec;
	SF.b2_start2sec = SF.BI_Boundary + SF.b2_start * SF.slot_length2sec + pSIFS;
	if(SF.b2_start == 0){
		SF.b2_start2sec = 0;
	}

	// SF.eap1_length2sec = SF.rap1_start * SF.slot_length2sec - beacon_frame_tx_time;
	SF.rap1_length2sec = (SF.rap1_end - SF.rap1_start + 1) * SF.slot_length2sec;

	SF.current_first_free_slot = SF.rap1_end + 1; // spec for hub assignment

	/* for node we should calculate the slot boundary */
	SF.current_slot = (int)floor((op_sim_time()-SF.BI_Boundary)/SF.slot_length2sec);

	op_prg_odb_bkpt("sch_beacon");
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
			SF.map1_start2sec = SF.rap1_end2sec;
			op_intrpt_schedule_self (SF.map1_start2sec, START_OF_MAP1_PERIOD_CODE);
		} else {

		}
	}

	if (SF.b2_start > 0) {
		if(OPC_TRUE == node_attr.is_BANhub){
			SF.map1_end = SF.b2_start;
			SF.map1_end2sec = SF.b2_start2sec - pSIFS;
			op_intrpt_schedule_self (SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
			op_intrpt_schedule_self (SF.b2_start2sec, SEND_B2_FRAME);
		} else {
			/* used for setup of Node */
			op_intrpt_schedule_self (SF.b2_start2sec, SEND_B2_FRAME);
		}
	} else {
		if(OPC_TRUE == node_attr.is_BANhub){
			SF.map1_end = SF.SD - 1;
			SF.map1_end2sec = SF.BI_Boundary + SF.BI*SF.slot_length2sec;
		}
	}
	// op_intrpt_schedule_self (SF.BI_Boundary + SF.SD*SF.slot_length2sec, START_OF_SLEEP_PERIOD);
	op_intrpt_schedule_self (SF.BI_Boundary + SF.BI*SF.slot_length2sec, BEACON_INTERVAL_CODE);
	// op_intrpt_schedule_remote (SF.BI_Boundary + SF.BI*SF.slot_length2sec, END_OF_SLEEP_PERIOD, node_attr.my_battery);
	//op_intrpt_priority_set (OPC_INTRPT_SELF, END_OF_CAP_PERIOD_CODE, -2);
	if (enable_log) {
		printf("Node %s Superframe parameters:\n", node_attr.name);
		printf("\tSF.eap1_start2sec=%f\n", SF.eap1_start2sec);
		printf("\tSF.eap1_end2sec=%f\n", SF.eap1_end2sec);
		printf("\tSF.rap1_start2sec=%f\n", SF.rap1_start2sec);
		printf("\tSF.rap1_end2sec=%f\n", SF.rap1_end2sec);
		printf("\tSF.map1_start2sec=%f\n", SF.map1_start2sec);
		printf("\tSF.map1_end2sec=%f\n", SF.map1_end2sec);
		printf("\tSF.map2_start2sec=%f\n", SF.map2_start2sec);
		printf("\tSF.map2_end2sec=%f\n", SF.map2_end2sec);

		// fprintf (log,"t=%f  -> Schedule Next Beacon at %f\n\n", op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec);
		printf (" [Node %s] t=%f  -> Schedule Next Beacon at %f\n\n", node_attr.name, op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec);
	}
	if(!IAM_BAN_HUB){
		conn_assign_attr.interval_start = 255;
		conn_assign_attr.interval_end = 0;
		wban_send_conn_req_frame();
	}
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_send_conn_req_frame
 *
 * Description:	Create a connection request frame and send it to the Radio (wban_mac) 
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_send_conn_req_frame () {
	Packet* conn_req_MSDU;
	Packet* conn_req_MPDU;
	int random_num;
	// double ph_tx_time; // PHY Header tx time
	// double subq_data_tx_time_total;
	// double conn_req_tx_time;
	// double conn_req_round;

	/* Stack tracing enrty point */
	FIN(wban_send_conn_req_frame);
	// calculate the normalized PHY bits
	// op_pk_total_size_set (empty_MPDU, 0);
	// ph_tx_time = TX_TIME(wban_norm_phy_bits(empty_MPDU), node_attr.data_rate);
	// printf("PHY Header tx time=%f.\n", ph_tx_time);
	/* get the queue infomation of SUBQ_DATA */
	// subq_data_tx_time_total = subq_info.pksize*pSIFS + subq_info.bitsize / (node_attr.data_rate*1000);
	// if(0 == node_attr.map_schedule){
	// 	conn_req_attr.allocation_length = (int)(subq_data_tx_time_total / SF.slot_length2sec + 0.4);
	// }
	printf("Connection Request allocation_length=%d for Node %s\n", conn_req_attr.allocation_length, node_attr.name);
	if (conn_req_attr.allocation_length > 0) {
		if(conn_req_attr.allocation_length > 3){
			/* maximum allocation length for a node is 3 */
			conn_req_attr.allocation_length = 3;
		}
	} else {
		printf("Node %s need not scheduling slots.\n", node_attr.name);
		FOUT;
	}
	random_num = rand_int(256);
	/* create a connection request frame */
	conn_req_MSDU = op_pk_create_fmt("wban_connection_request_frame_format");
	
	/* set the fields of the connection_request frame */
	op_pk_nfd_set (conn_req_MSDU, "Allocation Length", conn_req_attr.allocation_length);
	// op_pk_nfd_set (conn_req_MSDU, "User Priority", subq_info.up);

	/* create a MAC frame (MPDU) that encapsulates the connection_request payload (MSDU) */
	conn_req_MPDU = op_pk_create_fmt ("wban_frame_MPDU_format");

	op_pk_nfd_set (conn_req_MPDU, "Ack Policy", I_ACK_POLICY);
	op_pk_nfd_set (conn_req_MPDU, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (conn_req_MPDU, "Frame Subtype", CONNECTION_REQUEST);
	op_pk_nfd_set (conn_req_MPDU, "Frame Type", MANAGEMENT);
	op_pk_nfd_set (conn_req_MPDU, "B2", 1); // beacon2 enabled

	op_pk_nfd_set (conn_req_MPDU, "Sequence Number", random_num);
	op_pk_nfd_set (conn_req_MPDU, "Inactive", beacon_attr.inactive_duration); // connection_request and connection_request2 frame used

	op_pk_nfd_set (conn_req_MPDU, "Recipient ID", mac_attr.recipient_id);
	op_pk_nfd_set (conn_req_MPDU, "Sender ID", mac_attr.sender_id);
	op_pk_nfd_set (conn_req_MPDU, "BAN ID", mac_attr.ban_id);
	
	op_pk_nfd_set_pkt (conn_req_MPDU, "MAC Frame Payload", conn_req_MSDU); // wrap connection_request payload (MSDU) in MAC Frame (MPDU)

	// phy_to_radio(conn_assign_MPDU);

	// conn_req_tx_time = TX_TIME(wban_norm_phy_bits(conn_req_MPDU), node_attr.data_rate);
	// printf("conn_req_tx_time=%f\n", conn_req_tx_time);
	// printf("I_ACK_TX_TIME=%f\n", I_ACK_TX_TIME);
	// conn_req_round = conn_req_tx_time + 2*pSIFS + I_ACK_TX_TIME;
	// printf("conn_req_round=%f\n",conn_req_round);
	op_prg_odb_bkpt("pre_req");

	// op_intrpt_schedule_self (SF.rap1_end2sec+2*conn_req_round*(node_attr.objid % 8), SEND_CONN_REQ_CODE);

	/* put it into the queue with priority waiting for transmission */
	op_pk_priority_set (conn_req_MPDU, 6.0);
	if (op_subq_pk_insert(SUBQ_MAN, conn_req_MPDU, OPC_QPOS_TAIL) == OPC_QINS_OK) {
		if (enable_log) {
			// fprintf (log,"t=%f  -> Enqueuing of MAC MANAGEMENT frame [SEQ = %d]\n\n", op_sim_time(), random_num);
			printf (" [Node %s] t=%f  -> Enqueuing of MAC MANAGEMENT frame [SEQ = %d]\n\n", node_attr.name, op_sim_time(), random_num);
		}
	} else {
		if (enable_log) {
			// fprintf (log,"t=%f  -> MAC MANAGEMENT frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", op_sim_time());
			printf (" [Node %s] t=%f  -> MAC MANAGEMENT frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", node_attr.name, op_sim_time());
		}
		/* destroy the packet */
		op_pk_destroy (conn_req_MPDU);
	}

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
	int random_num;
	// double conn_assign_tx_time;
	int i;

	/* Stack tracing enrty point */
	FIN(wban_send_conn_assign_frame);

	random_num = rand_int(256);
	/* create a connection request frame */
	conn_assign_MSDU = op_pk_create_fmt ("wban_connection_assignment_frame_format");
	
	/* set the fields of the conn_assign frame */
	op_pk_nfd_set (conn_assign_MSDU, "Connection Status", 0);

	if(0 == node_attr.protocol_ver){
		if(SF.current_first_free_slot <= SF.current_slot){
			SF.current_first_free_slot = SF.current_slot + 1;
		}
		if(0 < SF.b2_start){
			/* do not use the second part of EAP2/RAP2/MAP */
			if(SF.current_first_free_slot + allocation_length < SF.b2_start + 1){
				/* allocation within the first MAP */
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
				printf("There are no enougth slots for scheduling.\n");
			}
		} else {
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
				} else {
					conn_assign_attr.interval_start = 255;
					conn_assign_attr.interval_end = 0;
					printf("Hub has no more free slots for allocation.\n");
				}
			} else {
				printf("current free slot > map1_end\n");
			}
		}
	}

	if(1 == node_attr.protocol_ver){
		/* Scheduling for proposed protocol */
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
	/* put it into the queue with priority waiting for transmission */
	op_pk_priority_set (conn_assign_MPDU, 6.0);
	if (op_subq_pk_insert(SUBQ_MAN, conn_assign_MPDU, OPC_QPOS_TAIL) == OPC_QINS_OK) {
		if (enable_log) {
			// fprintf (log,"t=%f  -> Enqueuing of MAC MANAGEMENT frame [SEQ = %d]\n\n", op_sim_time(), random_num);
			printf (" [Node %s] t=%f  -> Enqueuing of MAC MANAGEMENT frame [SEQ = %d]\n\n", node_attr.name, op_sim_time(), random_num);
		}
	} else {
		if (enable_log) {
			// fprintf (log,"t=%f  -> MAC MANAGEMENT frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", op_sim_time());
			printf (" [Node %s] t=%f  -> MAC MANAGEMENT frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", node_attr.name, op_sim_time());
		}
		/* destroy the packet */
		op_pk_destroy (conn_assign_MPDU);
	}

	// current_packet_txs = 0;
	// pkt_to_be_sent.enable = OPC_TRUE;

	// // phy_to_radio(conn_assign_MPDU);
	// wban_send_mac_pk_to_phy(conn_assign_MPDU);

	// conn_assign_tx_time = TX_TIME(wban_norm_phy_bits(conn_assign_MPDU), node_attr.data_rate);
	// printf("conn_assign_tx_time=%f\n", conn_assign_tx_time);

	// mac_attr.wait_for_ack = OPC_TRUE;
	// mac_attr.wait_ack_seq_num = random_num;
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
	/* Stack tracing enrty point */
	FIN(wban_send_i_ack_frame);

	if (enable_log) {
		// fprintf(log,"t=%f  -> Send ACK Frame [SEQ = %d] \n\n", op_sim_time(), ack_sequence_number);
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

	phy_to_radio(frame_MPDU);
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
	if (op_subq_pk_insert(SUBQ_DATA, data_frame_mpdu, OPC_QPOS_TAIL) == OPC_QINS_OK) {
		if (enable_log) {
			// fprintf (log,"t=%f  -> Enqueuing of MAC DATA frame [SEQ = %d, ACK? = %d] and try to send \n\n", op_sim_time(), seq_num, ack_policy);
			printf (" [Node %s] t=%f  -> Enqueuing of MAC DATA frame [SEQ = %d, ACK? = %d] and try to send \n\n", node_attr.name, op_sim_time(), seq_num, ack_policy);
		}
	} else {
		if (enable_log) {
			// fprintf (log,"t=%f  -> MAC DATA frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", op_sim_time());
			printf (" [Node %s] t=%f  -> MAC DATA frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", node_attr.name, op_sim_time());
		}
		
		/* destroy the packet */
		op_pk_destroy (data_frame_mpdu);
	}
	/* try to send the packet in SF active phase */
	if ((MAC_SLEEP != mac_state) && (MAC_SETUP != mac_state)) {
		op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
	}
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function: wban_mac_interrupt_process
 *
 * Description:	processes all the interrupts that occurs in an unforced state      
 * 
 * No parameters  
 *--------------------------------------------------------------------------------*/
static void wban_mac_interrupt_process() {
	double map_start2sec;
	double map_end2sec;
	/* Stack tracing enrty point */
	FIN(wban_mac_interrupt_process);
	
	switch (op_intrpt_type()) {
	
		case OPC_INTRPT_STRM: // incomming packet
		{
			// printf("Node %s received packets from incomming.\n", node_attr.name);
			wban_parse_incoming_frame();	// parse the incoming packet
			break;
		};/*end of OPC_INTRPT_STRM */
	
		case OPC_INTRPT_SELF:
		{
			switch (op_intrpt_code()) { /* begin switch (op_intrpt_code()) */
				case BEACON_INTERVAL_CODE: /* Beacon Interval Expiration - end of the Beacon Interval and start of a new one */
				{
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF BEACON PERIOD ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF SLEEP PERIOD ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					op_intrpt_schedule_remote(op_sim_time(), END_OF_SLEEP_PERIOD, node_attr.my_battery); 
					op_prg_odb_bkpt ("beacon_end");
					if (IAM_BAN_HUB) {
						/* value for the next superframe. End Device will obtain this value from beacon */
						wban_send_beacon_frame();
					} else {
						mac_state = MAC_SETUP;
					}
					break;
				};/*end of BEACON_INTERVAL_CODE */

				case INCREMENT_SLOT: {
					SF.current_slot++;
					if (SF.SD > SF.current_slot + 1) {
						op_intrpt_schedule_self (op_sim_time() + SF.slot_length2sec, INCREMENT_SLOT);
					}

					if(!IAM_BAN_HUB) {
						if((255 == conn_assign_attr.interval_start) && (0 == conn_assign_attr.interval_end)){
							break;
						}
						map_start2sec = SF.BI_Boundary + conn_assign_attr.interval_start * SF.slot_length2sec;
						map_end2sec = SF.BI_Boundary + (conn_assign_attr.interval_end+1) * SF.slot_length2sec;
						if(SF.current_slot == conn_assign_attr.interval_start) {
							SF.map1_start2sec = map_start2sec;
							SF.map1_end2sec = map_end2sec;
							printf("Node %s map1_start2sec=%f, map1_end2sec=%f.\n", node_attr.name, SF.map1_start2sec, SF.map1_end2sec);
							op_intrpt_schedule_self(max_double(SF.map1_start2sec, op_sim_time()), START_OF_MAP1_PERIOD_CODE);
							op_intrpt_schedule_self(SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
							op_prg_odb_bkpt("map");
							conn_assign_attr.interval_start = 255;
							conn_assign_attr.interval_end = 0;
						} else if ((SF.current_slot == conn_assign_attr.interval_start) && (SF.current_slot > SF.b2_start)) {
							SF.map2_start2sec = map_start2sec;
							SF.map2_end2sec = map_end2sec;
							op_intrpt_schedule_self(max_double(SF.map2_start2sec, op_sim_time()), START_OF_MAP2_PERIOD_CODE);
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
					pkt_to_be_sent.enable = OPC_FALSE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE EAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE EAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}

					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_EAP1_PERIOD_CODE */

				case END_OF_EAP1_PERIOD_CODE: /* end of EAP1 Period */
				{
					mac_state = MAC_SLEEP;
					// pkt_to_be_sent.enable = OPC_FALSE;
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE EAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE EAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					op_intrpt_schedule_remote(op_sim_time(), END_OF_ACTIVE_PERIOD_CODE, node_attr.my_battery);
					break;
				};/* end of END_OF_EAP1_PERIOD_CODE */

				case START_OF_RAP1_PERIOD_CODE: /* start of RAP1 Period */
				{
					mac_state = MAC_RAP1;
					phase_start_timeG = SF.rap1_start2sec;
					phase_end_timeG = SF.rap1_end2sec;
					SF.IN_MAP_PHASE = OPC_FALSE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
					// pkt_to_be_sent.enable = OPC_FALSE;

					// stat_vec.ppdu_sent_nbr = 0;
					// stat_vec.ppdu_sent_kbits = 0.0;
					stat_vec.ppdu_rap_sent_start = stat_vec.ppdu_sent_kbits;
					stat_vec.ppdu_rap_rcv_start = stat_vec.ppdu_rcv_kbits;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE RAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE RAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					op_prg_odb_bkpt("rap1");

					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_RAP1_PERIOD_CODE */

				case END_OF_RAP1_PERIOD_CODE: /* END of RAP1 Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_TRUE;

					stat_vec.ppdu_rap_sent_end = stat_vec.ppdu_sent_kbits;
					stat_vec.ppdu_rap_rcv_end = stat_vec.ppdu_rcv_kbits;
					stat_vec.ppdu_sent_kbits_rap = stat_vec.ppdu_rap_sent_end - stat_vec.ppdu_rap_sent_start;
					stat_vec.ppdu_rcv_kbits_rap = stat_vec.ppdu_rap_rcv_end - stat_vec.ppdu_rap_rcv_start;
					stat_vec.traffic_G = stat_vec.ppdu_sent_kbits_rap/(WBAN_DATA_RATE_KBITS*SF.rap1_length2sec);
					stat_vec.throughput_S = stat_vec.ppdu_rcv_kbits_rap/(WBAN_DATA_RATE_KBITS*SF.rap1_length2sec);
					fprintf(log,"STAT,CHANNEL_TRAFFIC_RAP_G=%f\n", stat_vec.traffic_G);
					fprintf(log,"STAT,CHANNEL_THROUGHPUT_RAP_S=%f\n", stat_vec.throughput_S);
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE RAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE RAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					op_intrpt_schedule_remote(op_sim_time(), END_OF_ACTIVE_PERIOD_CODE, node_attr.my_battery);
					// if(!node_attr.is_BANhub){
					// 	mac_state = CONN_SETUP;
					// 	phase_start_timeG = SF.rap1_end2sec;
					// 	phase_end_timeG = SF.rap1_end2sec+2*SF.slot_length2sec;
					// 	SF.IN_MAP_PHASE = OPC_FALSE;
					// 	SF.IN_EAP_PHASE = OPC_FALSE;
					// 	SF.ENABLE_TX_NEW = OPC_FALSE;
					// 	subq_info_get(SUBQ_DATA);
					// 	wban_send_conn_req_frame();
					// }
					
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
					// pkt_to_be_sent.enable = OPC_FALSE;
					if(OPC_FALSE == node_attr.is_BANhub){
						map_attr.TX_state = OPC_TRUE;
					}

					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE MAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE MAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					printf("Node %s Start MAP1 at %f, End MAP1 at %f.\n", node_attr.name, phase_start_timeG, phase_end_timeG);

					op_prg_odb_bkpt("map1_start");
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_MAP1_PERIOD_CODE */

				case END_OF_MAP1_PERIOD_CODE: /* end of MAP1 Period */
				{
					mac_state = MAC_SLEEP;
					// pkt_to_be_sent.enable = OPC_TRUE;
					SF.ENABLE_TX_NEW = OPC_FALSE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE MAP1 ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE MAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					op_intrpt_schedule_remote(op_sim_time(), END_OF_ACTIVE_PERIOD_CODE, node_attr.my_battery);
					break;
				};/* end of END_OF_MAP1_PERIOD_CODE */

				case SEND_B2_FRAME: 
				{
					op_prg_odb_bkpt("send_b2");
					if (IAM_BAN_HUB){
						printf("Hub start sending beacon2 frame at time %f.\n", op_sim_time());
						wban_send_beacon2_frame();
					} else {
						mac_state = MAC_SETUP;
						printf("Node %s is not Hub and not allowed sending beacon2 frame.\n", node_attr.name);
						FOUT;
					}
					break;
				}

				case START_OF_CAP_PERIOD_CODE:
				{
					printf("Node %s enters into CAP Period.\n", node_attr.name);
					mac_state = MAC_CAP;
					phase_start_timeG = SF.cap_start2sec;
					phase_end_timeG = SF.cap_end2sec;
					SF.IN_MAP_PHASE = OPC_FALSE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
					// pkt_to_be_sent.enable = OPC_FALSE;
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF THE CAP ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF THE CAP ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					op_intrpt_schedule_self(op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					break;
				}

				case END_OF_CAP_PERIOD_CODE: /* end of CAP Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_TRUE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ END OF THE CAP ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  END OF THE CAP ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					op_intrpt_schedule_remote(op_sim_time(), END_OF_ACTIVE_PERIOD_CODE, node_attr.my_battery);
					break;
				};/* end of END_OF_CAP_PERIOD_CODE */

				case START_OF_SLEEP_PERIOD: /* Start of Sleep Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_TRUE;
				
					if (enable_log) {
						fprintf (log,"t=%f  -> ++++++++++ START OF SLEEP PERIOD ++++++++++ \n\n", op_sim_time());
						printf (" [Node %s] t=%f  -> ++++++++++  START OF SLEEP PERIOD ++++++++++ \n\n", node_attr.name, op_sim_time());
					}
					break;
				};/* end of Start of Sleep Period */

				case END_PK_IN_MAP:
				{
					SF.ENABLE_TX_NEW = OPC_TRUE;
					op_intrpt_schedule_self(op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					break;
				}

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
						// fprintf (log,"t=%f  -------- START CCA CW = %d\n",op_sim_time(),csma.CW);
						printf (" [Node %s] t=%f  -------- START CCA CW = %d\n",node_attr.name, op_sim_time(), csma.CW);
					}
					// csma.next_slot_start = op_sim_time() + pCSMASlotLength2Sec;
					wban_battery_cca();
					op_intrpt_schedule_self (op_sim_time() + pCCATime, CCA_EXPIRATION_CODE);
					break;
				};/*end of CCA_START_CODE */
			
				case CCA_EXPIRATION_CODE :/*At the end of the CCA */
				{
					if(!can_fit_TX(&pkt_to_be_sent)) {
						attemptingToTX = OPC_FALSE;
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
						op_intrpt_schedule_self (wban_backoff_period_boundary_get()+2*pCSMASlotLength2Sec, CCA_START_CODE);
					} else {
						csma.backoff_counter--;
						printf("t = %f, CCA with IDLE, backoff_counter decrement to %d.\n", op_sim_time(), csma.backoff_counter);
						if (csma.backoff_counter > 0) {
							// printf("CCA at next available backoff boundary = %f sec.\n", csma.next_slot_start);
							// op_intrpt_schedule_self (csma.next_slot_start, CCA_START_CODE);
							printf("CCA at next available backoff boundary = %f sec.\n", wban_backoff_period_boundary_get());
							op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
						} else {
							printf("backoff_counter decrement to 0, %s start transmission at %f.\n", node_attr.name, wban_backoff_period_boundary_get());
							// op_intrpt_schedule_self (csma.next_slot_start, START_TRANSMISSION_CODE);
							op_prg_odb_bkpt("send_packet");
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

					wban_send_mac_pk_to_phy(frame_MPDU_to_be_sent);
					break;
				}; /*end of START_TRANSMISSION_CODE */

				case WAITING_ACK_END_CODE:	/* the timer for waiting an ACK has expired, the packet must be retransmitted */
				{
					waitForACK = OPC_FALSE;
					if(op_sim_time() > phase_end_timeG){
						op_stat_write(stat_vec.data_pkt_fail, op_pk_total_size_get(frame_MPDU_to_be_sent));
						op_stat_write(stat_vecG.data_pkt_fail, op_pk_total_size_get(frame_MPDU_to_be_sent));
						FOUT;
					}
					printf("Node %s waitting for ACK END at %f.\n", node_attr.name, op_sim_time());
					printf("op_sim_time=%f, phase_end_timeG=%f\n", op_sim_time(), phase_end_timeG);
					// double the Contention Window, after every second fail.
					if (OPC_TRUE == csma.CW_double) {
						csma.CW *=2;
						if (csma.CW > CWmax[pkt_to_be_sent.user_priority]) {
							csma.CW = CWmax[pkt_to_be_sent.user_priority];
						}
						printf("CW doubled after %d tries.\n", current_packet_txs + current_packet_CS_fails);
					}
					(csma.CW_double == OPC_TRUE) ? (csma.CW_double=OPC_FALSE) : (csma.CW_double=OPC_TRUE);
					// check if we reached the max number and if so delete the packet
					if (current_packet_txs + current_packet_CS_fails >= max_packet_tries) {
						// collect statistics
						op_stat_write(stat_vec.data_pkt_fail, 1.0);
						op_stat_write(stat_vec.data_pkt_suc1, 0.0);
						op_stat_write(stat_vec.data_pkt_suc2, 0.0);

						op_stat_write(stat_vecG.data_pkt_fail, 1.0);
						op_stat_write(stat_vecG.data_pkt_suc1, 0.0);
						op_stat_write(stat_vecG.data_pkt_suc2, 0.0);

						printf("Packet transmission exceeds max packet tries at time %f\n", op_sim_time());
						// remove MAC frame (MPDU) frame_MPDU_to_be_sent
						// op_pk_destroy(frame_MPDU_to_be_sent);
						// pkt_to_be_sent = NULL;
						pkt_to_be_sent.enable = OPC_FALSE;
						waitForACK = OPC_FALSE;
						current_packet_txs = 0;
						current_packet_CS_fails = 0;
						op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					}
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					//  else {
					// 	mac_attr.wait_for_ack = OPC_TRUE;
					// 	SF.ENABLE_TX_NEW = OPC_FALSE;
					// 	if (SF.IN_MAP_PHASE) {
					// 		wban_send_mac_pk_to_phy(frame_MPDU_to_be_sent);
					// 	} else {
					// 		// double the Contention Window, after every second fail.
					// 		if (OPC_TRUE == csma.CW_double) {
					// 			csma.CW *=2;
					// 			if (csma.CW > CWmax[pkt_to_be_sent.user_priority]) {
					// 				csma.CW = CWmax[pkt_to_be_sent.user_priority];
					// 			}
					// 			printf("CW doubled after %d tries.\n", current_packet_txs + current_packet_CS_fails);
					// 		}
					// 		(csma.CW_double == OPC_TRUE) ? (csma.CW_double=OPC_FALSE) : (csma.CW_double=OPC_TRUE);
					// 		wban_backoff_delay_set(pkt_to_be_sent.user_priority);
					// 		op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
					// 	}
					// 	// op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);	// try to send the same packet once more
					// }
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
					op_prg_odb_bkpt("send_conn_req");
					printf("t = %f, Node %s start sending connection request frame to Hub.\n", op_sim_time(), node_attr.name);
					current_packet_txs = 0;
					current_packet_CS_fails = 0;
					wban_send_mac_pk_to_phy(CONN_REQ_MPDU);
					// wban_send_connection_request_frame();
					break;
				};
				
				case SEND_CONN_ASSIGN_CODE:
				{
					printf("Hub start sending connection assignment frame at %f.\n", op_sim_time());
 					// wban_send_conn_assign_frame(conn_assign_attr.allocation_length);
 					wban_attempt_TX();

					break;
				};

				case SEND_I_ACK:
				{
					wban_send_i_ack_frame(ack_seq_num);
					break;
				};

				case N_ACK_PACKET_SENT:
					TX_NACK = OPC_FALSE;
					pkt_to_be_sent.enable = OPC_FALSE;
					break;

				default:
				{
				};	
				
			} /*end of switch (op_intrpt_code())*/ 
			
			break;
		};/*end of OPC_INTRPT_SELF */
		
		case OPC_INTRPT_ENDSIM:
		{
			fprintf(log,"STAT,PPDU_sent_nbr=%f\n", PPDU_sent_nbr);
			fprintf(log,"STAT,PPDU_rcv_nbr=%f\n", PPDU_rcv_nbr);
			fprintf(log,"STAT,CHANNEL_TRAFFIC_G=%f\n", PPDU_sent_kbits/(WBAN_DATA_RATE_KBITS*op_sim_time()));
			fprintf(log,"STAT,CHANNEL_THROUGHPUT_S=%f\n", PPDU_rcv_kbits/(WBAN_DATA_RATE_KBITS*op_sim_time()));
			if (enable_log) {
				fprintf (log, "t=%f  ***********   GAME OVER END - OF SIMULATION  ********************  \n\n",op_sim_time());
				printf (" [Node %s] t=%f  ***********   GAME OVER - END OF SIMULATION  *******************\n\n", node_attr.name, op_sim_time());
				
				fclose(log);
			}
			// wban_battery_end();

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
						// op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					}
					
					break;
				}
				
				case TX_BUSY_STAT :
				{
					if (op_stat_local_read (TX_BUSY_STAT) == 0.0)
						// op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					
					break;
				}
				
				case RX_COLLISION_STAT :
				{
					if (enable_log){
						// fprintf(log,"t=%f  -> $$$$$ COLLISION $$$$$$$  \n\n",op_sim_time());
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
	Packet* frame_MSDU;
	double ete_delay;
	int pk_size;
	int up_prio;
	int sender_id;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_data_frame);
	op_pk_nfd_get (frame_MPDU, "Ack Policy", &ack_policy);
	op_pk_nfd_get (frame_MPDU, "Sender ID", &sender_id);
	op_pk_nfd_get (frame_MPDU, "Frame Subtype", &up_prio);
	op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
	pk_size = op_pk_total_size_get(frame_MSDU);
	ete_delay = op_sim_time() - op_pk_creation_time_get(frame_MSDU);

	fprintf(log, "RX,DATA_MAC_STATE=%d,RAP_Length=%d,", mac_state, beacon_attr.rap1_length);
	fprintf(log, "t=%f,pk size of DATA_MSDU=%d,UPx=%d,received from ID=%d,", op_sim_time(), pk_size, up_prio, sender_id);
	fprintf(log, "ETE_DELAY=%f\n", ete_delay);
	/* check if any ACK is requested */
	switch (ack_policy) {
		case N_ACK_POLICY:
			break;
		case I_ACK_POLICY:
			// op_pk_nfd_get (frame_MPDU, "Sequence Number", &seq_num);
			// wban_send_i_ack_frame (seq_num);
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
	if((!pkt_to_be_sent.enable) || (0 == current_packet_txs) || (!waitForACK)){
		printf("No packet being TX while receive I-ACK.\n");
		FOUT;
	}
	
	op_pk_nfd_get (ack_frame, "Sequence Number", &seq_num);
	
	/* if I'm waiting for an ACK */
	if (waitForACK) {
		if (mac_attr.wait_ack_seq_num == seq_num) { /* yes, I have received my ACK */
			waitForACK = OPC_FALSE;
			attemptingToTX = OPC_FALSE;
			pkt_to_be_sent.enable = OPC_FALSE;
			current_packet_txs = 0;
			current_packet_CS_fails = 0;
			
			/* disable the invocation of only the next interrupt of WAITING_ACK_END_CODE */
			op_intrpt_disable (OPC_INTRPT_SELF, WAITING_ACK_END_CODE, OPC_TRUE);
			
			if (enable_log) {
				printf(" [Node %s] t=%f  -> ACK Frame Reception [Requested SEQ = %d]\n\n", node_attr.name, op_sim_time(), seq_num);
			}

			//collect statistics
			op_stat_write(stat_vec.data_pkt_fail, 0.0);
			if(1 == current_packet_txs + current_packet_CS_fails){
				op_stat_write(stat_vec.data_pkt_suc1, 1.0);
			} else {
				op_stat_write(stat_vec.data_pkt_suc2, 1.0);
			}
			
			op_stat_write(stat_vecG.data_pkt_fail, 0.0);
			if(1 == current_packet_txs + current_packet_CS_fails){
				op_stat_write(stat_vecG.data_pkt_suc1, 1.0);
			} else {
				op_stat_write(stat_vecG.data_pkt_suc2, 1.0);
			}

			/* Try to send another packet after pSIFS */
			op_intrpt_schedule_self (op_sim_time() + pSIFS, TRY_PACKET_TRANSMISSION_CODE);
		} else	{	/* No, This is not my ACK, I'm Still Waiting */
			if (enable_log) {
				// fprintf(log,"t=%f  -> WRONG ACK Frame Reception [RCV = %d], Still Waiting ACK [RQST = %d] \n\n", op_sim_time(), seq_num , mac_attr.wait_ack_seq_num );
				printf(" [Node %s] t=%f  -> WRONG ACK Frame Reception [RCV = %d], Still Waiting ACK [RQST = %d] \n\n", node_attr.name, op_sim_time(), seq_num , mac_attr.wait_ack_seq_num );
			}
		}
	} else {/* if (mac_attributes.wait_ack == OPC_FALSE) */ 
		if (enable_log) {
			// fprintf (log,"t=%f  -> I'm not Waiting ACK Frame - ACK Frame Destroyed. \n\n", op_sim_time());	
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
	// Packet* frame_MSDU_temp;
	// Packet* frame_MPDU_temp;
	// double conn_round;
	// double ack_tx_time;
	/* Stack tracing enrty point */
	FIN(wban_attempt_TX);

	if(waitForACK || attemptingToTX || TX_NACK){
		printf("A packet is in TX.\n");
		FOUT;
	}
	printf("%s mac_state=%d\n", node_attr.name, mac_state);
	// If we are not in an appropriate state, return
	switch (mac_state) {
		case MAC_EAP1: 
		case MAC_RAP1: 
		case MAC_EAP2: 
		case MAC_RAP2: 
		case MAC_CAP: 
			SF.IN_CAP_PHASE = OPC_TRUE;
			SF.IN_MAP_PHASE = OPC_FALSE;
			break;
		case MAC_MAP1: 
		case MAC_MAP2: 
			SF.IN_MAP_PHASE = OPC_TRUE;
			SF.IN_CAP_PHASE = OPC_FALSE;
			break;
		default: FOUT; // none of the valid state above
	}

	if (!((op_intrpt_type () == OPC_INTRPT_SELF) && (op_intrpt_code () == TRY_PACKET_TRANSMISSION_CODE))) {
		printf("stop1\n");
		FOUT;
	}
	if ((op_stat_local_read(TX_BUSY_STAT) == 1.0) || (MAC_SLEEP == mac_state) || (MAC_SETUP == mac_state)) {
		printf("stop2\n");
		FOUT;
	}
	if(CONN_SETUP == mac_state){
		printf("CONN_SETUP phase, TX data packet not allowed.\n");
		FOUT;
	}

	if((pkt_to_be_sent.enable) && (current_packet_txs + current_packet_CS_fails < max_packet_tries)){
		printf("%s retransmittion.\n", node_attr.name);
		if(SF.IN_CAP_PHASE){
			printf("%s retransmit with CSMA.\n", node_attr.name);
			wban_attempt_TX_CSMA();
		}
		if((SF.IN_MAP_PHASE) && (can_fit_TX(&pkt_to_be_sent))) {
			printf("%s retransmit with Scheduling.\n", node_attr.name);
			wban_send_mac_pk_to_phy(frame_MPDU_to_be_sent);
		}
		FOUT;
	}
	/* if there is still a packet in the buffer after max tries
	 * then delete it, reset relevant variables, and collect stats.
	 */
	if (pkt_to_be_sent.enable) {
		pkt_to_be_sent.enable = OPC_FALSE;
		current_packet_txs = 0;
		current_packet_CS_fails = 0;
	}
	// Try to draw a new packet from the data or Management buffers.
	if (!op_subq_empty(SUBQ_MAN)) {
		frame_MPDU_to_be_sent = op_subq_pk_remove(SUBQ_MAN, OPC_QPOS_PRIO);
		pkt_to_be_sent.enable = OPC_TRUE;
		pkt_to_be_sent.user_priority = 6; //set up of managemant frame with 6
		printf("Management frame TX.\n");
	} else if ((mac_attr.sender_id != UNCONNECTED_NID) && (!op_subq_empty(SUBQ_DATA))) {
		/* obtain the pointer to MAC frame (MPDU) stored in the adequate queue */
		frame_MPDU_to_be_sent = op_subq_pk_access (SUBQ_DATA, OPC_QPOS_PRIO);
		printf("PK_ACCESS, pk size of frame_MPDU=%d\n", op_pk_total_size_get(frame_MPDU_to_be_sent));
		op_prg_odb_bkpt("data_tx");
		op_pk_nfd_get(frame_MPDU_to_be_sent, "Frame Subtype", &pkt_to_be_sent.user_priority);
		if (SF.IN_EAP_PHASE) {
			printf("Node %s is in EAP phase.\n", node_attr.name);
			if (7 != pkt_to_be_sent.user_priority) {
				printf("%s have no UP=7 traffic in the SUBQ_DATA subqueue currently.\n", node_attr.name);
				FOUT;
			}
		}
		frame_MPDU_to_be_sent = op_subq_pk_remove(SUBQ_DATA, OPC_QPOS_PRIO);
		pkt_to_be_sent.enable = OPC_TRUE;
	} else {
		printf("%s queue is empty or unconnected.\n", node_attr.name);
		FOUT;
	}

	op_pk_nfd_get(frame_MPDU_to_be_sent, "Sequence Number", &pkt_to_be_sent.seq_num);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Ack Policy", &pkt_to_be_sent.ack_policy);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Recipient ID", &pkt_to_be_sent.recipient_id);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Frame Type", &pkt_to_be_sent.frame_type);
	
	/* for frame enqueued to subqueue before first beacon reception */
	if (pkt_to_be_sent.recipient_id == HUB_ID) {
		op_pk_nfd_set(frame_MPDU_to_be_sent, "Recipient ID", mac_attr.recipient_id);
		op_pk_nfd_set(frame_MPDU_to_be_sent, "Sender ID", mac_attr.sender_id);
		op_pk_nfd_set(frame_MPDU_to_be_sent, "BAN ID", mac_attr.ban_id);
		printf("HUB_ID, pk size of frame_MPDU=%d\n", op_pk_total_size_get(frame_MPDU_to_be_sent));
		pkt_to_be_sent.recipient_id = mac_attr.recipient_id;
		pkt_to_be_sent.sender_id = mac_attr.sender_id;
	}
	
	// if we found a packet in any of the buffers, try to TX it.
	if (pkt_to_be_sent.enable){
		if(SF.IN_CAP_PHASE){
			csma.CW = CWmin[pkt_to_be_sent.user_priority];
			csma.CW_double = OPC_FALSE;
			csma.backoff_counter = 0;
			wban_attempt_TX_CSMA();
		}
		if((SF.IN_MAP_PHASE) && (can_fit_TX(&pkt_to_be_sent))){
			wban_send_mac_pk_to_phy(frame_MPDU_to_be_sent);
		}
	}

	// pkt_to_be_sent.total_bits = op_pk_total_size_get(frame_MPDU_to_be_sent);
	/* remove the packet in the head of the queue */
	// SF.ENABLE_TX_NEW = OPC_FALSE;
	// pkt_to_be_sent.ack_bits = I_ACK_PPDU_SIZE_BITS;

	// current_packet_txs = 0;
	// current_packet_CS_fails = 0;

	// if (OPC_TRUE == SF.IN_MAP_PHASE) {
	// 	/* set the Ack Policy with N_ACK_POLICY */
	// 	// op_pk_nfd_set (frame_MPDU_to_be_sent, "Ack Policy", N_ACK_POLICY);
	// 	// printf("SET_ACK, pk size of frame_MPDU=%d\n", op_pk_total_size_get(frame_MPDU_to_be_sent));
	// 	// pkt_to_be_sent.ack_policy = N_ACK_POLICY;
	// 	/* update the ack bits of packet */
	// 	// pkt_to_be_sent.ack_bits = 0;
	// 	if(!can_fit_TX(&pkt_to_be_sent)){
	// 		printf("No enouth time for tx in MAP.\n");
	// 		op_prg_odb_bkpt("map_fail");
	// 		FOUT;
	// 	}
	// 	// frame_MPDU_temp = op_subq_pk_remove (SUBQ_DATA, OPC_QPOS_PRIO);
	// 	if (OPC_TRUE == map_attr.TX_state) {
	// 		op_prg_odb_bkpt("map_tx");
	// 		wban_send_mac_pk_to_phy(frame_MPDU_to_be_sent);
	// 		// op_intrpt_schedule_self(op_sim_time()+2*pSIFS+TX_TIME(wban_norm_phy_bits(frame_MPDU_to_be_sent), node_attr.data_rate)+I_ACK_TX_TIME, END_PK_IN_MAP);
	// 	}
	// } else {
	// 	// frame_MPDU_temp = op_subq_pk_remove (SUBQ_DATA, OPC_QPOS_PRIO);
	// 	/* update the ack bits of packet */
	// 	pkt_to_be_sent.ack_bits = I_ACK_PPDU_SIZE_BITS;
	// 	csma.CW = CWmin[pkt_to_be_sent.user_priority];
	// 	csma.CW_double = OPC_FALSE;
	// 	csma.backoff_counter = 0;
	// 	wban_attempt_TX_CSMA(pkt_to_be_sent.user_priority);
	// }

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
static void wban_attempt_TX_CSMA() {
	/* Stack tracing enrty point */
	FIN(wban_attempt_TX_CSMA);

	if (enable_log) {
		printf(" [Node %s] t=%f  - BACKOFF INIT  \n", node_attr.name, op_sim_time());
	}

	wban_backoff_delay_set(pkt_to_be_sent.user_priority);
	csma.backoff_counter_lock = OPC_FALSE;

	if(!can_fit_TX(&pkt_to_be_sent)) {
		current_packet_CS_fails++;
		csma.backoff_counter_lock = OPC_TRUE;

		FOUT;
	} else {
		csma.backoff_counter_lock = OPC_FALSE;
	}
	attemptingToTX = OPC_TRUE;
	//CCA
	op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
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
	
		// fprintf (log, "-------------------------- BACKOFF -----------------------------------\n");
		// fprintf (log, " [Node %s] ENTERS BACKOFF STATUT AT %f\n", node_attr.name, op_sim_time());
		// fprintf (log, "  Beacon Boundary = %f\n", SF.BI_Boundary);
		// fprintf (log, "  CW = %d\n", csma.CW);
		// fprintf (log, "  Random Backoff counter = %d\n", csma.backoff_counter);
		// fprintf (log, "    + Random Backoff time  = %f sec \n", csma.backoff_time);
		// fprintf (log, "    + Phase Remaining Length = %f sec \n", phase_remaining_time);
		// fprintf (log, "  Current Time Slot = %d\n", SF.current_slot);
		// fprintf (log, "  Backoff Boundary = %f sec \n", wban_backoff_period_boundary_get());
		// fprintf (log, "  Phase Start Time     = %f sec \n", phase_start_timeG);
		// fprintf (log, "  Phase End Time     = %f sec \n", phase_end_timeG);
		// fprintf (log, "  Difference       = %f sec \n", phase_end_timeG- wban_backoff_period_boundary_get());
		// fprintf (log, "  BackOff Expiration Time  = %f sec\n", csma.backoff_expiration_time);
		// fprintf (log, "----------------------------------------------------------------------------\n\n");
	}
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function: can_fit_TX
 *
 * Description:	if a transmission fits in the time we have (scheduled or RAP)
 * 
 * Input: packet_to_be_sent_attributes
 *--------------------------------------------------------------------------------*/
static Boolean can_fit_TX (packet_to_be_sent_attributes* pkt_to_be_sentL) {
	double phase_remaining_time;
	double pk_tx_time;

	FIN(can_fit_TX);

	if (!pkt_to_be_sentL->enable) {
		printf("NO packet being TX.\n");
		FRET(OPC_FALSE);
	}
	pk_tx_time = TX_TIME(wban_norm_phy_bits(frame_MPDU_to_be_sent), node_attr.data_rate);
	phase_remaining_time = phase_end_timeG - op_sim_time();
	if(pkt_to_be_sentL->ack_policy != N_ACK_POLICY){
		if (compare_doubles(phase_remaining_time, pk_tx_time+I_ACK_TX_TIME+2*pSIFS) >=0) {
			FRET(OPC_TRUE);
		} else {
			printf("No enougth time for I_ACK_POLICY packet transmission in this phase.\n");
			FRET(OPC_FALSE);
		}
	} else {
		if (compare_doubles(phase_remaining_time, pk_tx_time+pSIFS) >=0) {
			FRET(OPC_TRUE);
		} else {
			printf("No enougth time for N_ACK_POLICY packet transmission in this phase.\n");
			FRET(OPC_FALSE);
		}
	}

	// if (SF.IN_CAP_PHASE) {
	// 	/*check if the backoff time will exceed the remaining time in the Current Phase */
	// 	csma.backoff_expiration_time = wban_backoff_period_boundary_get() + csma.backoff_time;
	// 	phase_remaining_time = phase_end_timeG - wban_backoff_period_boundary_get();
	// 	/* the backoff is accepted if it expires at most pCSMASlotLength2Sec before the end of Current Phase */
	// 	// if(compare_doubles((phase_remaining_time - pCSMASlotLength2Sec), csma.backoff_time) >=0) {// THERE IS A PROBLEM WITH EQUALITY IN DOUBLE
	// 	// }
		// if(pkt_to_be_sentL->ack_policy != N_ACK_POLICY) {
		// 	if (compare_doubles(phase_remaining_time, pk_tx_time+I_ACK_TX_TIME+2*pSIFS) >=0) {
		// 		FRET(OPC_TRUE);
		// 	} else {
		// 		printf("No enougth time for I_ACK_POLICY packet transmission in this phase.\n");
		// 		FRET(OPC_FALSE);
		// 	}
		
	// 	} else {
	// 		if (compare_doubles(phase_remaining_time, pk_tx_time+pSIFS) >=0) {
	// 			FRET(OPC_TRUE);
	// 		} else {
	// 			printf("No enougth time for N_ACK_POLICY packet transmission in this phase.\n");
	// 			FRET(OPC_FALSE);
	// 		}
	// 	}
	// } else {
	// 	phase_remaining_time = phase_end_timeG - op_sim_time();

	// 	if(pkt_to_be_sentL->ack_policy != N_ACK_POLICY) {
	// 		if (compare_doubles(phase_remaining_time, pk_tx_time+ack_tx_time+2*pSIFS) >=0) {
	// 			FRET(OPC_TRUE);
	// 		} else {
	// 			printf("No enougth time for I_ACK_POLICY packet transmission in this phase.\n");
	// 			FRET(OPC_FALSE);
	// 		}
		
	// 	} else {
	// 		if (compare_doubles(phase_remaining_time, pk_tx_time+pSIFS) >=0) {
	// 			FRET(OPC_TRUE);
	// 		} else {
	// 			printf("No enougth time for N_ACK_POLICY packet transmission in this phase.\n");
	// 			FRET(OPC_FALSE);
	// 		}
	// 	}
	// }
	FRET(OPC_FALSE);
}

/*--------------------------------------------------------------------------------
 * Function:	wban_send_mac_pk_to_phy
 *
 * Description:	
 * 
 * No parameters  
 *--------------------------------------------------------------------------------*/
static void wban_send_mac_pk_to_phy(Packet* frame_MPDU) {
	double PPDU_tx_time;
	double ack_expire_time;
	double pk_create_time;
	int ppdu_bits;
	int frame_type;
	int frame_subtype;
	int ack_policy;
	int seq_num;
	Packet* MPDU_copy;

	/* Stack tracing enrty point */
	FIN(wban_send_mac_pk_to_phy);
	// we are starting to TX, so we are exiting the attemptingToTX (sub)state.
	attemptingToTX = OPC_FALSE;

	op_pk_nfd_get(frame_MPDU, "Sequence Number", &seq_num);
	op_pk_nfd_get(frame_MPDU, "Ack Policy", &ack_policy);
	op_pk_nfd_get(frame_MPDU, "Frame Type", &frame_type);
	op_pk_nfd_get(frame_MPDU, "Frame Subtype", &frame_subtype);
	ppdu_bits = wban_norm_phy_bits(frame_MPDU);
	printf("MAC_TO_PHY, pk size of frame_PPDU=%d\n", ppdu_bits);
	op_prg_odb_bkpt("mpdu_size");
	fprintf(log, "TX,MAC_STATE=%d,RAP1_Length=%d,send to ID=%d,", mac_state, beacon_attr.rap1_length, mac_attr.recipient_id);
	fprintf(log, "t=%f,FRAME_TYPE=%d,UPx=%d,PPDU_BITS=%d\n", op_sim_time(), frame_type, frame_subtype, ppdu_bits);

	switch (frame_type) {
		case DATA:
		{
			op_stat_write(stat_vec.data_pkt_sent, ppdu_bits);
			op_stat_write(stat_vecG.data_pkt_sent, ppdu_bits);
			switch(frame_subtype){
				case UP0:
					op_stat_write(stat_vec.up0_sent, ppdu_bits);
					op_stat_write(stat_vecG.up0_sent, ppdu_bits);
					break;
				case UP1:
					op_stat_write(stat_vec.up1_sent, ppdu_bits);
					op_stat_write(stat_vecG.up1_sent, ppdu_bits);
					break;
				case UP2:
					op_stat_write(stat_vec.up2_sent, ppdu_bits);
					op_stat_write(stat_vecG.up2_sent, ppdu_bits);
					break;
				case UP3:
					op_stat_write(stat_vec.up3_sent, ppdu_bits);
					op_stat_write(stat_vecG.up3_sent, ppdu_bits);
					break;
				case UP4:
					op_stat_write(stat_vec.up4_sent, ppdu_bits);
					op_stat_write(stat_vecG.up4_sent, ppdu_bits);
					break;
				case UP5:
					op_stat_write(stat_vec.up5_sent, ppdu_bits);
					op_stat_write(stat_vecG.up5_sent, ppdu_bits);
					break;
				case UP6:
					op_stat_write(stat_vec.up6_sent, ppdu_bits);
					op_stat_write(stat_vecG.up6_sent, ppdu_bits);
					break;
				case UP7:
					op_stat_write(stat_vec.up7_sent, ppdu_bits);
					op_stat_write(stat_vecG.up7_sent, ppdu_bits);
					break;
				default:
					break;
			}
			break;
		}
		case MANAGEMENT:
			switch(frame_subtype){
				case BEACON:
					op_stat_write (beacon_frame_hndl, 1.0);
					break;
				case CONNECTION_ASSIGNMENT:
					printf("Hub send conn_assign at %f.\n", op_sim_time());
					op_prg_odb_bkpt("send_assign");
					break;
				case CONNECTION_REQUEST:
					printf("Node send conn_req at %f.\n", op_sim_time());
					op_prg_odb_bkpt("send_req");
					break;
				default:
					break;
			}
			break;
		case COMMAND:
			break;
		default:
			break;
	}

	PPDU_tx_time = TX_TIME(wban_norm_phy_bits(frame_MPDU), node_attr.data_rate);
	ack_expire_time = op_sim_time() + PPDU_tx_time + I_ACK_TX_TIME + 2*pSIFS;

	switch (ack_policy){
		case N_ACK_POLICY:
			TX_NACK = OPC_TRUE;
			phy_to_radio(frame_MPDU);
			op_intrpt_schedule_self (op_sim_time() + PPDU_tx_time + pSIFS, N_ACK_PACKET_SENT);
			current_packet_txs = 0;
			break;
		case I_ACK_POLICY:
			waitForACK = OPC_TRUE;
			mac_attr.wait_for_ack = OPC_TRUE;
			mac_attr.wait_ack_seq_num = seq_num;
			pk_create_time = op_pk_creation_time_get(frame_MPDU);
			MPDU_copy = op_pk_copy(frame_MPDU);
			op_pk_creation_time_set(MPDU_copy, pk_create_time);
			phy_to_radio(MPDU_copy);

			current_packet_txs++;
			if (enable_log) {
				// fprintf(log,"t=%f   ----------- START TX [DEST_ID = %d, SEQ = %d, with ACK expiring at %f] %d retries  \n\n", op_sim_time(), pkt_to_be_sent.recipient_id, mac_attr.wait_ack_seq_num, ack_expire_time,current_packet_txs+current_packet_CS_fails);
				printf(" [Node %s] t=%f  ----------- START TX [DEST_ID = %d, SEQ = %d, with ACK expiring at %f] %d retries \n\n", node_attr.name, op_sim_time(), pkt_to_be_sent.recipient_id, mac_attr.wait_ack_seq_num, ack_expire_time,current_packet_txs+current_packet_CS_fails);
			}
			SF.ENABLE_TX_NEW = OPC_FALSE;
			if(ack_expire_time > phase_end_timeG){
				op_stat_write(stat_vec.data_pkt_fail, ppdu_bits);
				op_stat_write(stat_vecG.data_pkt_fail, ppdu_bits);
				FOUT;
			}
			op_intrpt_schedule_self(ack_expire_time, WAITING_ACK_END_CODE);
			
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

/*--------------------------------------------------------------------------------
 * Function:	wban_norm_phy_bits(Packet* frame_MPDU)
 *
 * Description:	normalize the PHY bit size of MPDU
 *             
 *--------------------------------------------------------------------------------*/
static int wban_norm_phy_bits(Packet* frame_MPDU) {
	int mpdu_size;

	mpdu_size = op_pk_total_size_get(frame_MPDU);
	return (int)ceil(mpdu_size + LOG_M*BCH_CODE*(N_preamble + S_header*N_header));
}

/*--------------------------------------------------------------------------------
 * Function:	phy_to_radio(Packet* frame_MPDU)
 *
 * Description:	encapsulate the frame_MPDU for ppdu 
 *             
 *--------------------------------------------------------------------------------*/
static void phy_to_radio(Packet* frame_MPDU) {
	Packet* frame_PPDU;
	int bulk_size;

	/* Stack tracing enrty point */
	FIN(phy_to_radio);
	/* create PHY frame (PPDU) that encapsulates connection_request MAC frame (MPDU) */
	frame_PPDU = op_pk_create_fmt("wban_frame_PPDU_format");

	op_pk_nfd_set (frame_PPDU, "RATE", node_attr.data_rate);
	/* wrap connection_request MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (frame_PPDU, "PSDU", frame_MPDU);
	op_pk_nfd_set (frame_PPDU, "LENGTH", ((double) op_pk_total_size_get(frame_MPDU))/8); //[bytes]
	// bulk_size = (int)((LOG_M*BCH_CODE - 1)*N_preamble + (LOG_M*BCH_CODE*S_header - 1)*N_header);
	bulk_size = wban_norm_phy_bits(frame_MPDU) - op_pk_total_size_get(frame_PPDU);
	op_pk_bulk_size_set (frame_PPDU, bulk_size);

	wban_battery_update_tx (frame_PPDU);
	PPDU_sent_kbits = PPDU_sent_kbits + ((double)(op_pk_total_size_get(frame_PPDU))/1000.0); // in kbits
	PPDU_sent_nbr = PPDU_sent_nbr + 1;
	stat_vec.ppdu_sent_kbits = stat_vec.ppdu_sent_kbits + ((double)(op_pk_total_size_get(frame_PPDU))/1000.0); // in kbits
	stat_vec.ppdu_sent_nbr = stat_vec.ppdu_sent_nbr + 1;
	if (op_stat_local_read(TX_BUSY_STAT) == 1.0){
		op_sim_end("ERROR : TRY TO SEND Packet WHILE THE TX CHANNEL IS BUSY","PK_SEND_CODE","","");
	}
	op_pk_send (frame_PPDU, STRM_FROM_MAC_TO_RADIO);

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_battery_update_tx
 *
 * Description:	send information about the operation to do to the battery module 
 *
 * Input : 	Packet* frame_PPDU
 *--------------------------------------------------------------------------------*/
static void wban_battery_update_tx(Packet* frame_PPDU) {
	Ici * iciptr;
	double PPDU_pksize;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_update_tx);
	
	PPDU_pksize = op_pk_total_size_get(frame_PPDU);
	iciptr = op_ici_create ("wban_battery_ici_format");
	op_ici_attr_set (iciptr, "PPDU Packet Size", PPDU_pksize);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), PACKET_TX_CODE, node_attr.my_battery); 
	op_ici_install (OPC_NIL);
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_battery_update_rx
 *
 * Description:	send information about the operation to do to the battery module 
 *
 * Input : 	Packet* frame_PPDU
 *--------------------------------------------------------------------------------*/
static void wban_battery_update_rx(Packet* frame_PPDU) {
	Ici * iciptr;
	int PPDU_pksize;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_update_rx);
	
	PPDU_pksize = op_pk_total_size_get(frame_PPDU);
	iciptr = op_ici_create ("wban_battery_ici_format");
	op_ici_attr_set (iciptr, "PPDU Packet Size", PPDU_pksize);
	// op_ici_attr_set (iciptr, "Frame Type", frame_type);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), PACKET_RX_CODE, node_attr.my_battery);
	op_ici_install (OPC_NIL);
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_battery_end
 *
 * Description:	send information about the operation to do to the battery module 
 *
 * Input : 	Packet* frame_MPDU
 *--------------------------------------------------------------------------------*/
static void wban_battery_end() {
	Ici * iciptr;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_end);
	
	iciptr = op_ici_create ("wban_battery_ici_format");
	// op_ici_attr_set (iciptr, "Frame Type", frame_type);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), END_OF_SIM, node_attr.my_battery);
	op_ici_install (OPC_NIL);
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_battery_cca
 *
 * Description:	send information about the operation to do to the battery module 
 *
 * Input : 
 *--------------------------------------------------------------------------------*/
static void wban_battery_cca() {
	Ici * iciptr;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_cca);
	
	iciptr = op_ici_create ("wban_battery_ici_format");
	// op_ici_attr_set (iciptr, "Frame Type", frame_type);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), CCA_CODE, node_attr.my_battery);
	op_ici_install (OPC_NIL);
	
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
				// fprintf(log,"t=%f  -> Subqueue #%d is empty, wait for MAC frames \n\t -> capacity [%#e frames, %#e bits]. \n\n", op_sim_time(), i, pk_capacity, bit_capacity);
				printf(" [Node %s] t=%f  -> Subqueue #%d is empty, wait for MAC frames \n\t -> capacity [%#e frames, %#e bits]. \n\n", node_attr.name, op_sim_time(), i, pk_capacity, bit_capacity);
			}
		} else {
			if (enable_log) {	 
				// fprintf(log,"t=%f  -> Subqueue #%d is non empty, \n\t -> occupied space [%#e frames, %#e bits] - empty space [%#e frames, %#e bits] \n\n", op_sim_time(), i, op_subq_stat (i, OPC_QSTAT_PKSIZE), op_subq_stat (i, OPC_QSTAT_BITSIZE), op_subq_stat (i, OPC_QSTAT_FREE_PKSIZE), op_subq_stat (i, OPC_QSTAT_FREE_BITSIZE));
				printf(" [Node %s] t=%f  -> Subqueue #%d is non empty,\n\t -> occupied space [%#e frames, %#e bits] - empty space [%#e frames, %#e bits] \n\n", node_attr.name, op_sim_time(), i, op_subq_stat (i, OPC_QSTAT_PKSIZE), op_subq_stat (i, OPC_QSTAT_BITSIZE), op_subq_stat (i, OPC_QSTAT_FREE_PKSIZE), op_subq_stat (i, OPC_QSTAT_FREE_BITSIZE));
			}
		}
	}
	
	/* Stack tracing exit point */
	FOUT;
}

/*-----------------------------------------------------------------------------
 * Function:	subq_info_get
 *
 * Description:	get the parameter of each subqueue
 *
 * Input : subq_index
 *-----------------------------------------------------------------------------*/
static void subq_info_get (int subq_index) {
	Objid queue_objid;
	Objid subq_objid;
	double bit_capacity;
	double pk_capacity;
	Packet * pk_test_up;

	/* Stack tracing enrty point */
	FIN(subq_info_get);

	/* get the subqueues object ID */
	op_ima_obj_attr_get (mac_attr.objid, "subqueue", &queue_objid);

	/* Obtain object ID of the subq_index th subqueue */
	subq_objid = op_topo_child (queue_objid, OPC_OBJMTYPE_ALL, subq_index);
	
	/* Get current subqueue attribute settings */
	op_ima_obj_attr_get (subq_objid, "bit capacity", &bit_capacity);
	op_ima_obj_attr_get (subq_objid, "pk capacity", &pk_capacity);
	
	if (op_subq_empty(subq_index)) {
		if (enable_log) {
			// fprintf(log,"t=%f  -> Subqueue #%d is empty, wait for MAC frames \n\t -> capacity [%#e frames, %#e bits]. \n\n", op_sim_time(), subq_index, pk_capacity, bit_capacity);
			printf(" [Node %s] t=%f  -> Subqueue #%d is empty, wait for MAC frames \n\t -> capacity [%#e frames, %#e bits]. \n\n", node_attr.name, op_sim_time(), subq_index, pk_capacity, bit_capacity);
		}
		subq_info.pksize = 0;
		subq_info.bitsize = 0;
	} else {
		subq_info.pksize = op_subq_stat (subq_index, OPC_QSTAT_PKSIZE);
		subq_info.bitsize = op_subq_stat (subq_index, OPC_QSTAT_BITSIZE);
		subq_info.free_pksize = op_subq_stat (subq_index, OPC_QSTAT_FREE_PKSIZE);
		subq_info.free_bitsize = op_subq_stat (subq_index, OPC_QSTAT_FREE_BITSIZE);
		if (enable_log) {
			printf(" [Node %s] t=%f  -> Subqueue #%d is non empty,\n\t -> occupied space [%#e frames, %#e bits] - empty space [%#e frames, %#e bits] \n\n", node_attr.name, op_sim_time(), subq_index, subq_info.pksize, subq_info.bitsize, subq_info.free_pksize, subq_info.free_bitsize);
		}

		pk_test_up = op_subq_pk_access(subq_index, OPC_QPOS_PRIO);
		subq_info.up = op_pk_priority_get(pk_test_up);
		printf("subq_info.up=UP%d\n", subq_info.up);
	}

	/* Stack tracing exit point */
	FOUT;
}

// static void wban_analyzer_update (){
// 	d
// }