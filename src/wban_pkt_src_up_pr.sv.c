/* App layer traffic MSDU start time */
double \msdu_start_t[UP_ALL];

/* App layer traffic MSDU stop time */
double \msdu_stop_t[UP_ALL];

/* Distribution handler of the interarrival time of the generated up-MSDUs. */
OmsT_Dist_Handle	\msdu_arrv_dist_handle[UP_ALL];

/* Distribution handler of the sizes of generated up-MSDUs. */
OmsT_Dist_Handle	\msdu_size_dist_handle[UP_ALL];

/* PDF used to determine the interarrival times of the generated up-MSDUs. */
char	\msdu_arrv_dist[UP_ALL][64];

/* PDF used to determine the sizes of generated up-MSDUs. */
char	\msdu_size_dist[UP_ALL][64];

/* Time when this source will start its up7-MSDU generation activities. */
double	\up7_start_time;

/* Time when this source will stop its up7-MSDU generation activities. */
double	\up7_stop_time;

/* PDF used to determine the interarrival times of the generated up7-MSDUs. */
OmsT_Dist_Handle	\up7_msdu_interarrival_dist_ptr;

/* PDF used to determine the sizes of generated up7-MSDUs. */
OmsT_Dist_Handle	\up7_msdu_size_dist_ptr;

/* PDF used to determine the interarrival times of the generated up7-MSDUs. */
char	\up7_msdu_interarrival_dist_str[128];

/* PDF used to determine the sizes of generated up7-MSDUs. */
char	\up7_msdu_size_dist_str[128];

/* Time when this source will start its up6-MSDU generation activities. */
double	\up6_start_time;

/* Time when this source will stop its up6-MSDU generation activities. */
double	\up6_stop_time;

/* PDF used to determine the interarrival times of the generated up6-MSDUs. */
OmsT_Dist_Handle	\up6_msdu_interarrival_dist_ptr;

/* PDF used to determine the sizes of generated up6-MSDUs. */
OmsT_Dist_Handle	\up6_msdu_size_dist_ptr;

/* PDF used to determine the interarrival times of the generated up6-MSDUs. */
char	\up6_msdu_interarrival_dist_str[128];

/* PDF used to determine the sizes of generated up6-MSDUs. */
char	\up6_msdu_size_dist_str[128];

/* Time when this source will start its up5-MSDU generation activities. */
double	\up5_start_time;

/* Time when this source will stop its up5-MSDU generation activities. */
double	\up5_stop_time;

/* PDF used to determine the interarrival times of the generated up5-MSDUs. */
OmsT_Dist_Handle	\up5_msdu_interarrival_dist_ptr;

/* PDF used to determine the sizes of generated up5-MSDUs. */
OmsT_Dist_Handle	\up5_msdu_size_dist_ptr;

/* PDF used to determine the interarrival times of the generated up5-MSDUs. */
char	\up5_msdu_interarrival_dist_str[128];

/* PDF used to determine the sizes of generated up5-MSDUs. */
char	\up5_msdu_size_dist_str[128];

/* Time when this source will start its up4-MSDU generation activities. */
double	\up4_start_time;

/* Time when this source will stop its up4-MSDU generation activities. */
double	\up4_stop_time;

/* PDF used to determine the interarrival times of the generated up4-MSDUs. */
OmsT_Dist_Handle	\up4_msdu_interarrival_dist_ptr;

/* PDF used to determine the sizes of generated up4-MSDUs. */
OmsT_Dist_Handle	\up4_msdu_size_dist_ptr;

/* PDF used to determine the interarrival times of the generated up4-MSDUs. */
char	\up4_msdu_interarrival_dist_str[128];

/* PDF used to determine the sizes of generated up4-MSDUs. */
char	\up4_msdu_size_dist_str[128];

/* Time when this source will start its up3-MSDU generation activities. */
double	\up3_start_time;

/* Time when this source will stop its up3-MSDU generation activities. */
double	\up3_stop_time;

/* PDF used to determine the interarrival times of the generated up3-MSDUs. */
OmsT_Dist_Handle	\up3_msdu_interarrival_dist_ptr;

/* PDF used to determine the sizes of generated up3-MSDUs. */
OmsT_Dist_Handle	\up3_msdu_size_dist_ptr;

/* PDF used to determine the interarrival times of the generated up3-MSDUs. */
char	\up3_msdu_interarrival_dist_str[128];

/* PDF used to determine the sizes of generated up3-MSDUs. */
char	\up3_msdu_size_dist_str[128];

/* Time when this source will start its up2-MSDU generation activities. */
double	\up2_start_time;

/* Time when this source will stop its up2-MSDU generation activities. */
double	\up2_stop_time;

/* PDF used to determine the interarrival times of the generated up2-MSDUs. */
OmsT_Dist_Handle	\up2_msdu_interarrival_dist_ptr;

