/*********************************************************************************
 * The Traffic source can produce UP0-UP7 MSDUs 
 * (MAC Service Dat Units = MAC Frame Payload) for WBAN.
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
	double temp;
	
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

	// printf("%s User Priority Traffic initialization.\n", node_name);
	op_ima_obj_attr_get (own_id, "User Priority 7 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	/* Read the values of the up7-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interval Time", up7_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up7_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up7_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up7_stop_time);	

	op_ima_obj_attr_get (own_id, "User Priority 6 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	/* Read the values of the up6-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interval Time", up6_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up6_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up6_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up6_stop_time);	

	op_ima_obj_attr_get (own_id, "User Priority 5 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	/* Read the values of the up5-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interval Time", up5_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up5_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up5_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up5_stop_time);	

	op_ima_obj_attr_get (own_id, "User Priority 4 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	/* Read the values of the up4-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interval Time", up4_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up4_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up4_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up4_stop_time);	

	op_ima_obj_attr_get (own_id, "User Priority 3 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	/* Read the values of the up3-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interval Time", up3_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up3_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up3_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up3_stop_time);	

	op_ima_obj_attr_get (own_id, "User Priority 2 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	/* Read the values of the up2-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interval Time", up2_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up2_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up2_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up2_stop_time);	

	op_ima_obj_attr_get (own_id, "User Priority 1 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	/* Read the values of the up1-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interval Time", up1_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up1_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up1_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up1_stop_time);	

	op_ima_obj_attr_get (own_id, "User Priority 0 Traffic Parameters", &traffic_source_id); 
	traffic_source_comp_id = op_topo_child (traffic_source_id, OPC_OBJTYPE_GENERIC, 0);
	/* Read the values of the up0-MSDU generation parameters, i.e. the attribute values of the surrounding module. */
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Interval Time", up0_msdu_interarrival_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "MSDU Size",              up0_msdu_size_dist_str);
	op_ima_obj_attr_get (traffic_source_comp_id, "Start Time",             &up0_start_time);
	op_ima_obj_attr_get (traffic_source_comp_id, "Stop Time",              &up0_stop_time);	

	/* if you are Hub do not send packets itself */
	if ((strcmp(device_mode, "Hub") == 0) && (destination_id == HUB_ID)) {
		up7_start_time = SC_INFINITE_TIME;
		up6_start_time = SC_INFINITE_TIME;
		up5_start_time = SC_INFINITE_TIME;
		up4_start_time = SC_INFINITE_TIME;
		up3_start_time = SC_INFINITE_TIME;
		up2_start_time = SC_INFINITE_TIME;
		up1_start_time = SC_INFINITE_TIME;
		up0_start_time = SC_INFINITE_TIME;
	}

	/* Load the PDFs that will be used in computing the MSDU Interval Times and MSDU Sizes. */
	up7_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up7_msdu_interarrival_dist_str);
	up7_msdu_size_dist_ptr       = oms_dist_load_from_string (up7_msdu_size_dist_str);

	/* Load the PDFs that will be used in computing the MSDU Interval Times and MSDU Sizes. */
	up6_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up6_msdu_interarrival_dist_str);
	up6_msdu_size_dist_ptr       = oms_dist_load_from_string (up6_msdu_size_dist_str);

	/* Load the PDFs that will be used in computing the MSDU Interval Times and MSDU Sizes. */
	up5_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up5_msdu_interarrival_dist_str);
	up5_msdu_size_dist_ptr       = oms_dist_load_from_string (up5_msdu_size_dist_str);

	/* Load the PDFs that will be used in computing the MSDU Interval Times and MSDU Sizes. */
	up4_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up4_msdu_interarrival_dist_str);
	up4_msdu_size_dist_ptr       = oms_dist_load_from_string (up4_msdu_size_dist_str);

	/* Load the PDFs that will be used in computing the MSDU Interval Times and MSDU Sizes. */
	up3_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up3_msdu_interarrival_dist_str);
	up3_msdu_size_dist_ptr       = oms_dist_load_from_string (up3_msdu_size_dist_str);

	/* Load the PDFs that will be used in computing the MSDU Interval Times and MSDU Sizes. */
	up2_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up2_msdu_interarrival_dist_str);
	up2_msdu_size_dist_ptr       = oms_dist_load_from_string (up2_msdu_size_dist_str);

	/* Load the PDFs that will be used in computing the MSDU Interval Times and MSDU Sizes. */
	up1_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up1_msdu_interarrival_dist_str);
	up1_msdu_size_dist_ptr       = oms_dist_load_from_string (up1_msdu_size_dist_str);

	/* Load the PDFs that will be used in computing the MSDU Interval Times and MSDU Sizes. */
	up0_msdu_interarrival_dist_ptr = oms_dist_load_from_string (up0_msdu_interarrival_dist_str);
	up0_msdu_size_dist_ptr       = oms_dist_load_from_string (up0_msdu_size_dist_str);

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
	if ((up6_stop_time <= up6_start_time) && (up6_stop_time != SC_INFINITE_TIME)) {
		/* Stop time is earlier than start time. Disable the source.	*/
		up6_start_time = SC_INFINITE_TIME;

		/* Display an appropriate warning.								*/
		op_prg_odb_print_major ("Warning from a Traffic source model:", 
								"Although the user priority 6 generator is not disabled (start time is set to a finite value) a stop time that is not later than the start time is specified.",
								"Disabling the user priority 6 generator.", OPC_NIL);
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

	/* Make sure we have valid start and stop times, i.e. stop time is not earlier than start time.	*/
	if ((up4_stop_time <= up4_start_time) && (up4_stop_time != SC_INFINITE_TIME)) {
		/* Stop time is earlier than start time. Disable the source.	*/
		up4_start_time = SC_INFINITE_TIME;

		/* Display an appropriate warning.								*/
		op_prg_odb_print_major ("Warning from a Traffic source model:", 
								"Although the user priority 4 generator is not disabled (start time is set to a finite value) a stop time that is not later than the start time is specified.",
								"Disabling the user priority 4 generator.", OPC_NIL);
	}

	/* Make sure we have valid start and stop times, i.e. stop time is not earlier than start time.	*/
	if ((up3_stop_time <= up3_start_time) && (up3_stop_time != SC_INFINITE_TIME)) {
		/* Stop time is earlier than start time. Disable the source.	*/
		up3_start_time = SC_INFINITE_TIME;

		/* Display an appropriate warning.								*/
		op_prg_odb_print_major ("Warning from a Traffic source model:", 
								"Although the user priority 3 generator is not disabled (start time is set to a finite value) a stop time that is not later than the start time is specified.",
								"Disabling the user priority 3 generator.", OPC_NIL);
	}

	/* Make sure we have valid start and stop times, i.e. stop time is not earlier than start time.	*/
	if ((up2_stop_time <= up2_start_time) && (up2_stop_time != SC_INFINITE_TIME)) {
		/* Stop time is earlier than start time. Disable the source.	*/
		up2_start_time = SC_INFINITE_TIME;

		/* Display an appropriate warning.								*/
		op_prg_odb_print_major ("Warning from a Traffic source model:", 
								"Although the user priority 2 generator is not disabled (start time is set to a finite value) a stop time that is not later than the start time is specified.",
								"Disabling the user priority 2 generator.", OPC_NIL);
	}

	/* Make sure we have valid start and stop times, i.e. stop time is not earlier than start time.	*/
	if ((up1_stop_time <= up1_start_time) && (up1_stop_time != SC_INFINITE_TIME)) {
		/* Stop time is earlier than start time. Disable the source.	*/
		up1_start_time = SC_INFINITE_TIME;

		/* Display an appropriate warning.								*/
		op_prg_odb_print_major ("Warning from a Traffic source model:", 
								"Although the user priority 1 generator is not disabled (start time is set to a finite value) a stop time that is not later than the start time is specified.",
								"Disabling the user priority 1 generator.", OPC_NIL);
	}

	/* Make sure we have valid start and stop times, i.e. stop time is not earlier than start time.	*/
	if ((up0_stop_time <= up0_start_time) && (up0_stop_time != SC_INFINITE_TIME)) {
		/* Stop time is earlier than start time. Disable the source.	*/
		up0_start_time = SC_INFINITE_TIME;

		/* Display an appropriate warning.								*/
		op_prg_odb_print_major ("Warning from a Traffic source model:", 
								"Although the user priority 0 generator is not disabled (start time is set to a finite value) a stop time that is not later than the start time is specified.",
								"Disabling the user priority 0 generator.", OPC_NIL);
	}

	/* Schedule a self interrupt that will indicate transition to next state.	*/
	if ((up7_start_time == SC_INFINITE_TIME) && (up6_start_time == SC_INFINITE_TIME) 
		&& (up5_start_time == SC_INFINITE_TIME) && (up4_start_time == SC_INFINITE_TIME) 
		&& (up3_start_time == SC_INFINITE_TIME) && (up2_start_time == SC_INFINITE_TIME) 
		&& (up1_start_time == SC_INFINITE_TIME) && (up0_start_time == SC_INFINITE_TIME) 
		) {
		op_intrpt_schedule_self (op_sim_time (), SC_STOP);	//DISABLED
	} else {
		op_intrpt_schedule_self (op_sim_time (), SC_START); //START
		
		/* In this case, also schedule the interrupt for starting of the MSDU generation */		
		if (up7_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up7_start_time, SC_GENERATE_UP7);	//UP7_MSDU_GENERATE
		if (up6_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up6_start_time, SC_GENERATE_UP6);	//UP6_MSDU_GENERATE
		if (up5_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up5_start_time, SC_GENERATE_UP5);	//UP5_MSDU_GENERATE	
		if (up4_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up4_start_time, SC_GENERATE_UP4);	//UP4_MSDU_GENERATE	
		if (up3_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up3_start_time, SC_GENERATE_UP3);	//UP3_MSDU_GENERATE
		if (up2_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up2_start_time, SC_GENERATE_UP2);	//UP2_MSDU_GENERATE
		if (up1_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up1_start_time, SC_GENERATE_UP1);	//UP1_MSDU_GENERATE	
		if (up0_start_time != SC_INFINITE_TIME)
			op_intrpt_schedule_self (up0_start_time, SC_GENERATE_UP0);	//UP0_MSDU_GENERATE	

		/* In this case, also schedule the interrupt when we will stop	generating */
		/* MSDUs, unless we are configured to run until the end of the simulation. */
		if ((up7_stop_time != SC_INFINITE_TIME) && (up6_stop_time != SC_INFINITE_TIME)
			&& (up5_stop_time != SC_INFINITE_TIME) && (up4_stop_time != SC_INFINITE_TIME) 
			&& (up3_stop_time != SC_INFINITE_TIME) && (up2_stop_time != SC_INFINITE_TIME) 
			&& (up1_stop_time != SC_INFINITE_TIME) && (up0_stop_time != SC_INFINITE_TIME) 
			){
			temp = max_double(max_double(up7_stop_time, up6_stop_time), max_double(up5_stop_time, up4_stop_time));
			temp = max_double(temp, max_double(max_double(up3_stop_time, up2_stop_time), max_double(up1_stop_time, up0_stop_time)));
			op_intrpt_schedule_self (temp, SC_STOP);
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
		// printf (" User Priority 7 Traffic parameters:\n");
		// printf ("\t MSDU Interarival PDF : %s\n", up7_msdu_interarrival_dist_str);
		// printf ("\t MSDU Size PDF        : %s\n", up7_msdu_size_dist_str);
		if (up7_start_time == -1) {
			// printf ("\t Start time           : Infinity \n");
		} else {
			// printf ("\t Start time           : %f\n", up7_start_time);
		}
		if (up7_stop_time == -1) {
			// printf ("\t Stop time            : Infinity \n");
		} else {
			// printf ("\t Stop time            : %f\n", up7_stop_time);
		}

		// printf (" User Priority 6 Traffic parameters:\n");
		// printf ("\t MSDU Interarival PDF : %s\n", up6_msdu_interarrival_dist_str);
		// printf ("\t MSDU Size PDF        : %s\n", up6_msdu_size_dist_str);
		if (up6_start_time == -1) {
			// printf ("\t Start time           : Infinity \n");
		} else {
			// printf ("\t Start time           : %f\n", up6_start_time);
		}
		if (up6_stop_time == -1) {
			// printf ("\t Stop time            : Infinity \n");
		} else {
			// printf ("\t Stop time            : %f\n", up6_stop_time);
		}

		// printf (" User Priority 5 Traffic parameters:\n");
		// printf ("\t MSDU Interarival PDF : %s\n", up5_msdu_interarrival_dist_str);
		// printf ("\t MSDU Size PDF        : %s\n", up5_msdu_size_dist_str);
		if (up5_start_time == -1) {
			// printf ("\t Start time           : Infinity \n");
		} else {
			// printf ("\t Start time           : %f\n", up5_start_time);
		}
		if (up5_stop_time == -1) {
			// printf ("\t Stop time            : Infinity \n");
		} else {
			// printf ("\t Stop time            : %f\n", up5_stop_time);
		}

		// printf (" User Priority 4 Traffic parameters:\n");
		// printf ("\t MSDU Interarival PDF : %s\n", up4_msdu_interarrival_dist_str);
		// printf ("\t MSDU Size PDF        : %s\n", up4_msdu_size_dist_str);
		if (up4_start_time == -1) {
			// printf ("\t Start time           : Infinity \n");
		} else {
			// printf ("\t Start time           : %f\n", up4_start_time);
		}
		if (up4_stop_time == -1) {
			// printf ("\t Stop time            : Infinity \n");
		} else {
			// printf ("\t Stop time            : %f\n", up4_stop_time);
		}

		// printf (" User Priority 3 Traffic parameters:\n");
		// printf ("\t MSDU Interarival PDF : %s\n", up3_msdu_interarrival_dist_str);
		// printf ("\t MSDU Size PDF        : %s\n", up3_msdu_size_dist_str);
		if (up3_start_time == -1) {
			// printf ("\t Start time           : Infinity \n");
		} else {
			// printf ("\t Start time           : %f\n", up3_start_time);
		}
		if (up3_stop_time == -1) {
			// printf ("\t Stop time            : Infinity \n");
		} else {
			// printf ("\t Stop time            : %f\n", up3_stop_time);
		}

		// printf (" User Priority 2 Traffic parameters:\n");
		// printf ("\t MSDU Interarival PDF : %s\n", up2_msdu_interarrival_dist_str);
		// printf ("\t MSDU Size PDF        : %s\n", up2_msdu_size_dist_str);
		if (up2_start_time == -1) {
			// printf ("\t Start time           : Infinity \n");
		} else {
			// printf ("\t Start time           : %f\n", up2_start_time);
		}
		if (up2_stop_time == -1) {
			// printf ("\t Stop time            : Infinity \n");
		} else {
			// printf ("\t Stop time            : %f\n", up2_stop_time);
		}

		// printf (" User Priority 1 Traffic parameters:\n");
		// printf ("\t MSDU Interarival PDF : %s\n", up1_msdu_interarrival_dist_str);
		// printf ("\t MSDU Size PDF        : %s\n", up1_msdu_size_dist_str);
		if (up1_start_time == -1) {
			// printf ("\t Start time           : Infinity \n");
		} else {
			// printf ("\t Start time           : %f\n", up1_start_time);
		}
		if (up1_stop_time == -1) {
			// printf ("\t Stop time            : Infinity \n");
		} else {
			// printf ("\t Stop time            : %f\n", up1_stop_time);
		}

		// printf (" User Priority 0 Traffic parameters:\n");
		// printf ("\t MSDU Interarival PDF : %s\n", up0_msdu_interarrival_dist_str);
		// printf ("\t MSDU Size PDF        : %s\n", up0_msdu_size_dist_str);
		if (up0_start_time == -1) {
			// printf ("\t Start time           : Infinity \n");
		} else {
			// printf ("\t Start time           : %f\n", up0_start_time);
		}
		if (up0_stop_time == -1) {
			// printf ("\t Stop time            : Infinity \n");
		} else {
			// printf ("\t Stop time            : %f\n", up0_stop_time);
		}

		if (destination_id == HUB_ID) {
			// printf (" Destination ID : HUB_ID \n");
		} else if (destination_id == 0xFF) {
			// printf (" Destination ID : Broadcast - %d (%#X) \n", destination_id, destination_id);
		} else {
			// printf (" Destination ID : %d (%#X)\n", destination_id, destination_id);
		}
		// printf ("|-----------------------------------------------------------------------------|\n\n");
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
	
	// app_sent_msdu_bits = app_sent_msdu_bits + (msdu_size/1000.0);	
	// app_sent_msdu_nbr = app_sent_msdu_nbr + 1;
	
	// printf(" [Node %s] t = %f, msdu_create_time = %f, app_pkt_create_time = %f\n", \
	// 	node_name, op_sim_time(), op_pk_creation_time_get(msdu_ptr), op_pk_creation_time_get(app_traffic_ptr));
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	// printf (" [Node %s] t= %f -> UP7 MSDU (size = %d bits) \
		// was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	if ((abs_next_intarr_time <= up7_stop_time) || (up7_stop_time == SC_INFINITE_TIME)) {
		up7_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP7);
		// printf ("\t Next UP7 MSDU will be generated at %f\n\n", abs_next_intarr_time);
	}

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_up6_traffic_generate
 *
 * Description:	creates a MSDU requiring acknowledge based on the MSDU generation		
 *				 specifications of the source model and sends it to the lower layer.
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_up6_traffic_generate() {
	Packet*	msdu_ptr;
	Packet*	app_traffic_ptr;

	int	msdu_size; /* size in bits */
	double next_intarr_time;	/*  interarrival time of next MSDU */
	double abs_next_intarr_time; /* absolute interarrival time of next MSDU */

	/* Stack tracing enrty point */
	FIN (wban_up6_traffic_generate);
	
	/* Generate a MSDU size outcome.		*/
	msdu_size = (int) ceil (oms_dist_outcome (up6_msdu_size_dist_ptr));
		
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
	op_pk_nfd_set (app_traffic_ptr, "User Priority", 6);
	op_pk_nfd_set_pkt (app_traffic_ptr, "MSDU Payload", msdu_ptr); // wrap msdu in app traffic

	/* schedule next MSDU generation */
	next_intarr_time = oms_dist_outcome (up6_msdu_interarrival_dist_ptr);

	/* Make sure that interarrival time is not negative. In that case it will be set to 0. */
	if (next_intarr_time <0)
		next_intarr_time = 0.0;

	abs_next_intarr_time = op_sim_time () + next_intarr_time;
	
	// printf(" [Node %s] t = %f, msdu_create_time = %f, app_pkt_create_time = %f\n", \
	// 	node_name, op_sim_time(), op_pk_creation_time_get(msdu_ptr), op_pk_creation_time_get(app_traffic_ptr));
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	// printf (" [Node %s] t= %f -> UP7 MSDU (size = %d bits) \
		// was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	if ((abs_next_intarr_time <= up6_stop_time) || (up6_stop_time == SC_INFINITE_TIME)) {
		up6_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP6);
		// printf ("\t Next UP6 MSDU will be generated at %f\n\n", abs_next_intarr_time);
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
	
	// app_sent_msdu_bits = app_sent_msdu_bits + (msdu_size/1000.0);	
	// app_sent_msdu_nbr = app_sent_msdu_nbr + 1;
	
	// printf(" [Node %s] t = %f, msdu_create_time = %f, app_pkt_create_time = %f\n", \
	// 	node_name, op_sim_time(), op_pk_creation_time_get(msdu_ptr), op_pk_creation_time_get(app_traffic_ptr));
	
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	// printf (" [Node %s] t= %f -> UP5 MSDU (size = %d bits) \
			// was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	
	if ((abs_next_intarr_time <= up5_stop_time) || (up5_stop_time == SC_INFINITE_TIME)) {
		up5_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP5);
			// printf ("\t Next UP5 MSDU will be generated at %f\n\n", abs_next_intarr_time);
		
	}
		
	/* Stack tracing exit point */
	FOUT;

}

