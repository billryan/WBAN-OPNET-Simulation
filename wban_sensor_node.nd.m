MIL_3_Tfile_Hdr_ 145A 140A modeler 6 531FD307 5552B487 65 IWCT307-WS8 Administrator 0 0 none none 0 0 none EA9F035B 5143 0 0 0 0 0 0 1bcc 1                                                                                                                                                                                                                                                                                                                                                                                     Ф═gЅ      8   <   [   _  x  #к  *,  9D  Br  Bz  B~  O3       
fixed_comm                           MiscLog File Directory                          Seafile                             Seafile      D:\Seafile\wban_raw_data\      yuanbin       D:\Seafile\wban_raw_data\yuanbin      baiyu      D:\Seafile\wban_raw_data\baiyu      wenlong       D:\Seafile\wban_raw_data\wenlong      duchen      D:\Seafile\wban_raw_data\duchen         TExisting directory path for storing an appropriate log file if a logging is enabled.ЦZ             MiscLog Level                           	ESSENTIAL                             	ESSENTIAL      0      VERBOSE      1         
Log level:   0: Essential     
1: VerboseЦZ             
MACBAN ID                      1         2                                     2                3                4                5                6                7                8                9      	          10      
          11                12                13                14                15                16                   lA hub shall select a one-octet body area network identifier (BAN ID) from an integer between 0x00 and 0xFF.    )Here we set it ranging from 0x02 to 0x10.ЦZ             MiscDevice Mode                    0      Node                             Hub      Hub      Node      Node         Two possible modes:     - Hub     - Node       dHub identifies its own wban (BAN ID) and provides synchronization through  the beacons transmission.ЦZ             
MACBeacon                                   count          
          
      list   	      
            
