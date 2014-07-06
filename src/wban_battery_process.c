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
	char node_name[15];
	char directory_path_name[80];
	char log_name[132];
	time_t timep;
	struct tm *p;
	int protocol_ver;
	int map_schedule;
	
	/* Stack tracing enrty point */
	FIN(wban_battery_init);
	/* get the ID of this module */
	battery.own_id = op_id_self ();
	/* get the ID of the node */
	battery.parent_id = op_topo_parent (battery.own_id);	
	
	/* get the value to check if this node is PAN Coordinator or not */
	op_ima_obj_attr_get (battery.parent_id, "Device Mode", &battery.Device_Mode);
	op_ima_obj_attr_get (battery.parent_id, "name", node_name);
	op_ima_obj_attr_get (battery.parent_id, "Log File Directory", directory_path_name);

	/* get the value of protocol version */
	op_ima_obj_attr_get (battery.parent_id, "Protocol Version", &protocol_ver);
	op_ima_obj_attr_get (battery.parent_id, "MAP Schedule", &map_schedule);


	enable_log = OPC_TRUE;
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
		sprintf (log_name, "%s%s-%d%d-%d%d%d-ver%d-map%d.energy", directory_path_name, node_name, (1+p->tm_mon), p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec, protocol_ver, map_schedule);
		printf ("Log file name: %s \n\n", log_name);
		log = fopen(log_name,"w");
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
	statistics.remaining_energy	= op_stat_reg ("Battery.Remaining Energy (Joule)", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	statistics.consumed_energy	= op_stat_reg ("Battery.Consumed Energy (Joule)", OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	statisticsG.consumed_energy	= op_stat_reg ("Battery.Consumed Energy (Joule)", OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	
	op_stat_write(statistics.remaining_energy,battery.current_energy);
	op_stat_write(statistics.consumed_energy,0.0);

	op_stat_write(statisticsG.consumed_energy,0.0);
	
	activity.is_idle = OPC_TRUE;
	activity.is_sleep = OPC_FALSE;
	activity.last_idle_time = 0.0;
	activity.sleeping_time = 0.0;
	
	/* Stack tracing exit point */
	FOUT;
}


/*--------------------------------------------------------------------------------
 * Function:	wban_battery_update
 *
 * Description:	compute the energy consumed and update enregy level
 *              ---------> consumed energy (JOULE) = current * time * voltage  = (AMPERE * SEC * VOLT) <--------------
 *
 * No parameters
 *--------------------------------------------------------------------------------*/

static void wban_battery_update() {

	Ici * iciptr;
	double tx_time;
	double rx_time;
	int pksize;
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
		{
			/* get the ICI information associated to the remote interrupt */
			iciptr = op_intrpt_ici();
			
			op_ici_attr_get(iciptr, "PPDU Packet Size", &pksize);
			
			op_ici_destroy(iciptr);
			
			/* compute the transmission time of the transmitted packet */
			tx_time = pksize/WBAN_DATA_RATE;
			
			/* compute the consumed energy when transmitting a packet */
			tx_energy = (battery.current_tx_mA * MILLI) * tx_time * battery.power_supply;

			/* compute the time spent by the node in idle state */
			idle_duration = op_sim_time()-activity.last_idle_time;
			idle_energy = (battery.current_idle_microA * MICRO) * idle_duration * battery.power_supply;
			if(is_BANhub){
				idle_energy = (battery.current_tx_mA * MILLI) * idle_duration * battery.power_supply;
			}
			/* update the consumed energy with the one of in idle state */
			consumed_energy= tx_energy +idle_energy;
			fprintf(log, "ENERGY,TX_CODE,IDLE,t=%f,idle_energy=%f\n", op_sim_time(), idle_energy);
			fprintf(log, "ENERGY,TX_CODE,TX,t=%f,tx_energy=%f\n", op_sim_time()+tx_time, tx_energy);
			/* update the current energy level */
			battery.current_energy = battery.current_energy - consumed_energy;
			
			/* update the time when the node enters the idle state. we add tx_time, because the remote process is generated at the time to start transmission */
			activity.last_idle_time = op_sim_time()+tx_time;
			
			/* update the statistics */
			op_stat_write(statistics.remaining_energy, battery.current_energy);
			op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
			
			op_stat_write(statisticsG.consumed_energy, consumed_energy);
			
			break;
		}
		
		case PACKET_RX_CODE :
		{
			/* get the ICI information associated to the remote interrupt */
			iciptr=op_intrpt_ici();
			
			op_ici_attr_get(iciptr, "PPDU Packet Size",&pksize);
			
			op_ici_destroy(iciptr);
			
			// printf("battery entered into RX.\n");
			/* compute the packet size of the transmitted packet */
			rx_time = pksize/WBAN_DATA_RATE;

			/* compute the consumed energy when receiving a packet */
			rx_energy = (battery.current_rx_mA * MILLI) * rx_time * battery.power_supply;
			
			/* compute the time spent by the node in idle state */
			idle_duration = op_sim_time()-rx_time-activity.last_idle_time;
			// printf("op_sim_time()=%f, rx_time=%f, activity.last_idle_time=%f.\n", op_sim_time(), rx_time, activity.last_idle_time);
			// printf("idle_duration=%f\n", idle_duration);
			op_prg_odb_bkpt("rx_e");
			if(compare_doubles(op_sim_time(), idle_duration) != 1){
				idle_duration = 0;
				// break;
			}
			idle_energy = (battery.current_idle_microA * MICRO) * idle_duration * battery.power_supply;
			if(is_BANhub){
				idle_energy = (battery.current_tx_mA * MILLI) * idle_duration * battery.power_supply;
			}
			// printf("ENERGY,RX_CODE,IDLE,t=%f,idle_energy=%f\n", op_sim_time()-rx_time, idle_energy);
			// printf("ENERGY,RX_CODE,RX,t=%f,rx_energy=%f\n", op_sim_time(), rx_energy);

			fprintf(log, "ENERGY,RX_CODE,IDLE,t=%f,idle_energy=%f\n", op_sim_time()-rx_time, idle_energy);
			fprintf(log, "ENERGY,RX_CODE,RX,t=%f,rx_energy=%f\n", op_sim_time(), rx_energy);
			op_prg_odb_bkpt("rx_e");

			/* update the consumed energy with the one of in idle state */
			consumed_energy= rx_energy + idle_energy;
			
			/* update the current energy level */
			battery.current_energy = battery.current_energy - consumed_energy;
			
			/* update the time when the node enters the idle state */
			activity.last_idle_time = op_sim_time();
			
			/* update the statistics */
			op_stat_write(statistics.remaining_energy, battery.current_energy);
			op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
			
			op_stat_write(statisticsG.consumed_energy, consumed_energy);
			
			break;
		}

		
		case CCA_CODE :
		{
			idle_duration = op_sim_time()-activity.last_idle_time;
			/* compute the time spent by the node in idle state */
			if(compare_doubles(op_sim_time(), idle_duration) != 1){
				idle_duration = 0;
				// break;
			}
			idle_energy = (battery.current_idle_microA * MICRO) * idle_duration * battery.power_supply;
			if(is_BANhub){
				idle_energy = (battery.current_tx_mA * MILLI) * idle_duration * battery.power_supply;
			}

			/* compute the consumed energy when receiving a packet */
			cca_energy = (battery.current_rx_mA * MILLI) * pCCATime * battery.power_supply;

			/* update the consumed energy with the one of in idle state */
			consumed_energy= cca_energy +idle_energy;
			fprintf(log, "ENERGY,CCA_CODE,IDLE,t=%f,idle_energy=%f\n", op_sim_time(), idle_energy);
			fprintf(log, "ENERGY,CCA_CODE,CCA,t=%f,cca_energy=%f\n", op_sim_time()+pCCATime, cca_energy);
			/* update the current energy level */
			battery.current_energy = battery.current_energy - consumed_energy;
			
			/* update the time when the node enters the idle state. we add pCCATime, because the remote process is generated at the time to start transmission */
			activity.last_idle_time = op_sim_time()+pCCATime;
			
			/* update the statistics */
			op_stat_write(statistics.remaining_energy, battery.current_energy);
			op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
			
			op_stat_write(statisticsG.consumed_energy, consumed_energy);
			
			break;
		}

		case END_OF_SLEEP_PERIOD :
		{
			/* compute the time spent by the node in sleep state */
			sleep_duration = op_sim_time()-activity.sleeping_time;
			
			/* energy consumed during sleeping mode */
			sleep_energy = (battery.current_sleep_microA * MICRO) * sleep_duration * battery.power_supply;
			fprintf(log, "ENERGY,SLEEP_CODE,SLEEP,t=%f,sleep_energy=%f\n", op_sim_time(), sleep_energy);
			
			// printf ("END_OF_SLEEP_PERIOD: current_sleep_microA = %f, time in the sleep period = %f , consumed_energy = %f mJoule\n", battery.current_sleep_microA, sleep_duration, sleep_energy*1000);
			
			/* update the current energy level */
			battery.current_energy = battery.current_energy - sleep_energy;
			
			op_stat_write(statistics.remaining_energy, battery.current_energy);
			op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
				
			op_stat_write(statisticsG.consumed_energy, sleep_energy);

			activity.last_idle_time = op_sim_time();
			activity.is_idle = OPC_TRUE;
			activity.is_sleep = OPC_FALSE;				
			
			break;		
		}
		
		case END_OF_ACTIVE_PERIOD_CODE :
		{
			/* compute the time spent by the node in idle state */
			idle_duration = op_sim_time()-activity.last_idle_time;
			if(compare_doubles(idle_duration, 0) != 1){
				idle_duration = 0;
				break;
			}
			
			/* update the consumed energy with the one of in idle state */
			idle_energy= (battery.current_idle_microA * MICRO) * idle_duration * battery.power_supply;
			if(is_BANhub){
				idle_energy = (battery.current_tx_mA * MILLI) * idle_duration * battery.power_supply;
			}
			fprintf(log, "ENERGY,IDLE_CODE,IDLE,t=%f,idle_energy=%f\n", op_sim_time(), idle_energy);
				
			/* update the current energy level */
			battery.current_energy = battery.current_energy - idle_energy;
			
			op_stat_write(statistics.remaining_energy, battery.current_energy);
			op_stat_write(statistics.consumed_energy, battery.initial_energy-battery.current_energy);
			
			op_stat_write(statisticsG.consumed_energy, idle_energy);
			
			activity.sleeping_time = op_sim_time();
			activity.is_idle = OPC_FALSE;
			activity.is_sleep = OPC_TRUE;
			
			break;
		}

		case END_OF_SIM :
		{
			if (enable_log) {
				fprintf (log, "t=%f  ***********   GAME OVER END - OF SIMULATION  ********************  \n\n",op_sim_time());
				
				fclose(log);
			}
			break;
		}
		
		default :
		{
		};		
	  }	
	}
	
	/* Stack tracing exit point */
	FOUT;
}
