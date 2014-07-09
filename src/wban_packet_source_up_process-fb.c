/*********************************************************************************
 * The Traffic source can produce unacknowledged and acknowledged MSDUs 
 * (MAC Service Dat Units = MAC Frame Payload) for CAP period.
**********************************************************************************/

/*--------------------------------------------------------------------------------
 * Function:	wban_source_init
 *
 * Description:	- initialize the process
 *				- read the values of source attributes
 *              - schedule a self interrupt that will indicate start time	
 *					for MSDU generation
 *
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_source_init() {

	Objid own_id;	/* Object ID of the surrounding processor or queue */
	Objid traffic_source_comp_id; 
	Objid traffic_source_id;
	char device_mode[20];	/* mode of the device: Hub or Node */
	
	/* Stack tracing enrty point */
	FIN(wban_source_init);

	/* obtain self object ID of the surrounding processor or queue */
	own_id = op_id_self ();

	/* obtain object ID of the parent object (node) */
	parent_id = op_topo_parent (own_id);
	
	/* set the initial number of sequence number to 0 */
	dataSN = 0;

	/* get the value to check if this node is PAN coordinator or not */
	op_ima_obj_attr_get (parent_id, "Device Mode", device_mode);

	/* get destination ID */
	op_ima_obj_attr_get (own_id, "Destination ID", &destination_id); 

	/* get the name of the node */
	op_ima_obj_attr_get (parent_id, "name", node_name);	
	

	op_ima_obj_attr_get (own_id, "User Priority 7 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	
	/* Read the values of the up7-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interarrival Time", up7_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up7_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up7_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up7_stop_time);	
		
		
	op_ima_obj_attr_get (own_id, "User Priority 5 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	
	/* Read the values of the up5-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interarrival Time", up5_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up5_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up5_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up5_stop_time);	

	/* if you are Hub do not send packets itself */
	if ((strcmp(device_mode, "Hub")==0) && (destination_id==HUB_ID)) {
		up7_start_time = SC_INFINITE_TIME;
		up5_start_time = SC_INFINITE_TIME;
	}
	
	/* Load the PDFs that will be used in computing the MSDU Interarrival Times and MSDU Sizes. */
	up7_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up7_msdu_interarrival_dist_str);
	up7_msdu_size_dist_ptr       = oms_dist_load_from_string (up7_msdu_size_dist_str);
	
	/* Load the PDFs that will be used in computing the MSDU Interarrival Times and MSDU Sizes. */
	up5_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up5_msdu_interarrival_dist_str);
	up5_msdu_size_dist_ptr       = oms_dist_load_from_string (up5_msdu_size_dist_str);

	/* Make sure we have valid start and stop times, i.e. stop time is not earlier than start time.	*/
	if ((up7_stop_time <= up7_start_time) && (up7_stop_time != SC_INFINITE_TIME)) {
		/* Stop time is earlier than start time. Disable the source.	*/
		up7_start_time = SC_INFINITE_TIME;

		/* Display an appropriate warning.								*/
		op_prg_odb_print_major ("Warning from a Traffic source model:", 
								"Although the user priority 7 generator is not disabled (start time is set to a finite value) a stop time that is not later than the start time is specified.",
								"Disabling the user priority 7 generator.", OPC_NIL);
	}
	
	/* Make sure we have valid start and stop times, i.e. stop time is not earlier than start time.	*/
	if ((up5_stop_time <= up5_start_time) && (up5_stop_time != SC_INFINITE_TIME)) {
		/* Stop time is earlier than start time. Disable the source.	*/
		up5_start_time = SC_INFINITE_TIME;

		/* Display an appropriate warning.								*/
		op_prg_odb_print_major ("Warning from a Traffic source model:", 
								"Although the user priority 5 generator is not disabled (start time is set to a finite value) a stop time that is not later than the start time is specified.",
								"Disabling the user priority 5 generator.", OPC_NIL);
	}

	/* Schedule a self interrupt that will indicate transition to next state.	*/
	if ((up7_start_time == SC_INFINITE_TIME) && (up5_start_time == SC_INFINITE_TIME)) {
		op_intrpt_schedule_self (op_sim_time (), SC_STOP);	//DISABLED
	} else {
		op_intrpt_schedule_self (op_sim_time (), SC_START); //START
		
		/* In this case, also schedule the interrupt for starting of the MSDU generation */		
		if (up7_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up7_start_time, SC_GENERATE_UP7);	//UP7_MSDU_GENERATE
		
		if (up5_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up5_start_time, SC_GENERATE_UP5);	//UP5_MSDU_GENERATE	
		
		/* In this case, also schedule the interrupt when we will stop	generating */
		/* MSDUs, unless we are configured to run until the end of the simulation. */
		if ((up7_stop_time != SC_INFINITE_TIME) && (up5_stop_time != SC_INFINITE_TIME))	{
			op_intrpt_schedule_self (max_double (up7_stop_time, up5_stop_time), SC_STOP);
		}

	}

	wban_print_parameters ();

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	 wban_print_parameters
 *
 * Description:	print the setting Traffic source parameters
 *				
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_print_parameters() {

	/* Stack tracing enrty point */	
	FIN(wban_print_parameters);

	
		printf (" User Priority 7 Traffic parameters:\n");
		printf ("\t MSDU Interarival PDF : %s\n", up7_msdu_interarrival_dist_str);
		printf ("\t MSDU Size PDF        : %s\n", up7_msdu_size_dist_str);
		if (up7_start_time == -1) {
			printf ("\t Start time           : Infinity \n");
		}
		else {
			printf ("\t Start time           : %f\n", up7_start_time);
		}
		if (up7_stop_time == -1) {
			printf ("\t Stop time            : Infinity \n");
		}
		else {
			printf ("\t Stop time            : %f\n", up7_stop_time);
		}

		printf (" User Priority 5 Traffic parameters:\n");
		printf ("\t MSDU Interarival PDF : %s\n", up5_msdu_interarrival_dist_str);
		printf ("\t MSDU Size PDF        : %s\n", up5_msdu_size_dist_str);
		if (up5_start_time == -1) {
			printf ("\t Start time           : Infinity \n");
		}
		else {
			printf ("\t Start time           : %f\n", up5_start_time);
		}
		if (up5_stop_time == -1) {
			printf ("\t Stop time            : Infinity \n");
		}
		else {
			printf ("\t Stop time            : %f\n", up5_stop_time);
		}

		if (destination_id == HUB_ID) {
			printf (" Destination ID : HUB_ID \n");
		}
		else if (destination_id == 0xFF) {
			printf (" Destination ID : Broadcast - %d (%#X) \n", destination_id, destination_id);
		}
		else {
			printf (" Destination ID : %d (%#X)\n", destination_id, destination_id);
		}
		printf ("|-----------------------------------------------------------------------------|\n\n");
	
	
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_up7_traffic_generate
 *
 * Description:	creates a MSDU requiring acknowledge based on the MSDU generation		
 *				 specifications of the source model and sends it to the lower layer.
 *
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_up7_traffic_generate() {
	
	Packet*	msdu_ptr;
	Packet*	app_traffic_ptr;

	int	msdu_size; /* size in bits */
	double next_intarr_time;	/*  interarrival time of next MSDU */
	double abs_next_intarr_time; /* absolute interarrival time of next MSDU */

	/* Stack tracing enrty point */
	FIN (wban_up7_traffic_generate);
	
	/* Generate a MSDU size outcome.		*/
	msdu_size = (int) ceil (oms_dist_outcome (up7_msdu_size_dist_ptr));
		
	/* 0 <= MAC frame body <= pMaxFrameBodyLength_Bits */
	if (msdu_size > pMaxFrameBodyLength_Bits)
		msdu_size = pMaxFrameBodyLength_Bits;	/* The size of generated MSDU is bigger than the maximum - the size is set to the maximum. */
		
	if (msdu_size < 0)
		msdu_size = 0;
	
	/* We produce unformatted packets. Create one.	*/
	msdu_ptr = op_pk_create (msdu_size);
	/* create a App traffic frame that encapsulates the msdu packet */
	app_traffic_ptr = op_pk_create_fmt ("wban_app_traffic_format");
	/* increment the data sequence number by 1 at a time */
	dataSN = (dataSN + 1) % 32768;
	op_pk_nfd_set (app_traffic_ptr, "App Sequence Number", dataSN);
	op_pk_nfd_set (app_traffic_ptr, "User Priority", 7);
	op_pk_nfd_set_pkt (app_traffic_ptr, "MSDU Payload", msdu_ptr); // wrap msdu in app traffic

	/* schedule next MSDU generation */
	next_intarr_time = oms_dist_outcome (up7_msdu_interarrival_dist_ptr);

	/* Make sure that interarrival time is not negative. In that case it will be set to 0. */
	if (next_intarr_time <0)
		next_intarr_time = 0.0;

	abs_next_intarr_time = op_sim_time () + next_intarr_time;
	
	/* Update the packet generation statistics.			*/
	op_stat_write (up7_msdus_sent_hndl, 1.0);
	op_stat_write (up7_msdus_sent_hndl, 0.0);
	op_stat_write (up7_bits_sent_hndl, (double) msdu_size);
	op_stat_write (up7_bits_sent_hndl, 0.0);
	op_stat_write (up7_bits_sent_hndlG, (double) msdu_size);
	op_stat_write (up7_bits_sent_hndlG, 0.0);
	op_stat_write (up7_msdu_size_hndl, (double) msdu_size);
	op_stat_write (up7_msdu_interarrival_time_hndl, next_intarr_time);
	op_stat_write (up7_msdus_hndl, 1.0);
	op_stat_write (up7_msdus_hndlG, 1.0);		
	
//	app_sent_msdu_bits = app_sent_msdu_bits + (msdu_size/1000.0);	
//	app_sent_msdu_nbr = app_sent_msdu_nbr + 1;
	
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	
	
		printf (" [Node %s] t= %f -> UP7 MSDU (size = %d bits) was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	
	if ((abs_next_intarr_time <= up7_stop_time) || (up7_stop_time == SC_INFINITE_TIME)) {
		up7_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP7);
		
		
			printf ("\t Next UP7 MSDU will be generated at %f\n\n", abs_next_intarr_time);
		
	}
		
	/* Stack tracing exit point */
	FOUT;

}


/*--------------------------------------------------------------------------------
 * Function:	wban_up5_traffic_generate
 *
 * Description:	creates a MSDU requiring acknowledge based on the MSDU generation		
 *				 specifications of the source model and sends it to the lower layer.
 *
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_up5_traffic_generate() {
	
	Packet*	msdu_ptr;
	Packet*	app_traffic_ptr;

	int	msdu_size; /* size in bits */
	double next_intarr_time;	/*  interarrival time of next MSDU */
	double abs_next_intarr_time; /* absolute interarrival time of next MSDU */

	/* Stack tracing enrty point */
	FIN (wban_up5_traffic_generate);
	
	/* Generate a MSDU size outcome.		*/
	msdu_size = (int) ceil (oms_dist_outcome (up5_msdu_size_dist_ptr));
		
	/* 0 <= MAC frame body <= pMaxFrameBodyLength_Bits */
	if (msdu_size > pMaxFrameBodyLength_Bits)
		msdu_size = pMaxFrameBodyLength_Bits;	/* The size of generated MSDU is bigger than the maximum - the size is set to the maximum. */
		
	if (msdu_size < 0)
		msdu_size = 0;
	
	/* We produce unformatted packets. Create one.	*/
	msdu_ptr = op_pk_create (msdu_size);
	/* create a App traffic frame that encapsulates the msdu packet */
	app_traffic_ptr = op_pk_create_fmt ("wban_app_traffic_format");
	/* increment the data sequence number by 1 at a time */
	dataSN = (dataSN + 1) % 32768;
	op_pk_nfd_set (app_traffic_ptr, "App Sequence Number", dataSN);
	op_pk_nfd_set (app_traffic_ptr, "User Priority", 5);
	op_pk_nfd_set_pkt (app_traffic_ptr, "MSDU Payload", msdu_ptr); // wrap msdu in app traffic

	/* schedule next MSDU generation */
	next_intarr_time = oms_dist_outcome (up5_msdu_interarrival_dist_ptr);

	/* Make sure that interarrival time is not negative. In that case it will be set to 0. */
	if (next_intarr_time <0)
		next_intarr_time = 0.0;

	abs_next_intarr_time = op_sim_time () + next_intarr_time;
	
	/* Update the packet generation statistics.			*/
	op_stat_write (up5_msdus_sent_hndl, 1.0);
	op_stat_write (up5_msdus_sent_hndl, 0.0);
	op_stat_write (up5_bits_sent_hndl, (double) msdu_size);
	op_stat_write (up5_bits_sent_hndl, 0.0);
	op_stat_write (up5_bits_sent_hndlG, (double) msdu_size);
	op_stat_write (up5_bits_sent_hndlG, 0.0);
	op_stat_write (up5_msdu_size_hndl, (double) msdu_size);
	op_stat_write (up5_msdu_interarrival_time_hndl, next_intarr_time);
	op_stat_write (up5_msdus_hndl, 1.0);
	op_stat_write (up5_msdus_hndlG, 1.0);		
	
//	app_sent_msdu_bits = app_sent_msdu_bits + (msdu_size/1000.0);	
//	app_sent_msdu_nbr = app_sent_msdu_nbr + 1;
	
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	

		printf (" [Node %s] t= %f -> UP5 MSDU (size = %d bits) was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	
	if ((abs_next_intarr_time <= up5_stop_time) || (up5_stop_time == SC_INFINITE_TIME)) {
		up5_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP5);
		
		
			printf ("\t Next UP5 MSDU will be generated at %f\n\n", abs_next_intarr_time);
		
	}
		
	/* Stack tracing exit point */
	FOUT;

}