RAP1 Start          
           
      B2 Start          
           
   
                                 @This WBAN Settings compound attributes is only useful by a Hub.    DThe slaves only wait for these attributes be means of beacon frames.   count                                                                     ЦZ             list   	          	                                                   Beacon Period Length                            (                                         32                 64      @          128      ђ             AThis value specifies the beacon orderr of the corresponding WPAN.   ЁOnly the PAN Coordinator must specify this value. Then, it will be transmitted to other nodes using the beacon frame sychronization. ЦZ             Allocation Slot Length                                                                       ЦZ             
RAP1 Start                                                                          ћThis attributes specifies the PAN ID. It is only useful for the PAN Coordinator. The other nodes will get this value by means of the beacon frames.    BOnly the PAN Coordinator should specify a unique value for its PANЦZ             RAP1 End                                                                        ЦZ             B2 Start                                                                        ЦZ             
RAP2 Start                                                                        ЦZ             Inactive Duration                                                                        ЦZ          ЦZ          ЦZ             MACSender Address                            ■   Auto Assigned                                     Auto Assigned      ■             Sender address of the node.ЦZ             MACProtocol Version                                                                      PAPER1                BASE0                  ЦZ             PHYWBAN DATA RATE                       @ј[33333   971.4                                             121.4   @^YЎЎЎЎџ          242.9   @n\╠╠╠╠═          485.7   @~[33333          971.4   @ј[33333           ЦZ             MACConnection Request                                   count          
          
      list   	      
            Allocation Length          
   
       
   
                                 GThis Connection Request compound attributes is useful by Hub and node.    count                                                                     ЦZ             list   	          	                                                   Allocation Length                                                                       ЦZ          ЦZ          ЦZ             MACConnection Assignment                                   count          
          
      list   	      
            
EAP2 Start          
           
   
                                 IThis Connection Asignment compound attributes is useful by Hub and node.    count                                                                     ЦZ             list   	          	                                                   
EAP2 Start                                                                       ЦZ          ЦZ          ЦZ             MACBeacon2                                   count          
          
      list   	      
          
                                 Beacon2 format   count                                                                     ЦZ             list   	          	                                                   CAP End                                255                                     0                 255                  ЦZ             MAP2 End                                                                      255                 35      #           ЦZ          ЦZ          ЦZ                   EnergyInitial Energy      Battery.Initial Energy                                                                                           MACMax Packet Retry      ,wban_mac.MAC Attributes [0].Max Packet Tries                                                                               EnergyPower Supply      Battery.Power Supply                                                                                          APPUP0      4Traffic Source_UP.User Priority 0 Traffic Parameters                                                        count                                                                       ЦZ             list   	          	                                                  ЦZ                       APPUP1      4Traffic Source_UP.User Priority 1 Traffic Parameters                                                        count                                                                       ЦZ             list   	          	                                                  ЦZ                       APPUP2      4Traffic Source_UP.User Priority 2 Traffic Parameters                                                        count                                                                       ЦZ             list   	          	                                                  ЦZ                       APPUP3      4Traffic Source_UP.User Priority 3 Traffic Parameters                                                        count                                                                       ЦZ             list   	          	                                                  ЦZ                       APPUP4      4Traffic Source_UP.User Priority 4 Traffic Parameters                                                        count                                                                       ЦZ             list   	          	                                                  ЦZ                       APPUP5      4Traffic Source_UP.User Priority 5 Traffic Parameters                                                        count                                                                       ЦZ             list   	          	                                                  ЦZ                       APPUP6      4Traffic Source_UP.User Priority 6 Traffic Parameters                                                        count                                                                       ЦZ             list   	          	                                                  ЦZ                       APPUP7      4Traffic Source_UP.User Priority 7 Traffic Parameters                                                        count                                                                       ЦZ             list   	          	                                                  ЦZ                       Initial Energy         @ЯЯ        2 AA Batteries (1.5V, 1600 mAh)      Max Packet Retry                       Power Supply         @         2 AA Batteries (3V)      
TIM source            none      UP0                     count          
          
      list   	      
          
      UP1                     count          
          
      list   	      
          
      UP2                     count          
          
      list   	      
          
      UP3                     count          
          
      list   	      
          
      UP4                     count          
          
      list   	      
            	MSDU Size         
   constant (1600.0)   
      
Start Time         
┐­         Infinity   
   
      UP5         
            count          
          
      list   	      
          
   
   UP6         
            count          
          
      list   	      
            	MSDU Size         
   constant (800)   
      
Start Time         
?PbMмыЕЧ       
   
   
   UP7         
            count          
          
      list   	      
          
   
   altitude         
@$             
   altitude modeling            relative to subnet-platform      	condition                      financial cost            0.00      minimized icon            circle/#708090      phase                           priority                        role                   user id                                 Ш   ╚          
   wban_mac   
       
   wban_mac_process   
          queue                   MAC Attributes         
            count          
          
      list   	      
            Batterie Life Extension         
           
      Max Packet Tries          	          	   
   
      #MAC Attributes [0].Max Packet Tries          
          
   	     R  R          
   rx   
       
            count          
          
      list   	      
            	data rate         
A-Ц           
      packet formats         
   wban_frame_PPDU_format   
      	bandwidth         
@Ъ@            
      min frequency         
@б┬            
   
   
       
   qpsk   
       ?­                                             
dra_ragain             	dra_power          
   dra_bkgnoise   
       
   
dra_inoise   
       
   dra_snr   
       
   dra_ber   
       
   	dra_error   
       
   dra_ecc   
          ra_rx                       nd_radio_receiver            џ  R          
   tx   
       
            count          
          
      list   	      
            	data rate         
A-Ц           
      packet formats         
   wban_frame_PPDU_format   
      	bandwidth         
@Ъ@            
      min frequency         
@б┬            
      power         
?Эхѕсhы       
   
   
       
   qpsk   
          dra_rxgroup             	dra_txdel          
   dra_closure   
          dra_chanmatch             
dra_tagain          
   dra_propdel   
          ra_tx                       nd_radio_transmitter          	   l   >          
   Traffic Sink   
       
   wban_sink_process   
          	processor                       «   ╚          
   Battery   
       
   wban_battery_process   
          	processor                   Current Draw         
            count          
          
      list   	      
            Receive Mode         
@8╠╠╠╠╠═   TelosB   
      Transmission Mode         
@1ffffff   TelosB (0 dBm)   
      	Idle Mode         
@:ЎЎЎЎџ   TelosB   
      
Sleep Mode         
@ffffff   TelosB   
   
   
       ?   Ш   >          
   Traffic Source_UP   
       
   wban_packet_source_up_process   
          	processor                   Destination ID          
       HUB_ID   
      "User Priority 0 Traffic Parameters         	            count          
          
      list   	      
          
   	      "User Priority 1 Traffic Parameters         	            count          
          
      list   	      
          
   	      "User Priority 2 Traffic Parameters         	            count          
          
      list   	      
          
   	      "User Priority 3 Traffic Parameters         	            count          
          
      list   	      
          
   	      "User Priority 4 Traffic Parameters         	            count          
          
      list   	      
            	MSDU Size         
   constant (800.0)   
      
Start Time         
?PbMмыЕЧ       
   
   	      "User Priority 5 Traffic Parameters         	            count          
          
      list   	      
            
Start Time         
?PbMмыЕЧ       
   
   	      "User Priority 6 Traffic Parameters         	            count          
          
      list   	      
            	MSDU Size         
   constant (800)   
      
Start Time         
┐­         Infinity   
   
   	      "User Priority 7 Traffic Parameters         	            count          
          
      list   	      
            
Start Time         
?PbMмыЕЧ       
   
   	          	            Э   ╔   Џ  R   
       
   STRM_FROM_MAC_TO_RADIO   
       
   src stream [0]   
       
   dest stream [0]   
                                                                                                nd_packet_stream                     S  R   ы   └   
       
   STRM_FROM_RADIO_TO_MAC   
       
   src stream [0]   
       
   dest stream [0]   
                                                                                                nd_packet_stream                     Ќ  I   ы   ╔          
   tx_busy_stat   
       
   channel [0]   
       
   radio transmitter.busy   
       
   
instat [0]   
                                                                                                                    н▓IГ%ћ├}              н▓IГ%ћ├}                 0                                               nd_statistic_wire                    V  J   Ч   ╠          
   rx_busy_stat   
       
   channel [0]   
       
   radio receiver.busy   
       
   
instat [1]   
                                                               
          
                                           н▓IГ%ћ├}              н▓IГ%ћ├}                 0                                               nd_statistic_wire                    Z  F   §   ─          
   collision_rx   
       
   channel [0]   
       
   radio receiver.collision status   
       
   
