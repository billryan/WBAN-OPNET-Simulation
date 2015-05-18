/*********************************************************************************
 * The battery module computes the consumed and remaining energy levels.
 * The default values of the current draws are set to those of the MICAz mote.
**********************************************************************************/

/*--------------------------------------------------------------------------------
 * Function:	wban_battery_init
 *
 * Description:	- initialize the process
 *				- read the attributes and set the global variables
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_battery_init() {
	Objid current_draw_comp_id; 
	Objid current_draw_id;
	int protocol_ver;
	char dir_path[200];
	char date_time[30];
	char net_name[100];
	int i = 0;
	time_t rawtime;
	struct tm *p;

	/* Stack tracing enrty point */
	FIN(wban_battery_init);
	/* get the ID of this module */
	battery.own_id = op_id_self ();
	/* get the ID of the node */
	battery.parent_id = op_topo_parent (battery.own_id);
	node_id = battery.parent_id;
	
	/* get the value to check if this node is PAN Coordinator or not */
	op_ima_obj_attr_get (battery.parent_id, "Device Mode", &battery.Device_Mode);
	op_ima_obj_attr_get (battery.parent_id, "name", node_name);
	op_ima_obj_attr_get (battery.parent_id, "Log File Directory", dir_path);

	/* get the value of protocol version */
	op_ima_obj_attr_get (battery.parent_id, "Protocol Version", &protocol_ver);
	op_ima_obj_attr_get (battery.parent_id, "Log File Directory", dir_path);

	/* verification if the dir_path is a valid directory */
	if (prg_path_name_is_dir (dir_path) == PrgC_Path_Name_Is_Not_Dir) {
		op_sim_end("ERROR : Log File Directory is not valid directory name.","INVALID_DIR", "","");
	}

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

	op_ima_obj_attr_get (battery.own_id, "Power Supply", &battery.power_supply);
	op_ima_obj_attr_get (battery.own_id, "Initial Energy", &battery.initial_energy);
	op_ima_obj_attr_get (battery.own_id, "Current Draw", &current_draw_id);	
	current_draw_comp_id = op_topo_child (current_draw_id, OPC_OBJTYPE_GENERIC, 0);
	
	op_ima_obj_attr_get (current_draw_comp_id, "Receive Mode", &battery.current_rx_mA);
	op_ima_obj_attr_get (current_draw_comp_id, "Transmission Mode", &battery.current_tx_mA);
	op_ima_obj_attr_get (current_draw_comp_id, "Idle Mode", &battery.current_idle_microA);
	op_ima_obj_attr_get (current_draw_comp_id, "Sleep Mode", &battery.current_sleep_microA);
	
	battery.current_energy = battery.initial_energy;
	
	/* register the statistics that will be maintained by this model */
	// statistics.remaining_energy	= op_stat_reg ("Battery.Remaining Energy (Joule)", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	// statistics.consumed_energy	= op_stat_reg ("Battery.Consumed Energy (Joule)", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	// statisticsG.consumed_energy	= op_stat_reg ("Battery.Consumed Energy (Joule)", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	
	// op_stat_write(statistics.remaining_energy,battery.current_energy);
	// op_stat_write(statistics.consumed_energy,0.0);

	// op_stat_write(statisticsG.consumed_energy,0.0);
	
	activity.is_idle = OPC_TRUE;
	activity.is_sleep = OPC_FALSE;
	activity.last_idle_time = 0.0;
	activity.sleeping_time = 0.0;

	/* initilization for energy consume */
	energy_consume.tx = 0;
	energy_consume.rx = 0;
	energy_consume.cca = 0;
	energy_consume.idle = 0;
	energy_consume.sleep = 0;
	/* Stack tracing exit point */
	FOUT;
}

/*--------------------------------------------------------------------------------
 * Function:	wban_battery_update
 *
 * Description:	compute the energy consumed and update enregy level
 *---------> consumed energy (JOULE) = current * time * voltage  = (AMPERE * SEC * VOLT) <--------------
 *
 * No parameters
 *--------------------------------------------------------------------------------*/