/* PDF used to determine the sizes of generated up2-MSDUs. */
OmsT_Dist_Handle	\up2_msdu_size_dist_ptr;

/* PDF used to determine the interarrival times of the generated up2-MSDUs. */
char	\up2_msdu_interarrival_dist_str[128];

/* PDF used to determine the sizes of generated up2-MSDUs. */
char	\up2_msdu_size_dist_str[128];

/* Time when this source will start its up1-MSDU generation activities. */
double	\up1_start_time;

/* Time when this source will stop its up1-MSDU generation activities. */
double	\up1_stop_time;

/* PDF used to determine the interarrival times of the generated up1-MSDUs. */
OmsT_Dist_Handle	\up1_msdu_interarrival_dist_ptr;

/* PDF used to determine the sizes of generated up1-MSDUs. */
OmsT_Dist_Handle	\up1_msdu_size_dist_ptr;

/* PDF used to determine the interarrival times of the generated up1-MSDUs. */
char	\up1_msdu_interarrival_dist_str[128];

/* PDF used to determine the sizes of generated up1-MSDUs. */
char	\up1_msdu_size_dist_str[128];

/* Time when this source will start its up0-MSDU generation activities. */
double	\up0_start_time;

/* Time when this source will stop its up0-MSDU generation activities. */
double	\up0_stop_time;

/* PDF used to determine the interarrival times of the generated up0-MSDUs. */
OmsT_Dist_Handle	\up0_msdu_interarrival_dist_ptr;

/* PDF used to determine the sizes of generated up0-MSDUs. */
OmsT_Dist_Handle	\up0_msdu_size_dist_ptr;

/* PDF used to determine the interarrival times of the generated up0-MSDUs. */
char	\up0_msdu_interarrival_dist_str[128];

/* PDF used to determine the sizes of generated up0-MSDUs. */
char	\up0_msdu_size_dist_str[128];

/* Destination ID for data transmission */
int	\destination_id;

/* Sequence Number for App layer traffic packet */
int	\dataSN;

/* App UP */
int \app_up;

/* Name of the node */
char	\node_name[30];

/* Object ID of the parent object (node) */
Objid	\parent_id;

/* Is the logging to the file enabled? */
Boolean	\enable_log;

/* Statistic handle for "Traffic Sent (bits/sec)" statistic. */
Stathandle	\up7_bits_sent_hndl;

/* Statistic handle for "Traffic Sent (MSDUs/sec)" statistic. */
Stathandle	\up7_msdus_sent_hndl;

/* Statistic handle for "MSDU Size (bits)" statistic. */
Stathandle	\up7_msdu_size_hndl;

/* Statistic handle for "MSDU Interaarival Time (secs)" statistic. */
Stathandle	\up7_msdu_interarrival_time_hndl;

/* Statistic handle for "Traffic Sent (bits/sec)" statistic. */
Stathandle	\up7_bits_sent_hndlG;

/* Statistic handle for the total number of generated MSDUs. */
Stathandle	\up7_msdus_hndl;

Stathandle	\up7_msdus_hndlG;

/* Statistic handle for "Traffic Sent (bits/sec)" statistic. */
Stathandle	\up5_bits_sent_hndl;

/* Statistic handle for "Traffic Sent (MSDUs/sec)" statistic. */
Stathandle	\up5_msdus_sent_hndl;

/* Statistic handle for "MSDU Size (bits)" statistic. */
Stathandle	\up5_msdu_size_hndl;

/* Statistic handle for "MSDU Interaarival Time (secs)" statistic. */
Stathandle	\up5_msdu_interarrival_time_hndl;

/* Statistic handle for "Traffic Sent (bits/sec)" statistic. */
Stathandle	\up5_bits_sent_hndlG;

/* Statistic handle for the total number of generated MSDUs. */
Stathandle	\up5_msdus_hndl;

Stathandle	\up5_msdus_hndlG;

/* Event handle for the arrival of next user priority 7 MSDU. */
Evhandle	\next_msdu_evh;

/* Event handle for the arrival of next user priority 7 MSDU. */
Evhandle	\up7_next_msdu_evh;

/* Event handle for the arrival of next user priority 6 MSDU. */
Evhandle	\up6_next_msdu_evh;

/* Event handle for the arrival of next user priority 5 MSDU. */
Evhandle	\up5_next_msdu_evh;

/* Event handle for the arrival of next user priority 4 MSDU. */
Evhandle	\up4_next_msdu_evh;

/* Event handle for the arrival of next user priority 3 MSDU. */
Evhandle	\up3_next_msdu_evh;

/* Event handle for the arrival of next user priority 2 MSDU. */
Evhandle	\up2_next_msdu_evh;

/* Event handle for the arrival of next user priority 1 MSDU. */
Evhandle	\up1_next_msdu_evh;

/* Event handle for the arrival of next user priority 0 MSDU. */
Evhandle	\up0_next_msdu_evh;

