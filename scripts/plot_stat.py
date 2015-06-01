#!/usr/bin/env python2
#-*-coding:utf-8 -*-
import os
import sys
import numpy as np
import matplotlib.pyplot as plt
import load_data as ld

fig_dir = 'fig_out/'
raw_data_dir = 'raw_data/'
stat_dir = 'stat_data/'
stat_file = 'stat_data/stat_avg.log'

def load_stat(file_in):
    version,rate,latency,energy,thput,pkt_loss,pkt_queue = np.loadtxt(
        file_in,dtype={'names':
            ('version','rate','latency','energy','throughput','pkt_loss_rate','pkt_queue_rate'),
            'formats':('i','f','f','f','f','f','f')},comments='#',unpack=True)
    #latency_ms = latency * 1000.0
    latency_ms = latency
    pkt_loss_ratio = 100 * pkt_loss
    pkt_queue_ratio = 100 * pkt_queue
    std_indices = np.where(version == 0)
    proposal_indices = np.where(version == 1)
    
    # Latency
    plt.figure(1)
    plt.plot(rate[std_indices], latency_ms[std_indices], 'o-', linewidth=2, label="IEEE 802.15.6")
    plt.plot(rate[proposal_indices], latency_ms[proposal_indices], 'o-', linewidth=2, label="Proposal")
    plt.xlabel("Arrival Rate(kbps)")
    plt.ylabel("Latency(s)")
    plt.title('Latency')
    plt.grid()
    plt.legend(loc='upper left')
    #plt.show()
    plt.savefig(fig_dir + 'latency.png')
    plt.clf()

    # Energy 
    plt.figure(2)
    plt.plot(rate[std_indices], energy[std_indices], 'o-', linewidth=2, label="IEEE 802.15.6")
    plt.plot(rate[proposal_indices], energy[proposal_indices], 'o-', linewidth=2, label="Proposal")
    plt.xlabel("Arrival Rate(kbps)")
    plt.ylabel("Energy Consuming(Joule)")
    plt.title('Energy consumption')
    plt.grid()
    plt.legend(loc='upper left')
    #plt.show()
    plt.savefig(fig_dir + 'energy.png')
    plt.clf()

    # Throughput
    plt.figure(3)
    plt.plot(rate[std_indices], thput[std_indices], 'o-', linewidth=2, label="IEEE 802.15.6")
    plt.plot(rate[proposal_indices], thput[proposal_indices], 'o-', linewidth=2, label="Proposal")
    plt.xlabel("Arrival Rate(kbps)")
    plt.ylabel("Throughpu(kbps)")
    plt.title('Throughput')
    plt.grid()
    plt.legend(loc='upper left')
    #plt.show()
    plt.savefig(fig_dir + 'throughput.png')
    plt.clf()

    # Packet loss
    plt.figure(4)
    plt.plot(rate[std_indices], pkt_loss_ratio[std_indices], 'o-', linewidth=2, label="IEEE 802.15.6")
    plt.plot(rate[proposal_indices], pkt_loss_ratio[proposal_indices], 'o-', linewidth=2, label="Proposal")
    plt.xlabel("Arrival Rate(kbps)")
    plt.ylabel("Packet loss rate ratio(%)")
    plt.title('Packet Loss rate')
    plt.grid()
    plt.legend(loc='upper left')
    #plt.show()
    plt.savefig(fig_dir + 'packet_loss_rate.png')
    plt.clf()

    # Packet subq
    plt.figure(5)
    plt.plot(rate[std_indices], pkt_queue_ratio[std_indices], 'o-', linewidth=2, label="IEEE 802.15.6")
    plt.plot(rate[proposal_indices], pkt_queue_ratio[proposal_indices], 'o-', linewidth=2, label="Proposal")
    plt.xlabel("Arrival Rate(kbps)")
    plt.ylabel("Packets in queue ratio(%)")
    plt.title('Packet in queue')
    plt.grid()
    plt.legend(loc='upper left')
    #plt.show()
    plt.savefig(fig_dir + 'packet_in_subq.png')
    plt.clf()
    #plt.savefig('latency.png', dpi=120)
    #plt.savefig(fig_dir+'latency.pdf', bbox_inches='tight')

def ensure_dir(f):
    d = os.path.dirname(f)
    if not os.path.exists(d):
        os.makedirs(d)
    return 0

ensure_dir(fig_dir)
ld.split_raw_file(raw_data_dir)
ld.write_stat_log(stat_dir)
load_stat(stat_file)
