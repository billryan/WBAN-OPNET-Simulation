/*--------------------------------------------------------------------------------
 * Function:	wban_mac_init
 *
 * Description:	- initialize the process
 *				- read the attributes and set the state variables
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void
wban_mac_init ()
	{
	Objid beacon_attr_comp_id;
	Objid beacon_attr_id;
	Objid conn_req_attr_comp_id;
	Objid conn_req_attr_id;
	Objid mac_attr_comp_id;
	Objid mac_attr_id;
	Objid appid, srcid, src_comp_id;
	int i, j;
	/* APP Parameters */
	// char str_int[2] = "", up_i_para[80] = "";
	char up_msdu_inter[50] = "", up_msdu_size[50] = "";
	double up_start_t = -1, up_stop_t = -1;
	// double t_send_beacon;

	/* Stack tracing enrty point */
	FIN(wban_mac_init);
	
	/* obtain self object ID of the surrounding processor or queue */
	mac_attr.objid = op_id_self ();
	/* obtain object ID of the parent object (node) */
	nodeid = op_topo_parent (mac_attr.objid);
	nd_attrG[nodeid].objid = nodeid;
	/* get the name of the node */
	op_ima_obj_attr_get (nodeid, "name", &nd_attrG[nodeid].name);
	/* get the geographic position of the node */
	// op_ima_obj_attr_get (nodeid, "x position", &nd_attrG[nodeid].x);
	// op_ima_obj_attr_get (nodeid, "y position", &nd_attrG[nodeid].y);
	// op_ima_obj_attr_get (nodeid, "altitude", &nd_attrG[nodeid].altitude);
	op_ima_obj_attr_get (nodeid, "WBAN DATA RATE", &nd_attrG[nodeid].data_rate);
	/* get the value of the BAN ID */
	op_ima_obj_attr_get (nodeid, "BAN ID", &nd_attrG[nodeid].bid);
	mac_attr.bid = nd_attrG[nodeid].bid;
	/* get the Sender Address of the node */
	op_ima_obj_attr_get (nodeid, "Sender Address", &nd_attrG[nodeid].send_addr);
	/* Sender Address is not specified - Auto Assigned(-2) from node objid */
	if (nd_attrG[nodeid].send_addr == AUTO_ASSIGNED_NID) {
		nd_attrG[nodeid].send_addr = nodeid;
	}
	/* get the value of protocol version */
	op_ima_obj_attr_get (nodeid, "Protocol Version", &nd_attrG[nodeid].protocol_ver);
	/* obtain object ID of the Traffic Source node (APP Layer) */
	appid = op_id_from_name(nodeid, OPC_OBJTYPE_PROC, "Traffic Source_UP");
	/* obtain destination ID for data transmission */
	op_ima_obj_attr_get (appid, "Destination ID", &nd_attrG[nodeid].dest_id);
	/* get the MAC settings */
	op_ima_obj_attr_get (mac_attr.objid, "MAC Attributes", &mac_attr_id);
	mac_attr_comp_id = op_topo_child (mac_attr_id, OPC_OBJTYPE_GENERIC, 0);
	op_ima_obj_attr_get (mac_attr_comp_id, "Max Packet Tries", &max_packet_tries);
	op_ima_obj_attr_get (mac_attr_comp_id, "MGMT Buffer Size", &mac_attr.MGMT_buffer_size);
	op_ima_obj_attr_get (mac_attr_comp_id, "DATA Buffer Size", &mac_attr.DATA_buffer_size);
	mac_attr.wait_for_ack = OPC_FALSE;

	/* get the battery attributes */
	wban_battery_init();
	wban_log_file_init();
	/* get the value to check if this node is Hub or not */
	op_ima_obj_attr_get (nodeid, "Device Mode", &nd_attrG[nodeid].dev_mode);

	mac_state = MAC_SETUP;
	init_flag = OPC_TRUE;
	if (IAM_BAN_HUB) {
		nd_attrG[nodeid].nid = nd_attrG[nodeid].bid + 15; // set the value of HID=BAN ID + 15
		mac_attr.sendid = nd_attrG[nodeid].bid + 15;
		mac_attr.rcvid = BROADCAST_NID; // default value, usually overwritten
		map1_sche_map[nodeid].bid = mac_attr.bid;
		map1_sche_map[nodeid].nid = mac_attr.bid;
		/* get the beacon attributes for the Hub */
		beacon_attr.send_addr = nd_attrG[nodeid].send_addr;
		op_ima_obj_attr_get (nodeid, "Beacon", &beacon_attr_id);
		beacon_attr_comp_id = op_topo_child (beacon_attr_id, OPC_OBJTYPE_GENERIC, 0);
		op_ima_obj_attr_get (beacon_attr_comp_id, "Beacon Period Length", &beacon_attr.beacon_period_length);
		op_ima_obj_attr_get (beacon_attr_comp_id, "Allocation Slot Length", &beacon_attr.allocation_slot_length);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP1 Start", &beacon_attr.rap1_start);
		// op_ima_obj_attr_get (beacon_attr_comp_id, "RAP1 Length", &beacon_attr.rap1_length);
		op_ima_obj_attr_get (beacon_attr_comp_id, "RAP1 End", &beacon_attr.rap1_end);
		op_ima_obj_attr_get (beacon_attr_comp_id, "Inactive Duration", &beacon_attr.inactive_duration);
		/* update rap1_end with rap1_start + rap1_length */
		// beacon_attr.rap1_end = beacon_attr.rap1_start + beacon_attr.rap1_length - 1;
		beacon_attr.rap1_length = beacon_attr.rap1_end - beacon_attr.rap1_start + 1;
		// t_send_beacon = op_sim_time() + floor (op_dist_uniform(12)) * hp_tx_time(BEACON_PPDU_BITS);
		// op_intrpt_schedule_self(t_send_beacon, BEACON_INTERVAL_CODE);
		wban_send_beacon_frame ();
	} else { /* if the node is not Hub */
		/* get the Connection Request for the Node */
		op_ima_obj_attr_get (nodeid, "Connection Request", &conn_req_attr_id);
		conn_req_attr_comp_id = op_topo_child (conn_req_attr_id, OPC_OBJTYPE_GENERIC, 0);
		op_ima_obj_attr_get (conn_req_attr_comp_id, "Allocation Length", &conn_req_attr.allocation_length);
		map1_sche_map[nodeid].slotnum = conn_req_attr.allocation_length;
		map1_sche_map[nodeid].bid = mac_attr.bid;
		/* start assigning connected NID from ID 32 */
		nd_attrG[nodeid].nid = 32 + ((current_free_connected_NID - 32) % 214);
		mac_attr.sendid = nd_attrG[nodeid].nid;
		mac_attr.rcvid = UNCONNECTED;
		current_free_connected_NID++;
		map1_sche_map[nodeid].nid = mac_attr.sendid;
	}
	/* obtain the APP Layer Parameters */
	for (i = 0; i < UP_ALL; ++i) {
		char str_int[2] = "", up_i_para[80] = "";
		sprintf(str_int, "%d", i); /* convert int to str */
		strcat (up_i_para, "User Priority ");
		strcat (up_i_para, str_int);
		strcat (up_i_para, " Traffic Parameters");
		op_ima_obj_attr_get (appid, up_i_para, &srcid); 
		src_comp_id = op_topo_child (srcid, OPC_OBJTYPE_GENERIC, 0);
		/* Read the values of the up-MSDU generation parameters, 
		 * i.e. the attribute values of the surrounding module. */
		op_ima_obj_attr_get (src_comp_id, "MSDU Interval Time", up_msdu_inter);
		op_ima_obj_attr_get (src_comp_id, "MSDU Size",          up_msdu_size);
		op_ima_obj_attr_get (src_comp_id, "Start Time",         &up_start_t);
		op_ima_obj_attr_get (src_comp_id, "Stop Time",          &up_stop_t);
		log = fopen(log_name, "a");
		fprintf(log, "t=%f,node_name=%s,", op_sim_time(), nd_attrG[nodeid].name);
		fprintf(log, "bid=%d,nid=%d,", nd_attrG[nodeid].bid, nd_attrG[nodeid].nid);
		fprintf(log, "nodeid=%d,init,src,", nodeid);
		fprintf(log, "up=%d,", i);
		fprintf(log, "msdu_interval_time=%s,msdu_size=%s,", up_msdu_inter, up_msdu_size);
		fprintf(log, "start_time=%f,stop_time=%f\n", up_start_t, up_stop_t);
		fclose(log);
		// printf("UP=%d,", i);
		// printf("MSDU Interval Time=%s,MSDU Size=%s,", up_msdu_inter, up_msdu_size);
		// printf("Start Time=%f,Stop Time=%f\n", up_start_t, up_stop_t);
	}

	SF.SLEEP = OPC_TRUE;
	SF.ENABLE_TX_NEW = OPC_FALSE;
	pkt_to_be_sent.enable = OPC_FALSE;
	pkt_tx_total = 0;
	pkt_tx_fail = 0;
	pkt_tx_out_phase = 0;
	waitForACK = OPC_FALSE;
	TX_ING = OPC_FALSE;
	attemptingToTX = OPC_FALSE;
	/* initialization for data_stat */
	latency_avg_all = 0.0;
	for(i=0; i<UP_ALL; i++){
		latency_avg[i] = 0.0;
		for(j=0; j<DATA_STATE; j++){
			sv_data_stat[i][j].number = 0.0;
			sv_data_stat[i][j].ppdu_kbits = 0.0;
			hb_data_stat[i][j].number = 0.0;
			hb_data_stat[i][j].ppdu_kbits = 0.0;
		}
	}
	for(i=0; i<NODE_MAX; i++){
		ack_seq_nid[i] = -1;
	}
	/* initialization for TX/RX STAT */
	sv_t_tx_start = 0;
	sv_t_tx_end = 0;
	sv_t_tx_duration = 0;
	sv_t_rx_start = 0;
	sv_t_rx_end = 0;
	sv_t_rx_duration = 0;

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
static void
wban_log_file_init ()
	{
	char dir_path[200];
	char date_time[30];
	char net_name[100];
	int i = 0;
	time_t rawtime;
	struct tm *p;

	/* Stack tracing enrty point */
	FIN(wban_log_file_init);

	op_ima_obj_attr_get (nodeid, "Log File Directory", dir_path);
	op_ima_obj_attr_get (nodeid, "Log Level", &log_level);

	time(&rawtime);
	p=localtime(&rawtime);
	// strftime(date_time, 30, "%Y-%m-%d_%H-%M-%S", p);
	// strftime(date_time, 30, "%Y-%m-%d_%H-%M", p);
	strftime(date_time, 30, "%Y_%m_%d_%H_%M", p);
	while (dir_path[i++] != '\0')
		;
	if(prg_file_path_create(dir_path, PRGC_FILE_PATH_CREATE_OPT_DIRECTORY) == PrgC_Compcode_Failure){
		op_sim_end("ERROR : Log File is not valid.","INVALID_FILE", "","");
	}
	// obtain the project-scenario-number
	op_sim_info_get (OPC_STRING, OPC_SIM_INFO_OUTPUT_FILE_NAME, net_name);
	if(dir_path[i-1] == '\\'){
		sprintf(log_name, "%s%s-%s.trace", dir_path, net_name, date_time);
	}else{
		sprintf(log_name, "%s\\%s-%s.trace", dir_path, net_name, date_time);
	}
	/* verification if the dir_path is a valid directory */
	if (prg_path_name_is_dir (dir_path) == PrgC_Path_Name_Is_Not_Dir) {
		op_sim_end("ERROR : Log File Directory is not valid directory name.","INVALID_DIR", "","");
	}

	/* Stack tracing exit point */
	FOUT;
	}

static void
wban_init_channel(Objid nodeidL)
	{
	Objid channel_id, rx_channel_info_id, tx_channel_info_id;
	/** initialize channel data rate and tx power **/
	FIN(wban_init_channel());
	/* Store the channel object ids as state variables. */
	radio_tx_id = op_id_from_name (nodeidL, OPC_OBJTYPE_RATX, "tx");
	op_ima_obj_attr_get (radio_tx_id, "channel", &channel_id);
	tx_channel_info_id = op_topo_child (channel_id, OPC_OBJTYPE_RATXCH, 0);
	op_ima_obj_attr_set (tx_channel_info_id, "data rate", nd_attrG[nodeidL].data_rate);
	// op_ima_obj_attr_set (tx_channel_info_id, "power", nd_attrG[nodeidL].tx_power);
	radio_rx_id = op_id_from_name (nodeidL, OPC_OBJTYPE_RARX, "rx");
	op_ima_obj_attr_get (radio_rx_id, "channel", &channel_id);
	rx_channel_info_id = op_topo_child (channel_id, OPC_OBJTYPE_RARXCH, 0);
	op_ima_obj_attr_set (rx_channel_info_id, "data rate", nd_attrG[nodeidL].data_rate);
	/* Set RX power threshold as a reciever state */
	// op_ima_obj_state_set (radio_rx_id, &(modmem_ptr->rx_power_threshold));
	FOUT;
	}

