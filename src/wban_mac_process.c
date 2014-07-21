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
	int i, j;

	/* Stack tracing enrty point */
	FIN(wban_mac_init);
	
	/* obtain self object ID of the surrounding processor or queue */
	mac_attr.objid = op_id_self ();
	/* obtain object ID of the parent object (node) */
	node_attr.objid = op_topo_parent (mac_attr.objid);
	node_id = node_attr.objid;
	/* get the name of the node */
	op_ima_obj_attr_get (node_attr.objid, "name", &node_attr.name);
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
	
	/* obtain object ID of the Traffic Source node */
	traffic_source_up_id = op_id_from_name(node_attr.objid, OPC_OBJTYPE_PROC, "Traffic Source_UP");
	/* obtain destination ID for data transmission  */
	op_ima_obj_attr_get (traffic_source_up_id, "Destination ID", &node_attr.traffic_dest_id);
	/* get the MAC settings */
	op_ima_obj_attr_get (mac_attr.objid, "MAC Attributes", &mac_attr_id);
	mac_attr_comp_id = op_topo_child (mac_attr_id, OPC_OBJTYPE_GENERIC, 0);
	// op_ima_obj_attr_get (mac_attr_comp_id, "Batterie Life Extension", &mac_attr.Battery_Life_Extension);
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

	mac_state = MAC_SETUP;
	init_flag = OPC_TRUE;
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
		// op_ima_obj_attr_get (conn_assign_attr_comp_id, "EAP2 Start", &conn_assign_attr.eap2_start);
		// SF.eap2_start = conn_assign_attr.eap2_start;
		// if (SF.eap2_start > 0){
		// 	SF.map1_end = SF.eap2_start - 1;
		// }

		/* get the beacon2 frame for the Hub*/
		op_ima_obj_attr_get (node_attr.objid, "Beacon2", &b2_attr_id);
		b2_attr_comp_id = op_topo_child (b2_attr_id, OPC_OBJTYPE_GENERIC, 0);
		op_ima_obj_attr_get (b2_attr_comp_id, "CAP End", &b2_attr.cap_end);
		op_ima_obj_attr_get (b2_attr_comp_id, "MAP2 End", &b2_attr.map2_end);

		SF.current_first_free_slot = beacon_attr.rap1_start + beacon_attr.rap1_length;

		// register the beacon frame statistics
		beacon_frame_hndl = op_stat_reg ("MANAGEMENT.Number of Generated Beacon Frame", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
		wban_send_beacon_frame ();
	} else { /* if the node is not a Hub */
		mac_attr.sender_id = UNCONNECTED_NID;
		mac_attr.recipient_id = UNCONNECTED;
		//node_attr.unconnectedNID = 1 + (unconnectedNID - 1) % 16;
		//unconnectedNID++;
		node_attr.unconnectedNID = node_attr.objid;
		conn_assign_attr.interval_start = 255;
		conn_assign_attr.interval_end = 0;
		
		
			// fprintf (log," [Node %s] initialized with unconnectedNID %d \n\n", node_attr.name, node_attr.unconnectedNID);
			printf (" [Node %s] initialized with unconnectedNID %d \n\n", node_attr.name, node_attr.unconnectedNID);
		
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
	SF.SLEEP = OPC_TRUE;
	SF.ENABLE_TX_NEW = OPC_FALSE;
	pkt_to_be_sent.enable = OPC_FALSE;
	current_packet_txs = 0;
	current_packet_CS_fails = 0;
	waitForACK = OPC_FALSE;
	TX_ING = OPC_FALSE;
	attemptingToTX = OPC_FALSE;
	throughput = 0.0;
	temp_ppdu_kbits = 0.0;
	temp_last_ppdu_kbits = 0.0;
	throughput_rap_kbits = 0.0;
	throughput_map_kbits = 0.0;
	throughput_rcv_kbits = 0.0;
	throughput_rcv_nbr = 0.0;
	throughput_rap_msdu_kbits = 0;
	lastSF_msdu_nbr = 0;
	data_rx_nbr = 0;
	/* initialization for data_stat */
	for(i=0; i<UP_ALL; i++){
		latency_avg[i] = 0.0;
		for(j=0; j<DATA_STATE; j++){
			data_stat_local[i][j].number = 0.0;
			data_stat_local[i][j].ppdu_kbits = 0.0;
			data_stat_all[i][j].number = 0.0;
			data_stat_all[i][j].ppdu_kbits = 0.0;
		}
	}
	/* register the statistics */
	// stat_vec.data_pkt_fail = op_stat_reg("DATA.Data Packet failed", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	// stat_vec.data_pkt_suc1 = op_stat_reg("DATA.Data Packet Succed 1", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	// stat_vec.data_pkt_suc2 = op_stat_reg("DATA.Data Packet Succed 2", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	// stat_vec.data_pkt_sent = op_stat_reg("DATA.Data Packet Sent", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	// stat_vec.up7_sent = op_stat_reg("DATA.UP7 Sent", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	// stat_vec.up5_sent = op_stat_reg("DATA.UP5 Sent", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	// stat_vec.data_pkt_rec = op_stat_reg("DATA.Data Packet Received", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	// stat_vec.data_pkt_rec_map1 = op_stat_reg("DATA.Data Packet Received MAP1", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	/* register the GLOBAL statistics */
	// stat_vecG.data_pkt_fail = op_stat_reg("DATA.Data Packet failed", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	// stat_vecG.data_pkt_suc1 = op_stat_reg("DATA.Data Packet Succed 1", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	// stat_vecG.data_pkt_suc2 = op_stat_reg("DATA.Data Packet Succed 2", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	// stat_vecG.data_pkt_sent = op_stat_reg("DATA.Data Packet Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	// stat_vecG.up7_sent = op_stat_reg("DATA.UP7 Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	// stat_vecG.up5_sent = op_stat_reg("DATA.UP5 Sent", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	// stat_vecG.data_pkt_rec = op_stat_reg("DATA.Data Packet Received", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);


	// stat_vec.ppdu_sent_kbits = 0;
	// stat_vec.ppdu_sent_nbr = 0;
	// stat_vec.ppdu_sent_kbits_rap = 0;
	// stat_vec.ppdu_sent_nbr_rap = 0;
	// for Hub only
	// stat_vec.ppdu_rcv_kbits = 0;
	// stat_vec.ppdu_rcv_nbr = 0;
	// stat_vec.ppdu_rcv_kbits_rap = 0;
	// stat_vec.ppdu_rcv_nbr_rap = 0;
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
	char directory_path_name[200];
	char buffer[30];
	time_t rawtime;
	struct tm *p;

	/* Stack tracing enrty point */
	FIN(wban_log_file_init);

	op_ima_obj_attr_get (node_attr.objid, "Log File Directory", directory_path_name);

	/* verification if the directory_path_name is a valid directory */
	if (prg_path_name_is_dir (directory_path_name) == PrgC_Path_Name_Is_Not_Dir) {
		op_sim_end("ERROR : Log File Directory is not valid directory name.","INVALID_DIR", "","");
	}

	time(&rawtime);
	p=localtime(&rawtime);
    // strftime(buffer, 30, "%Y-%m-%d_%H-%M-%S", p);
    strftime(buffer, 30, "%Y-%m-%d_%H-%M", p);
    sprintf(log_name, "%s%s-ver%d.trace", directory_path_name, buffer, node_attr.protocol_ver);

    /* Check for existence */
    // if((_access( log_name, 0 )) != -1){
    //     printf("File %s exists\n", log_name);
    // 	log = fopen(log_name, "a");
    // } else {
    // 	printf("File %s unexists.\n", log_name);
    // 	log = fopen(log_name, "w");
    // }
	
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
	int ppdu_bits;
	double ete_delay;

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
			/* get MAC frame (MPDU=PSDU) from received PHY frame (PPDU)*/
			op_pk_nfd_get_pkt (rcv_frame, "PSDU", &frame_MPDU);
			/*update the battery*/
			// packet_size = op_pk_total_size_get(frame_MPDU);
			// printf("Received packet size=%d.\n", packet_size);
			ete_delay = op_sim_time() - op_pk_creation_time_get(frame_MPDU);
			if (MAC_SLEEP == mac_state){
				FOUT;
			}
			
			op_pk_nfd_get (frame_MPDU, "BAN ID", &ban_id);
			op_pk_nfd_get (frame_MPDU, "Recipient ID", &recipient_id);
			op_pk_nfd_get (frame_MPDU, "Sender ID", &sender_id);
			// filter the incoming BAN packet - not implemented entirely
    		if (!is_packet_for_me(frame_MPDU, ban_id, recipient_id, sender_id)) {
    			if((waitForACK) && (sender_id != mac_attr.sender_id)){
					wban_battery_update_rx(ppdu_bits, mac_state);
    			}
    			FOUT;
    		} else {
				wban_battery_update_rx(ppdu_bits, mac_state);
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
			log = fopen(log_name, "a");
			fprintf(log, "t=%f,NODE_ID=%d,MAC_STATE=%d,RX,RECIPIENT_ID=%d,SENDER_ID=%d,", op_sim_time(), node_id, mac_state, recipient_id, sender_id);
			fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d,ETE_DELAY=%f\n", frame_type_fd, frame_subtype_fd, ppdu_bits, ete_delay);
			fclose(log);
			if(I_ACK_POLICY == ack_policy_fd){
				ack_seq_num = sequence_number_fd;
				op_intrpt_schedule_self(op_sim_time()+pSIFS, SEND_I_ACK);
			}
			switch (frame_type_fd) {
				case DATA: /* Handle data packets */
					printf (" [Node %s] t=%f  !!!!!!!!! Data Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
					/* collect statistics */
					latency_avg[frame_subtype_fd] = (latency_avg[frame_subtype_fd] * data_stat_local[frame_subtype_fd][RCV].number + ete_delay)/(data_stat_local[frame_subtype_fd][RCV].number + 1);
					data_stat_local[frame_subtype_fd][RCV].number += 1;
					data_stat_local[frame_subtype_fd][RCV].ppdu_kbits += 0.001*ppdu_bits;
					// throughput_rcv_kbits += 0.001*ppdu_bits;
					// throughput_rcv_nbr += 1;
					if(MAC_RAP1 == mac_state){
						throughput_rap_msdu_kbits += 0.001*(ppdu_bits - header4mac2phy() - 72);
						throughput_rap_kbits += 0.001*ppdu_bits;
					}else if(SF.IN_MAP_PHASE){
						throughput_map_kbits += 0.001*ppdu_bits;
					}
					// stat_vec.ppdu_rcv_nbr = stat_vec.ppdu_rcv_nbr + 1;
					// stat_vec.ppdu_rcv_kbits = stat_vec.ppdu_rcv_kbits + 1.0*ppdu_bits/1000.0;
					// if((mac_state == MAC_RAP1) && (node_attr.is_BANhub)){
					// 	stat_vec.ppdu_rcv_kbits_rap = stat_vec.ppdu_rcv_kbits_rap + 1.0*ppdu_bits/1000.0;
					// 	stat_vec.ppdu_rcv_nbr_rap = stat_vec.ppdu_rcv_nbr_rap + 1;
					// // }
					// op_stat_write(stat_vec.data_pkt_rec, op_pk_total_size_get(frame_MPDU));
					// op_stat_write(stat_vecG.data_pkt_rec, op_pk_total_size_get(frame_MPDU));

					wban_extract_data_frame (frame_MPDU);
					/* send to higher layer for statistics */
					op_pk_send (frame_MPDU, STRM_FROM_MAC_TO_SINK);
					break;
				case MANAGEMENT: /* Handle management packets */
					// op_stat_write(stat_vec.data_pkt_rec, 0.0);
					printf (" [Node %s] t=%f  !!!!!!!!! Management Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
					
					switch (frame_subtype_fd) {
						case BEACON: 
							printf (" [Node %s] t=%f  -> Beacon Frame Reception From @%d \n\n", node_attr.name, op_sim_time(), sender_id);
						
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
							printf (" [Node %s] t=%f  !!!!!!!!! Connection request Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
							wban_extract_conn_req_frame (frame_MPDU);
							break;
						case CONNECTION_ASSIGNMENT:
						{
							printf (" [Node %s] t=%f  !!!!!!!!! Connection assignment Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
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
				case CONTROL: /* Handle control packets */
					printf (" [Node %s] t=%f  !!!!!!!!! Control Frame Reception From @%d !!!!!!!!! \n\n", node_attr.name, op_sim_time(), sender_id);
					switch (frame_subtype_fd) {
						case I_ACK:
							wban_extract_i_ack_frame (frame_MPDU);
							break;
						case B_ACK:
							// not implemented
							break;
						case BEACON2: 
						{
							wban_battery_sleep_start(mac_state);
							mac_state = MAC_SLEEP;
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
		printf (" [Node %s] t=%f  -> Loop: DISCARD FRAME \n\n",node_attr.name, op_sim_time());
		op_pk_destroy (frame_MPDU);

		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}

	if (node_attr.ban_id != ban_id) {
		printf (" [Node %s] The packet from BAN ID %d is not the same with my BAN ID %d.\n", node_attr.name, ban_id, node_attr.ban_id);
		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}

	if ((mac_attr.sender_id == recipient_id) || (BROADCAST_NID == recipient_id)) {
		/* Stack tracing exit point */
		FRET(OPC_TRUE);
	} else {
		// printf (" [Node %s] t=%f  -> Not the frame for me: DISCARD FRAME \n\n",node_attr.name, op_sim_time());
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
	// double beacon_frame_tx_time;
	extern int sequence_num_beaconG;

	/* Stack tracing enrty point */
	FIN(wban_send_beacon_frame);
	
	printf (" \nNODE \"%s\" IS A HUB WITH A SUPERFRAME STRUCTURE AS FOLLOWS \n", node_attr.name);
	printf (" \t Beacon Period Length   : %d \n", beacon_attr.beacon_period_length);
	printf (" \t Allocation Slot Length : %d \n", beacon_attr.allocation_slot_length);
	printf (" \t RAP1 Start             : %d \n", beacon_attr.rap1_start);
	printf (" \t RAP1 End               : %d \n", beacon_attr.rap1_end);
	// printf (" \t RAP2 Start             : %d \n", beacon_attr.rap2_start);
	// printf (" \t RAP2 End               : %d \n", beacon_attr.rap2_end);
	printf (" \t B2 Start               : %d \n", beacon_attr.b2_start);
	// printf (" \t Inactive Duration      : %d\n\n", beacon_attr.inactive_duration);
	
	/* create a beacon frame */
	beacon_MSDU = op_pk_create_fmt ("wban_beacon_MSDU_format");
	
	/* set the fields of the beacon frame */
	op_pk_nfd_set (beacon_MSDU, "Sender Address", beacon_attr.sender_address);
	op_pk_nfd_set (beacon_MSDU, "Beacon Period Length", beacon_attr.beacon_period_length);
	op_pk_nfd_set (beacon_MSDU, "Allocation Slot Length", beacon_attr.allocation_slot_length);
	op_pk_nfd_set (beacon_MSDU, "RAP1 End", beacon_attr.rap1_end);
	// op_pk_nfd_set (beacon_MSDU, "RAP2 Start", beacon_attr.rap2_start);
	// op_pk_nfd_set (beacon_MSDU, "RAP2 End", beacon_attr.rap2_end);
	op_pk_nfd_set (beacon_MSDU, "RAP1 Start", beacon_attr.rap1_start);
	op_pk_nfd_set (beacon_MSDU, "B2 Start", beacon_attr.b2_start);
	// op_pk_nfd_set (beacon_MSDU, "Inactive Duration", beacon_attr.inactive_duration);
	
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
	// beacon_frame_tx_time = TX_TIME(wban_norm_phy_bits(beacon_MPDU), node_attr.data_rate);
	op_prg_odb_bkpt("send_beacon");
	// if(SF.rap1_start > 0){
	// 	SF.eap1_start2sec = SF.BI_Boundary + beacon_frame_tx_time + pSIFS;
	// }
	if(init_flag){
		log = fopen(log_name, "a");
		fprintf(log, "t=%f,NODE_ID=%d,INIT,NODE_NAME=%s,NID=%d,", op_sim_time(), node_id, node_attr.name, mac_attr.sender_id);
		fprintf(log, "SUPERFRAME_LENGTH=%d,RAP1_LENGTH=%d,B2_START=%d\n", beacon_attr.beacon_period_length, beacon_attr.rap1_length, beacon_attr.b2_start);
		fclose(log);
		init_flag = OPC_FALSE;
	}

	/* send the MPDU to PHY and calculate the energy consuming */
	wban_send_mac_pk_to_phy(beacon_MPDU);
	
	
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
	int free_slot;
	int i;
	int j;
	int shuffle;
	int slot_num;
	int slot_avg;
	int slot_req;
	int slot_req_total;
	int slot_map2_total;

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
	beacon2_frame_tx_time = TX_TIME(wban_norm_phy_bits(beacon2_MPDU), node_attr.data_rate);
	/* send the MPDU to PHY and calculate the energer consuming */
	wban_send_mac_pk_to_phy(beacon2_MPDU);
	// op_prg_odb_bkpt("send_b2");
	SF.cap_end = b2_attr.cap_end;
	if((SF.cap_end > 0) && (SF.cap_end < SF.b2_start)){
		op_sim_end("ERROR : CAP_END must greater than B2_START","B2_CAP","","");
	}else if(SF.cap_end < 0){
		op_sim_end("ERROR : CAP_END must greater than 0","CAP","","");
	}else if(SF.cap_end > 255){
		op_sim_end("ERROR : CAP_END must less than 256","CAP","","");
	}
	// SF.map2_end = b2_attr.map2_end;
	/* Additional processing for proposed protocol */
	if (1 == node_attr.protocol_ver) {
		SF.map2_end = b2_attr.map2_end;
		if(SF.map2_end == 0){
			op_sim_end("ERROR : MAP2_END must greater than 0 for Protocol version 1","MAP2_END", "","");
		}
		if(255 == SF.map2_end){
			SF.map2_end = SF.SD - 1;
		}
		if(255 == SF.cap_end){
			SF.cap_end = SF.SD - 1;
		}
		if(SF.cap_end > SF.map2_end){
			SF.cap_start = SF.b2_start;
			SF.cap_start2sec = SF.BI_Boundary + (SF.map2_end+1) * SF.slot_length2sec;
			SF.cap_end2sec = SF.BI_Boundary + (SF.cap_end+1) * SF.slot_length2sec;
			op_intrpt_schedule_self(SF.cap_start2sec, START_OF_CAP_PERIOD_CODE);
			op_intrpt_schedule_self(SF.cap_end2sec, END_OF_CAP_PERIOD_CODE);
		}
		free_slot = SF.b2_start;
		j = rand_int(current_free_connected_NID - 32);
		slot_avg = (SF.map2_end - SF.b2_start + 1)/(current_free_connected_NID - 32);
		slot_req_total = 0;
		slot_map2_total = SF.map2_end - SF.b2_start + 1;
		for(i=32; i < current_free_connected_NID; i++){
			slot_req_total = slot_req_total + assign_map[i%NODE_MAX].slotnum;
		}
		for(i=32; i < current_free_connected_NID; i++){
			shuffle = 32 + (i+j)%(current_free_connected_NID - 32);
			if((assign_map[shuffle%NODE_MAX].slotnum > slot_avg) && (slot_req_total > slot_map2_total)){
				slot_req = max_int(slot_avg, (assign_map[shuffle%NODE_MAX].slotnum-(slot_req_total-slot_map2_total)));
				slot_req_total = slot_req_total - (assign_map[shuffle%NODE_MAX].slotnum - slot_req);
				assign_map[shuffle%NODE_MAX].slotnum = slot_req;
			}
			if(assign_map[shuffle%NODE_MAX].slotnum > 0){
				slot_num = assign_map[shuffle%NODE_MAX].slotnum;
				// if(slot_num > 4){
				// 	slot_num = 4;
				// }
				if(free_slot + slot_num < SF.map2_end + 2){
					assign_map[shuffle%NODE_MAX].map2_slot_start = free_slot;
					assign_map[shuffle%NODE_MAX].map2_slot_end = assign_map[shuffle%NODE_MAX].map2_slot_start+slot_num-1;
					free_slot = assign_map[shuffle%NODE_MAX].map2_slot_end + 1;
				}else{
					assign_map[shuffle%NODE_MAX].map2_slot_start = 0;
					assign_map[shuffle%NODE_MAX].map2_slot_end = 0;
				}
			} else {
				assign_map[shuffle%NODE_MAX].map2_slot_start = 0;
				assign_map[shuffle%NODE_MAX].map2_slot_end = 0;
			}
		}
		SF.map2_start = SF.b2_start;
		SF.map2_start2sec = op_sim_time() + beacon2_frame_tx_time + pSIFS;
		SF.map2_end2sec = SF.BI_Boundary + (SF.map2_end+1)*SF.slot_length2sec;
		op_intrpt_schedule_self(SF.map2_start2sec, START_OF_MAP2_PERIOD_CODE);
		op_intrpt_schedule_self(SF.map2_end2sec, END_OF_MAP2_PERIOD_CODE);
		op_prg_odb_bkpt("debug_map2");
		
		// for(i=32; i < current_free_connected_NID; i++){
		// 	printf("NID=%d,", i);
		// 	printf("map2_slot_start=%d,", assign_map[i%NODE_MAX].map2_slot_start);
		// 	printf("map2_slot_end=%d\n", assign_map[i%NODE_MAX].map2_slot_end);
		// }
		// op_prg_odb_bkpt("debug");
	}else{
		if(0 == SF.cap_end){
			op_sim_end("ERROR : CAP_END must greater than 0 for Protocol version 0","CAP","","");
		}
		SF.cap_start2sec = SF.b2_start2sec + beacon2_frame_tx_time + pSIFS;
		if(255 == SF.cap_end){
			SF.cap_end = SF.SD - 1;
		}
		SF.cap_end2sec = SF.BI_Boundary + (SF.cap_end+1) * SF.slot_length2sec;
		op_intrpt_schedule_self(SF.cap_start2sec, START_OF_CAP_PERIOD_CODE);
		op_intrpt_schedule_self(SF.cap_end2sec, END_OF_CAP_PERIOD_CODE);
	}

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

	op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
	op_pk_nfd_get (frame_MSDU, "Allocation Length", &allocation_length);
	op_pk_nfd_get (frame_MSDU, "User Priority", &user_priority);

	conn_assign_attr.allocation_length = allocation_length;
	printf("NID=%d Allocation Length=%d.\n", mac_attr.recipient_id, allocation_length);
	op_prg_odb_bkpt("rcv_req");
	// wban_send_conn_assign
	wban_send_conn_assign_frame(allocation_length);
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
	// log = fopen(log_name, "a");
	// fprintf(log, "MAP_allocation,Interval_Start=%d,Interval_End=%d\n", conn_assign_attr.interval_start,conn_assign_attr.interval_end);
	// fclose(log);
	op_prg_odb_bkpt("debug");
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
	beacon_frame_tx_time = TX_TIME(wban_norm_phy_bits(beacon_MPDU_rx), node_attr.data_rate);
	op_pk_nfd_get_pkt (beacon_MPDU_rx, "MAC Frame Payload", &beacon_MSDU_rx);
	// if I'm a End Device, I get the information and synchronize myself
	if (!node_attr.is_BANhub) {
		op_pk_nfd_get (beacon_MPDU_rx, "Sender ID", &rcv_sender_id);
		op_pk_nfd_get (beacon_MPDU_rx, "Sequence Number", &sequence_number_fd);
		op_pk_nfd_get (beacon_MPDU_rx, "EAP Indicator", &eap_indicator_fd);
		op_pk_nfd_get (beacon_MPDU_rx, "B2", &beacon2_enabled_fd);

		if (node_attr.ban_id + 15 != rcv_sender_id) {
			printf(" [Node %s] t=%f  -> Beacon Frame Reception - but not from Hub. \n", node_attr.name, op_sim_time());
			/* Stack tracing exit point */
			FOUT;
		} else {
			printf ("   -> Sequence Number              : %d \n", sequence_number_fd);
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
			mac_attr.sender_id = current_free_connected_NID++;
			// current_free_connected_NID++;
			// printf("Node %s get the NID=%d\n", node_attr.name, mac_attr.sender_id);
			// op_prg_odb_bkpt("debug");
			log = fopen(log_name, "a");
			fprintf(log, "t=%f,NODE_ID=%d,INIT,NODE_NAME=%s,NID=%d,", op_sim_time(), node_id, node_attr.name, mac_attr.sender_id);
			fprintf(log, "SUPERFRAME_LENGTH=%d,RAP1_LENGTH=%d,B2_START=%d\n", beacon_attr.beacon_period_length, beacon_attr.rap1_length, beacon_attr.b2_start);
			fclose(log);
			// op_prg_odb_bkpt("get_nid");
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
	// int i;
	double beacon2_frame_tx_time;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_beacon2_frame);

	beacon2_PPDU_size = wban_norm_phy_bits(beacon2_MPDU_rx);
	op_pk_nfd_get_pkt (beacon2_MPDU_rx, "MAC Frame Payload", &beacon2_MSDU_rx);
	op_pk_nfd_get (beacon2_MPDU_rx, "Sender ID", &rcv_sender_id);
	op_pk_nfd_get (beacon2_MPDU_rx, "Sequence Number", &sequence_number_fd);
	op_pk_nfd_get (beacon2_MPDU_rx, "EAP Indicator", &eap_indicator_fd);
	op_pk_nfd_get (beacon2_MPDU_rx, "B2", &beacon2_enabled_fd);

	// if (node_attr.ban_id + 15 != rcv_sender_id) {
	// 	printf(" [Node %s] t=%f  -> Beacon2 Frame Reception - but not from Hub. \n", node_attr.name, op_sim_time());
	// 	/* Stack tracing exit point */
	// 	FOUT;
	// } else {
	// 	printf (" [Node %s] t=%f  -> Beacon2 Frame Reception - synchronization. \n", node_attr.name, op_sim_time());
	// 	printf ("   -> Sequence Number              : %d \n", sequence_number_fd);
	// }
	// op_pk_nfd_get (beacon2_MSDU_rx, "Beacon Period Length", &beacon_attr.beacon_period_length);
	// op_pk_nfd_get (beacon2_MSDU_rx, "Allocation Slot Length", &beacon_attr.allocation_slot_length);
	op_pk_nfd_get (beacon2_MSDU_rx, "CAP End", &b2_attr.cap_end);
	op_pk_nfd_get (beacon2_MSDU_rx, "MAP2 End", &b2_attr.map2_end);

	SF.cap_end = b2_attr.cap_end;
	if((SF.cap_end > 0) && (SF.cap_end < SF.b2_start)){
		op_sim_end("ERROR : CAP_END must greater than B2_START","B2_CAP","","");
	}else if(SF.cap_end < 0){
		op_sim_end("ERROR : CAP_END must greater than 0","CAP","","");
	}else if(SF.cap_end > 255){
		op_sim_end("ERROR : CAP_END must less than 256","CAP","","");
	}
	beacon2_frame_tx_time = TX_TIME(beacon2_PPDU_size, node_attr.data_rate);
	// printf("SF.b2_start2sec=%f\n", SF.b2_start2sec);
	// printf("beacon2_frame_tx_time=%f\n", beacon2_frame_tx_time);
	// printf("SF.b2_start2sec + beacon2_frame_tx_time + pSIFS=");
	// printf("%f\n", SF.b2_start2sec + beacon2_frame_tx_time + pSIFS);
	/* Additional processing for proposed protocol */
	if (1 == node_attr.protocol_ver) {
		if(255 == SF.cap_end){
			SF.cap_end = SF.SD - 1;
		}
		if(SF.cap_end > b2_attr.map2_end){
			SF.cap_start = b2_attr.map2_end+1;
			SF.cap_start2sec = SF.BI_Boundary + (b2_attr.map2_end+1) * SF.slot_length2sec;
			SF.cap_end2sec = SF.BI_Boundary + (SF.cap_end+1) * SF.slot_length2sec;
			op_intrpt_schedule_self(SF.cap_start2sec, START_OF_CAP_PERIOD_CODE);
			op_intrpt_schedule_self(SF.cap_end2sec, END_OF_CAP_PERIOD_CODE);
		}
		printf("%s cap_start2sec=%f,cap_end2sec=%f\n", node_attr.name, SF.cap_start2sec,SF.cap_end2sec);
		// printf("SF.cap_start2sec replaced with %f for protocol version 1.\n", SF.cap_start2sec);
		
		if(assign_map[mac_attr.sender_id%NODE_MAX].map2_slot_start > 0){
			SF.map2_start = assign_map[mac_attr.sender_id%NODE_MAX].map2_slot_start;
			SF.map2_end = assign_map[mac_attr.sender_id%NODE_MAX].map2_slot_end;
			SF.map2_start2sec = SF.BI_Boundary + SF.map2_start * SF.slot_length2sec;
			SF.map2_end2sec = SF.BI_Boundary + (SF.map2_end+1) * SF.slot_length2sec;
			if (SF.map2_start == SF.b2_start){
				SF.map2_start2sec = op_sim_time() + pSIFS;
			}
			printf("NID=%d allocated with map2_start=%d, map2_end=%d\n", mac_attr.sender_id, SF.map2_start, SF.map2_end);
			op_intrpt_schedule_self(max_double(SF.map2_start2sec, op_sim_time()), START_OF_MAP2_PERIOD_CODE);
			op_intrpt_schedule_self(SF.map2_end2sec, END_OF_MAP2_PERIOD_CODE);
		}
		wban_battery_sleep_start(mac_state);
		mac_state = MAC_SLEEP;
		op_prg_odb_bkpt("debug");
	}else{
		if(0 == SF.cap_end){
			op_sim_end("ERROR : CAP_END must greater than 0 for Protocol version 0","CAP","","");
		}
		SF.cap_start2sec = SF.b2_start2sec + beacon2_frame_tx_time + pSIFS;
		if(255 == SF.cap_end){
			SF.cap_end = SF.SD - 1;
		}
		SF.cap_end2sec = SF.BI_Boundary + (SF.cap_end+1) * SF.slot_length2sec;
		op_intrpt_schedule_self(SF.cap_start2sec, START_OF_CAP_PERIOD_CODE);
		op_intrpt_schedule_self(SF.cap_end2sec, END_OF_CAP_PERIOD_CODE);
	}
	op_prg_odb_bkpt("extract_b2");

	op_pk_destroy (beacon2_MSDU_rx);
	op_pk_destroy (beacon2_MPDU_rx);

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
	double queue_conn_req;
	/* Stack tracing enrty point */
	FIN(wban_schedule_next_beacon);

	// update the superframe parameters
	SF.SD = beacon_attr.beacon_period_length; // the superframe duration(beacon preriod length) in slots
	SF.BI = beacon_attr.beacon_period_length * (1+ beacon_attr.inactive_duration); // active and inactive superframe
	SF.sleep_period = SF.SD * beacon_attr.inactive_duration;

	SF.slot_length2sec = 0.001*allocationSlotLength2ms; // transfer allocation slot length from ms to sec.
	SF.duration = SF.BI*SF.slot_length2sec;
	SF.rap1_start = beacon_attr.rap1_start;
	SF.rap1_end = beacon_attr.rap1_end;
	// SF.rap2_start = beacon_attr.rap2_start;
	// SF.rap2_end = beacon_attr.rap2_end;
	SF.b2_start = beacon_attr.b2_start;
	SF.current_slot = 0;
	// SF.current_first_free_slot = beacon_attr.rap1_end + 1; // spec for hub assignment

	/* Allocation for EAP1 and RAP1 */
	if(SF.rap1_start > 0){
		SF.rap1_start2sec = SF.BI_Boundary + SF.rap1_start * SF.slot_length2sec;
		if(IAM_BAN_HUB){
			SF.eap1_start2sec = op_sim_time();
		}else{
			SF.eap1_start2sec = op_sim_time() + pSIFS;
		}
		SF.eap1_end2sec = SF.rap1_start2sec;
		op_intrpt_schedule_self (SF.eap1_start2sec, START_OF_EAP1_PERIOD_CODE);
		op_intrpt_schedule_self (SF.eap1_end2sec, END_OF_EAP1_PERIOD_CODE);
	}else{
		if(IAM_BAN_HUB){
			SF.rap1_start2sec = op_sim_time();
		}else{
			SF.rap1_start2sec = op_sim_time() + pSIFS;
		}
	}

	// if((SF.rap1_start > 0) && (SF.rap1_end > 0)){
	if(SF.rap1_end > 0){
		SF.rap1_end2sec = SF.BI_Boundary + (SF.rap1_end+1)*SF.slot_length2sec;
		SF.rap1_length2sec = SF.rap1_end2sec - SF.rap1_start2sec;
		op_intrpt_schedule_self(SF.rap1_start2sec, START_OF_RAP1_PERIOD_CODE);
		op_intrpt_schedule_self(SF.rap1_end2sec, END_OF_RAP1_PERIOD_CODE);
		if((IAM_BAN_HUB) && (SF.rap1_end < SF.SD - 1) && (SF.b2_start != SF.rap1_end+1)){
			SF.map1_start = SF.rap1_end + 1;
			SF.map1_start2sec = SF.rap1_end2sec;
			op_intrpt_schedule_self (SF.map1_start2sec, START_OF_MAP1_PERIOD_CODE);
		}
	}

	if(SF.b2_start >= (SF.rap1_end+1)){
		SF.b2_start2sec = SF.BI_Boundary + SF.b2_start * SF.slot_length2sec;
		op_intrpt_schedule_self (SF.b2_start2sec, SEND_B2_FRAME);
		if((IAM_BAN_HUB) && (SF.b2_start > (SF.rap1_end+1))) {
			SF.map1_end = SF.b2_start - 1;
			SF.map1_end2sec = SF.b2_start2sec;
			op_intrpt_schedule_self (SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
		}
	}else if((SF.rap1_end < SF.SD - 1) && (0 == SF.b2_start)){
		if(IAM_BAN_HUB){
			SF.map1_end = SF.SD - 1;
			SF.map1_end2sec = SF.BI_Boundary + SF.BI*SF.slot_length2sec;
			op_intrpt_schedule_self (SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
		}
	}else if(SF.b2_start < SF.rap1_end){
		op_sim_end("ERROR : B2_START must greater than RAP1_END","RAP_B2","","");
	}

	// SF.current_first_free_slot = SF.rap1_end + 1; // spec for hub assignment

	// op_prg_odb_bkpt("sch_beacon");
	/* INCREMENT_SLOT at slot boundary */
	op_intrpt_schedule_self (SF.BI_Boundary + SF.slot_length2sec, INCREMENT_SLOT);
	op_intrpt_schedule_self (SF.BI_Boundary + SF.BI*SF.slot_length2sec, BEACON_INTERVAL_CODE);
	
	// printf("Node %s Superframe parameters:\n", node_attr.name);
	// printf("\tSF.eap1_start2sec=%f\n", SF.eap1_start2sec);
	// printf("\tSF.eap1_end2sec=%f\n", SF.eap1_end2sec);
	// printf("\tSF.rap1_start2sec=%f\n", SF.rap1_start2sec);
	// printf("\tSF.rap1_end2sec=%f\n", SF.rap1_end2sec);
	// printf("\tSF.map1_start2sec=%f\n", SF.map1_start2sec);
	// printf("\tSF.map1_end2sec=%f\n", SF.map1_end2sec);
	// printf("\tSF.map2_start2sec=%f\n", SF.map2_start2sec);
	// printf("\tSF.map2_end2sec=%f\n", SF.map2_end2sec);
	// fprintf (log,"t=%f  -> Schedule Next Beacon at %f\n\n", op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec);
	// printf (" [Node %s] t=%f  -> Schedule Next Beacon at %f\n\n", node_attr.name, op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_length2sec);
	
	if(!IAM_BAN_HUB){
		if((conn_assign_attr.interval_end > 0) && (conn_assign_attr.interval_start > SF.rap1_end)) {
			// printf("%s has been assigned with interval_start=%d, interval_end=%d\n", node_attr.name, conn_assign_attr.interval_start, conn_assign_attr.interval_end);
			FOUT;
		}
		queue_conn_req = op_sim_time()+(mac_attr.sender_id%8)*3*I_ACK_TX_TIME + 2*pSIFS;
		op_intrpt_schedule_self(queue_conn_req, SEND_CONN_REQ_CODE);
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

	/* Stack tracing enrty point */
	FIN(wban_send_conn_req_frame);
	printf("Connection Request allocation_length=%d for Node %s\n", conn_req_attr.allocation_length, node_attr.name);
	if (conn_req_attr.allocation_length > 0) {
		// if(conn_req_attr.allocation_length > 8){
			/* maximum allocation length for a node is 8 */
		// 	conn_req_attr.allocation_length = 8;
		// }
	} else {
		// printf("Node %s need not scheduling slots.\n", node_attr.name);
		FOUT;
	}

	op_prg_odb_bkpt("pre_req");
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

	/* put it into the queue with priority waiting for transmission */
	op_pk_priority_set (conn_req_MPDU, 6.0);
	if (op_subq_pk_insert(SUBQ_MAN, conn_req_MPDU, OPC_QPOS_TAIL) == OPC_QINS_OK) {
		// printf (" [Node %s] t=%f  -> Enqueuing of MAC MANAGEMENT frame [SEQ = %d]\n\n", node_attr.name, op_sim_time(), random_num);
	} else {
		// printf (" [Node %s] t=%f  -> MAC MANAGEMENT frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", node_attr.name, op_sim_time());
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
	int i;

	/* Stack tracing enrty point */
	FIN(wban_send_conn_assign_frame);
	random_num = rand_int(256);
	/* create a connection request frame */
	conn_assign_MSDU = op_pk_create_fmt ("wban_connection_assignment_frame_format");
	
	/* set the fields of the conn_assign frame */
	op_pk_nfd_set (conn_assign_MSDU, "Connection Status", 0);

	if(assign_map[mac_attr.recipient_id%10].slot_start != 0){
		/* set the fields of the conn_assign frame */
		// op_pk_nfd_set (conn_assign_MSDU, "EAP2 Start", );
		op_pk_nfd_set (conn_assign_MSDU, "Interval Start", assign_map[mac_attr.recipient_id%10].slot_start);
		op_pk_nfd_set (conn_assign_MSDU, "Interval End", assign_map[mac_attr.recipient_id%10].slot_end);

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
			// printf (" [Node %s] t=%f  -> Enqueuing of MAC MANAGEMENT frame [SEQ = %d]\n\n", node_attr.name, op_sim_time(), random_num);
		} else {
			// printf (" [Node %s] t=%f  -> MAC MANAGEMENT frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", node_attr.name, op_sim_time());
			/* destroy the packet */
			op_pk_destroy (conn_assign_MPDU);
		}
		printf("NID %d has been assigned with slot %d to %d.\n", mac_attr.recipient_id, assign_map[mac_attr.recipient_id%10].slot_start, assign_map[mac_attr.recipient_id%10].slot_end);
		FOUT;
	}

	if((0 == node_attr.protocol_ver) || (1 == node_attr.protocol_ver)){
		if(SF.current_first_free_slot <= SF.current_slot){
			printf("%s current_slot=%d,current_first_free_slot=%d\n", node_attr.name, SF.current_slot, SF.current_first_free_slot);
			// op_sim_end("ERROR : MAP allocation ERROR","MAP allocation","","");
			// SF.current_first_free_slot = SF.current_slot + 1;
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

	assign_map[mac_attr.recipient_id%10].nid = mac_attr.recipient_id;
	assign_map[mac_attr.recipient_id%10].slot_start = conn_assign_attr.interval_start;
	assign_map[mac_attr.recipient_id%10].slot_end = conn_assign_attr.interval_end;

	printf("NID=%d allocated with slot_start=%d, slot_end=%d\n", mac_attr.recipient_id, conn_assign_attr.interval_start, conn_assign_attr.interval_end);
	op_prg_odb_bkpt("map_alloc");
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
		// printf (" [Node %s] t=%f  -> Enqueuing of MAC MANAGEMENT frame [SEQ = %d]\n\n", node_attr.name, op_sim_time(), random_num);
	} else {
		// printf (" [Node %s] t=%f  -> MAC MANAGEMENT frame cannot be enqueuing - FRAME IS DROPPED !!!! \n\n", node_attr.name, op_sim_time());
		/* destroy the packet */
		op_pk_destroy (conn_assign_MPDU);
	}

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
	double ack_sent;
	Packet* frame_MPDU;
	/* Stack tracing enrty point */
	FIN(wban_send_i_ack_frame);

	// printf(" [Node %s] t=%f  -> Send ACK Frame [SEQ = %d] \n\n", node_attr.name, op_sim_time(), seq_num);
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

	log = fopen(log_name, "a");
	fprintf(log, "t=%f,NODE_ID=%d,MAC_STATE=%d,TX,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), node_id, mac_state, mac_attr.sender_id, mac_attr.recipient_id);
	fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", CONTROL, I_ACK, wban_norm_phy_bits(frame_MPDU));
	fclose(log);

	phy_to_radio(frame_MPDU);
	ack_sent = op_sim_time() + I_ACK_TX_TIME + pSIFS;
	if(ack_sent < phase_end_timeG){
		op_intrpt_schedule_self(ack_sent, TRY_PACKET_TRANSMISSION_CODE);
	}
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

	op_pk_nfd_get (data_frame_up, "User Priority", &user_priority);
	op_pk_nfd_get (data_frame_up, "App Sequence Number", &seq_num);
	op_pk_nfd_get_pkt (data_frame_up, "MSDU Payload", &data_frame_msdu);

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
	data_stat_local[user_priority][GEN].number += 1;
	data_stat_local[user_priority][GEN].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
	data_stat_all[user_priority][GEN].number += 1;
	data_stat_all[user_priority][GEN].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
	// log = fopen(log_name, "a");
	if (op_subq_pk_insert(SUBQ_DATA, data_frame_mpdu, OPC_QPOS_TAIL) == OPC_QINS_OK) {
		data_stat_local[user_priority][QUEUE_SUCC].number += 1;
		data_stat_local[user_priority][QUEUE_SUCC].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
		data_stat_all[user_priority][QUEUE_SUCC].number += 1;
		data_stat_all[user_priority][QUEUE_SUCC].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
		// fprintf(log, "t=%f,NODE_ID=%d,MAC_STATE=%d,APP_LAYER_ENQUEUE_SUCC,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), node_id, mac_state, mac_attr.sender_id, dest_id);
		// fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", DATA, user_priority, wban_norm_phy_bits(data_frame_mpdu));
	} else {
		data_stat_local[user_priority][QUEUE_FAIL].number += 1;
		data_stat_local[user_priority][QUEUE_FAIL].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
		data_stat_all[user_priority][QUEUE_FAIL].number += 1;
		data_stat_all[user_priority][QUEUE_FAIL].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
		// fprintf(log, "t=%f,NODE_ID=%d,MAC_STATE=%d,APP_LAYER_ENQUEUE_FAIL,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), node_id, mac_state, mac_attr.sender_id, dest_id);
		// fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", DATA, user_priority, wban_norm_phy_bits(data_frame_mpdu));
		/* destroy the packet */
		op_pk_destroy (data_frame_mpdu);
	}
	// fclose(log);
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
	double tx_suc_nbr;
	double tx_suc_ppdu_kbits;
	double th_msdu_sf_kbps;
	double th_msdu_kbps;
	int i, j;
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
					// pkt_to_be_sent.enable = OPC_FALSE;
					attemptingToTX = OPC_FALSE;
					TX_ING = OPC_FALSE;
					/* This connection is terminated. */
					// if (op_subq_empty (SUBQ_MAN) == OPC_FALSE){
						/* Subqueue contains messages queued for processing.	*/
						/* Flush the subqueue of messages. */
					// 	op_subq_flush (SUBQ_MAN);
					// }
					for(i=32; i<current_free_connected_NID; i++){
						assign_map[i%10].map2_slot_start = 0;
					}
					if(!IAM_BAN_HUB){
						wban_battery_sleep_end(mac_state);
					}else{
						log = fopen(log_name, "a");
						/* collect statistics */
						temp_last_ppdu_kbits = temp_ppdu_kbits;
						lastSF_msdu_nbr = data_rx_nbr;
						temp_ppdu_kbits = 0.0;
						tx_suc_nbr = 0;
						tx_suc_ppdu_kbits = 0;
						data_rx_nbr = 0;
						for(i=0; i<UP_ALL; i++){
							data_rx_nbr += data_stat_local[i][RCV].number;
							temp_ppdu_kbits += data_stat_local[i][RCV].ppdu_kbits;
							tx_suc_nbr += data_stat_all[i][RCV].number;
							tx_suc_ppdu_kbits += data_stat_all[i][RCV].ppdu_kbits;
						}
						th_msdu_kbps = (temp_ppdu_kbits - 0.001*data_rx_nbr*(header4mac2phy()+72))/op_sim_time();
						throughput = (temp_ppdu_kbits - temp_last_ppdu_kbits)/SF.duration;
						th_msdu_sf_kbps = throughput - (0.001*(data_rx_nbr-lastSF_msdu_nbr)*(header4mac2phy()+72))/SF.duration;

						if(SF.rap1_end > 0){
							fprintf(log, "t=%f,NODE_ID=%d,STAT,THROUGHPUT_RAP_MSDU_kbps=%f\n", op_sim_time(), node_id, throughput_rap_msdu_kbits/SF.rap1_length2sec);
							fprintf(log, "t=%f,NODE_ID=%d,STAT,THROUGHPUT_RAP_PPDU_kbps=%f\n", op_sim_time(), node_id, throughput_rap_kbits/SF.rap1_length2sec);
						}
						fprintf(log, "t=%f,NODE_ID=%d,STAT,THROUGHPUT_SF_MSDU_kbps=%f\n", op_sim_time(), node_id, th_msdu_sf_kbps);
						fprintf(log, "t=%f,NODE_ID=%d,STAT,THROUGHPUT_SF_PPDU_kbps=%f\n", op_sim_time(), node_id, throughput);
						fprintf(log, "t=%f,NODE_ID=%d,STAT,THROUGHPUT_RCV_MSDU_kbps=%f\n", op_sim_time(), node_id, th_msdu_kbps);
						fprintf(log, "t=%f,NODE_ID=%d,STAT,THROUGHPUT_RCV_PPDU_kbps=%f\n", op_sim_time(), node_id, temp_ppdu_kbits/op_sim_time());
						fprintf(log, "t=%f,NODE_ID=%d,STAT,THROUGHPUT_TX_SUC_PPDU_kbps=%f\n", op_sim_time(), node_id, tx_suc_ppdu_kbits/op_sim_time());
						fprintf(log, "t=%f,NODE_ID=%d,STAT,THROUGHPUT_RCV_nbr=%f\n", op_sim_time(), node_id, data_rx_nbr);
						fprintf(log, "t=%f,NODE_ID=%d,STAT,THROUGHPUT_TX_SUC_nbr=%f\n", op_sim_time(), node_id, tx_suc_nbr);
						fclose(log);
						throughput_rap_kbits = 0;
						throughput_rap_msdu_kbits = 0;
						// throughput_map_kbits = 0;
						/* value for the next superframe. End Device will obtain this value from beacon */
						wban_send_beacon_frame();
						op_prg_odb_bkpt("debug");
					}
					mac_state = MAC_SETUP;

					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f  -> ++++++++++ START OF BEACON PERIOD ++++++++++ \n\n", op_sim_time());
					// fclose(log);
					// op_prg_odb_bkpt ("beacon_end");
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
							// printf("Node %s map1_start2sec=%f, map1_end2sec=%f.\n", node_attr.name, SF.map1_start2sec, SF.map1_end2sec);
							op_intrpt_schedule_self(max_double(SF.map1_start2sec, op_sim_time()), START_OF_MAP1_PERIOD_CODE);
							op_intrpt_schedule_self(SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
							op_prg_odb_bkpt("map");
							// conn_assign_attr.interval_start = 255;
							// conn_assign_attr.interval_end = 0;
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
					phase_end_timeG = SF.eap1_end2sec;
					SF.IN_MAP_PHASE = OPC_FALSE;
					SF.IN_EAP_PHASE = OPC_TRUE;
					if(pkt_to_be_sent.frame_subtype != 7){
						pkt_to_be_sent.enable = OPC_FALSE;
					}
					attemptingToTX = OPC_FALSE;
				
					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ START OF THE EAP1 ++++++++++ \n\n", op_sim_time(), node_id);
					// printf (" [Node %s] t=%f  -> ++++++++++  START OF THE EAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					// fclose(log);

					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_EAP1_PERIOD_CODE */

				case END_OF_EAP1_PERIOD_CODE: /* end of EAP1 Period */
				{
					mac_state = MAC_SLEEP;
					// pkt_to_be_sent.enable = OPC_FALSE;
					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ END OF THE EAP1 ++++++++++ \n\n", op_sim_time(), node_id);
					// printf (" [Node %s] t=%f  -> ++++++++++  END OF THE EAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					// fclose(log);
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
					attemptingToTX = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_FALSE;

					// stat_vec.ppdu_sent_nbr = 0;
					// stat_vec.ppdu_sent_kbits = 0.0;
					// stat_vec.ppdu_rap_sent_start = stat_vec.ppdu_sent_kbits;
					// stat_vec.ppdu_rap_rcv_start = stat_vec.ppdu_rcv_kbits;
				
					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ START OF THE RAP1 ++++++++++ \n\n", op_sim_time(), node_id);
					// fclose(log);
					// printf (" [Node %s] t=%f  -> ++++++++++  START OF THE RAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					
					// op_prg_odb_bkpt("rap1");

					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_RAP1_PERIOD_CODE */

				case END_OF_RAP1_PERIOD_CODE: /* END of RAP1 Period */
				{
					if(!IAM_BAN_HUB){
						wban_battery_sleep_start(mac_state);
					}
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_TRUE;

					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ END OF THE RAP1 ++++++++++ \n\n", op_sim_time(), node_id);
					// printf (" [Node %s] t=%f  -> ++++++++++  END OF THE RAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					// fclose(log);
					
					// stat_vec.ppdu_rap_sent_end = stat_vec.ppdu_sent_kbits;
					// stat_vec.ppdu_rap_rcv_end = stat_vec.ppdu_rcv_kbits;
					// stat_vec.ppdu_sent_kbits_rap = stat_vec.ppdu_rap_sent_end - stat_vec.ppdu_rap_sent_start;
					// stat_vec.ppdu_rcv_kbits_rap = stat_vec.ppdu_rap_rcv_end - stat_vec.ppdu_rap_rcv_start;
					// stat_vec.traffic_G = stat_vec.ppdu_sent_kbits_rap/(WBAN_DATA_RATE_KBITS*SF.rap1_length2sec);
					// stat_vec.throughput_S = stat_vec.ppdu_rcv_kbits_rap/(WBAN_DATA_RATE_KBITS*SF.rap1_length2sec);
					// log = fopen(log_name, "a");
					// fprintf(log,"STAT,CHANNEL_TRAFFIC_RAP_G=%f\n", stat_vec.traffic_G);
					// fprintf(log,"STAT,CHANNEL_THROUGHPUT_RAP_S=%f\n", stat_vec.throughput_S);
					// fclose(log);
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
					if(!IAM_BAN_HUB){
						wban_battery_sleep_end(mac_state);
					}
					mac_state = MAC_MAP1;
					// mac_state = MAC_SLEEP;
					phase_start_timeG = SF.map1_start2sec;
					phase_end_timeG = SF.map1_end2sec;
					SF.IN_MAP_PHASE = OPC_TRUE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
					attemptingToTX = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_FALSE;
					if(OPC_FALSE == node_attr.is_BANhub){
						map_attr.TX_state = OPC_TRUE;
					}

					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ START OF THE MAP1 ++++++++++ \n\n", op_sim_time(), node_id);
					// fclose(log);
					printf (" [Node %s] t=%f  -> ++++++++++  START OF THE MAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					printf("Node %s Start MAP1 at %f, End MAP1 at %f.\n", node_attr.name, phase_start_timeG, phase_end_timeG);

					op_prg_odb_bkpt("map1_start");
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_MAP1_PERIOD_CODE */

				case END_OF_MAP1_PERIOD_CODE: /* end of MAP1 Period */
				{
					if(!IAM_BAN_HUB){
						wban_battery_sleep_start(mac_state);
					}
					mac_state = MAC_SLEEP;
					// pkt_to_be_sent.enable = OPC_TRUE;
					SF.ENABLE_TX_NEW = OPC_FALSE;
				
					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ END OF THE MAP1 ++++++++++ \n\n", op_sim_time(), node_id);
					// fclose(log);
					printf (" [Node %s] t=%f  -> ++++++++++  END OF THE MAP1 ++++++++++ \n\n", node_attr.name, op_sim_time());
					break;
				};/* end of END_OF_MAP1_PERIOD_CODE */

				case SEND_B2_FRAME: 
				{
					if(!IAM_BAN_HUB){
						wban_battery_sleep_end(mac_state);
					}
					mac_state = MAC_SETUP;
					op_prg_odb_bkpt("send_b2");
					if (IAM_BAN_HUB){
						wban_send_beacon2_frame();
					} else {
						FOUT;
					}
					break;
				}

				case START_OF_MAP2_PERIOD_CODE: /* start of MAP2 Period */
				{
					if(!IAM_BAN_HUB){
						op_prg_odb_bkpt("map2_send");
						wban_battery_sleep_end(mac_state);
					}
					mac_state = MAC_MAP2;
					// mac_state = MAC_SLEEP;
					phase_start_timeG = SF.map2_start2sec;
					phase_end_timeG = SF.map2_end2sec;
					SF.IN_MAP_PHASE = OPC_TRUE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
					attemptingToTX = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_FALSE;
					if(OPC_FALSE == node_attr.is_BANhub){
						map_attr.TX_state = OPC_TRUE;
					}

					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ START OF THE MAP2 ++++++++++ \n\n", phase_start_timeG, node_id);
					// // printf (" [Node %s] t=%f  -> ++++++++++  START OF THE MAP2 ++++++++++ \n\n", node_attr.name, op_sim_time());
					// fclose(log);
					printf("Node %s Start MAP2 at %f, End MAP2 at %f.\n", node_attr.name, phase_start_timeG, phase_end_timeG);
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);				
					break;
				};/* end of START_OF_MAP2_PERIOD_CODE */

				case END_OF_MAP2_PERIOD_CODE: /* end of MAP2 Period */
				{
					if(!IAM_BAN_HUB){
						wban_battery_sleep_start(mac_state);
					}
					mac_state = MAC_SLEEP;
					// pkt_to_be_sent.enable = OPC_TRUE;
					SF.ENABLE_TX_NEW = OPC_FALSE;
				
					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ END OF THE MAP2 ++++++++++ \n\n", op_sim_time(), node_id);
					// printf (" [Node %s] t=%f  -> ++++++++++  END OF THE MAP2 ++++++++++ \n\n", node_attr.name, op_sim_time());
					// fclose(log);
					break;
				};/* end of END_OF_MAP1_PERIOD_CODE */

				case START_OF_CAP_PERIOD_CODE:
				{
					if(!IAM_BAN_HUB){
						wban_battery_sleep_end(mac_state);
					}
					// printf("Node %s enters into CAP Period.\n", node_attr.name);
					mac_state = MAC_CAP;
					phase_start_timeG = SF.cap_start2sec;
					phase_end_timeG = SF.cap_end2sec;
					SF.IN_MAP_PHASE = OPC_FALSE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
					attemptingToTX = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_FALSE;
					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ START OF THE CAP ++++++++++ \n\n", op_sim_time(), node_id);
					// printf (" [Node %s] t=%f  -> ++++++++++  START OF THE CAP ++++++++++ \n\n", node_attr.name, op_sim_time());
					// fclose(log);
					op_intrpt_schedule_self(op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					break;
				}

				case END_OF_CAP_PERIOD_CODE: /* end of CAP Period */
				{
					if(!IAM_BAN_HUB){
						wban_battery_sleep_start(mac_state);
					}
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_TRUE;
				
					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ END OF THE CAP ++++++++++ \n\n", op_sim_time(), node_id);
					// printf (" [Node %s] t=%f  -> ++++++++++  END OF THE CAP ++++++++++ \n\n", node_attr.name, op_sim_time());
					// fclose(log);
					break;
				};/* end of END_OF_CAP_PERIOD_CODE */

				case START_OF_SLEEP_PERIOD: /* Start of Sleep Period */
				{
					mac_state = MAC_SLEEP;
					SF.ENABLE_TX_NEW = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_TRUE;

					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,NODE_ID=%d  -> ++++++++++ START OF SLEEP PERIOD ++++++++++ \n\n", op_sim_time(), node_id);
					// printf (" [Node %s] t=%f  -> ++++++++++  START OF SLEEP PERIOD ++++++++++ \n\n", node_attr.name, op_sim_time());
					// fclose(log);
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
					if(!can_fit_TX(&pkt_to_be_sent)) {
						// attemptingToTX = OPC_FALSE;
						current_packet_CS_fails++;
						break;
					}
					/* do check if the channel is idle at the start of cca */
					/* at the start the channel is assumed idle, any change to busy, the CCA will report a busy channel */
					printf (" [Node %s] t=%f --- START CCA CW = %d, retry_times=%d\n",node_attr.name, op_sim_time(), csma.CW, current_packet_txs+current_packet_CS_fails);
					
					/* check at the beginning of CCA, if the channel is busy */
					csma.CCA_CHANNEL_IDLE = OPC_TRUE;
					if (op_stat_local_read (RX_BUSY_STAT) == 1.0) {
						csma.CCA_CHANNEL_IDLE = OPC_FALSE;
					}
					wban_battery_cca(mac_state);
					op_intrpt_schedule_self (op_sim_time() + pCCATime, CCA_EXPIRATION_CODE);
					break;
				};/*end of CCA_START_CODE */
			
				case CCA_EXPIRATION_CODE :/*At the end of the CCA */
				{
					/* bug with open-zigbee, for statwire interupt can sustain a duration */
					if ((!csma.CCA_CHANNEL_IDLE) || (op_stat_local_read (RX_BUSY_STAT) == 1.0)) {
						printf("t = %f, %s CCA with BUSY.\n", op_sim_time(), node_attr.name);
						// op_intrpt_schedule_self (csma.next_slot_start, CCA_START_CODE);
						op_intrpt_schedule_self (op_sim_time()+pCSMAMACPHYTime+2*pCSMASlotLength2Sec, CCA_START_CODE);
					} else {
						csma.backoff_counter--;
						printf("t = %f, %s CCA with IDLE, backoff_counter decrement to %d\n", op_sim_time(), node_attr.name, csma.backoff_counter);

						if (csma.backoff_counter > 0) {
							printf("CCA at next available backoff boundary = %f sec.\n", op_sim_time()+pCSMAMACPHYTime);
							// op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
							op_intrpt_schedule_self (op_sim_time()+pCSMAMACPHYTime, CCA_START_CODE);
						} else {
							if(csma.backoff_counter < 0){
								if(waitForACK) printf("waitForACK=True,");
								if(attemptingToTX) printf("attemptingToTX=True,");
								if(TX_ING) printf("TX_ING=True\n");
								// break;
								// csma.backoff_counter = 0;
								op_sim_end("ERROR : TRY TO SEND Packet WHILE backoff_counter < 0","PK_SEND_CODE","","");
							}
							printf("backoff_counter decrement to 0, %s start transmission at %f.\n", node_attr.name, op_sim_time()+pCSMAMACPHYTime);
							// op_intrpt_schedule_self (csma.next_slot_start, START_TRANSMISSION_CODE);
							op_prg_odb_bkpt("send_packet");
							// op_intrpt_schedule_self (wban_backoff_period_boundary_get(), START_TRANSMISSION_CODE);
							op_intrpt_schedule_self (op_sim_time()+pCSMAMACPHYTime, START_TRANSMISSION_CODE);
							// wban_backoff_delay_set(pkt_to_be_sent.user_priority);
						}
					}
					break;
				};
			
				case START_TRANSMISSION_CODE: /* successful end of backoff and CCA or MAP period */
				{	
					/*backoff_start_time is initialized in the "init_backoff" state*/
					// op_stat_write(statistic_vector.mac_delay, op_sim_time()-backoff_start_time);
					// op_stat_write(statistic_global_vector.mac_delay, op_sim_time()-backoff_start_time);
					if(can_fit_TX(&pkt_to_be_sent)){
						wban_send_mac_pk_to_phy(frame_MPDU_to_be_sent);
					}
					break;
				}; /*end of START_TRANSMISSION_CODE */

				case WAITING_ACK_END_CODE:	/* the timer for waiting an ACK has expired, the packet must be retransmitted */
				{
					waitForACK = OPC_FALSE;
					printf("\nNode %s wait for ACK END at %f.\n", node_attr.name, op_sim_time());
					printf("t=%f, phase_end_timeG=%f\n", op_sim_time(), phase_end_timeG);
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
						// op_stat_write(stat_vec.data_pkt_fail, 1.0);
						// op_stat_write(stat_vec.data_pkt_suc1, 0.0);
						// op_stat_write(stat_vec.data_pkt_suc2, 0.0);

						// op_stat_write(stat_vecG.data_pkt_fail, 1.0);
						// op_stat_write(stat_vecG.data_pkt_suc1, 0.0);
						// op_stat_write(stat_vecG.data_pkt_suc2, 0.0);
						log = fopen(log_name, "a");
						fprintf(log, "t=%f,NODE_ID=%d,MAC_STATE=%d,TX_FAIL,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), node_id, mac_state, mac_attr.sender_id, mac_attr.recipient_id);
						fprintf(log, "TX_TIME=%d,OUT_BOUND=%d,", current_packet_txs, current_packet_CS_fails);
						fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", pkt_to_be_sent.frame_type, pkt_to_be_sent.frame_subtype, pkt_to_be_sent.ppdu_bits);
						fclose(log);
						printf("Packet transmission exceeds max packet tries at time %f\n", op_sim_time());
						// remove MAC frame (MPDU) frame_MPDU_to_be_sent
						// op_pk_destroy(frame_MPDU_to_be_sent);
						pkt_to_be_sent.enable = OPC_FALSE;
						waitForACK = OPC_FALSE;
						TX_ING = OPC_FALSE;
						attemptingToTX = OPC_FALSE;
						current_packet_txs = 0;
						current_packet_CS_fails = 0;
					} else {
						TX_ING = OPC_FALSE;
						// attemptingToTX = OPC_FALSE;
					}
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
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
					// op_prg_odb_bkpt("send_conn_req");
					// printf("t = %f, Node %s start sending connection request frame to Hub.\n", op_sim_time(), node_attr.name);
					// current_packet_txs = 0;
					// current_packet_CS_fails = 0;
					// wban_send_mac_pk_to_phy(CONN_REQ_MPDU);
					// wban_send_connection_request_frame();
					wban_send_conn_req_frame();
					op_intrpt_schedule_self(op_sim_time(),TRY_PACKET_TRANSMISSION_CODE);
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
				{
					if((MANAGEMENT == pkt_to_be_sent.frame_type) && (BEACON == pkt_to_be_sent.frame_subtype)){
						wban_schedule_next_beacon(); //update the superframe
					}
					TX_ING = OPC_FALSE;
					// attemptingToTX = OPC_FALSE;
					waitForACK = OPC_FALSE;
					pkt_to_be_sent.enable = OPC_FALSE;
					current_packet_txs = 0;
					current_packet_CS_fails = 0;
					op_intrpt_schedule_self(op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					break;
				};

				default:
				{
				};	
				
			} /*end of switch (op_intrpt_code())*/ 
			
			break;
		};/*end of OPC_INTRPT_SELF */
		
		case OPC_INTRPT_ENDSIM:
		{
			// if(!IAM_BAN_HUB){
			subq_data_info_get();
			log = fopen(log_name, "a");
			for(i=0; i<UP_ALL; i++){
				if(IAM_BAN_HUB){
					fprintf(log, "t=%f,NODE_ID=%d,STAT,DATA_LATENCY,", op_sim_time(), node_id);
					fprintf(log, "UP=%d,LATENCY_AVG=%f\n", i, latency_avg[i]);
				}
				for(j=0; j<DATA_STATE; j++){
					fprintf(log, "t=%f,NODE_ID=%d,STAT,DATA,", op_sim_time(), node_id);
					if(IAM_BAN_HUB){
						fprintf(log, "UP=%d,STATE=%d,NUMBER=%f,PPDU_KBITS=%f\n", i,j,data_stat_all[i][j].number,data_stat_all[i][j].ppdu_kbits);
					}else{
						fprintf(log, "UP=%d,STATE=%d,NUMBER=%f,PPDU_KBITS=%f\n", i,j,data_stat_local[i][j].number,data_stat_local[i][j].ppdu_kbits);
					}
				}
			}
			// }
			fclose(log);
			
			// op_prg_odb_bkpt("debug_end");
			
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
					
						// fprintf(log,"t=%f  -> $$$$$ COLLISION $$$$$$$  \n\n",op_sim_time());
						printf(" [Node %s] t=%f  -> COLLISION OCCUR \n",node_attr.name, op_sim_time());
					
					
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
	// double ete_delay;
	// int pk_size;
	int slotnum;
	int up_prio;
	int sender_id;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_data_frame);
	op_pk_nfd_get (frame_MPDU, "Ack Policy", &ack_policy);
	op_pk_nfd_get (frame_MPDU, "Sender ID", &sender_id);
	op_pk_nfd_get (frame_MPDU, "Frame Subtype", &up_prio);
	op_pk_nfd_get (frame_MPDU, "slotnum", &slotnum);
	op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
	if(1 == node_attr.protocol_ver){
		if(MAC_MAP1 == mac_state) {
			if(assign_map[mac_attr.recipient_id%10].slot_start > 0){
				if(assign_map[mac_attr.recipient_id%10].slot_end == SF.current_slot){
					assign_map[mac_attr.recipient_id%10].slotnum = slotnum;
				}
			}
			printf("NID=%d,slotnum=%d\n", mac_attr.recipient_id, slotnum);
			// op_prg_odb_bkpt("debug");
		}
	}
	op_prg_odb_bkpt("rcv_data");

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
	// int retry_times;
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
			
			/* disable the invocation of only the next interrupt of WAITING_ACK_END_CODE */
			op_intrpt_disable (OPC_INTRPT_SELF, WAITING_ACK_END_CODE, OPC_TRUE);
			printf(" [Node %s] t=%f  -> ACK Frame Reception [Requested SEQ = %d]\n\n", node_attr.name, op_sim_time(), seq_num);

			log = fopen(log_name, "a");
			fprintf(log, "t=%f,NODE_ID=%d,MAC_STATE=%d,TX_SUCC,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), node_id, mac_state, mac_attr.sender_id, mac_attr.recipient_id);
			fprintf(log, "TX_TIME=%d,OUT_BOUND=%d,", current_packet_txs, current_packet_CS_fails);
			fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n",pkt_to_be_sent.frame_type,pkt_to_be_sent.frame_subtype,pkt_to_be_sent.ppdu_bits);
			fclose(log);
			//collect statistics
			if(DATA == pkt_to_be_sent.frame_type){
			// 	retry_times = current_packet_txs + current_packet_CS_fails;
			// 	log = fopen(log_name, "a");
			// 	fprintf(log,"STAT,MAC_STATE=%d,PKT_TX_RETRY=%d,t=%f,DATA_PPDU_BITS=%d,UPx=%d\n",mac_state,retry_times,op_sim_time(),pkt_to_be_sent.ppdu_bits,pkt_to_be_sent.frame_subtype);
			// 	fclose(log);
				// fprintf(log, "RX,MAC_STATE=%d,RAP_Length=%d,", mac_state, beacon_attr.rap1_length);
				// fprintf(log, "t=%f,DATA_PPDU=%d,UPx=%d,", op_sim_time(), pk_size, up_prio);
				// fprintf(log, "ETE_DELAY=%f\n", ete_delay);
				// stat_vec.ppdu_rcv_nbr = stat_vec.ppdu_rcv_nbr + 1;
				// stat_vec.ppdu_rcv_kbits = stat_vec.ppdu_rcv_kbits + 1.0*pkt_to_be_sent.ppdu_bits/1000.0;
				data_stat_local[pkt_to_be_sent.frame_subtype][RCV].number += 1;
				data_stat_local[pkt_to_be_sent.frame_subtype][RCV].ppdu_kbits += 0.001*pkt_to_be_sent.ppdu_bits;
				data_stat_all[pkt_to_be_sent.frame_subtype][RCV].number += 1;
				data_stat_all[pkt_to_be_sent.frame_subtype][RCV].ppdu_kbits += 0.001*pkt_to_be_sent.ppdu_bits;
			}
			waitForACK = OPC_FALSE;
			TX_ING = OPC_FALSE;
			// attemptingToTX = OPC_FALSE;
			pkt_to_be_sent.enable = OPC_FALSE;
			current_packet_txs = 0;
			current_packet_CS_fails = 0;
			// op_stat_write(stat_vec.data_pkt_fail, 0.0);
			// if(1 == current_packet_txs + current_packet_CS_fails){
			// 	op_stat_write(stat_vec.data_pkt_suc1, 1.0);
			// } else {
			// 	op_stat_write(stat_vec.data_pkt_suc2, 1.0);
			// }
			
			// op_stat_write(stat_vecG.data_pkt_fail, 0.0);
			// if(1 == current_packet_txs + current_packet_CS_fails){
			// 	op_stat_write(stat_vecG.data_pkt_suc1, 1.0);
			// } else {
			// 	op_stat_write(stat_vecG.data_pkt_suc2, 1.0);
			// }

			/* Try to send another packet after pSIFS */
			op_intrpt_schedule_self (op_sim_time() + pSIFS, TRY_PACKET_TRANSMISSION_CODE);
		} else	{	/* No, This is not my ACK, I'm Still Waiting */
			printf(" [Node %s] t=%f  -> WRONG ACK Frame Reception [RCV = %d], Still Waiting ACK [RQST = %d] \n\n", node_attr.name, op_sim_time(), seq_num , mac_attr.wait_ack_seq_num );
		}
	} else {/* if (mac_attributes.wait_ack == OPC_FALSE) */ 
		printf (" [Node %s] t=%f  -> I'm not Waiting ACK Frame - ACK Frame Destroyed. \n\n", node_attr.name, op_sim_time());
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
	int slotnum;
	/* Stack tracing enrty point */
	FIN(wban_attempt_TX);

	if(waitForACK || attemptingToTX || TX_ING){
		if(waitForACK) printf("waitForACK=True,");
		if(attemptingToTX) printf("attemptingToTX=True,");
		if(TX_ING) printf("TX_ING=True,");
		printf("t=%f, %s A packet is TX.\n", op_sim_time(), node_attr.name);
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

	if (op_stat_local_read(TX_BUSY_STAT) == 1.0) {
		printf("t=%f, %s A packet is TX.\n", op_sim_time(), node_attr.name);
		FOUT;
	}

	if((pkt_to_be_sent.enable) && (current_packet_txs + current_packet_CS_fails < max_packet_tries)){
		printf("%s retransmittion with retry_times=%d\n", node_attr.name, current_packet_txs + current_packet_CS_fails);
		// current_packet_txs++;
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
	// current_packet_txs = 0;
	// current_packet_CS_fails = 0;
	// Try to draw a new packet from the data or Management buffers.
	if (!op_subq_empty(SUBQ_MAN)) {
		// if((op_sim_time() > SF.rap1_end2sec) && (!IAM_BAN_HUB)){
		// 	printf("%s cannot send MANAGEMENT frame current for over RAP1.\n", node_attr.name);
		// 	op_subq_flush (SUBQ_MAN);
		// 	FOUT;
		// }
		frame_MPDU_to_be_sent = op_subq_pk_remove(SUBQ_MAN, OPC_QPOS_PRIO);
		pkt_to_be_sent.enable = OPC_TRUE;
		pkt_to_be_sent.user_priority = 6; //set up of managemant frame with 6
		printf("Management frame TX.\n");
	} else if ((mac_attr.sender_id != UNCONNECTED_NID) && (!op_subq_empty(SUBQ_DATA))) {
		/* obtain the pointer to MAC frame (MPDU) stored in the adequate queue */
		frame_MPDU_to_be_sent = op_subq_pk_access (SUBQ_DATA, OPC_QPOS_PRIO);
		// printf("PK_ACCESS, pk size of frame_MPDU=%d\n", op_pk_total_size_get(frame_MPDU_to_be_sent));
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
		if((1 == node_attr.protocol_ver) && (MAC_MAP1 == mac_state)){
			subq_info_get(SUBQ_DATA);
			slotnum = (int)((subq_info.bitsize + subq_info.pksize *2* header4mac2phy())/(node_attr.data_rate*1000.0*SF.slot_length2sec));
			// if(slotnum > 4){
			// 	slotnum = 4;
			// }
			op_pk_nfd_set (frame_MPDU_to_be_sent, "slotnum", slotnum);
			printf("NID=%d Required slotnum=%d\n", mac_attr.sender_id, slotnum);
			op_prg_odb_bkpt("debug");
		}
		pkt_to_be_sent.enable = OPC_TRUE;
	} else {
		pkt_to_be_sent.enable = OPC_FALSE;
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
		current_packet_CS_fails = 0;
		current_packet_txs = 0;
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
	printf(" [Node %s] t=%f  - Attempt TX CSMA  \n", node_attr.name, op_sim_time());
	
	if(!can_fit_TX(&pkt_to_be_sent)){
		FOUT;
	}

	wban_backoff_delay_set(pkt_to_be_sent.user_priority);

	// if(!can_fit_TX(&pkt_to_be_sent)) {
	// 	attemptingToTX = OPC_FALSE;
	// 	current_packet_CS_fails++;
	// 	csma.backoff_counter_lock = OPC_TRUE;

	// 	FOUT;
	// } else {
	// 	csma.backoff_counter_lock = OPC_FALSE;
	// }
	// if(attemptingToTX){
	// 	printf("%s has pkt being TX.\n", node_attr.name);
	// 	FOUT;
	// }
	attemptingToTX = OPC_TRUE;
	//CCA
	// op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
	op_intrpt_schedule_self (op_sim_time(), CCA_START_CODE);
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
	// csma.backoff_expiration_time = wban_backoff_period_boundary_get() + csma.backoff_time;
	csma.backoff_expiration_time = op_sim_time() + csma.backoff_time;
	
	// phase_remaining_time = phase_end_timeG - wban_backoff_period_boundary_get();
	phase_remaining_time = phase_end_timeG - op_sim_time();
	// op_intrpt_schedule_self (csma.backoff_expiration_time, BACKOFF_EXPIRATION_CODE);

	
	// printf ("-------------------------- BACKOFF -----------------------------------\n");
	// printf (" [Node %s] ENTERS BACKOFF STATUS AT %f\n", node_attr.name, op_sim_time());
	// printf ("  Beacon Boundary = %f\n", SF.BI_Boundary);
	// printf ("  CW = %d\n", csma.CW);
	// printf ("  Random Backoff counter = %d\n", csma.backoff_counter);
	// printf ("    + Random Backoff time  = %f sec \n", csma.backoff_time);
	// printf ("    + Phase Remaining Length = %f sec \n", phase_remaining_time);
	// printf ("  Current Time Slot = %d\n", SF.current_slot);
	// printf ("  Backoff Boundary = %f sec \n", wban_backoff_period_boundary_get());
//       printf ("  Phase Start Time     = %f sec \n", phase_start_timeG);
	// printf ("  Phase End Time     = %f sec \n", phase_end_timeG);
	// printf ("  Difference       = %f sec \n", phase_end_timeG- wban_backoff_period_boundary_get());
	// printf ("  BackOff Expiration Time  = %f sec\n", csma.backoff_expiration_time);
	// printf ("----------------------------------------------------------------------------\n\n");

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
		printf("%s has NO packet being TX.\n", node_attr.name);
		FRET(OPC_FALSE);
	}
	pk_tx_time = TX_TIME(wban_norm_phy_bits(frame_MPDU_to_be_sent), node_attr.data_rate);
	phase_remaining_time = phase_end_timeG - op_sim_time();
	if(pkt_to_be_sentL->ack_policy != N_ACK_POLICY){
		if (compare_doubles(phase_remaining_time, pk_tx_time+I_ACK_TX_TIME+pSIFS+3*MICRO) >=0) {
			FRET(OPC_TRUE);
		} else {
			printf("%s No enougth time for I_ACK_POLICY packet transmission in this phase.\n", node_attr.name);
			FRET(OPC_FALSE);
		}
	} else {
		if (compare_doubles(phase_remaining_time, pk_tx_time+3*MICRO) >=0) {
			FRET(OPC_TRUE);
		} else {
			printf("%s No enougth time for N_ACK_POLICY packet transmission in this phase.\n", node_attr.name);
			FRET(OPC_FALSE);
		}
	}
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
	// we are starting to TX, so we are exiting the attemptingToTX (sub)state. bug with OPNET
	attemptingToTX = OPC_FALSE;
	TX_ING = OPC_TRUE;

	op_pk_nfd_get(frame_MPDU, "Sequence Number", &seq_num);
	op_pk_nfd_get(frame_MPDU, "Ack Policy", &ack_policy);
	op_pk_nfd_get(frame_MPDU, "Frame Type", &frame_type);
	op_pk_nfd_get(frame_MPDU, "Frame Subtype", &frame_subtype);
	pkt_to_be_sent.ack_policy = ack_policy;
	pkt_to_be_sent.frame_type = frame_type;
	pkt_to_be_sent.frame_subtype = frame_subtype;
	ppdu_bits = wban_norm_phy_bits(frame_MPDU);
	pkt_to_be_sent.ppdu_bits = ppdu_bits;
	printf("MAC_TO_PHY, pk size of frame_PPDU=%d\n", ppdu_bits);
	op_prg_odb_bkpt("mpdu_size");

	log = fopen(log_name, "a");
	fprintf(log, "t=%f,NODE_ID=%d,MAC_STATE=%d,TX,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), node_id, mac_state, mac_attr.sender_id, mac_attr.recipient_id);
	fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", frame_type, frame_subtype, ppdu_bits);
	fclose(log);
	switch (frame_type) {
		case DATA:
		{
			data_stat_local[frame_subtype][SENT].number += 1;
			data_stat_local[frame_subtype][SENT].ppdu_kbits += 0.001*ppdu_bits;
			data_stat_all[frame_subtype][SENT].number += 1;
			data_stat_all[frame_subtype][SENT].ppdu_kbits += 0.001*ppdu_bits;

			// stat_vec.ppdu_sent_kbits = stat_vec.ppdu_sent_kbits + 1.0*ppdu_bits/1000.0; // in kbits
			// stat_vec.ppdu_sent_nbr = stat_vec.ppdu_sent_nbr + 1;
			// op_stat_write(stat_vec.data_pkt_sent, ppdu_bits);
			// op_stat_write(stat_vecG.data_pkt_sent, ppdu_bits);
			switch(frame_subtype){
				case UP0:
					// op_stat_write(stat_vec.up0_sent, ppdu_bits);
					// op_stat_write(stat_vecG.up0_sent, ppdu_bits);
					break;
				case UP1:
					// op_stat_write(stat_vec.up1_sent, ppdu_bits);
					// op_stat_write(stat_vecG.up1_sent, ppdu_bits);
					break;
				case UP2:
					// op_stat_write(stat_vec.up2_sent, ppdu_bits);
					// op_stat_write(stat_vecG.up2_sent, ppdu_bits);
					break;
				case UP3:
					// op_stat_write(stat_vec.up3_sent, ppdu_bits);
					// op_stat_write(stat_vecG.up3_sent, ppdu_bits);
					break;
				case UP4:
					// op_stat_write(stat_vec.up4_sent, ppdu_bits);
					// op_stat_write(stat_vecG.up4_sent, ppdu_bits);
					break;
				case UP5:
					// op_stat_write(stat_vec.up5_sent, ppdu_bits);
					// op_stat_write(stat_vecG.up5_sent, ppdu_bits);
					break;
				case UP6:
					// op_stat_write(stat_vec.up6_sent, ppdu_bits);
					// op_stat_write(stat_vecG.up6_sent, ppdu_bits);
					break;
				case UP7:
					// op_stat_write(stat_vec.up7_sent, ppdu_bits);
					// op_stat_write(stat_vecG.up7_sent, ppdu_bits);
					break;
				default:
					break;
			}
			break;
		}
		case MANAGEMENT:
			switch(frame_subtype){
				case BEACON:
					// op_stat_write (beacon_frame_hndl, 1.0);
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

	PPDU_tx_time = TX_TIME(ppdu_bits, node_attr.data_rate);

	switch (ack_policy){
		case N_ACK_POLICY:
			waitForACK = OPC_FALSE;
			phy_to_radio(frame_MPDU);
			// op_intrpt_schedule_self (op_sim_time() + PPDU_tx_time + pSIFS, N_ACK_PACKET_SENT);
			op_intrpt_schedule_self (op_sim_time() + PPDU_tx_time + pSIFS, N_ACK_PACKET_SENT);
			break;
		case I_ACK_POLICY:
			ack_expire_time = op_sim_time() + PPDU_tx_time + I_ACK_TX_TIME + 2*pSIFS;
			if(ack_expire_time > phase_end_timeG){
				ack_expire_time = op_sim_time() + PPDU_tx_time + I_ACK_TX_TIME + 3*MICRO;
			}
			waitForACK = OPC_TRUE;
			mac_attr.wait_ack_seq_num = seq_num;
			current_packet_txs++;

			pk_create_time = op_pk_creation_time_get(frame_MPDU);
			MPDU_copy = op_pk_copy(frame_MPDU);
			op_pk_creation_time_set(MPDU_copy, pk_create_time);
			phy_to_radio(MPDU_copy);
			
			// fprintf(log,"t=%f   ----------- START TX [DEST_ID = %d, SEQ = %d, with ACK expiring at %f] %d retries  \n\n", op_sim_time(), pkt_to_be_sent.recipient_id, mac_attr.wait_ack_seq_num, ack_expire_time,current_packet_txs+current_packet_CS_fails);
			printf(" [Node %s] t=%f  ----- START TX with ACK expiring at %f, %d retries \n\n", node_attr.name, op_sim_time(), ack_expire_time, current_packet_txs+current_packet_CS_fails);
			
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

	wban_battery_update_tx ((int)(op_pk_total_size_get(frame_PPDU)), mac_state);
	if (op_stat_local_read(TX_BUSY_STAT) == 1.0){
		// op_intrpt_schedule_self(op_sim_time()+pSIFS, TRY_PACKET_TRANSMISSION_CODE);
		// FOUT;
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
 * Input : 	PPDU BITS and MAC STATE
 *--------------------------------------------------------------------------------*/
static void wban_battery_update_tx(int ppdu_bitsL, int mac_stateL) {
	Ici * iciptr;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_update_tx);
	
	iciptr = op_ici_create ("wban_battery_ici_format");
	op_ici_attr_set (iciptr, "PPDU BITS", ppdu_bitsL);
	op_ici_attr_set (iciptr, "MAC STATE", mac_stateL);
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
 * Input : 	PPDU BITS and MAC STATE
 *--------------------------------------------------------------------------------*/
static void wban_battery_update_rx(int ppdu_bitsL, int mac_stateL) {
	Ici * iciptr;

	/* Stack tracing enrty point */
	FIN(wban_battery_update_rx);
	
	iciptr = op_ici_create ("wban_battery_ici_format");
	op_ici_attr_set (iciptr, "PPDU BITS", ppdu_bitsL);
	op_ici_attr_set (iciptr, "MAC STATE", mac_stateL);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), PACKET_RX_CODE, node_attr.my_battery);
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
static void wban_battery_cca(int mac_stateL) {
	Ici * iciptr;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_cca);

	iciptr = op_ici_create ("wban_battery_ici_format");
	op_ici_attr_set (iciptr, "MAC STATE", mac_stateL);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), CCA_CODE, node_attr.my_battery);
	op_ici_install (OPC_NIL);
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_battery_sleep_start
 *
 * Description:	send information about the operation to do to the battery module 
 *
 * Input : 
 *--------------------------------------------------------------------------------*/
static void wban_battery_sleep_start(int mac_stateL) {
	Ici * iciptr;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_sleep_start);

	iciptr = op_ici_create ("wban_battery_ici_format");
	op_ici_attr_set (iciptr, "MAC STATE", mac_stateL);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), START_OF_SLEEP_PERIOD_CODE, node_attr.my_battery);
	op_ici_install (OPC_NIL);
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_battery_sleep_end
 *
 * Description:	send information about the operation to do to the battery module 
 *
 * Input : 
 *--------------------------------------------------------------------------------*/
static void wban_battery_sleep_end(int mac_stateL) {
	Ici * iciptr;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_sleep_end);

	iciptr = op_ici_create ("wban_battery_ici_format");
	op_ici_attr_set (iciptr, "MAC STATE", mac_stateL);
	op_ici_install (iciptr);
	op_intrpt_schedule_remote (op_sim_time(), END_OF_SLEEP_PERIOD_CODE, node_attr.my_battery);
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
			printf(" [Node %s] t=%f  -> Subqueue #%d is empty, wait for MAC frames \n\t -> capacity [%#e frames, %#e bits]. \n\n", node_attr.name, op_sim_time(), i, pk_capacity, bit_capacity);
		} else {
			printf(" [Node %s] t=%f  -> Subqueue #%d is non empty,\n\t -> occupied space [%#e frames, %#e bits] - empty space [%#e frames, %#e bits] \n\n", node_attr.name, op_sim_time(), i, op_subq_stat (i, OPC_QSTAT_PKSIZE), op_subq_stat (i, OPC_QSTAT_BITSIZE), op_subq_stat (i, OPC_QSTAT_FREE_PKSIZE), op_subq_stat (i, OPC_QSTAT_FREE_BITSIZE));
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
		printf(" [Node %s] t=%f  -> Subqueue #%d is empty, wait for MAC frames \n\t -> capacity [%#e frames, %#e bits]. \n\n", node_attr.name, op_sim_time(), subq_index, pk_capacity, bit_capacity);
		
		subq_info.pksize = 0;
		subq_info.bitsize = 0;
	} else {
		subq_info.pksize = op_subq_stat (subq_index, OPC_QSTAT_PKSIZE);
		subq_info.bitsize = op_subq_stat (subq_index, OPC_QSTAT_BITSIZE);
		subq_info.free_pksize = op_subq_stat (subq_index, OPC_QSTAT_FREE_PKSIZE);
		subq_info.free_bitsize = op_subq_stat (subq_index, OPC_QSTAT_FREE_BITSIZE);
		printf(" [Node %s] t=%f  -> Subqueue #%d is non empty,\n\t -> occupied space [%#e frames, %#e bits] - empty space [%#e frames, %#e bits] \n\n", node_attr.name, op_sim_time(), subq_index, subq_info.pksize, subq_info.bitsize, subq_info.free_pksize, subq_info.free_bitsize);
		
		pk_test_up = op_subq_pk_access(subq_index, OPC_QPOS_PRIO);
		subq_info.up = op_pk_priority_get(pk_test_up);
		printf("subq_info.up=UP%d\n", subq_info.up);
	}

	/* Stack tracing exit point */
	FOUT;
}

/*-----------------------------------------------------------------------------
 * Function:	subq_data_info_get
 *
 * Description:	get the parameter of data subqueue
 *
 * Input : subq_index
 *-----------------------------------------------------------------------------*/
static void subq_data_info_get () {
	int i;
	int pksize;
	int up;
	Packet * pk_stat_info;

	/* Stack tracing enrty point */
	FIN(subq_data_info_get);
	
	if (!op_subq_empty(SUBQ_DATA)) {
		pksize = (int)(op_subq_stat(SUBQ_DATA, OPC_QSTAT_PKSIZE));
		for(i=0; i<pksize; i++){
			pk_stat_info = op_subq_pk_remove(SUBQ_DATA, OPC_QPOS_PRIO);
			up = op_pk_priority_get(pk_stat_info);
			data_stat_local[up][SUBQ].number += 1;
			data_stat_local[up][SUBQ].ppdu_kbits += 0.001*wban_norm_phy_bits(pk_stat_info);
			data_stat_all[up][SUBQ].number += 1;
			data_stat_all[up][SUBQ].ppdu_kbits += 0.001*wban_norm_phy_bits(pk_stat_info);
		}
	}

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	header4mac2phy()
 *
 * Description:	header bits add for mac to phy
 *             
 *--------------------------------------------------------------------------------*/
static int header4mac2phy() {
	return (int)ceil(LOG_M*BCH_CODE*(N_preamble + S_header*N_header));
}