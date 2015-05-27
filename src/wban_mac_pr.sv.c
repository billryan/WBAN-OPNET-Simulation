/* the attributes of the node (Device Mode, BAN ID, ...) */
wban_node_attributes	\node_attr;

/* node objid */
Objid \nodeid;

/* wireless tx objid */
Objid \radio_tx_id;

/* wireless rx objid */
Objid \radio_rx_id;

/* the Medium Access Attributes */
wban_mac_attributes	\mac_attr;

/* the MAP Attributes */
wban_map_attributes	\map_attr;

/* the attributes of csma/ca (backoff, BE, ...) */
wban_csma_attributes	\csma;

/* the attributes of the beacon (EAP, RAP, MAP...) */
beacon_attributes	\beacon_attr;

/* connection request attributes of Node */
connection_request_attributes	\conn_req_attr;

/* the parameters of the superframe structure */
wban_superframe_strucuture	\SF;

/* the parameters of packet to be sent */
packet_to_be_sent_attributes	\pkt_to_be_sent;

/* the parameters of subqueue */
subqueue_info	\subq_info;

double	\latency_avg[UP_ALL];

double	\latency_avg_all;

/* the parameters of packet to be sent in EAP for temporary */
packet_to_be_sent_attributes	\packet_eap_temp;

/* Packet to be sent pointer */
Packet *	\frame_MPDU_to_be_sent;

Packet *	\frame_PPDU_to_be_sent;

Packet *	\frame_MPDU_EAP_temp;

Packet *	\frame_PPDU_EAP;

Packet *	\CONN_REQ_MPDU;

/* a log file to store the operations for each node */
FILE*	\log;

/* log file name */
char	\log_name[250];

/* Log level */
int	\log_level;

/* the statistic vector */
wban_statistic_vector	\stat_vec;

wban_global_statistic_vector	\stat_vecG;

/* Statistic handle for the total number of generated beacons. */
Stathandle	\beacon_frame_hndl;

/* sequence number of the received packet */
int	\ack_seq_num;

int	\ack_seq_nid[NODE_MAX];

int	\max_packet_tries;

int	\pkt_tx_total;

int	\pkt_tx_fail;

/* not enough time to tx in current phase(EAP,RAP,CAP,etc...) */
int	\pkt_tx_out_phase;

/* the time when the packet enters the backoff */
double	\backoff_start_time;

/* MAP request was issued after reception of first beacon */
/* then check if Hub accepts Connection request           */
Boolean	\init_flag;

/* start time in various Phase */
double	\phase_start_timeG;

/* end time in various Phase */
double	\phase_end_timeG;

int	\mac_state;

Boolean	\waitForACK;

Boolean	\attemptingToTX;

Boolean	\TX_ING;

/* Wireless tx lock, 1 for txing, 0 otherwise */
int \sv_tx_stat;

double	\sv_t_tx_start;

double	\sv_t_tx_end;

double	\sv_t_tx_duration;

double	\sv_t_rx_start;

double	\sv_t_rx_end;

double	\sv_t_rx_duration;

/* Stat for Data */
data_stat_info	\data_stat_local[UP_ALL][DATA_STATE];

/* statistics of packet received for SNR */
st_pkt_rx	\sv_st_pkt_rx[NODE_ALL_MAX][SF_NUM];

/* statistics of packet received for MAP */
st_map_pkt  \sv_st_map_pkt[NODE_ALL_MAX][SF_NUM];

/* length of MAP and valid packets number in a SF */
st_map_pkt \sv_st_map_hub[SF_NUM];

/* record map1 mapping */
int \sv_slot_nodeid[SLOT_NUM];

/* statistics of packet collided in MAP(HUB) */
int	\sv_map_pkt_col[NODE_ALL_MAX][SF_NUM];

/* statistics of packet received in MAP(HUB) */
int	\sv_map_pkt_thr[NODE_ALL_MAX][SF_NUM];

int	\pkt_num_sf[NODE_ALL_MAX];

int	\sv_beacon_seq;

double	\sv_rho_hub[NODE_ALL_MAX];

nid_rho \sv_nid_rho[NODE_ALL_MAX];

/* SINR received by Hub(Node -> Hub) */
double	\sv_snr_hub[NODE_ALL_MAX][SF_NUM];