/*--------------------------------------------------------------------------------
 * Function:	wban_parse_incoming_frame
 *
 * Description:	parse the incoming packet and make the adequate processing
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void
wban_parse_incoming_frame ()
	{
	Packet* frame_MPDU;
	Packet* frame_MSDU;
	Packet* rcv_frame;
	int Stream_ID;
	int ban_id;
	int recipient_id;
	int sender_id;
	/* sender nodeid */
	int sid;
	int sf_slot_nodeid;
	int ack_policy_fd;
	// int eap_indicator_fd;
	int frame_type_fd;
	int frame_subtype_fd, up;
	// int beacon2_enabled_fd;
	int sequence_number_fd;
	// int inactive_fd;
	int ppdu_bits;
	double ete_delay;
	double app_latency;
	double snr;

	/* Stack tracing enrty point */
	FIN(wban_parse_incoming_frame);

	/* get the packet from the input stream	*/
	Stream_ID = op_intrpt_strm();
	rcv_frame = op_pk_get (Stream_ID);
	
	frame_type_fd = 0;
	/* check from what input stream the packet is received and do the right processing*/
	switch (Stream_ID) {
		case STRM_FROM_RADIO_TO_MAC: /*A PHY FRAME (PPDU) FROM THE RADIO RECIEVER*/
			ppdu_bits = op_pk_total_size_get(rcv_frame);
			/* get MAC frame (MPDU=PSDU) from received PHY frame (PPDU)*/
			op_pk_nfd_get_pkt (rcv_frame, "PSDU", &frame_MPDU);
			ete_delay = op_sim_time() - op_pk_creation_time_get(frame_MPDU);
			if (MAC_SLEEP == mac_state){
				FOUT;
			}
			op_pk_nfd_get (frame_MPDU, "BAN ID", &ban_id);
			op_pk_nfd_get (frame_MPDU, "Recipient ID", &recipient_id);
			op_pk_nfd_get (frame_MPDU, "Sender ID", &sender_id);
			/*acquire "Frame Type" field*/
			op_pk_nfd_get (frame_MPDU, "Frame Type", &frame_type_fd);
			op_pk_nfd_get (frame_MPDU, "Frame Subtype", &frame_subtype_fd);
			op_pk_nfd_get (frame_MPDU, "Ack Policy", &ack_policy_fd);
			// op_pk_nfd_get (frame_MPDU, "EAP Indicator", &eap_indicator_fd);
			// op_pk_nfd_get (frame_MPDU, "B2", &beacon2_enabled_fd);
			op_pk_nfd_get (frame_MPDU, "Sequence Number", &sequence_number_fd);
			// op_pk_nfd_get (frame_MPDU, "Inactive", &inactive_fd);
			if (!is_packet_for_me(frame_MPDU, ban_id, recipient_id, sender_id)) {
				FOUT;
			}
			/* repalce the mac_attr.receipient_id with Sender ID */
			mac_attr.rcvid = sender_id;
			/* reset sv_beacon_seq while receiving beacon */
			if ((frame_type_fd == MANAGEMENT) && (frame_subtype_fd == BEACON)) {
				sv_beacon_seq = sequence_number_fd;
				reset_pkt_snr_rx(sv_beacon_seq);
			}
			/* collect statistics of SNR received in recent SF */
			snr = op_td_get_dbl (rcv_frame, OPC_TDA_RA_SNR);
			sid = hp_rfind_nodeid(sender_id);
			sv_st_pkt_rx[sid][sv_beacon_seq % SF_NUM].number += 1;
			// sv_st_pkt_rx[sid][sv_beacon_seq % SF_NUM].ppdu_kbits += ppdu_bits / 1000.0;
			sv_st_pkt_rx[sid][sv_beacon_seq % SF_NUM].snr_db += snr;
			// if (!IAM_BAN_HUB) {
			// printf("node=%s,snr=%f,from_node=%s\n", nd_attrG[nodeid].name, snr, nd_attrG[sid].name);
			// 	op_prg_odb_bkpt("snr_db");
			// }
			// op_prg_odb_bkpt("debug_app");
			// log = fopen(log_name, "a");
			// fprintf(log, "t=%f,NODE_NAME=%s,nodeid=%d,MAC_STATE=%d,RX,RECIPIENT_ID=%d,SENDER_ID=%d,", op_sim_time(), nd_attrG[nodeid].name, nodeid, mac_state, recipient_id, sender_id);
			// fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d,ETE_DELAY=%f\n", frame_type_fd, frame_subtype_fd, ppdu_bits, ete_delay);
			// fclose(log);
			// printf("t=%f,NODE_NAME=%s,nodeid=%d,MAC_STATE=%d,RX,RECIPIENT_ID=%d,SENDER_ID=%d,", op_sim_time(), nd_attrG[nodeid].name, nodeid, mac_state, recipient_id, sender_id);
			// printf("FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d,ETE_DELAY=%f\n", frame_type_fd, frame_subtype_fd, ppdu_bits, ete_delay);
			// op_prg_odb_bkpt("debug_app");
			if(I_ACK_POLICY == ack_policy_fd){
				ack_seq_num = sequence_number_fd;
				op_intrpt_schedule_self(op_sim_time()+pSIFS, SEND_I_ACK);
				if(ack_seq_nid[sender_id % NODE_MAX] != ack_seq_num){
					ack_seq_nid[sender_id % NODE_MAX] = ack_seq_num;
				}else{
					// printf("\t  Duplicate packet received\n");
					op_pk_destroy (rcv_frame);
					op_pk_destroy (frame_MPDU);
					FOUT;
				}
			}

			switch (frame_type_fd) {
				case DATA: /* Handle data packets */
					/* collect statistics */
					op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
					app_latency = op_sim_time() - op_pk_creation_time_get(frame_MSDU);
					// log = fopen(log_name, "a");
					// fprintf(log, "t=%f,NODE_NAME=%s,nodeid=%d,MAC_STATE=%d,RX,RECIPIENT_ID=%d,SENDER_ID=%d,", op_sim_time(), nd_attrG[nodeid].name, nodeid, mac_state, recipient_id, sender_id);
					// fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d,APP_DELAY=%f\n", frame_type_fd, frame_subtype_fd, ppdu_bits, app_latency);
					// fclose(log);
					// op_prg_odb_bkpt("debug_app");
					if (SF.IN_MAP_PHASE) {
						sf_slot_nodeid = sv_slot_nodeid[SF.current_slot];
						++sv_st_map_pkt[sf_slot_nodeid][sv_beacon_seq % SF_NUM].thr;
						++sv_st_map_hub[sv_beacon_seq % SF_NUM].thr;
					}
					up = frame_subtype_fd;
					// temp latency_avg
					latency_avg[up] = latency_avg[up] * sv_data_stat[up][RCV].number + ete_delay;
					sv_data_stat[up][RCV].number += 1;
					sv_data_stat[up][RCV].ppdu_kbits += 0.001 * ppdu_bits;
					latency_avg[up] = latency_avg[up] / sv_data_stat[up][RCV].number;
					// wban_extract_data_frame (frame_MPDU);
					/* send to higher layer for statistics */
					// op_pk_send (frame_MPDU, STRM_FROM_MAC_TO_SINK);
					break;
				case MANAGEMENT: /* Handle management packets */
					switch (frame_subtype_fd) {
						case BEACON: 
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
							// printf ("\t    Connection Request Packet reception\n");
							// wban_extract_conn_req_frame (frame_MPDU);
							break;
						case CONNECTION_ASSIGNMENT:
							// printf ("\t    Connection Assignment Packet reception\n");
							// wban_extract_conn_assign_frame (frame_MPDU);
							break;
						case DISCONNECTION:
							// not implemented
							break;
						case COMMAND:
							// not implemented
							break;
					}
					break;
				case CONTROL: /* Handle control packets */
					// printf ("\t  Control Packet reception\n");
					switch (frame_subtype_fd) {
						case I_ACK:
							// printf ("\t    I-ACK Packet reception\n");
							wban_extract_i_ack_frame (frame_MPDU);
							break;
						case B_ACK:
							// not implemented
							break;
						case BEACON2: 
							// printf ("\t    BEACON2 Packet reception\n");
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
					break;
				default:	/*OTHER FRAME TYPES*/
					// printf("\t  Other Packet reception\n");
					break;
			}
			break;
		case STRM_FROM_TRAFFIC_UP_TO_MAC: /* INCOMMING PACKETS(MSDU) FROM THE TRAFFIC SOURCE */
			wban_encapsulate_and_enqueue_data_frame (rcv_frame, I_ACK_POLICY, nd_attrG[nodeid].dest_id);			
			break;
		default : break;
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
static Boolean
is_packet_for_me (Packet* frame_MPDU, int ban_id, int recipient_id, int sender_id)
	{
	/* Stack tracing enrty point */
	FIN(is_packet_for_me);
	
	// printf("\nt=%f,NODE_NAME=%s,NID=%d,MAC_STATE=%d\n", op_sim_time(), nd_attrG[nodeid].name, mac_attr.sendid, mac_state);
	/*Check if the frame is loop*/
	if (mac_attr.sendid == sender_id) {
		op_pk_destroy (frame_MPDU);
		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}
	if (nd_attrG[nodeid].bid != ban_id) {
		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}
	if ((mac_attr.sendid == recipient_id) || (BROADCAST_NID == recipient_id)) {
		/* Stack tracing exit point */
		FRET(OPC_TRUE);
	} else {
		op_pk_destroy (frame_MPDU);
		/* Stack tracing exit point */
		FRET(OPC_FALSE);
	}
	/* Stack tracing exit point */
	FRET(OPC_FALSE);
	}

void
reset_map1_map()
	{
	int slotnum, i;
	double slot_bits, data_ack_bits, snr_db;
	FIN(reset_map1_map);

	subq_info_get(SUBQ_DATA);
	slot_bits = nd_attrG[nodeid].data_rate * SF.slot_sec;
	data_ack_bits = subq_info.bitsize + subq_info.pksize *2* HEADER_BITS;
	slotnum = (int)(ceil)(data_ack_bits / slot_bits);
	// cut-off
	if (slotnum > SLOT_REQ_MAX) slotnum = SLOT_REQ_MAX;
	/* collect the average SNR received from Hub */
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		// ignore self
		if (i == nodeid) {
			continue;
		}
		// same BAN ID
		if (nd_attrG[i].bid == nd_attrG[nodeid].bid) {
			if (!IAM_BAN_HUB && nd_attrG[i].dev_mode == HUB) {
				snr_db = hp_avg_snr(i);
				break;
			}
		}
	}
	// reset map1_sche_map
	map1_sche_map[nodeid].slotnum = slotnum;
	map1_sche_map[nodeid].up = subq_info.up;
	map1_sche_map[nodeid].snr = snr_db;

	FOUT;
	}

static double
hp_avg_snr(int nodeid_l)
	{
	int i = 0;
	double snr_sf_i = 0;
	double snr_avg = 0, snr_var = 0;
	FIN(hp_snr_avg);
	for (i = 0; i < SF_NUM; ++i) {
		if (sv_st_pkt_rx[nodeid_l][i].number > 0) {
			snr_sf_i = sv_st_pkt_rx[nodeid_l][i].snr_db / sv_st_pkt_rx[nodeid_l][i].number;
		} else {
			snr_sf_i = 0;
		}
		snr_avg += snr_sf_i;
		snr_var += snr_sf_i * snr_sf_i;
	}
	snr_avg /= SF_NUM;
	snr_var = snr_var / SF_NUM - snr_avg * snr_avg;
	if ((int)op_sim_time() < 1) {
		snr_var = 0;
	}
	snr_avg -= snr_var;
	FRET(snr_avg);
	}

static double
hp_avg_pkt_thr(int nodeid_l)
	{
	int i = 0;
	double thr_sf_i = 0;
	double thr_avg = 0, thr_var = 0;
	FIN(hp_avg_pkt_thr);
	for (i = 0; i < SF_NUM; ++i) {
		if (sv_st_map_pkt[nodeid_l][i].slot > 0) {
			thr_sf_i = 1.0 * sv_st_map_pkt[nodeid_l][i].thr / sv_st_map_pkt[nodeid_l][i].slot;
		} else {
			thr_sf_i = 0;
		}
		thr_avg += thr_sf_i;
		thr_var += thr_sf_i * thr_sf_i;
	}
	thr_avg /= SF_NUM;
	thr_var = thr_var / SF_NUM - thr_avg * thr_avg;
	if ((int)op_sim_time() < 1) {
		thr_var = 0;
	}
	// thr_avg = thr_var - thr_avg;
	FRET(thr_avg);
	}

/*
 * calculate the rho by Hub
 */
void calc_prio_hub()
	{
	/** 
	*	rho = UP + alpha*f(SINR) + beta*f(I) 
	**/
	double alpha = 2, beta = 2;
	/* Average SNR of Node i */
	double snr_avg_i, thr_avg_i, nid_i;
	/* eta = alpha * f(SINR) */
	double eta_i;
	/* gamma = beta * f(I) */
	double gamma_i;
	/* rho = UP + alpha*f(SINR) + beta*f(I) */
	double rho_i;
	double pkt_thr_util[NODE_ALL_MAX];
	double snr_max = INT_MIN, snr_min = INT_MAX, snr_avg_db;
	double thr_max = INT_MIN, thr_min = INT_MAX, thr_avg;
	int i = 0, j = 0;
	FIN(calc_prio_hub);
	/* obtain the snr_max(thr_max) and snr_min(thr_min) */
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		pkt_thr_util[i] = 0;
		// ignore self
		if (i == nodeid) {
			continue;
		}
		// same BAN ID
		if (nd_attrG[i].bid == nd_attrG[nodeid].bid) {
			snr_avg_i = map1_sche_map[i].snr;
			if (snr_avg_i > snr_max) {
				snr_max = snr_avg_i;
			}
			if (snr_avg_i <= snr_min) {
				snr_min = snr_avg_i;
			}
			thr_avg_i = hp_avg_pkt_thr(i);
			pkt_thr_util[i] = thr_avg_i;
			if (thr_avg_i > thr_max) {
				thr_max = thr_avg_i;
			}
			if (thr_avg_i <= thr_min) {
				thr_min = thr_avg_i;
			}
		}
	}
	snr_avg_db = (snr_max + snr_min) / 2;
	thr_avg    = (thr_max + thr_min) / 2;
	/* pre-processing avoid zero INF */
	if (snr_avg_db == snr_min) snr_avg_db = snr_min + 0.001;
	if (thr_avg == thr_min) thr_avg = thr_min + 0.001;

	/* second traverse, get the eta([0,2]) */
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		sv_rho_hub[i] = 0;
		// ignore self
		if (i == nodeid) {
			continue;
		}
		// same BAN ID
		if (nd_attrG[i].bid == nd_attrG[nodeid].bid) {
			eta_i = (map1_sche_map[i].snr - snr_min) / (snr_avg_db - snr_min);
			eta_i = eta_i * eta_i;
			gamma_i = (pkt_thr_util[i] - thr_min) / (thr_avg - thr_min);
			gamma_i = gamma_i * gamma_i;
			sv_rho_hub[i] = map1_sche_map[i].up + alpha * eta_i + beta * gamma_i;
			if (nd_attrG[nodeid].protocol_ver == 2) {
				sv_rho_hub[i] = map1_sche_map[i].up;
			}
		}
	}
	/* sort rho by nid */
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		sv_nid_rho[i].nid = i;
		sv_nid_rho[i].rho = sv_rho_hub[i];
		// same BAN ID
		if (i == nodeid || nd_attrG[i].bid != nd_attrG[nodeid].bid) {
			sv_nid_rho[i].rho = -1;
		}
	}
	/* insertion sort */
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		j = i;
		rho_i = sv_nid_rho[i].rho;
		nid_i = sv_nid_rho[i].nid;
		while (j > 0 && sv_nid_rho[j - 1].rho < rho_i) {
			sv_nid_rho[j].nid = sv_nid_rho[j - 1].nid;
			sv_nid_rho[j].rho = sv_nid_rho[j - 1].rho;
			j = j - 1;
		}
		sv_nid_rho[j].nid = nid_i;
		sv_nid_rho[j].rho = rho_i;
	}
	FOUT;
	}

