/*****************************************************************************
 * The Traffic source can produce UP0-UP7 MSDUs 
 * (MAC Service Dat Units = MAC Frame Payload) for WBAN.
*****************************************************************************/

/*----------------------------------------------------------------------------
 * Function:	wban_source_init
 *
 * Description:	- initialize the process
 *				- read the values of source attributes
 *              - schedule a self interrupt that will indicate start time	
 *					for MSDU generation
 *
 * No parameters
 *--------------------------------------------------------------------------*/
static void wban_source_init() {
	Objid own_id;	/* Object ID of the surrounding processor or queue */
	Objid tf_src_id, tf_src_comp_id;
	int dev_mode; // mode of the device: Hub(0) or Node(1)
	int i = 0; // for loop
	int stop_flag = 0;
	char up_tf_para[] = "User Priority 0 Traffic Parameters";
	
	/* Stack tracing enrty point */
	FIN(wban_source_init);

	/* obtain self object ID of the surrounding processor or queue */
	own_id = op_id_self();
	/* obtain object ID of the parent object (node) */
	parent_id = op_topo_parent(own_id);
	/* get the value to check if this node is Hub or not */
	op_ima_obj_attr_get(parent_id, "Device Mode", &dev_mode);
	/* get destination ID */
	op_ima_obj_attr_get(own_id, "Destination ID", &destination_id);
	/* get the name of the node */
	op_ima_obj_attr_get(parent_id, "name", node_name);

	// printf("%s User Priority Traffic initialization.\n", node_name);
    // char up_tf_para[] = "User Priority 0 Traffic Parameters";
	for (i = 0; i < UP_ALL; i++) {
		up_tf_para[14] = '0' + i; // '0' ~ '7'
		op_ima_obj_attr_get(own_id, up_tf_para, &tf_src_id);
		tf_src_comp_id = op_topo_child(tf_src_id, OPC_OBJTYPE_GENERIC, 0);
		/* Read the values of the upi-MSDU generation parameters */
		op_ima_obj_attr_get(tf_src_comp_id, "MSDU Interval Time", msdu_arrv_dist[i]);
		op_ima_obj_attr_get(tf_src_comp_id, "MSDU Size", msdu_size_dist[i]);
		op_ima_obj_attr_get(tf_src_comp_id, "Start Time", &msdu_start_t[i]);
		op_ima_obj_attr_get(tf_src_comp_id, "Stop Time", &msdu_stop_t[i]);
	}

	// op_ima_obj_attr_get (own_id, "User Priority 0 Traffic Parameters", &tf_src_id); 
	// tf_src_comp_id = op_topo_child (tf_src_id, OPC_OBJTYPE_GENERIC, 0);
	/* Read the values of the up0-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	// op_ima_obj_attr_get (tf_src_comp_id, "MSDU Interval Time", up0_msdu_interarrival_dist_str);
	// op_ima_obj_attr_get (tf_src_comp_id, "MSDU Size",              up0_msdu_size_dist_str);
	// op_ima_obj_attr_get (tf_src_comp_id, "Start Time",             &up0_start_time);
	// op_ima_obj_attr_get (tf_src_comp_id, "Stop Time",              &up0_stop_time);	

	/* if you are Hub do not send packets itself */
	if ((HUB == dev_mode) && (destination_id == HUB_ID)) {
		for (i = 0; i < UP_ALL; i++) {
			msdu_start_t[i] = SC_INFINITE_TIME;
		}
	}

	/* Load the PDFs that will be used in computing the 
	 * MSDU Interval Times and MSDU Sizes.
	 */
	for (i = 0; i < UP_ALL; i++) {
		msdu_arrv_dist_handle[i] = oms_dist_load_from_string(msdu_arrv_dist[i]);
		msdu_size_dist_handle[i] = oms_dist_load_from_string(msdu_size_dist[i]);
	}

	/* Load the PDFs that will be used in computing the MSDU Interval Times and MSDU Sizes. */
	// up0_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up0_msdu_interarrival_dist_str);
	// up0_msdu_size_dist_ptr       = oms_dist_load_from_string (up0_msdu_size_dist_str);

	/* Make sure we have valid start and stop times,
	 * i.e. stop time is not earlier than start time.
	 */
	for (i = 0; i < UP_ALL; i++) {
		up_tf_para[14] = '0' + i; // '0' ~ '7'
		/* Stop time is earlier than start time. Disable the source */
		if ((msdu_stop_t[i] <= msdu_start_t[i]) && 
			(msdu_stop_t[i] != SC_INFINITE_TIME)) {

			msdu_start_t[i] = SC_INFINITE_TIME;
		}
		/* Display an appropriate warning. */
		op_prg_odb_print_major("Warning from a Traffic source model:",
							   "stop time < start time! for ",
							   up_tf_para, OPC_NIL);

	}

	/* Make sure we have valid start and stop times, i.e. stop time is not earlier than start time.	*/
	// if ((up0_stop_time <= up0_start_time) && (up0_stop_time != SC_INFINITE_TIME)) {
		/* Stop time is earlier than start time. Disable the source.	*/
		// up0_start_time = SC_INFINITE_TIME;

		/* Display an appropriate warning.								*/
		// op_prg_odb_print_major ("Warning from a Traffic source model:", 
								// "Although the user priority 0 generator is not disabled (start time is set to a finite value) a stop time that is not later than the start time is specified.",
								// "Disabling the user priority 0 generator.", OPC_NIL);
	// }

    for (i = 0; i < UP_ALL; i++) {
    	stop_flag |= (msdu_start_t[i] != SC_INFINITE_TIME);
    }

    if (stop_flag == 0) {
    	op_intrpt_schedule_self(op_sim_time(), SC_STOP);
    } else {
    	op_intrpt_schedule_self(op_sim_time(), SC_START);
    	// get the traffic with the highest user priority
    	for (i = 0; i < UP_ALL; i++) {
    		if (msdu_start_t[i] > 0.000001) app_up = i;
    	}
    	/* schedule the interrupt to generate the MSDU
    	 * SC_GENERATE_UP2 = 12
    	 */
    	op_intrpt_schedule_self(msdu_start_t[app_up], SC_GEN_TF);
    	/* schedule the interrupt to stop generating */
    	if (msdu_stop_t[app_up] != SC_INFINITE_TIME)
    		op_intrpt_schedule_self(msdu_stop_t[app_up], SC_STOP);
    }

	wban_print_parameters();

	/* Stack tracing exit point */
	FOUT;
}