instat [2]   
                                                                                                                    н▓IГ%ћ├}              н▓IГ%ћ├}              
@ ђ        
                                        nd_statistic_wire          !      	      Ь   ┐   s   D   
       
   STRM_FROM_MAC_TO_SINK   
       
   src stream [1]   
       
   dest stream [0]   
                                                                                                nd_packet_stream          A   ?         з   C   з   Й   
       
   STRM_FROM_UP_TO_MAC   
       
   src stream [0]   
       
   dest stream [1]   
                                                                                                nd_packet_stream      @   D       	                  
   shape_0   
       
           
                           0              
@g░            
       
@P             
       
@n             
       
@Qђ            
          	annot_box             Annotation Palette          
E▄s:       
                                                             
   text_0   
       
      APPLICATION LAYER   
                        
          
                         
@j             
       
@Cђ            
       
@_ђ            
       
@1             
          
annot_text             Annotation Palette          
E▄sX       
                     
@└└└       
                                                                     
   text_1   
       
      UP   TRAFFIC   
                        
           
                         
@l             
       
@_@            
       
@C             
       
@5             
          
annot_text             Annotation Palette          
E▄s▒       
                     
@└└└       
                                                                     
   text_3   
       
      PHYSICAL LAYER   
                        
          
                         
@o            
       
@sѕ            
       
@Z└            
       
@*             
          
annot_text             Annotation Palette          
E▄t'       
                     
@└└└       
                                                                      
   shape_2   
       
           
                           0              
@nљ            
       
@u            
       
@r­            
       
@Q             
          	annot_box             Annotation Palette          
E▄s:       
                                                              
   shape_3   
       
           
                           0              
@l0            
       
@h­            
       
@dЯ            
       
@T@            
          	annot_box             Annotation Palette          
E▄s:       
                                                             
   text_4   
       
      	MAC LAYER   
                        
          
                         
@g`            
       
@hа            
       
@P└            
       
@*             
          
annot_text             Annotation Palette          
E▄t'       
                     
@└└└       
                                                                      
   shape_4   
       
           
                           0              
@z░            
       
@i            
       
@Zђ            
       
@T@            
          	annot_box             Annotation Palette          
E▄s:       
                                                             
   text_5   
       
      BATTERY MODULE   
                        
          
                         
@zл            
       
@e­            
       
@Zђ            
       
@9             
          
annot_text             Annotation Palette          
E▄t'       
                     
@└└└       
                                                                    