static void
resize_map_len()
	{
	int i;
	int map_slots = 0, map_thr = 0;
	int slot_req_total = 0;
	double map_util = 0, epsilon1 = 0.50, epsilon2 = 0.75;
	int miu = 2, rap_start_min = 3, rap_start_max = 255;
	FIN(resize_map_len);
	// resize MAP after SF_NUM SF
	if (sv_beacon_seq < SF_NUM) {
		FOUT;
	}
	for (i = 0; i < SF_NUM; ++i) {
		map_slots += sv_st_map_hub[i].slot;
		map_thr   += sv_st_map_hub[i].thr;
	}
	// MAP utilization
	if (map_slots > 0) {
		map_util = 1.0 * map_thr / map_slots;
	} else {
		map_util = 0;
	}
	// resize MAP length
	if (map_util > epsilon2) {
		SF.rap1_start += miu;
	} else if (map_util < epsilon1) {
		SF.rap1_start -= miu;
	}
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		// ignore self
		if (i == nodeid) {
			continue;
		}
		// same BAN ID
		if (nd_attrG[i].bid == nd_attrG[nodeid].bid) {
			slot_req_total += map1_sche_map[i].slotnum;
		}
	}
	// slot require by node
	if (SF.first_free_slot + slot_req_total < SF.rap1_start) {
		SF.rap1_start = SF.first_free_slot + slot_req_total;
	}
	// post processing for MAP length
	rap_start_max = SF.SD - 4;
	if (SF.rap1_start > rap_start_max) SF.rap1_start = rap_start_max;
	if (SF.rap1_start < rap_start_min) SF.rap1_start = rap_start_min;
	beacon_attr.rap1_start = SF.rap1_start;

	FOUT;
	}

void
map1_scheduling()
	{
	int i = 0, j;
	int slot_seq;
	FIN(map1_scheduling);
	calc_prio_hub();
	// disable map1
	if (SF.rap1_start == 0) {
		FOUT;
	}
	// update free_slot
	SF.free_slot = SF.first_free_slot;
	// reset slot nid mapping
	reset_slot_nid(SLOT_NUM);
	reset_map1_sche_map();
	reset_map1_pkt_thr(sv_beacon_seq);
	/* reset MAP1 stat */
	sv_st_map_hub[sv_beacon_seq % SF_NUM].slot = SF.map1_end - SF.free_slot + 1;
	sv_st_map_hub[sv_beacon_seq % SF_NUM].thr = 0;
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		/* mapping to real nid */
		j = sv_nid_rho[i].nid;
		// ignore self
		if (j == nodeid) {
			continue;
		}
		// same BAN ID
		if (nd_attrG[j].bid != nd_attrG[nodeid].bid) {
			continue;
		}
		// ignore node which do not use MAP1
		if (map1_sche_map[j].slotnum <= 0) {
			continue;
		}
		// exceeding map end
		if (SF.free_slot > SF.map1_end) {
			continue;
		}
		// allocation
		slot_seq = SF.free_slot;
		if (SF.free_slot + map1_sche_map[j].slotnum <= SF.map1_end + 1) {
			map1_sche_map[j].slot_start = SF.free_slot;
			map1_sche_map[j].slot_end = SF.free_slot + map1_sche_map[j].slotnum - 1;
			while (slot_seq <= map1_sche_map[j].slot_end) {
				sv_slot_nodeid[slot_seq] = j;
				++sv_st_map_pkt[j][sv_beacon_seq % SF_NUM].slot;
				++slot_seq;
			}
			SF.free_slot = map1_sche_map[j].slot_end + 1;
		} else {
			map1_sche_map[j].slot_start = SF.free_slot;
			map1_sche_map[j].slot_end = SF.map1_end;
			SF.free_slot = map1_sche_map[j].slot_end + 1;
		}
	}
	FOUT;
	}