/*----------------------------------------------------------------------------
 * Function:	 wban_print_parameters
 *
 * Description:	print the setting Traffic source parameters
 *				
 * No parameters
 *--------------------------------------------------------------------------*/
static void wban_print_parameters() {
	int i = 0;

	/* Stack tracing enrty point */	
	FIN(wban_print_parameters);	
	for (i = 0; i < UP_ALL; i++) {
		if (compare_doubles(msdu_start_t[i], -1) == 0) {
			printf ("\t UP%d Start time: Infinity \n", i);
		} else {
			printf ("\t UP%d Start time: %f\n", i, msdu_start_t[i]);
		}
		printf ("\t MSDU Interarival PDF : %s\n", msdu_arrv_dist[i]);
		printf ("\t MSDU Size PDF        : %s\n\n", msdu_size_dist[i]);
	}
	printf("\t Final MSDU UP = %d\n", app_up);

	if (destination_id == HUB_ID) {
		// printf (" Destination ID : HUB_ID \n");
	} else if (destination_id == 0xFF) {
		// printf (" Destination ID : Broadcast\n");
	} else {
		// printf (" Destination ID : %d\n", destination_id);
	}
	/* Stack tracing exit point */
	FOUT;
}

/*----------------------------------------------------------------------------
 * Function:	wban_gen_tf_up
 *
 * Description:	creates a MSDU based on the MSDU generation	PDF and 
 *              sends it to the MAC Layer.
 *
 * input: user priority
 *--------------------------------------------------------------------------*/
static void wban_gen_tf_up(int up) {
	Packet*	msdu_ptr;
	int msdu_size; // size in bits
	double next_arrv_t;	// interarrival time of next MSDU
	double abs_next_arrv_t; // absolute interarrival time of next MSDU
	/* Stack tracing enrty point */
	FIN(wban_gen_tf_up);

	/* Generate a MSDU size outcome. */
	msdu_size = (int)ceil(oms_dist_outcome(msdu_size_dist_handle[up]));
	/* 0 <= MAC frame body <= pMaxFrameBodyLength_Bits */
	if (msdu_size > pMaxFrameBodyLength_Bits)
		msdu_size = pMaxFrameBodyLength_Bits;

	if (msdu_size < 0) msdu_size = 0;
	
	/* We produce unformatted packets. Create one. */
	msdu_ptr = op_pk_create (msdu_size);
	/* schedule next MSDU generation */
	next_arrv_t = oms_dist_outcome(msdu_arrv_dist_handle[up]);
	/* 
	 * Make sure that interarrival time is not negative.
	 * In that case it will be set to 0.
	 */
	if (next_arrv_t <0) next_arrv_t = 0.0;
	
	abs_next_arrv_t = op_sim_time() + next_arrv_t;
	
	/* Update the packet generation statistics. */
	// op_stat_write(up7_msdus_sent_hndl, 1.0);
	// op_stat_write(up7_msdus_sent_hndl, 0.0);
	// op_stat_write(up7_bits_sent_hndl, (double)msdu_size);
	// op_stat_write(up7_bits_sent_hndl, 0.0);
	// op_stat_write(up7_bits_sent_hndlG, (double)msdu_size);
	// op_stat_write(up7_bits_sent_hndlG, 0.0);
	// op_stat_write(up7_msdu_size_hndl, (double)msdu_size);
	// op_stat_write(up7_msdu_interarrival_time_hndl, next_arrv_t);
	// op_stat_write(up7_msdus_hndl, 1.0);
	// op_stat_write(up7_msdus_hndlG, 1.0);
	
	// app_sent_msdu_bits = app_sent_msdu_bits + (msdu_size/1000.0);	
	// app_sent_msdu_nbr = app_sent_msdu_nbr + 1;
	
	/* send the App traffic via the stream to the MAC layer. */
	op_pk_send(msdu_ptr, STRM_FROM_UP_TO_MAC);
	if ((abs_next_arrv_t <= msdu_stop_t[up]) ||
		(msdu_stop_t[up] == SC_INFINITE_TIME)) {

		next_msdu_evh = op_intrpt_schedule_self(abs_next_arrv_t, SC_GEN_TF);
		// printf ("\t Next MSDU will be generated at %f\n\n", abs_next_arrv_t);
	}
	/* Stack tracing exit point */
	FOUT;
}