/*--------------------------------------------------------------------------------
 * Function:	wban_up4_traffic_generate
 *
 * Description:	creates a MSDU requiring acknowledge based on the MSDU generation		
 *				 specifications of the source model and sends it to the lower layer.
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_up4_traffic_generate() {
	Packet*	msdu_ptr;
	Packet*	app_traffic_ptr;

	int	msdu_size; /* size in bits */
	double next_intarr_time;	/*  interarrival time of next MSDU */
	double abs_next_intarr_time; /* absolute interarrival time of next MSDU */

	/* Stack tracing enrty point */
	FIN (wban_up4_traffic_generate);
	
	/* Generate a MSDU size outcome.		*/
	msdu_size = (int) ceil (oms_dist_outcome (up4_msdu_size_dist_ptr));
		
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
	op_pk_nfd_set (app_traffic_ptr, "User Priority", 4);
	op_pk_nfd_set_pkt (app_traffic_ptr, "MSDU Payload", msdu_ptr); // wrap msdu in app traffic

	/* schedule next MSDU generation */
	next_intarr_time = oms_dist_outcome (up4_msdu_interarrival_dist_ptr);

	/* Make sure that interarrival time is not negative. In that case it will be set to 0. */
	if (next_intarr_time <0)
		next_intarr_time = 0.0;

	abs_next_intarr_time = op_sim_time () + next_intarr_time;
	
	// printf(" [Node %s] t = %f, msdu_create_time = %f, app_pkt_create_time = %f\n", \
	// 	node_name, op_sim_time(), op_pk_creation_time_get(msdu_ptr), op_pk_creation_time_get(app_traffic_ptr));
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	// printf (" [Node %s] t= %f -> UP7 MSDU (size = %d bits) \
		// was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	if ((abs_next_intarr_time <= up4_stop_time) || (up4_stop_time == SC_INFINITE_TIME)) {
		up4_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP4);
		// printf ("\t Next UP4 MSDU will be generated at %f\n\n", abs_next_intarr_time);
	}

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_up3_traffic_generate
 *
 * Description:	creates a MSDU requiring acknowledge based on the MSDU generation		
 *				 specifications of the source model and sends it to the lower layer.
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_up3_traffic_generate() {
	Packet*	msdu_ptr;
	Packet*	app_traffic_ptr;

	int	msdu_size; /* size in bits */
	double next_intarr_time;	/*  interarrival time of next MSDU */
	double abs_next_intarr_time; /* absolute interarrival time of next MSDU */

	/* Stack tracing enrty point */
	FIN (wban_up3_traffic_generate);
	
	/* Generate a MSDU size outcome.		*/
	msdu_size = (int) ceil (oms_dist_outcome (up3_msdu_size_dist_ptr));
		
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
	op_pk_nfd_set (app_traffic_ptr, "User Priority", 3);
	op_pk_nfd_set_pkt (app_traffic_ptr, "MSDU Payload", msdu_ptr); // wrap msdu in app traffic

	/* schedule next MSDU generation */
	next_intarr_time = oms_dist_outcome (up3_msdu_interarrival_dist_ptr);

	/* Make sure that interarrival time is not negative. In that case it will be set to 0. */
	if (next_intarr_time <0)
		next_intarr_time = 0.0;

	abs_next_intarr_time = op_sim_time () + next_intarr_time;
	
	// printf(" [Node %s] t = %f, msdu_create_time = %f, app_pkt_create_time = %f\n", \
	// 	node_name, op_sim_time(), op_pk_creation_time_get(msdu_ptr), op_pk_creation_time_get(app_traffic_ptr));
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	// printf (" [Node %s] t= %f -> UP7 MSDU (size = %d bits) \
		// was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	if ((abs_next_intarr_time <= up3_stop_time) || (up3_stop_time == SC_INFINITE_TIME)) {
		up3_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP3);
		// printf ("\t Next UP3 MSDU will be generated at %f\n\n", abs_next_intarr_time);
	}

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_up2_traffic_generate
 *
 * Description:	creates a MSDU requiring acknowledge based on the MSDU generation		
 *				 specifications of the source model and sends it to the lower layer.
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_up2_traffic_generate() {
	Packet*	msdu_ptr;
	Packet*	app_traffic_ptr;

	int	msdu_size; /* size in bits */
	double next_intarr_time;	/*  interarrival time of next MSDU */
	double abs_next_intarr_time; /* absolute interarrival time of next MSDU */

	/* Stack tracing enrty point */
	FIN (wban_up2_traffic_generate);
	
	/* Generate a MSDU size outcome.		*/
	msdu_size = (int) ceil (oms_dist_outcome (up2_msdu_size_dist_ptr));
		
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
	op_pk_nfd_set (app_traffic_ptr, "User Priority", 2);
	op_pk_nfd_set_pkt (app_traffic_ptr, "MSDU Payload", msdu_ptr); // wrap msdu in app traffic

	/* schedule next MSDU generation */
	next_intarr_time = oms_dist_outcome (up2_msdu_interarrival_dist_ptr);

	/* Make sure that interarrival time is not negative. In that case it will be set to 0. */
	if (next_intarr_time <0)
		next_intarr_time = 0.0;

	abs_next_intarr_time = op_sim_time () + next_intarr_time;
	
	// printf(" [Node %s] t = %f, msdu_create_time = %f, app_pkt_create_time = %f\n", \
	// 	node_name, op_sim_time(), op_pk_creation_time_get(msdu_ptr), op_pk_creation_time_get(app_traffic_ptr));
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	// printf (" [Node %s] t= %f -> UP7 MSDU (size = %d bits) \
		// was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	if ((abs_next_intarr_time <= up2_stop_time) || (up2_stop_time == SC_INFINITE_TIME)) {
		up2_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP2);
		// printf ("\t Next UP2 MSDU will be generated at %f\n\n", abs_next_intarr_time);
	}

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_up1_traffic_generate
 *
 * Description:	creates a MSDU requiring acknowledge based on the MSDU generation		
 *				 specifications of the source model and sends it to the lower layer.
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_up1_traffic_generate() {
	Packet*	msdu_ptr;
	Packet*	app_traffic_ptr;

	int	msdu_size; /* size in bits */
	double next_intarr_time;	/*  interarrival time of next MSDU */
	double abs_next_intarr_time; /* absolute interarrival time of next MSDU */

	/* Stack tracing enrty point */
	FIN (wban_up1_traffic_generate);
	
	/* Generate a MSDU size outcome.		*/
	msdu_size = (int) ceil (oms_dist_outcome (up1_msdu_size_dist_ptr));
		
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
	op_pk_nfd_set (app_traffic_ptr, "User Priority", 1);
	op_pk_nfd_set_pkt (app_traffic_ptr, "MSDU Payload", msdu_ptr); // wrap msdu in app traffic

	/* schedule next MSDU generation */
	next_intarr_time = oms_dist_outcome (up1_msdu_interarrival_dist_ptr);

	/* Make sure that interarrival time is not negative. In that case it will be set to 0. */
	if (next_intarr_time <0)
		next_intarr_time = 0.0;

	abs_next_intarr_time = op_sim_time () + next_intarr_time;
	
	// printf(" [Node %s] t = %f, msdu_create_time = %f, app_pkt_create_time = %f\n", \
	// 	node_name, op_sim_time(), op_pk_creation_time_get(msdu_ptr), op_pk_creation_time_get(app_traffic_ptr));
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	// printf (" [Node %s] t= %f -> UP7 MSDU (size = %d bits) \
		// was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	if ((abs_next_intarr_time <= up1_stop_time) || (up1_stop_time == SC_INFINITE_TIME)) {
		up1_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP1);
		// printf ("\t Next UP1 MSDU will be generated at %f\n\n", abs_next_intarr_time);
	}

	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_up0_traffic_generate
 *
 * Description:	creates a MSDU requiring acknowledge based on the MSDU generation		
 *				 specifications of the source model and sends it to the lower layer.
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_up0_traffic_generate() {
	Packet*	msdu_ptr;
	Packet*	app_traffic_ptr;

	int	msdu_size; /* size in bits */
	double next_intarr_time;	/*  interarrival time of next MSDU */
	double abs_next_intarr_time; /* absolute interarrival time of next MSDU */

	/* Stack tracing enrty point */
	FIN (wban_up0_traffic_generate);
	
	/* Generate a MSDU size outcome.		*/
	msdu_size = (int) ceil (oms_dist_outcome (up0_msdu_size_dist_ptr));
		
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
	op_pk_nfd_set (app_traffic_ptr, "User Priority", 0);
	op_pk_nfd_set_pkt (app_traffic_ptr, "MSDU Payload", msdu_ptr); // wrap msdu in app traffic

	/* schedule next MSDU generation */
	next_intarr_time = oms_dist_outcome (up0_msdu_interarrival_dist_ptr);

	/* Make sure that interarrival time is not negative. In that case it will be set to 0. */
	if (next_intarr_time <0)
		next_intarr_time = 0.0;

	abs_next_intarr_time = op_sim_time () + next_intarr_time;
	
	// printf(" [Node %s] t = %f, msdu_create_time = %f, app_pkt_create_time = %f\n", \
	// 	node_name, op_sim_time(), op_pk_creation_time_get(msdu_ptr), op_pk_creation_time_get(app_traffic_ptr));
	/* send the App traffic via the stream to the lower layer.	*/
	op_pk_send (app_traffic_ptr, STRM_FROM_UP_TO_MAC);
	// printf (" [Node %s] t= %f -> UP7 MSDU (size = %d bits) \
		// was generated and sent to MAC layer.\n", node_name, op_sim_time(), msdu_size);
	
	if ((abs_next_intarr_time <= up0_stop_time) || (up0_stop_time == SC_INFINITE_TIME)) {
		up0_next_msdu_evh = op_intrpt_schedule_self (abs_next_intarr_time, SC_GENERATE_UP0);
		// printf ("\t Next UP0 MSDU will be generated at %f\n\n", abs_next_intarr_time);
	}

	/* Stack tracing exit point */
	FOUT;
}