/*--------------------------------------------------------------------------------
 * Function:	wban_send_beacon_frame
 *
 * Description:	Create a beacon frame and send it to the Radio
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void
wban_send_beacon_frame ()
	{
	Packet* beacon_MSDU;
	Packet* beacon_MPDU;
	// double beacon_frame_tx_time;
	int ppdu_bits = 0;
	/* Stack tracing enrty point */
	FIN(wban_send_beacon_frame);
	/* create a beacon frame */
	beacon_MSDU = op_pk_create_fmt ("wban_beacon_MSDU_format");
	/* set the fields of the beacon frame */
	op_pk_nfd_set (beacon_MSDU, "Sender Address", beacon_attr.send_addr);
	op_pk_nfd_set (beacon_MSDU, "Beacon Period Length", beacon_attr.beacon_period_length);
	op_pk_nfd_set (beacon_MSDU, "Allocation Slot Length", beacon_attr.allocation_slot_length);
	op_pk_nfd_set (beacon_MSDU, "RAP1 Start", beacon_attr.rap1_start);
	op_pk_nfd_set (beacon_MSDU, "RAP1 End", beacon_attr.rap1_end);
	// op_pk_nfd_set (beacon_MSDU, "Inactive Duration", beacon_attr.inactive_duration);
	
	/* create a MAC frame (MPDU) that encapsulates the beacon payload (MSDU) */
	beacon_MPDU = op_pk_create_fmt ("wban_frame_MPDU_format");

	op_pk_nfd_set (beacon_MPDU, "Ack Policy", N_ACK_POLICY);
	// op_pk_nfd_set (beacon_MPDU, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (beacon_MPDU, "Frame Subtype", BEACON);
	op_pk_nfd_set (beacon_MPDU, "Frame Type", MANAGEMENT);
	// op_pk_nfd_set (beacon_MPDU, "B2", 1); // beacon2 enabled
	op_pk_nfd_set (beacon_MPDU, "Sequence Number", ((++sv_beacon_seq) % 256));
	// op_pk_nfd_set (beacon_MPDU, "Inactive", beacon_attr.inactive_duration); // beacon and beacon2 frame used
	op_pk_nfd_set (beacon_MPDU, "Recipient ID", BROADCAST_NID);
	op_pk_nfd_set (beacon_MPDU, "Sender ID", mac_attr.sendid);
	op_pk_nfd_set (beacon_MPDU, "BAN ID",  mac_attr.bid);
	op_pk_nfd_set_pkt (beacon_MPDU, "MAC Frame Payload", beacon_MSDU); // wrap beacon payload (MSDU) in MAC Frame (MPDU)
	// update the superframe parameters
	SF.SD = beacon_attr.beacon_period_length; // the superframe duration(beacon preriod length) in slots
	SF.BI = beacon_attr.beacon_period_length * (1+ beacon_attr.inactive_duration); // active and inactive superframe
	SF.sleep_period = SF.SD * beacon_attr.inactive_duration;
	SF.duration = SF.BI*SF.slot_sec;
	SF.rap1_start = beacon_attr.rap1_start;
	SF.rap1_end = beacon_attr.rap1_end;

	SF.BI_Boundary = op_pk_creation_time_get (beacon_MPDU);
	ppdu_bits = op_pk_total_size_get(beacon_MPDU) + MAC2PHY_BITS;
	if(init_flag){
		// log = fopen(log_name, "a");
		// fprintf(log, "t=%f,node_name=%s,nodeid=%d,init,nid=%d,", op_sim_time(), nd_attrG[nodeid].name, nodeid, mac_attr.sendid);
		// fprintf(log, "SUPERFRAME_LENGTH=%d,RAP1_LENGTH=%d\n", beacon_attr.beacon_period_length, beacon_attr.rap1_length);
		// fclose(log);
		init_flag = OPC_FALSE;
		SF.slot_sec = (pAllocationSlotMin + beacon_attr.allocation_slot_length*pAllocationSlotResolution) * 0.000001;
		// printf("\nt=%f,NODE_NAME=%s,NID=%d,INIT,MAC_STATE=%d\n", op_sim_time(), nd_attrG[nodeid].name, mac_attr.sendid, mac_state);
		// printf("\t  SUPERFRAME_LENGTH=%d,RAP1_LENGTH=%d,B2_START=%d\n", beacon_attr.beacon_period_length, beacon_attr.rap1_length, beacon_attr.b2_start);
		// printf("\t  allocation_slot_length=%d,SF.slot_sec=%f\n", beacon_attr.allocation_slot_length, SF.slot_sec);
		// op_prg_odb_bkpt("init");
		SF.first_free_slot = 1 + (int)(hp_tx_time(ppdu_bits) / SF.slot_sec);
	}
	SF.current_slot = (int)(hp_tx_time(ppdu_bits) / SF.slot_sec);
	/* send the MPDU to PHY and calculate the energy consuming */
	wban_send_mac_pk_to_phy(beacon_MPDU);
	wban_schedule_next_beacon();
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
static void
wban_extract_beacon_frame (Packet* beacon_MPDU_rx)
	{
	Packet* beacon_MSDU_rx;
	int rcv_sender_id;
	// int eap_indicator_fd;
	// int beacon2_enabled_fd;
	double beacon_frame_tx_time;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_beacon_frame);
	beacon_frame_tx_time = hp_tx_time(wban_norm_phy_bits(beacon_MPDU_rx));
	op_pk_nfd_get_pkt (beacon_MPDU_rx, "MAC Frame Payload", &beacon_MSDU_rx);
	op_pk_nfd_get (beacon_MPDU_rx, "Sender ID", &rcv_sender_id);
	op_pk_nfd_get (beacon_MPDU_rx, "Sequence Number", &sv_beacon_seq);
	// op_pk_nfd_get (beacon_MPDU_rx, "EAP Indicator", &eap_indicator_fd);
	// op_pk_nfd_get (beacon_MPDU_rx, "B2", &beacon2_enabled_fd);
	if (nd_attrG[nodeid].bid + 15 != rcv_sender_id) {
		FOUT;
	}
	op_pk_nfd_get (beacon_MSDU_rx, "Sender Address", &beacon_attr.send_addr);
	op_pk_nfd_get (beacon_MSDU_rx, "Beacon Period Length", &beacon_attr.beacon_period_length);
	op_pk_nfd_get (beacon_MSDU_rx, "Allocation Slot Length", &beacon_attr.allocation_slot_length);
	op_pk_nfd_get (beacon_MSDU_rx, "RAP1 End", &beacon_attr.rap1_end);
	op_pk_nfd_get (beacon_MSDU_rx, "RAP1 Start", &beacon_attr.rap1_start);
	op_pk_nfd_get (beacon_MSDU_rx, "Inactive Duration", &beacon_attr.inactive_duration);
	// update with actual Hub id
	if (HUB_ID == nd_attrG[nodeid].dest_id) {
		nd_attrG[nodeid].dest_id = rcv_sender_id;
	}
	/*update rap length*/
	beacon_attr.rap1_length = beacon_attr.rap1_end - beacon_attr.rap1_start + 1;
	SF.BI_Boundary = op_pk_creation_time_get (beacon_MPDU_rx);
	// op_prg_odb_bkpt("rcv_beacon");
	op_pk_destroy (beacon_MSDU_rx);
	op_pk_destroy (beacon_MPDU_rx);
	mac_attr.rcvid = rcv_sender_id;
	if (UNCONNECTED_NID == mac_attr.sendid) {
		/* We will try to connect to this BAN  if our scheduled access length
		 * is NOT set to unconnected (-1). If it is set to 0, it means we are
		 * establishing a sleeping pattern and waking up only to hear beacons
		 * and are only able to transmit in RAP periods.
		 */
		/* initialize the NID from 32 */
		mac_attr.sendid = current_free_connected_NID++;  //current_free_connected_NID is global variable
		// log = fopen(log_name, "a");
		// fprintf(log, "t=%f,nodeid=%d,INIT,NODE_NAME=%s,NID=%d,", op_sim_time(), nodeid, nd_attrG[nodeid].name, mac_attr.sendid);
		// fprintf(log, "SUPERFRAME_LENGTH=%d,RAP1_LENGTH=%d,B2_START=%d\n", beacon_attr.beacon_period_length, beacon_attr.rap1_length, beacon_attr.b2_start);
		// fclose(log);
		// printf("t=%f,nodeid=%d,INIT,NODE_NAME=%s,NID=%d\n", op_sim_time(), nodeid, nd_attrG[nodeid].name, mac_attr.sendid);
		// op_prg_odb_bkpt("init");

	}
	// update the superframe parameters
	SF.SD = beacon_attr.beacon_period_length; // the superframe duration(beacon preriod length) in slots
	SF.BI = beacon_attr.beacon_period_length * (1+ beacon_attr.inactive_duration); // active and inactive superframe
	SF.sleep_period = SF.SD * beacon_attr.inactive_duration;
	SF.duration = SF.BI*SF.slot_sec;
	SF.rap1_start = beacon_attr.rap1_start;
	SF.rap1_end = beacon_attr.rap1_end;
	SF.slot_sec = (pAllocationSlotMin + \
				beacon_attr.allocation_slot_length*pAllocationSlotResolution) * 0.000001;
	SF.first_free_slot = 1 + (int)(beacon_frame_tx_time/SF.slot_sec);
	SF.current_slot = (int)((op_sim_time() - SF.BI_Boundary) / SF.slot_sec);
	wban_schedule_next_beacon();
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
static void
wban_schedule_next_beacon()
	{
	/* Stack tracing enrty point */
	FIN(wban_schedule_next_beacon);
	// exception
	if (SF.rap1_start != 255 && SF.rap1_start >= SF.SD) {
		op_sim_end("ERROR : RAP start > SF.SD","","","");
	}
	if (SF.rap1_start < 0 || SF.rap1_end < 0) {
		op_sim_end("ERROR : RAP start/end < 0","","","");
	}
	if (SF.rap1_end < SF.rap1_start) {
		op_sim_end("ERROR : RAP start > end","","","");
	}

	// case 1 - RAP1 only
	if (SF.rap1_start == 0) {
		// RAP1 start after beacon
		if (IAM_BAN_HUB) {
			SF.rap1_start2sec = op_sim_time();
		} else {
			SF.rap1_start2sec = op_sim_time() + pSIFS;
		}
		op_intrpt_schedule_self(SF.rap1_start2sec, START_OF_RAP1_PERIOD_CODE);
	} else if (SF.rap1_start > 0 && SF.rap1_start < SF.SD) {
		// case 2 - RAP1 and MAP1
		SF.rap1_start2sec = SF.BI_Boundary + \
							SF.rap1_start * SF.slot_sec;
		if(IAM_BAN_HUB){
			SF.map1_start = SF.first_free_slot;
			SF.map1_end = SF.rap1_start - 1;
			SF.map1_start2sec = SF.BI_Boundary + \
								SF.first_free_slot * SF.slot_sec;
			SF.map1_end2sec = SF.BI_Boundary + \
								SF.rap1_start * SF.slot_sec;
			op_intrpt_schedule_self (SF.map1_start2sec, START_OF_MAP1_PERIOD_CODE);
			op_intrpt_schedule_self (SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
		} else if (map1_sche_map[nodeid].slot_start > 0) {
			// allocation for Node
			SF.map1_start = map1_sche_map[nodeid].slot_start;
			SF.map1_end = map1_sche_map[nodeid].slot_end;
			SF.map1_start2sec = SF.BI_Boundary + \
								map1_sche_map[nodeid].slot_start * SF.slot_sec;
			SF.map1_end2sec = SF.BI_Boundary + \
								(1 + map1_sche_map[nodeid].slot_end) * SF.slot_sec;
			op_intrpt_schedule_self (SF.map1_start2sec, START_OF_MAP1_PERIOD_CODE);
			op_intrpt_schedule_self (SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
		}
		op_intrpt_schedule_self(SF.rap1_start2sec, START_OF_RAP1_PERIOD_CODE);
	} else if (SF.rap1_start == 255) {
		// MAP1 only
		if(IAM_BAN_HUB){
			SF.map1_start = SF.first_free_slot;
			SF.map1_end = SF.BI - 1;
			SF.map1_start2sec = SF.BI_Boundary + \
								SF.first_free_slot * SF.slot_sec;
			SF.map1_end2sec = SF.BI_Boundary + \
								SF.BI * SF.slot_sec;
			op_intrpt_schedule_self (SF.map1_start2sec, START_OF_MAP1_PERIOD_CODE);
			op_intrpt_schedule_self (SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
		} else if (map1_sche_map[nodeid].slot_start > 0) {
			// allocation for Node
			SF.map1_start = map1_sche_map[nodeid].slot_start;
			SF.map1_end = map1_sche_map[nodeid].slot_end;
			SF.map1_start2sec = SF.BI_Boundary + \
								map1_sche_map[nodeid].slot_start * SF.slot_sec;
			SF.map1_end2sec = SF.BI_Boundary + \
								(1 + map1_sche_map[nodeid].slot_end) * SF.slot_sec;
			if (op_sim_time() + pSIFS < SF.BI_Boundary + (SF.current_slot + 1) * SF.slot_sec) {
				op_intrpt_schedule_self (op_sim_time() + pSIFS, START_OF_SLEEP_PERIOD);
			}
			op_intrpt_schedule_self (SF.map1_start2sec, START_OF_MAP1_PERIOD_CODE);
			op_intrpt_schedule_self (SF.map1_end2sec, END_OF_MAP1_PERIOD_CODE);
		}
	}
	
	// processing RAP1 End
	if (SF.rap1_end > SF.rap1_start) {
		SF.rap1_end2sec = SF.BI_Boundary + (SF.rap1_end+1)*SF.slot_sec;
		// SF.rap1_end == 255 ==> till SF end
		if (SF.rap1_end == 255) {
			SF.rap1_end2sec = SF.BI_Boundary + SF.BI*SF.slot_sec;
		}
		SF.rap1_length2sec = SF.rap1_end2sec - SF.rap1_start2sec;
		op_intrpt_schedule_self(SF.rap1_end2sec, END_OF_RAP1_PERIOD_CODE);
	}

	if (IAM_BAN_HUB && SF.map1_start > 0) {
		SF.map1_len = SF.map1_end - SF.map1_start + 1;
		// do map1 scheduling
		op_intrpt_schedule_self (op_sim_time() + pSIFS, MAP1_SCHEDULE);
	}

	// printf("NODE_NAME=%s\n", nd_attrG[nodeid].name);
	// printf("\t  Superframe parameters:\n");
	// printf("\t  SF.first_free_slot=%d\n", SF.first_free_slot);
	// printf("\t  SF.rap1_start=%d\n", SF.rap1_start);
	// printf("\t  SF.rap1_start2sec=%f\n", SF.rap1_start2sec);
	// printf("\t  SF.rap1_end=%d\n", SF.rap1_end);
	// printf("\t  SF.rap1_end2sec=%f\n", SF.rap1_end2sec);
	// printf("\t  SF.map1_start=%d\n", SF.map1_start);
	// printf("\t  SF.map1_start2sec=%f\n", SF.map1_start2sec);
	// printf("\t  SF.map1_end=%d\n", SF.map1_end);
	// printf("\t  SF.map1_end2sec=%f\n", SF.map1_end2sec);
	// fprintf (log,"t=%f  -> Schedule Next Beacon at %f\n\n", op_sim_time(), SF.BI_Boundary+SF.BI*SF.slot_sec);
	// printf ("Schedule Next Beacon at %f\n", SF.BI_Boundary+SF.BI*SF.slot_sec);
	// op_prg_odb_bkpt("sch_beacon");

	/* INCREMENT_SLOT at slot boundary, after beacon */
	op_intrpt_schedule_self (SF.BI_Boundary + (SF.current_slot + 1) * SF.slot_sec, INCREMENT_SLOT);
	op_intrpt_schedule_self (SF.BI_Boundary + SF.BI*SF.slot_sec, BEACON_INTERVAL_CODE);
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
static void
wban_send_i_ack_frame (int seq_num)
	{
	double ack_sent;
	Packet* frame_MPDU;
	/* Stack tracing enrty point */
	FIN(wban_send_i_ack_frame);

	// printf(" [Node %s] t=%f  -> Send ACK Frame [SEQ = %d] \n\n", nd_attrG[nodeid].name, op_sim_time(), seq_num);
	/* create a MAC frame (MPDU) that encapsulates the connection_request payload (MSDU) */
	frame_MPDU = op_pk_create_fmt ("wban_frame_MPDU_format");
	op_pk_nfd_set (frame_MPDU, "Ack Policy", N_ACK_POLICY);
	op_pk_nfd_set (frame_MPDU, "Frame Subtype", I_ACK);
	op_pk_nfd_set (frame_MPDU, "Frame Type", CONTROL);
	op_pk_nfd_set (frame_MPDU, "Sequence Number", seq_num);
	op_pk_nfd_set (frame_MPDU, "Recipient ID", mac_attr.rcvid);
	op_pk_nfd_set (frame_MPDU, "Sender ID", mac_attr.sendid);
	op_pk_nfd_set (frame_MPDU, "BAN ID", mac_attr.bid);
	// Hub may contain sync information

	// log = fopen(log_name, "a");
	// fprintf(log, "t=%f,NODE_NAME=%s,nodeid=%d,MAC_STATE=%d,TX,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), nd_attrG[nodeid].name, nodeid, mac_state, mac_attr.sendid, mac_attr.rcvid);
	// fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", CONTROL, I_ACK, wban_norm_phy_bits(frame_MPDU));
	// fclose(log);

	phy_to_radio(frame_MPDU);
	ack_sent = op_sim_time() + hp_tx_time(I_ACK_PPDU_BITS) + pSIFS;
	if(ack_sent < phase_end_timeG){
		op_intrpt_schedule_self(ack_sent, TRY_PACKET_TRANSMISSION_CODE);
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
static void
wban_extract_i_ack_frame (Packet* ack_frame)
	{
	int seq_num;
	// int retry_times;
	// Packet* mac_frame_dup;
	
	/* Stack tracing enrty point */
	FIN(wban_extract_i_ack_frame);
	if((!pkt_to_be_sent.enable) || (0 == pkt_tx_total) || (!waitForACK)){
		// printf("\t  No packet being TX while receive I-ACK.\n");
		FOUT;
	}
	op_pk_nfd_get (ack_frame, "Sequence Number", &seq_num);
	
	/* if I'm waiting for an ACK */
	if (waitForACK) {
		if (mac_attr.wait_ack_seq_num == seq_num) { /* yes, I have received my ACK */
			/* disable the invocation of only the next interrupt of WAITING_ACK_END_CODE */
			op_intrpt_disable (OPC_INTRPT_SELF, WAITING_ACK_END_CODE, OPC_TRUE);
			// printf("\nt=%f,NODE_NAME=%s,MAC_STATE=%d,TX_SUCC,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), nd_attrG[nodeid].name, mac_state, mac_attr.sendid, mac_attr.rcvid);
			// printf("\t  ACK Frame Reception [Requested SEQ = %d]\n", seq_num);
			// log = fopen(log_name, "a");
			// fprintf(log, "t=%f,NODE_NAME=%s,nodeid=%d,MAC_STATE=%d,TX_SUCC,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), nd_attrG[nodeid].name, nodeid, mac_state, mac_attr.sendid, mac_attr.rcvid);
			// fprintf(log, "TX_TIME=%d,OUT_BOUND=%d,", pkt_tx_total, pkt_tx_out_phase);
			// fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n",pkt_to_be_sent.frame_type,pkt_to_be_sent.frame_subtype,pkt_to_be_sent.ppdu_bits);
			// fclose(log);
			//collect statistics
			if(DATA == pkt_to_be_sent.frame_type){
				sv_data_stat[pkt_to_be_sent.frame_subtype][RCV].number += 1;
				sv_data_stat[pkt_to_be_sent.frame_subtype][RCV].ppdu_kbits += 0.001*pkt_to_be_sent.ppdu_bits;
				hb_data_stat[pkt_to_be_sent.frame_subtype][RCV].number += 1;
				hb_data_stat[pkt_to_be_sent.frame_subtype][RCV].ppdu_kbits += 0.001*pkt_to_be_sent.ppdu_bits;
			}
			waitForACK = OPC_FALSE;
			TX_ING = OPC_FALSE;
			// attemptingToTX = OPC_FALSE;
			pkt_to_be_sent.enable = OPC_FALSE;
			pkt_tx_total = 0;
			pkt_tx_out_phase = 0;
			pkt_tx_fail = 0;
			/* Try to send another packet after pSIFS */
			op_intrpt_schedule_self (op_sim_time() + pSIFS, TRY_PACKET_TRANSMISSION_CODE);
		} else	{	/* No, This is not my ACK, I'm Still Waiting */
		}
	} else {/* if (mac_attributes.wait_ack == OPC_FALSE) */ 
	}
	
	/* destroy the ACK packet */
	op_pk_destroy (ack_frame);
	
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
static void
wban_encapsulate_and_enqueue_data_frame (Packet* data_frame_up, enum ack_type ack_policy, int dest_id)
	{
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
	// op_pk_nfd_set (data_frame_mpdu, "EAP Indicator", 1); // EAP1 enabled
	op_pk_nfd_set (data_frame_mpdu, "Frame Subtype", user_priority);
	op_pk_nfd_set (data_frame_mpdu, "Frame Type", DATA);
	// op_pk_nfd_set (data_frame_mpdu, "B2", 1); // beacon2 enabled
	op_pk_nfd_set (data_frame_mpdu, "Sequence Number", seq_num);
	op_pk_nfd_set (data_frame_mpdu, "Inactive", beacon_attr.inactive_duration); // beacon and beacon2 frame used
	op_pk_nfd_set (data_frame_mpdu, "Recipient ID", dest_id);
	op_pk_nfd_set (data_frame_mpdu, "Sender ID", mac_attr.sendid);
	op_pk_nfd_set (data_frame_mpdu, "BAN ID", mac_attr.bid);
	op_pk_nfd_set_pkt (data_frame_mpdu, "MAC Frame Payload", data_frame_msdu); // wrap data_frame_msdu (MSDU) in MAC Frame (MPDU)

	/* put it into the queue with priority waiting for transmission */
	op_pk_priority_set (data_frame_mpdu, (double)user_priority);
	sv_data_stat[user_priority][GEN].number += 1;
	sv_data_stat[user_priority][GEN].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
	hb_data_stat[user_priority][GEN].number += 1;
	hb_data_stat[user_priority][GEN].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
	// log = fopen(log_name, "a");
	if (op_subq_pk_insert(SUBQ_DATA, data_frame_mpdu, OPC_QPOS_TAIL) == OPC_QINS_OK) {
		sv_data_stat[user_priority][QUEUE_SUCC].number += 1;
		sv_data_stat[user_priority][QUEUE_SUCC].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
		hb_data_stat[user_priority][QUEUE_SUCC].number += 1;
		hb_data_stat[user_priority][QUEUE_SUCC].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
		// printf("\nt=%f,NODE_NAME=%s,MAC_STATE=%d,APP_LAYER_ENQUEUE_SUCC,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), nd_attrG[nodeid].name, mac_state, mac_attr.sendid, dest_id);
		// printf("\t  FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", DATA, user_priority, wban_norm_phy_bits(data_frame_mpdu));
		// printf(" [Node %s] t = %f, data_frame_msdu_create_time = %f, data_frame_mpdu_create_time = %f\n", \
		// 	nd_attrG[nodeid].name, op_sim_time(), op_pk_creation_time_get(data_frame_msdu), op_pk_creation_time_get(data_frame_mpdu));
		// op_prg_odb_bkpt("debug_app");
	} else {
		sv_data_stat[user_priority][QUEUE_FAIL].number += 1;
		sv_data_stat[user_priority][QUEUE_FAIL].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
		hb_data_stat[user_priority][QUEUE_FAIL].number += 1;
		hb_data_stat[user_priority][QUEUE_FAIL].ppdu_kbits += 0.001*wban_norm_phy_bits(data_frame_mpdu);
		// fprintf(log, "t=%f,nodeid=%d,MAC_STATE=%d,APP_LAYER_ENQUEUE_FAIL,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), nodeid, mac_state, mac_attr.sendid, dest_id);
		// fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", DATA, user_priority, wban_norm_phy_bits(data_frame_mpdu));
		/* destroy the packet */
		op_pk_destroy (data_frame_mpdu);
	}
	// fclose(log);
	/* try to send the packet in SF active phase */
	if ((MAC_SLEEP != mac_state) && (MAC_SETUP != mac_state)) {
		op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
	}
	FOUT;
	}

/*--------------------------------------------------------------------------------
 * Function: wban_mac_interrupt_process
 *
 * Description:	processes all the interrupts that occurs in an unforced state      
 * 
 * No parameters  
 *--------------------------------------------------------------------------------*/
static void
wban_mac_interrupt_process()
	{
	int i, j, nodeid_i = 0;
	double data_pkt_num;
	double data_pkt_latency_total;
	double data_pkt_latency_avg;
	double data_pkt_ppdu_kbits;
	double thput_msdu_kbps;
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
						/* Subqueue contains messages queued for processing. */
						/* Flush the subqueue of messages. */
					// 	op_subq_flush (SUBQ_MAN);
					// }
					if(!IAM_BAN_HUB){
						if (nd_attrG[nodeid].protocol_ver >= PAPER1) {
							// update map1_map slotnum
							reset_map1_map();
						}
						wban_battery_sleep_end(mac_state);
					}else{
						if (nd_attrG[nodeid].protocol_ver >= PAPER1) {
							resize_map_len();
						}
						/* value for the next superframe. End Device will obtain this value from beacon */
						wban_send_beacon_frame();
					}
					mac_state = MAC_SETUP;

					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f  -> ++++++++++ START OF BEACON PERIOD ++++++++++ \n\n", op_sim_time());
					// fclose(log);
					// op_prg_odb_bkpt ("beacon_end");
					break;
				};/*end of BEACON_INTERVAL_CODE */

				// map1_scheduling via Hub
				case MAP1_SCHEDULE: {
					map1_scheduling();
					break;
				}

				case INCREMENT_SLOT: {
					SF.current_slot++;
					if (SF.SD > SF.current_slot + 1) {
						op_intrpt_schedule_self (op_sim_time() + SF.slot_sec, INCREMENT_SLOT);
					}
					break;
				};

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

					// printf (" [Node %s] t=%f  -> ++++++++++  START OF THE RAP1 ++++++++++ \n\n", nd_attrG[nodeid].name, op_sim_time());
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
					// fprintf (log,"t=%f,nodeid=%d  -> ++++++++++ END OF THE RAP1 ++++++++++ \n\n", op_sim_time(), nodeid);
					// printf (" [Node %s] t=%f  -> ++++++++++  END OF THE RAP1 ++++++++++ \n\n", nd_attrG[nodeid].name, op_sim_time());
					// fclose(log);
					break;
				};/* end of END_OF_RAP1_PERIOD_CODE */

				case START_OF_MAP1_PERIOD_CODE: /* start of RAP1 Period */
				{
					if(!IAM_BAN_HUB){
						wban_battery_sleep_end(mac_state);
					}
					mac_state = MAC_MAP1;
					phase_start_timeG = SF.map1_start2sec;
					phase_end_timeG = SF.map1_end2sec;
					SF.IN_MAP_PHASE = OPC_TRUE;
					SF.IN_EAP_PHASE = OPC_FALSE;
					SF.ENABLE_TX_NEW = OPC_TRUE;
					attemptingToTX = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_FALSE;
					if(!IAM_BAN_HUB){
						map_attr.TX_state = OPC_TRUE;
					}

					// log = fopen(log_name, "a");
					// fprintf (log,"t=%f,nodeid=%d  -> ++++++++++ START OF THE MAP1 ++++++++++ \n\n", op_sim_time(), nodeid);
					// fclose(log);
					// printf (" [Node %s] t=%f  -> ++++++++++  START OF THE MAP1 ++++++++++ \n\n", nd_attrG[nodeid].name, op_sim_time());
					// printf("\t  Node %s Start MAP1 at %f, End MAP1 at %f.\n", nd_attrG[nodeid].name, phase_start_timeG, phase_end_timeG);
					// op_prg_odb_bkpt("map1_start");
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
					// fprintf (log,"t=%f,nodeid=%d  -> ++++++++++ END OF THE MAP1 ++++++++++ \n\n", op_sim_time(), nodeid);
					// fclose(log);
					// printf (" [Node %s] t=%f  -> ++++++++++  END OF THE MAP1 ++++++++++ \n\n", nd_attrG[nodeid].name, op_sim_time());
					// op_prg_odb_bkpt("map1_end");
					break;
				};/* end of END_OF_MAP1_PERIOD_CODE */

				case START_OF_SLEEP_PERIOD: /* Start of Sleep Period */
				{
					mac_state = MAC_SLEEP;
					wban_battery_sleep_start(mac_state);
					SF.ENABLE_TX_NEW = OPC_FALSE;
					// pkt_to_be_sent.enable = OPC_TRUE;
					break;
				};/* end of Start of Sleep Period */

				case TRY_PACKET_TRANSMISSION_CODE :
				{
					// SF.ENABLE_TX_NEW = OPC_TRUE;
					wban_attempt_TX();
					break;
				};

				case CCA_START_CODE: /*At the start of the CCA */
				{
					if(!can_fit_TX(&pkt_to_be_sent)) {
						// attemptingToTX = OPC_FALSE;
						// pkt_tx_out_phase++;
						break;
					}
					/* do check if the channel is idle at the start of cca */
					/* at the start the channel is assumed idle, any change to busy, the CCA will report a busy channel */
					// printf ("\nt=%f,NODE_NAME=%s,NID=%d START CCA, CW=%d,bc=%d,tx_faile_times=%d,pkt_up=%d\n",op_sim_time(), nd_attrG[nodeid].name, mac_attr.sendid, csma.CW, csma.backoff_counter, pkt_tx_fail, pkt_to_be_sent.user_priority);
					/* check at the beginning of CCA, if the channel is busy */
					csma.CCA_CHANNEL_IDLE = OPC_TRUE;
					if (op_stat_local_read (RX_BUSY_STAT) == 1.0) {
						csma.CCA_CHANNEL_IDLE = OPC_FALSE;
					}
					op_intrpt_schedule_self (op_sim_time() + pCCATime, CCA_EXPIRATION_CODE);
					break;
				};/*end of CCA_START_CODE */
			
				case CCA_EXPIRATION_CODE :/*At the end of the CCA */
				{
					wban_battery_cca(mac_state);
					/* bug with open-zigbee, for statwire interupt can sustain a duration */
					if ((!csma.CCA_CHANNEL_IDLE) || (op_stat_local_read (RX_BUSY_STAT) == 1.0)) {
						// printf("t=%f,%s CCA with BUSY\n", op_sim_time(), nd_attrG[nodeid].name);
						// op_intrpt_schedule_self (csma.next_slot_start, CCA_START_CODE);
						op_intrpt_schedule_self (op_sim_time()+pCSMAMACPHYTime+2*pCSMASlotLength2Sec, CCA_START_CODE);
					} else {
						csma.backoff_counter--;
						// printf("t=%f,%s CCA with IDLE, backoff_counter decrement to %d\n", op_sim_time(), nd_attrG[nodeid].name, csma.backoff_counter);
						if (csma.backoff_counter > 0) {
							// printf("\t  CCA at next available backoff boundary=%f sec\n", op_sim_time()+pCSMAMACPHYTime);
							// op_intrpt_schedule_self (wban_backoff_period_boundary_get(), CCA_START_CODE);
							op_intrpt_schedule_self (op_sim_time()+pCSMAMACPHYTime, CCA_START_CODE);
						} else {
							if(csma.backoff_counter < 0){
								// if(waitForACK) printf("waitForACK=True,");
								// if(attemptingToTX) printf("attemptingToTX=True,");
								// if(TX_ING) printf("TX_ING=True\n");
								// break;
								// csma.backoff_counter = 0;
								op_sim_end("ERROR : TRY TO SEND Packet WHILE backoff_counter < 0","PK_SEND_CODE","","");
							}
							// printf("\t  backoff_counter decrement to 0, %s start transmission at %f.\n", nd_attrG[nodeid].name, op_sim_time()+pCSMAMACPHYTime);
							// op_intrpt_schedule_self (csma.next_slot_start, START_TRANSMISSION_CODE);
							// op_prg_odb_bkpt("send_packet");
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
					if(can_fit_TX(&pkt_to_be_sent)){
						pkt_tx_total++;
						wban_send_mac_pk_to_phy(frame_MPDU_to_be_sent);
					}
					break;
				}; /*end of START_TRANSMISSION_CODE */

				case WAITING_ACK_END_CODE:	/* the timer for waiting an ACK has expired, the packet must be retransmitted */
				{
					waitForACK = OPC_FALSE;
					pkt_tx_fail++;
					// printf("\nt=%f,NODE_NAME=%s,NID=%d,mac_state=%d\n", op_sim_time(), nd_attrG[nodeid].name,mac_attr.sendid,mac_state);
					// printf("\t  Wait for ACK END at %f,pkt_tx_fail=%d,phase_end_timeG=%f\n", op_sim_time(), pkt_tx_fail, phase_end_timeG);
					// op_prg_odb_bkpt("debug_ack");
					if(!SF.IN_MAP_PHASE){
						if(pkt_tx_fail%2 == 0){
							csma.CW_double = OPC_TRUE;
						}else{
							csma.CW_double = OPC_FALSE;
						}
						// double the Contention Window, after every second fail.
						if (OPC_TRUE == csma.CW_double) {
							csma.CW *=2;
							if (csma.CW > CWmax[pkt_to_be_sent.user_priority]) {
								csma.CW = CWmax[pkt_to_be_sent.user_priority];
							}
							// printf("\t  csma.CW=%d doubled after pkt_tx_fail=%d\n", csma.CW, pkt_tx_fail);
						}
					}
					// check if we reached the max number and if so delete the packet
					if (pkt_tx_fail >= max_packet_tries) {
						// log = fopen(log_name, "a");
						// fprintf(log, "t=%f,nodeid=%d,MAC_STATE=%d,TX_FAIL,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), nodeid, mac_state, mac_attr.sendid, mac_attr.rcvid);
						// fprintf(log, "TX_TIME=%d,OUT_BOUND=%d,", pkt_tx_total, pkt_tx_out_phase);
						// fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", pkt_to_be_sent.frame_type, pkt_to_be_sent.frame_subtype, pkt_to_be_sent.ppdu_bits);
						// fclose(log);
						// printf("\t  Packet transmission exceeds max packet tries at time\n");
						// remove MAC frame (MPDU) frame_MPDU_to_be_sent
						// op_pk_destroy(frame_MPDU_to_be_sent);
						if(pkt_to_be_sent.frame_type == DATA){
							sv_data_stat[pkt_to_be_sent.frame_subtype][FAIL].number += 1;
							sv_data_stat[pkt_to_be_sent.frame_subtype][FAIL].ppdu_kbits += 0.001*pkt_to_be_sent.ppdu_bits;
							hb_data_stat[pkt_to_be_sent.frame_subtype][FAIL].number += 1;
							hb_data_stat[pkt_to_be_sent.frame_subtype][FAIL].ppdu_kbits += 0.001*pkt_to_be_sent.ppdu_bits;
						}
						pkt_to_be_sent.enable = OPC_FALSE;
						waitForACK = OPC_FALSE;
						TX_ING = OPC_FALSE;
						attemptingToTX = OPC_FALSE;
						pkt_tx_total = 0;
						pkt_tx_out_phase = 0;
						pkt_tx_fail = 0;
					} else {
						TX_ING = OPC_FALSE;
						// attemptingToTX = OPC_FALSE;
					}
					op_intrpt_schedule_self (op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					break;
				}; /*end of WAITING_ACK_END_CODE */
				
				case SEND_I_ACK:
					wban_send_i_ack_frame(ack_seq_num);
					break;

				case N_ACK_PACKET_SENT:
					if((MANAGEMENT == pkt_to_be_sent.frame_type) && \
					   (BEACON == pkt_to_be_sent.frame_subtype)){
						reset_pkt_snr_rx(sv_beacon_seq);
						// wban_schedule_next_beacon(); //update the superframe
					}
					TX_ING = OPC_FALSE;
					// attemptingToTX = OPC_FALSE;
					waitForACK = OPC_FALSE;
					pkt_to_be_sent.enable = OPC_FALSE;
					pkt_tx_total = 0;
					pkt_tx_out_phase = 0;
					pkt_tx_fail = 0;
					op_intrpt_schedule_self(op_sim_time(), TRY_PACKET_TRANSMISSION_CODE);
					break;

				default: break;
			} /*end of switch (op_intrpt_code())*/ 
			
			break;
		};/*end of OPC_INTRPT_SELF */
				
		case OPC_INTRPT_STAT: // statistic interrupt from PHY layer
		{
			switch (op_intrpt_stat()) {	/*begin switch (op_intrpt_stat())*/ 
				case RX_BUSY_STAT:
					/* do not receive any packets if SLEEP */
					if (mac_state == MAC_SLEEP) {
						break;
					}
					if (op_stat_local_read (RX_BUSY_STAT) == 1.0) {
						sv_t_rx_start = op_sim_time();
						/* exclude self interupt */
						if (0 == sv_tx_stat) {
							csma.CCA_CHANNEL_IDLE = OPC_FALSE;
						}
					} else {
						sv_t_rx_end = op_sim_time();
						sv_t_rx_duration = sv_t_rx_end - sv_t_rx_start;
						wban_battery_update_rx(sv_t_rx_duration, mac_state);
					}
					// printf("t=%f, NODE_NAME=%s, RX_BUSY_STAT=%f\n", op_sim_time(), nd_attrG[nodeid].name, op_stat_local_read (RX_BUSY_STAT));
					break;
				case TX_BUSY_STAT:
					/* Set TX Stat, half duplex support */
					if (op_stat_local_read (TX_BUSY_STAT) == 1.0) {
						sv_tx_stat = 1;
						sv_t_tx_start = op_sim_time();
					} else {
						sv_tx_stat = 0;
						sv_t_tx_end = op_sim_time();
						sv_t_tx_duration = sv_t_tx_end - sv_t_tx_start;
						wban_battery_update_tx(sv_t_tx_duration, mac_state);
					}
					// printf("t=%f, NODE_NAME=%s, TX_BUSY_STAT=%f\n", op_sim_time(), nd_attrG[nodeid].name, op_stat_local_read (TX_BUSY_STAT));
					break;
				case RX_COLLISION_STAT:
					if (SF.IN_MAP_PHASE && IAM_BAN_HUB) {
						// printf("t=%f, node_rx=%s, ", op_sim_time(), nd_attrG[nodeid].name);
						// printf("node_col=%s, BUSY_STAT=%f\n", nd_attrG[sv_slot_nodeid[SF.current_slot]].name, op_stat_local_read (RX_COLLISION_STAT));
						// printf("current_slot=%d,slot_nid=%d\n", SF.current_slot,sv_slot_nodeid[SF.current_slot]);
						op_prg_odb_bkpt("debug_col");
					}
					
					break;
				default : break;
			}/*end switch (op_intrpt_stat())*/
			
			break;
		};/*end of OPC_INTRPT_STAT */

		case OPC_INTRPT_ENDSIM:
		{
			data_pkt_num = 0;
			data_pkt_latency_total = 0;
			data_pkt_latency_avg = 0;
			data_pkt_ppdu_kbits = 0;
			thput_msdu_kbps = 0;
			/* If Hub process first then it will not get the real subq data info */
			subq_data_info_get();
			log = fopen(log_name, "a");
			for(i=0; i<UP_ALL; i++){
				for (j = 0; j < DATA_STATE; ++j) {
					fprintf(log, "t=%f,node_name=%s,", op_sim_time(), nd_attrG[nodeid].name);
					fprintf(log, "bid=%d,nid=%d,", nd_attrG[nodeid].bid, nd_attrG[nodeid].nid);
					fprintf(log, "nodeid=%d,stat,sv_data,", nodeid);
					fprintf(log, "up=%d,state=%d,", i, j);
					fprintf(log, "number=%f,", sv_data_stat[i][j].number);
					fprintf(log, "ppdu_kbits=%f\n", sv_data_stat[i][j].ppdu_kbits);
					if (IAM_BAN_HUB) {
						fprintf(log, "t=%f,node_name=%s,", op_sim_time(), nd_attrG[nodeid].name);
						fprintf(log, "bid=%d,nid=%d,", nd_attrG[nodeid].bid, nd_attrG[nodeid].nid);
						fprintf(log, "nodeid=%d,stat,hb_data,", nodeid);
						fprintf(log, "up=%d,state=%d,", i, j);
						fprintf(log, "number=%f,", hb_data_stat[i][j].number);
						fprintf(log, "ppdu_kbits=%f\n", hb_data_stat[i][j].ppdu_kbits);
					}
				}
				if(IAM_BAN_HUB){
					fprintf(log, "t=%f,node_name=%s,", op_sim_time(), nd_attrG[nodeid].name);
					fprintf(log, "bid=%d,nid=%d,", nd_attrG[nodeid].bid, nd_attrG[nodeid].nid);
					fprintf(log, "nodeid=%d,stat,latency,", nodeid);
					fprintf(log, "up=%d,latency_avg=%f\n", i, latency_avg[i]);
					data_pkt_num += sv_data_stat[i][RCV].number;
					data_pkt_ppdu_kbits += sv_data_stat[i][RCV].ppdu_kbits;
					data_pkt_latency_total += latency_avg[i]*sv_data_stat[i][RCV].number;
				}
			}
			if(IAM_BAN_HUB){
				thput_msdu_kbps = (data_pkt_ppdu_kbits - 0.001*data_pkt_num*HEADER_BITS)/(op_sim_time());
				fprintf(log, "t=%f,node_name=%s,", op_sim_time(), nd_attrG[nodeid].name);
				fprintf(log, "bid=%d,nid=%d,", nd_attrG[nodeid].bid, nd_attrG[nodeid].nid);
				fprintf(log, "nodeid=%d,stat,throughput,", nodeid);
				fprintf(log, "rcv_msdu_kbps=%f,", thput_msdu_kbps);
				fprintf(log, "rcv_ppdu_kbps=%f\n", data_pkt_ppdu_kbits / op_sim_time());
				/* UP=8 means the UP in total */
				data_pkt_latency_avg = data_pkt_latency_total / data_pkt_num;
				fprintf(log, "t=%f,node_name=%s,", op_sim_time(), nd_attrG[nodeid].name);
				fprintf(log, "bid=%d,nid=%d,", nd_attrG[nodeid].bid, nd_attrG[nodeid].nid);
				fprintf(log, "nodeid=%d,stat,latency,", nodeid);
				fprintf(log, "up=8,latency_avg=%f\n", data_pkt_latency_avg);
			}
			/* Energy Statistics */
			fprintf(log, "t=%f,node_name=%s,", op_sim_time(), nd_attrG[nodeid].name);
			fprintf(log, "bid=%d,nid=%d,", nd_attrG[nodeid].bid, nd_attrG[nodeid].nid);
			fprintf(log, "node_id=%d,stat,energy,", nodeid);
			fprintf(log, "tx=%f,rx=%f,", bat_attrG[nodeid].engy_tx, bat_attrG[nodeid].engy_rx);
			fprintf(log, "cca=%f,idle=%f,", bat_attrG[nodeid].engy_cca, bat_attrG[nodeid].engy_idle);
			fprintf(log, "sleep=%f,", bat_attrG[nodeid].engy_sleep);
			fprintf(log, "remainning=%f,", bat_attrG[nodeid].engy_remainning);
			fprintf(log, "total=%f\n", bat_attrG[nodeid].engy_consumed);
			fclose(log);
			// op_prg_odb_bkpt("debug_end");
			// wban_battery_end();
			break;
		};	/*end of OPC_INTRPT_ENDSIM */

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
static void
wban_extract_data_frame (Packet* frame_MPDU)
	{
	int ack_policy;
	// Packet* frame_MSDU;
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
	// op_pk_nfd_get_pkt (frame_MPDU, "MAC Frame Payload", &frame_MSDU);
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

/*--------------------------------------------------------------------------------
 * Function: wban_attempt_TX
 *
 * Description:	attempt to TX in all TX access states (EAP, RAP, MAP, CAP)
 * It will check whether we need to retransmit the current packet, or prepare
 * a new packet from the MAC data buffer to be sent.
 * 
 * Input: 
 *--------------------------------------------------------------------------------*/
static void
wban_attempt_TX()
	{
	// Packet* frame_MSDU_temp;
	// Packet* frame_MPDU_temp;
	// double conn_round;
	// double ack_tx_time;
	// int slotnum;
	/* Stack tracing enrty point */
	FIN(wban_attempt_TX);

	// printf("\nt=%f,NODE_NAME=%s,mac_state=%d\n", op_sim_time(), nd_attrG[nodeid].name, mac_state);
	if(waitForACK || attemptingToTX || TX_ING){
		// if(waitForACK) printf("\t  waitForACK=True,");
		// if(attemptingToTX) printf("\t  attemptingToTX=True,");
		// if(TX_ING) printf("\t  TX_ING=True,");
		// printf("A packet is TX\n");
		FOUT;
	}
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
		// printf("\t  A packet is TX while TX_ING=false\n");
		FOUT;
	}

	if((SF.IN_MAP_PHASE == OPC_TRUE) && (IAM_BAN_HUB)){
		// printf("Hub do not allow TX Data and Management Packet in MAP");
		FOUT;
	}
	if((pkt_to_be_sent.enable) && (pkt_tx_total < max_packet_tries)){
		/* A packet is drawn from queue but cannot tx in current phase */
		if(!can_fit_TX(&pkt_to_be_sent)){
			// printf("\t  Draw a packet from queue but cannot tx in current phase\n");
			pkt_tx_out_phase++;
			pkt_tx_total++;
			// printf("\t  csma.CW=%d,pkt_tx_fail=%d,pkt_tx_out_phase=%d\n", csma.CW, pkt_tx_fail, pkt_tx_out_phase);
			FOUT;
		}
		// printf("Retransmit with %d times in total including %d fail\n", pkt_tx_total, pkt_tx_fail);
		if(SF.IN_CAP_PHASE){
			// printf("\t  Retransmit with CSMA\n");
			wban_attempt_TX_CSMA();
		}
		if(SF.IN_MAP_PHASE){
			// printf("\t  Retransmit with Scheduling\n");
			pkt_tx_total++;
			wban_send_mac_pk_to_phy(frame_MPDU_to_be_sent);
		}
		FOUT;
	}
	/* if there is still a packet in the buffer after max tries
	 * then delete it, reset relevant variables, and collect stats.
	 */
	if((pkt_to_be_sent.enable) && (pkt_tx_fail >= max_packet_tries)){
		pkt_to_be_sent.enable = OPC_FALSE;
		pkt_tx_total = 0;
		pkt_tx_out_phase = 0;
		pkt_tx_fail = 0;
		if(pkt_to_be_sent.frame_type == DATA){
			sv_data_stat[pkt_to_be_sent.frame_subtype][FAIL].number += 1;
			sv_data_stat[pkt_to_be_sent.frame_subtype][FAIL].ppdu_kbits += 0.001*pkt_to_be_sent.ppdu_bits;
			hb_data_stat[pkt_to_be_sent.frame_subtype][FAIL].number += 1;
			hb_data_stat[pkt_to_be_sent.frame_subtype][FAIL].ppdu_kbits += 0.001*pkt_to_be_sent.ppdu_bits;
		}
	}

	// Try to draw a new packet from the data or Management buffers.
	if (!op_subq_empty(SUBQ_MAN)) {
		// if((op_sim_time() > SF.rap1_end2sec) && (!IAM_BAN_HUB)){
		// 	printf("%s cannot send MANAGEMENT frame current for over RAP1.\n", nd_attrG[nodeid].name);
		// 	op_subq_flush (SUBQ_MAN);
		// 	FOUT;
		// }
		frame_MPDU_to_be_sent = op_subq_pk_remove(SUBQ_MAN, OPC_QPOS_PRIO);
		pkt_to_be_sent.enable = OPC_TRUE;
		pkt_to_be_sent.user_priority = 6; //set up of managemant frame with 6
		csma.CW = CWmin[pkt_to_be_sent.user_priority];
		// printf("\t  Draw managemant frame.\n");
	} else if ((mac_attr.sendid != UNCONNECTED_NID) && (!op_subq_empty(SUBQ_DATA))) {
		/* obtain the pointer to MAC frame (MPDU) stored in the adequate queue */
		frame_MPDU_to_be_sent = op_subq_pk_access (SUBQ_DATA, OPC_QPOS_PRIO);
		op_pk_nfd_get(frame_MPDU_to_be_sent, "Frame Subtype", &pkt_to_be_sent.user_priority);
		if (SF.IN_EAP_PHASE) {
			// printf("\t  Node %s is in EAP phase.\n", nd_attrG[nodeid].name);
			if (7 != pkt_to_be_sent.user_priority) {
				// printf("\t  UP7 packet in the SUBQ_DATA subqueue currently\n");
				FOUT;
			}
		}
		frame_MPDU_to_be_sent = op_subq_pk_remove(SUBQ_DATA, OPC_QPOS_PRIO);
		// if((1 == nd_attrG[nodeid].protocol_ver) && (MAC_MAP1 == mac_state)){
		// 	subq_info_get(SUBQ_DATA);
		// 	slotnum = (int)(((subq_info.pksize*3*pSIFS)/SF.slot_sec) + (subq_info.bitsize + subq_info.pksize *2* MAC2PHY_BITS)/(nd_attrG[nodeid].data_rate*1000.0*SF.slot_sec));
		// 	op_pk_nfd_set (frame_MPDU_to_be_sent, "slotnum", slotnum);
		// 	// printf("\t  Requires slotnum=%d for MAP2\n", slotnum);
		// 	// op_prg_odb_bkpt("req_map2");
		// }
		pkt_to_be_sent.enable = OPC_TRUE;
	} else {
		pkt_to_be_sent.enable = OPC_FALSE;
		// printf("\t  Queue is empty or unconnected\n");
		FOUT;
	}

	op_pk_nfd_get(frame_MPDU_to_be_sent, "Sequence Number", &pkt_to_be_sent.seq_num);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Ack Policy", &pkt_to_be_sent.ack_policy);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Recipient ID", &pkt_to_be_sent.rcvid);
	op_pk_nfd_get(frame_MPDU_to_be_sent, "Frame Type", &pkt_to_be_sent.frame_type);
	
	/* for frame enqueued to subqueue before first beacon reception */
	if (pkt_to_be_sent.rcvid == HUB_ID) {
		op_pk_nfd_set(frame_MPDU_to_be_sent, "Recipient ID", mac_attr.rcvid);
		op_pk_nfd_set(frame_MPDU_to_be_sent, "Sender ID", mac_attr.sendid);
		op_pk_nfd_set(frame_MPDU_to_be_sent, "BAN ID", mac_attr.bid);
		// printf("HUB_ID, pk size of frame_MPDU=%d\n", op_pk_total_size_get(frame_MPDU_to_be_sent));
		pkt_to_be_sent.rcvid = mac_attr.rcvid;
		pkt_to_be_sent.sendid = mac_attr.sendid;
	}
	
	// if we found a packet in any of the buffers, try to TX it.
	if(pkt_to_be_sent.enable){
		if(!can_fit_TX(&pkt_to_be_sent)){
			pkt_tx_out_phase++;
			pkt_tx_total++;
			// printf("\t  csma.CW=%d,pkt_tx_fail=%d,pkt_tx_out_phase=%d\n", csma.CW, pkt_tx_fail, pkt_tx_out_phase);
			FOUT;
		}
		pkt_tx_out_phase = 0;
		pkt_tx_total = 0;
		pkt_tx_fail = 0;
		if(SF.IN_CAP_PHASE){
			csma.CW = CWmin[pkt_to_be_sent.user_priority];
			csma.CW_double = OPC_FALSE;
			csma.backoff_counter = 0;
			wban_attempt_TX_CSMA();
		}
		if(SF.IN_MAP_PHASE){
			pkt_tx_total++;
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
static void
wban_attempt_TX_CSMA()
	{
	/* Stack tracing enrty point */
	FIN(wban_attempt_TX_CSMA);
	if(!can_fit_TX(&pkt_to_be_sent)){
		FOUT;
	}
	wban_backoff_delay_set(pkt_to_be_sent.user_priority);
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
static double
wban_backoff_period_boundary_get ()
	{
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
static void
wban_backoff_delay_set(int user_priority)
	{
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
	// printf (" [Node %s] ENTERS BACKOFF STATUS AT %f\n", nd_attrG[nodeid].name, op_sim_time());
	// printf ("  Beacon Boundary = %f\n", SF.BI_Boundary);
	// printf ("  CW = %d\n", csma.CW);
	// printf ("  Random Backoff counter = %d\n", csma.backoff_counter);
	// printf ("    + Random Backoff time  = %f sec \n", csma.backoff_time);
	// printf ("    + Phase Remaining Length = %f sec \n", phase_remaining_time);
	// printf ("  Current Time Slot = %d\n", SF.current_slot);
	// printf ("  Backoff Boundary = %f sec \n", wban_backoff_period_boundary_get());
	// printf ("  Phase Start Time     = %f sec \n", phase_start_timeG);
	// printf ("  Phase End Time     = %f sec \n", phase_end_timeG);
	// printf ("  Difference       = %f sec \n", phase_end_timeG- wban_backoff_period_boundary_get());
	// printf ("  BackOff Expiration Time  = %f sec\n", csma.backoff_expiration_time);
	// printf ("----------------------------------------------------------------------------\n\n");

		// fprintf (log, "-------------------------- BACKOFF -----------------------------------\n");
		// fprintf (log, " [Node %s] ENTERS BACKOFF STATUT AT %f\n", nd_attrG[nodeid].name, op_sim_time());
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
static Boolean
can_fit_TX (packet_to_be_sent_attributes* pkt_to_be_sentL)
	{
	double phase_remaining_time;
	double pk_tx_time;

	FIN(can_fit_TX);

	if (!pkt_to_be_sentL->enable) {
		// printf("%s has NO packet TXing\n", nd_attrG[nodeid].name);
		FRET(OPC_FALSE);
	}
	pk_tx_time = hp_tx_time(wban_norm_phy_bits(frame_MPDU_to_be_sent));
	phase_remaining_time = phase_end_timeG - op_sim_time();
	if(pkt_to_be_sentL->ack_policy != N_ACK_POLICY){
		if (compare_doubles(phase_remaining_time, pk_tx_time+hp_tx_time(I_ACK_PPDU_BITS)+pSIFS+3*MICRO) >=0) {
			FRET(OPC_TRUE);
		} else {
			// printf("%s No enough time for I_ACK_POLICY packet transmission in this phase.\n", nd_attrG[nodeid].name);
			// printf("\t   mac_state=%d,phase_end_timeG=%f\n", mac_state, phase_end_timeG);
			// FRET(OPC_FALSE);
		}
	} else {
		if (compare_doubles(phase_remaining_time, pk_tx_time+3*MICRO) >=0) {
			FRET(OPC_TRUE);
		} else {
			// printf("%s No enough time for N_ACK_POLICY packet transmission in this phase.\n", nd_attrG[nodeid].name);
			// printf("\t   mac_state=%d,phase_end_timeG=%f\n", mac_state, phase_end_timeG);
			// FRET(OPC_FALSE);
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
static void
wban_send_mac_pk_to_phy(Packet* frame_MPDU)
	{
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

	// log = fopen(log_name, "a");
	// fprintf(log, "t=%f,NODE_NAME=%s,nodeid=%d,MAC_STATE=%d,TX,SENDER_ID=%d,RECIPIENT_ID=%d,", op_sim_time(), nd_attrG[nodeid].name, nodeid, mac_state, mac_attr.sendid, mac_attr.rcvid);
	// fprintf(log, "FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", frame_type, frame_subtype, ppdu_bits);
	// fclose(log);
	// printf("\nt=%f,NODE_NAME=%s,MAC_STATE=%d,TX,SENDER_ID=%d,RECIPIENT_ID=%d\n", op_sim_time(), nd_attrG[nodeid].name, mac_state, mac_attr.sendid, mac_attr.rcvid);
	// printf("\t  FRAME_TYPE=%d,FRAME_SUBTYPE=%d,PPDU_BITS=%d\n", frame_type, frame_subtype, ppdu_bits);
	switch (frame_type) {
		case DATA:
		{
			sv_data_stat[frame_subtype][SENT].number += 1;
			sv_data_stat[frame_subtype][SENT].ppdu_kbits += 0.001*ppdu_bits;
			hb_data_stat[frame_subtype][SENT].number += 1;
			hb_data_stat[frame_subtype][SENT].ppdu_kbits += 0.001*ppdu_bits;
			break;
		}
		case MANAGEMENT:
			switch(frame_subtype){
				case BEACON:
					break;
				case CONNECTION_ASSIGNMENT:
					break;
				case CONNECTION_REQUEST:
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

	PPDU_tx_time = hp_tx_time(ppdu_bits);

	switch (ack_policy){
		case N_ACK_POLICY:
			waitForACK = OPC_FALSE;
			phy_to_radio(frame_MPDU);
			// op_intrpt_schedule_self (op_sim_time() + PPDU_tx_time + pSIFS, N_ACK_PACKET_SENT);
			op_intrpt_schedule_self (op_sim_time() + PPDU_tx_time + pSIFS, N_ACK_PACKET_SENT);
			break;
		case I_ACK_POLICY:
			ack_expire_time = op_sim_time() + PPDU_tx_time + hp_tx_time(I_ACK_PPDU_BITS) + 2 * pSIFS;
			if(ack_expire_time > phase_end_timeG){
				ack_expire_time = op_sim_time() + PPDU_tx_time + hp_tx_time(I_ACK_PPDU_BITS) + pSIFS + 3*MICRO;
			}
			waitForACK = OPC_TRUE;
			mac_attr.wait_ack_seq_num = seq_num;

			pk_create_time = op_pk_creation_time_get(frame_MPDU);
			MPDU_copy = op_pk_copy(frame_MPDU);
			op_pk_creation_time_set(MPDU_copy, pk_create_time);
			phy_to_radio(MPDU_copy);
			
			// fprintf(log,"t=%f   ----------- START TX [DEST_ID = %d, SEQ = %d, with ACK expiring at %f] %d retries  \n\n", op_sim_time(), pkt_to_be_sent.rcvid, mac_attr.wait_ack_seq_num, ack_expire_time,pkt_tx_total+pkt_tx_out_phase);
			// printf("t=%f,%s start TX with ACK expiring at %f, %d tx failure\n", op_sim_time(), nd_attrG[nodeid].name, ack_expire_time, pkt_tx_fail);
			op_intrpt_schedule_self(ack_expire_time, WAITING_ACK_END_CODE);
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
static int
wban_norm_phy_bits (Packet* frame_MPDU)
	{
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
static void
phy_to_radio (Packet* frame_MPDU)
	{
	Packet* frame_PPDU;
	int bulk_size;

	/* Stack tracing enrty point */
	FIN(phy_to_radio);
	/* create PHY frame (PPDU) that encapsulates connection_request MAC frame (MPDU) */
	frame_PPDU = op_pk_create_fmt("wban_frame_PPDU_format");
	// op_pk_nfd_set (frame_PPDU, "RATE", nd_attrG[nodeid].data_rate);
	/* wrap connection_request MAC frame (MPDU) in PHY frame (PPDU) */
	op_pk_nfd_set_pkt (frame_PPDU, "PSDU", frame_MPDU);
	op_pk_nfd_set (frame_PPDU, "LENGTH", ((double) op_pk_total_size_get(frame_MPDU))/8); //[bytes]
	// bulk_size = (int)((LOG_M*BCH_CODE - 1)*N_preamble + (LOG_M*BCH_CODE*S_header - 1)*N_header);
	bulk_size = wban_norm_phy_bits(frame_MPDU) - op_pk_total_size_get(frame_PPDU);
	op_pk_bulk_size_set (frame_PPDU, bulk_size);
	if (op_stat_local_read(TX_BUSY_STAT) == 1.0){
		op_sim_end("ERROR : TRY TO SEND Packet WHILE THE TX CHANNEL IS BUSY","PK_SEND_CODE","","");
	}
	op_pk_send (frame_PPDU, STRM_FROM_MAC_TO_RADIO);
	/* Stack tracing exit point */
	FOUT;
	}

static void
wban_battery_init ()
	{
	/* Stack tracing enrty point */
	FIN(wban_battery_init ());
	/** read Battery Attributes and set the activity var **/
	op_ima_obj_attr_get (nodeid, "Voltage", &bat_attrG[nodeid].voltage);
	op_ima_obj_attr_get (nodeid, "Init Energy", &bat_attrG[nodeid].engy_init);
	bat_attrG[nodeid].engy_remainning = bat_attrG[nodeid].engy_init;
	bat_attrG[nodeid].engy_consumed = 0;
	/* get the current draw in different mode */
	op_ima_obj_attr_get (nodeid, "Current TX", &bat_attrG[nodeid].tx_mA);
	op_ima_obj_attr_get (nodeid, "Current RX", &bat_attrG[nodeid].rx_mA);
	op_ima_obj_attr_get (nodeid, "Current Idle", &bat_attrG[nodeid].idle_uA);
	op_ima_obj_attr_get (nodeid, "Current Sleep", &bat_attrG[nodeid].sleep_uA);
	/* init the activity variable */
	nd_attrG[nodeid].is_idle = OPC_TRUE;
	nd_attrG[nodeid].is_sleep = OPC_FALSE;
	nd_attrG[nodeid].last_idle_time = 0.0;
	nd_attrG[nodeid].sleeping_time = 0.0;
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
static void
wban_battery_update_tx (double tx_timeL, int mac_stateL)
	{
	double tx_energy, rx_energy, idle_energy, consumed_energy;
	double idle_duration;
	/* Stack tracing enrty point */
	FIN(wban_battery_update_tx);
	/* compute the consumed energy when transmitting a packet */
	tx_energy = bat_attrG[nodeid].tx_mA * MILLI * \
				tx_timeL * bat_attrG[nodeid].voltage;
	/* node can rx while tx at the same time in OPNET(duplex) */
	rx_energy = bat_attrG[nodeid].rx_mA * MILLI * \
				tx_timeL * bat_attrG[nodeid].voltage;
	/* compute the time spent by the node in idle state */
	idle_duration = op_sim_time() - nd_attrG[nodeid].last_idle_time - tx_timeL;
	// if(idle_duration < 0){
	// 	idle_duration = 0;
	// }
	idle_energy = bat_attrG[nodeid].idle_uA * MICRO * \
				  idle_duration * bat_attrG[nodeid].voltage;
	bat_attrG[nodeid].engy_tx += tx_energy;
	/* decrease rx energy in tx stage, increse it in the rx stage */
	bat_attrG[nodeid].engy_rx -= rx_energy;
	bat_attrG[nodeid].engy_idle += idle_energy;
	/* update the consumed energy with the one of in idle state */
	consumed_energy = tx_energy + idle_energy - rx_energy;
	bat_attrG[nodeid].engy_consumed += consumed_energy;
	/* update the current energy level */
	bat_attrG[nodeid].engy_remainning -= consumed_energy;
	/* update the time when the node enters the idle state */
	nd_attrG[nodeid].last_idle_time = op_sim_time();
	// printf("t=%f,NODE_NAME=%s,ENERGY,", op_sim_time(), nd_attrG[nodeid].name);
	// printf("tx=%f,rx=%f,cca=0.0,idle=%f,sleep=0.0\n", tx_energy, rx_energy, idle_energy);
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
static void
wban_battery_update_rx (double rx_timeL, int mac_stateL)
	{
	double rx_energy, idle_energy, consumed_energy;
	double idle_duration;
	/* Stack tracing enrty point */
	FIN(wban_battery_update_rx);
	/* do not add energy for receiving other nodes (Node mode) */
	if (!IAM_BAN_HUB && !waitForACK) {
		FOUT;
	}
	/* compute the consumed energy when receiving a packet */
	rx_energy = bat_attrG[nodeid].rx_mA * MILLI * \
				rx_timeL * bat_attrG[nodeid].voltage;
	/* compute the time spent by the node in idle state */
	idle_duration = op_sim_time() - nd_attrG[nodeid].last_idle_time - rx_timeL;
	/* idle energy has been calculated in tx stage */
	if (0 == compare_doubles(op_sim_time(), nd_attrG[nodeid].last_idle_time)) {
		idle_duration = 0;
	}
	idle_energy = bat_attrG[nodeid].idle_uA * MICRO * \
				  idle_duration * bat_attrG[nodeid].voltage;
	bat_attrG[nodeid].engy_rx += rx_energy;
	bat_attrG[nodeid].engy_idle += idle_energy;
	/* update the consumed energy with the one of in idle state */
	consumed_energy= rx_energy + idle_energy;
	/* update the current energy level */
	bat_attrG[nodeid].engy_consumed += consumed_energy;
	bat_attrG[nodeid].engy_remainning -= consumed_energy;
	/* update the time when the node enters the idle state */
	nd_attrG[nodeid].last_idle_time = op_sim_time();
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
static void
wban_battery_cca(int mac_stateL)
	{
	double cca_energy, idle_energy, consumed_energy;
	double idle_duration;
	/* Stack tracing enrty point */
	FIN(wban_battery_cca);
	idle_duration = op_sim_time() - nd_attrG[nodeid].last_idle_time - pCCATime;
	idle_energy = bat_attrG[nodeid].idle_uA * MICRO * \
				  idle_duration * bat_attrG[nodeid].voltage;
	/* compute the consumed energy when receiving a packet */
	cca_energy = bat_attrG[nodeid].rx_mA * MILLI * \
				 pCCATime * bat_attrG[nodeid].voltage;
	bat_attrG[nodeid].engy_cca += cca_energy;
	bat_attrG[nodeid].engy_idle += idle_energy;
	/* update the consumed energy with the one of in idle state */
	consumed_energy = cca_energy + idle_energy;
	/* update the current energy level */
	bat_attrG[nodeid].engy_consumed += consumed_energy;
	bat_attrG[nodeid].engy_remainning -= consumed_energy;
	nd_attrG[nodeid].last_idle_time = op_sim_time();
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
static void
wban_battery_sleep_start(int mac_stateL)
	{
	double idle_energy, idle_duration;
	/* Stack tracing enrty point */
	FIN(wban_battery_sleep_start);
	/* compute the time spent by the node in idle state */
	idle_duration = op_sim_time() - nd_attrG[nodeid].last_idle_time;
	/* update the consumed energy with the one of in idle state */
	idle_energy = bat_attrG[nodeid].idle_uA * MICRO * \
				  idle_duration * bat_attrG[nodeid].voltage;
	bat_attrG[nodeid].engy_idle += idle_energy;
	/* update the current energy level */
	bat_attrG[nodeid].engy_consumed += idle_energy;
	bat_attrG[nodeid].engy_remainning -= idle_energy;
	nd_attrG[nodeid].sleeping_time = op_sim_time();
	nd_attrG[nodeid].is_idle = OPC_FALSE;
	nd_attrG[nodeid].is_sleep = OPC_TRUE;
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
static void
wban_battery_sleep_end (int mac_stateL)
	{
	double sleep_duration, sleep_energy;
	/* Stack tracing enrty point */
	FIN(wban_battery_sleep_end);
	/* compute the time spent by the node in sleep state */
	sleep_duration = op_sim_time() - nd_attrG[nodeid].sleeping_time;
	/* energy consumed during sleeping mode */
	sleep_energy = bat_attrG[nodeid].sleep_uA * MICRO * \
				   sleep_duration * bat_attrG[nodeid].voltage;
	bat_attrG[nodeid].engy_sleep += sleep_energy;
	/* update the current energy level */
	bat_attrG[nodeid].engy_consumed += sleep_energy;
	bat_attrG[nodeid].engy_remainning -= sleep_energy;
	nd_attrG[nodeid].last_idle_time = op_sim_time();
	nd_attrG[nodeid].is_idle = OPC_TRUE;
	nd_attrG[nodeid].is_sleep = OPC_FALSE;
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
static void
queue_status()
	{
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
		// set pk capacity of SUBQ_MAN and SUBQ_DATA
		if (SUBQ_MAN == i) {
			op_ima_obj_attr_set (subq_objid, "pk capacity", (double)mac_attr.MGMT_buffer_size);
		} else if (SUBQ_DATA == i) {
			op_ima_obj_attr_set (subq_objid, "pk capacity", (double)mac_attr.DATA_buffer_size);
		}
		/* Get current subqueue attribute settings */
		op_ima_obj_attr_get (subq_objid, "bit capacity", &bit_capacity);
		op_ima_obj_attr_get (subq_objid, "pk capacity", &pk_capacity);
		// if (op_subq_empty(i)) {
		// } else {
		// }
	}
	FOUT;
	}

/*-----------------------------------------------------------------------------
 * Function:	subq_info_get
 *
 * Description:	get the parameter of each subqueue
 *
 * Input : subq_index
 *-----------------------------------------------------------------------------*/
static void
subq_info_get (int subq_index)
	{
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
		subq_info.pksize = 0;
		subq_info.bitsize = 0;
	} else {
		subq_info.pksize = op_subq_stat (subq_index, OPC_QSTAT_PKSIZE);
		subq_info.bitsize = op_subq_stat (subq_index, OPC_QSTAT_BITSIZE);
		subq_info.free_pksize = op_subq_stat (subq_index, OPC_QSTAT_FREE_PKSIZE);
		subq_info.free_bitsize = op_subq_stat (subq_index, OPC_QSTAT_FREE_BITSIZE);
		pk_test_up = op_subq_pk_access(subq_index, OPC_QPOS_PRIO);
		subq_info.up = op_pk_priority_get(pk_test_up);
	}
	FOUT;
	}

/*-----------------------------------------------------------------------------
 * Function:	subq_data_info_get
 *
 * Description:	get the parameter of data subqueue
 *
 * Input : subq_index
 *-----------------------------------------------------------------------------*/
static void
subq_data_info_get ()
	{
	int i;
	int pksize;
	int up;
	Packet * pk_stat_info;
	FIN(subq_data_info_get);
	if (!op_subq_empty(SUBQ_DATA)) {
		pksize = (int)(op_subq_stat(SUBQ_DATA, OPC_QSTAT_PKSIZE));
		for(i=0; i<pksize; i++){
			pk_stat_info = op_subq_pk_remove(SUBQ_DATA, OPC_QPOS_PRIO);
			up = op_pk_priority_get(pk_stat_info);
			sv_data_stat[up][SUBQ].number += 1;
			sv_data_stat[up][SUBQ].ppdu_kbits += 0.001*wban_norm_phy_bits(pk_stat_info);
			hb_data_stat[up][SUBQ].number += 1;
			hb_data_stat[up][SUBQ].ppdu_kbits += 0.001*wban_norm_phy_bits(pk_stat_info);
		}
	}
	FOUT;
	}

static int 
hp_rfind_nodeid (int nid)
	{
	int i;
	/** find node id with nid(sender_id). **/
	FIN (hp_rfind_nodeid ());
	for (i = 0; i < NODE_ALL_MAX; ++i)
		{
		if (nd_attrG[i].nid == nid) FRET(i);
		}
	FRET(-1);
	}

static double
hp_tx_time (int ppdu_bits)
	{
	FIN (hp_tx_time());
	FRET(ppdu_bits / nd_attrG[nodeid].data_rate);
	}

static void
reset_pkt_snr_rx(int seq)
	{
	int i = 0;
	FIN(reset_pkt_snr_rx);
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		sv_st_pkt_rx[i][seq % SF_NUM].number = 0;
		sv_st_pkt_rx[i][seq % SF_NUM].ppdu_kbits = 0;
		sv_st_pkt_rx[i][seq % SF_NUM].snr_db = 0;
	}
	FOUT;
	}

static void
reset_slot_nid(int seq)
	{
	int i;
	FIN(reset_slot_nid);
	for (i = 0; i < seq; ++i) {
		sv_slot_nodeid[i] = 0;
	}
	FOUT;
	}

static void
reset_map1_sche_map()
	{
	int i;
	FIN(reset_map1_sche_map);
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		// same BAN ID
		if (nd_attrG[i].bid == nd_attrG[nodeid].bid) {
			map1_sche_map[i].slot_start = 0;
			map1_sche_map[i].slot_end = 0;
		}
	}
	FOUT;
	}

static void
reset_map1_pkt_thr(int seq)
	{
	int i;
	FIN(reset_map1_pkt_thr);
	for (i = 0; i < NODE_ALL_MAX; ++i) {
		// same BAN ID
		if (nd_attrG[i].bid == nd_attrG[nodeid].bid) {
			sv_st_map_pkt[i][seq % SF_NUM].slot = 0;
			sv_st_map_pkt[i][seq % SF_NUM].thr = 0;
		}
	}
	FOUT;
	}