static void wban_battery_update() {
	Ici * iciptr;
	double tx_time;
	double rx_time;
	int ppdu_bits;
	int mac_state;
	double consumed_energy;
	double tx_energy;
	double rx_energy;
	double cca_energy;
	double idle_energy;
	double sleep_energy;
	double idle_duration;
	double sleep_duration;
	Boolean is_BANhub;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_update);

	/* get the value to check if this node is Hub or not */
	is_BANhub = OPC_FALSE;
	if (strcmp(battery.Device_Mode, "Hub") == 0) {
		is_BANhub = OPC_TRUE;
	}

	if (op_intrpt_type() == OPC_INTRPT_REMOTE) {
		switch (op_intrpt_code()) {
			case PACKET_TX_CODE :
				/* get the ICI information associated to the remote interrupt */
				iciptr = op_intrpt_ici();
				op_ici_attr_get(iciptr, "PPDU BITS", &ppdu_bits);
				op_ici_attr_get(iciptr, "MAC STATE", &mac_state);
				op_ici_attr_get(iciptr, "TIME", &tx_time);
				op_ici_destroy(iciptr);
				/* compute the consumed energy when transmitting a packet */
				tx_energy = (battery.current_tx_mA * MILLI) * tx_time * battery.power_supply;
				/* node can rx while tx at OPNET */
				rx_energy = (battery.current_rx_mA * MILLI) * tx_time * battery.power_supply;
				/* compute the time spent by the node in idle state */
				idle_duration = op_sim_time()-tx_time-activity.last_idle_time;
				// printf("t=%f,NODE_NAME=%s,PACKET_TX_CODE,tx_time=%f,activity.last_idle_time=%f\n", op_sim_time(),node_name,tx_time,activity.last_idle_time);
				if(idle_duration < 0){
					idle_duration = 0;
				}
				idle_energy = (battery.current_idle_microA * MICRO) * idle_duration * battery.power_supply;
				energy_consume.tx += tx_energy;
				energy_consume.rx -= rx_energy;
				energy_consume.idle += idle_energy;
				/* update the consumed energy with the one of in idle state */
				consumed_energy= tx_energy + idle_energy - rx_energy;
				/* update the current energy level */
				battery.current_energy = battery.current_energy - consumed_energy;
				
				/* update the time when the node enters the idle state. we add tx_time, because the remote process is generated at the time to start transmission */
				activity.last_idle_time = op_sim_time();
				/* update the statistics */
				// op_stat_write(statistics.remaining_energy, battery.current_energy);
				// op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
				// op_stat_write(statisticsG.consumed_energy, consumed_energy);
				break;
			
			case PACKET_RX_CODE :
				/* get the ICI information associated to the remote interrupt */
				iciptr=op_intrpt_ici();
				op_ici_attr_get(iciptr, "PPDU BITS", &ppdu_bits);
				op_ici_attr_get(iciptr, "MAC STATE", &mac_state);
				op_ici_attr_get(iciptr, "TIME", &rx_time);
				op_ici_destroy(iciptr);

				/* compute the consumed energy when receiving a packet */
				rx_energy = (battery.current_rx_mA * MILLI) * rx_time * battery.power_supply;
				/* compute the time spent by the node in idle state */
				if(op_sim_time() < activity.last_idle_time){
					idle_duration = 0;
				}else{
					idle_duration = op_sim_time()-rx_time-activity.last_idle_time;
				}
				// printf("t=%f,NODE_NAME=%s,PACKET_RX_CODE,rx_time=%f,activity.last_idle_time=%f\n", op_sim_time(),node_name,rx_time,activity.last_idle_time);
				if(idle_duration < 0){
					idle_duration = 0;
				}
				// if(compare_doubles(idle_duration, 0) != 1){
				// 	idle_duration = 0;
				// }
				idle_energy = (battery.current_idle_microA * MICRO) * idle_duration * battery.power_supply;
				energy_consume.rx += rx_energy;
				energy_consume.idle += idle_energy;
				/* update the consumed energy with the one of in idle state */
				consumed_energy= rx_energy + idle_energy;
				/* update the current energy level */
				battery.current_energy = battery.current_energy - consumed_energy;
				/* update the time when the node enters the idle state */
				activity.last_idle_time = op_sim_time();
				/* the sleep time shoulde be at least at current time */
				// if(compare_doubles(activity.sleeping_time, op_sim_time()) != 1){
				// 	activity.sleeping_time = op_sim_time();
				// }
				
				/* update the statistics */
				// op_stat_write(statistics.remaining_energy, battery.current_energy);
				// op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
				// op_stat_write(statisticsG.consumed_energy, consumed_energy);
				break;

			case CCA_CODE :
				/* get the ICI information associated to the remote interrupt */
				iciptr=op_intrpt_ici();
				op_ici_attr_get(iciptr, "MAC STATE", &mac_state);
				op_ici_destroy(iciptr);
				idle_duration = op_sim_time()-activity.last_idle_time;
				// printf("t=%f,NODE_NAME=%s,CCA_CODE,activity.last_idle_time=%f\n", op_sim_time(),node_name,activity.last_idle_time);
				if(idle_duration < 0){
					idle_duration = 0;
				}
				/* compute the time spent by the node in idle state */
				// if(compare_doubles(idle_duration, 0) != 1){
				// 	idle_duration = 0;
				// }
				idle_energy = (battery.current_idle_microA * MICRO) * idle_duration * battery.power_supply;
				// if(is_BANhub){
				// 	idle_energy = (battery.current_tx_mA * MILLI) * idle_duration * battery.power_supply;
				// }
				/* compute the consumed energy when receiving a packet */
				cca_energy = (battery.current_rx_mA * MILLI) * pCCATime * battery.power_supply;
				energy_consume.cca += cca_energy;
				energy_consume.idle += idle_energy;
				/* update the consumed energy with the one of in idle state */
				consumed_energy= cca_energy +idle_energy;
				battery.current_energy = battery.current_energy - consumed_energy;
				/* update the time when the node enters the idle state. we add pCCATime, because the remote process is generated at the time to start transmission */
				/* do not update idle_time in CCA for breaking idle time while RX */
				// activity.last_idle_time = op_sim_time()+pCCATime;
				/* update the statistics */
				// op_stat_write(statistics.remaining_energy, battery.current_energy);
				// op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
				// op_stat_write(statisticsG.consumed_energy, consumed_energy);
				break;

			case START_OF_SLEEP_PERIOD_CODE :
				/* get the ICI information associated to the remote interrupt */
				iciptr=op_intrpt_ici();
				op_ici_attr_get(iciptr, "MAC STATE", &mac_state);
				op_ici_destroy(iciptr);
				/* compute the time spent by the node in idle state */
				idle_duration = op_sim_time()-activity.last_idle_time;
				// printf("t=%f,NODE_NAME=%s,START_OF_SLEEP_PERIOD_CODE,activity.last_idle_time=%f\n", op_sim_time(),node_name,activity.last_idle_time);
				if(idle_duration < 0){
					idle_duration = 0;
				}
				if(idle_duration > 0.000070){
					/* update the consumed energy with the one of in idle state */
					idle_energy= (battery.current_idle_microA * MICRO) * idle_duration * battery.power_supply;
					energy_consume.idle += idle_energy;
					/* update the current energy level */
					battery.current_energy = battery.current_energy - idle_energy;
					// op_stat_write(statistics.remaining_energy, battery.current_energy);
					// op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
					// op_stat_write(statisticsG.consumed_energy, idle_energy);
				}
				activity.sleeping_time = op_sim_time();
				activity.is_idle = OPC_FALSE;
				activity.is_sleep = OPC_TRUE;
				break;

			case END_OF_SLEEP_PERIOD_CODE :
				/* get the ICI information associated to the remote interrupt */
				iciptr=op_intrpt_ici();
				op_ici_attr_get(iciptr, "MAC STATE", &mac_state);
				op_ici_destroy(iciptr);
				/* compute the time spent by the node in sleep state */
				sleep_duration = op_sim_time()-activity.sleeping_time;
				if(sleep_duration > 0.000070){
					/* energy consumed during sleeping mode */
					sleep_energy = (battery.current_sleep_microA * MICRO) * sleep_duration * battery.power_supply;
					energy_consume.sleep += sleep_energy;
					/* update the current energy level */
					battery.current_energy = battery.current_energy - sleep_energy;
					// op_stat_write(statistics.remaining_energy, battery.current_energy);
					// op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
					// op_stat_write(statisticsG.consumed_energy, sleep_energy);
				}

				activity.last_idle_time = op_sim_time();
				activity.is_idle = OPC_TRUE;
				activity.is_sleep = OPC_FALSE;
				break;
			
			default : break;
	    }
	} else if (op_intrpt_type() == OPC_INTRPT_ENDSIM){
		log = fopen(log_name, "a");
		fprintf(log, "t=%f,NODE_NAME=%s,NODE_ID=%d,STAT,ENERGY,", op_sim_time(), node_name, node_id);
		fprintf(log, "TX=%f,RX=%f,", energy_consume.tx, energy_consume.rx);
		fprintf(log, "CCA=%f,IDLE=%f,", energy_consume.cca, energy_consume.idle);
		fprintf(log, "SLEEP=%f,", energy_consume.sleep);
		fprintf(log, "TOTAL=%f\n", battery.initial_energy - battery.current_energy);
		fclose(log);
	}
	/* Stack tracing exit point */
	FOUT;
}