#!/usr/bin/env python
# -*-coding: utf-8 -*-

import os
import sys
import glob
import numpy as np
import matplotlib.pyplot as plt


def load_stat_log(stat_log, fig_dir):
    stat_log_dir = stat_log
    if not os.path.isdir(stat_log):
        stat_log_dir = os.path.dirname(stat_log)
    else:
        stat_log_files = stat_log + '/*.log'
        stat_log = max(glob.iglob(stat_log_files), key=os.path.getctime)
    if not stat_log.endswith('.log'):
        print("file %s is not a valid stat.log") % stat_log
        sys.exit(1)
    if not fig_dir:
        fig_dir = os.path.abspath(os.path.join(stat_log_dir, os.pardir))
    if not os.path.isdir(fig_dir):
        print("!!!fig_dir %s is not a valid dir!!!") % fig_dir
        sys.exit(1)
    fig_dir = os.path.join(fig_dir, 'stat_fig')
    make_dir(fig_dir)
    stat_log_np = np.loadtxt(
        stat_log, dtype={'names': (
            'network', 'scene',
            'traffic arrival mode', 'traffic arrival rate',
            'latency',
            'packets generated', 'packets received', 'packets eta',
            'energy consumed',
            'throughput-msdu', 'throughput-ppdu'),
            'formats': ('i', 'i',
                        'i', 'f',
                        'f',
                        'i', 'i', 'f',
                        'f',
                        'f', 'f')},
        comments='#', unpack=True)
    network = stat_log_np[0]
    scene = stat_log_np[1]
    tf_mode = stat_log_np[2]
    tf_rate = stat_log_np[3]
    latency = stat_log_np[4]
    pkt_gen = stat_log_np[5]
    pkt_rcv = stat_log_np[6]
    pkt_eta = stat_log_np[7]
    energy_total = stat_log_np[8]
    thr_msdu = stat_log_np[9]
    thr_ppdu = stat_log_np[10]
    # n1: N4x2, s0: ver0_CSMA, m0: constant
    cond = np.logical_and.reduce((network == 1, scene == 0, tf_mode == 0))
    n1s0m0 = np.where(cond)
    # n1: N4x2, s1: ver0_TDMA, m0: constant
    cond = np.logical_and.reduce((network == 1, scene == 1, tf_mode == 0))
    n1s1m0 = np.where(cond)
    # n1: N4x2, s2: ver0_hybrid, m0: constant
    cond = np.logical_and.reduce((network == 1, scene == 2, tf_mode == 0))
    n1s2m0 = np.where(cond)
    # n1: N4x2, s3: ver1_hybrid, m0: constant
    cond = np.logical_and.reduce((network == 1, scene == 3, tf_mode == 0))
    n1s3m0 = np.where(cond)
    # n1: N4x2, s4: ver1_CSMA, m0: constant
    cond = np.logical_and.reduce((network == 1, scene == 4, tf_mode == 0))
    n1s4m0 = np.where(cond)
    # n1: N4x2, s0: ver0_CSMA, m1: poisson
    cond = np.logical_and.reduce((network == 1, scene == 0, tf_mode == 1))
    n1s0m1 = np.where(cond)
    # n1: N4x2, s1: ver0_TDMA, m1: poisson
    cond = np.logical_and.reduce((network == 1, scene == 1, tf_mode == 1))
    n1s1m1 = np.where(cond)
    # n1: N4x2, s2: ver0_hybrid, m1: poisson
    cond = np.logical_and.reduce((network == 1, scene == 2, tf_mode == 1))
    n1s2m1 = np.where(cond)
    # n1: N4x2, s3: ver1_hybrid, m1: poisson
    cond = np.logical_and.reduce((network == 1, scene == 3, tf_mode == 1))
    n1s3m1 = np.where(cond)
    # n1: N4x2, s4: ver1_CSMA, m1: poisson
    cond = np.logical_and.reduce((network == 1, scene == 4, tf_mode == 1))
    n1s4m1 = np.where(cond)

    # Draw Packet Ratio - Constant
    plt.figure(1)
    plt.plot(tf_rate[n1s0m0], pkt_eta[n1s0m0], 'o-',
             linewidth=2, label="IEEE 802.15.6 CSMA")
    plt.plot(tf_rate[n1s1m0], pkt_eta[n1s1m0], 'o-',
             linewidth=2, label="IEEE 802.15.6 TDMA")
    # plt.plot(tf_rate[n1s2m0], pkt_eta[n1s2m0], 'o-',
    #         linewidth=2, label="IEEE 802.15.6 Hybrid")
    plt.plot(tf_rate[n1s3m0], pkt_eta[n1s3m0], 'o-',
             linewidth=2, label="AI MAC")
    plt.plot(tf_rate[n1s4m0], pkt_eta[n1s4m0], 'o-',
             linewidth=2, label="AI MAC - without rho")
    plt.xlabel("Interval Time(packet/slot)")
    plt.ylabel("Packet Receive Ratio")
    plt.title('Packet Receive Ratio - Constant Packet Arrival')
    plt.grid()
    plt.legend(loc='lower left')
    plt.show()
    # plt.savefig(fig_dir + 'pkt_eta.png')
    plt.clf()

    # Draw Packet Ratio - Poisson
    plt.figure(1)
    plt.plot(tf_rate[n1s0m1], pkt_eta[n1s0m1], 'o-',
             linewidth=2, label="IEEE 802.15.6 CSMA")
    plt.plot(tf_rate[n1s1m1], pkt_eta[n1s1m1], 'o-',
             linewidth=2, label="IEEE 802.15.6 TDMA")
    # plt.plot(tf_rate[n1s2m1], pkt_eta[n1s2m1], 'o-',
    #          linewidth=2, label="IEEE 802.15.6 Hybrid")
    plt.plot(tf_rate[n1s3m1], pkt_eta[n1s3m1], 'o-',
             linewidth=2, label="AI MAC")
    plt.plot(tf_rate[n1s4m1], pkt_eta[n1s4m1], 'o-',
             linewidth=2, label="AI MAC - without rho")
    plt.xlabel("Interval Time(packet/slot)")
    plt.ylabel("Packet Receive Ratio")
    plt.title('Packet Receive Ratio - Poisson Packet Arrival')
    plt.grid()
    plt.legend(loc='lower left')
    plt.show()
    # plt.savefig(fig_dir + 'pkt_eta.png')
    plt.clf()

    # Draw Throughput - Constant
    plt.figure(1)
    plt.plot(tf_rate[n1s0m0], thr_msdu[n1s0m0], 'o-',
             linewidth=2, label="IEEE 802.15.6 CSMA")
    plt.plot(tf_rate[n1s1m0], thr_msdu[n1s1m0], 'o-',
             linewidth=2, label="IEEE 802.15.6 TDMA")
    # plt.plot(tf_rate[n1s2m0], thr_msdu[n1s2m0], 'o-',
    #         linewidth=2, label="IEEE 802.15.6 Hybrid")
    plt.plot(tf_rate[n1s3m0], thr_msdu[n1s3m0], 'o-',
             linewidth=2, label="AI MAC")
    plt.plot(tf_rate[n1s4m0], thr_msdu[n1s4m0], 'o-',
             linewidth=2, label="AI MAC - without rho")
    plt.xlabel("Interval Time(packet/slot)")
    plt.ylabel("Throughput")
    plt.title('Throughput - Constant Packet Arrival')
    plt.grid()
    plt.legend(loc='upper left')
    plt.show()
    # plt.savefig(fig_dir + 'thr_msdu.png')
    plt.clf()

    # Draw Throughput - Poisson
    plt.figure(1)
    plt.plot(tf_rate[n1s0m1], thr_msdu[n1s0m1], 'o-',
             linewidth=2, label="IEEE 802.15.6 CSMA")
    plt.plot(tf_rate[n1s1m1], thr_msdu[n1s1m1], 'o-',
             linewidth=2, label="IEEE 802.15.6 TDMA")
    # plt.plot(tf_rate[n1s2m1], thr_msdu[n1s2m1], 'o-',
    #          linewidth=2, label="IEEE 802.15.6 Hybrid")
    plt.plot(tf_rate[n1s3m1], thr_msdu[n1s3m1], 'o-',
             linewidth=2, label="AI MAC")
    plt.plot(tf_rate[n1s4m1], thr_msdu[n1s4m1], 'o-',
             linewidth=2, label="AI MAC - without rho")
    plt.xlabel("Interval Time(packet/slot)")
    plt.ylabel("Throughput")
    plt.title('Throughput - Poisson Packet Arrival')
    plt.grid()
    plt.legend(loc='upper left')
    plt.show()
    # plt.savefig(fig_dir + 'thr_msdu.png')
    plt.clf()

    # Draw Latency - Constant
    plt.figure(1)
    plt.plot(tf_rate[n1s0m0], latency[n1s0m0], 'o-',
             linewidth=2, label="IEEE 802.15.6 CSMA")
    plt.plot(tf_rate[n1s1m0], latency[n1s1m0], 'o-',
             linewidth=2, label="IEEE 802.15.6 TDMA")
    # plt.plot(tf_rate[n1s2m0], latency[n1s2m0], 'o-',
    #         linewidth=2, label="IEEE 802.15.6 Hybrid")
    plt.plot(tf_rate[n1s3m0], latency[n1s3m0], 'o-',
             linewidth=2, label="AI MAC")
    plt.plot(tf_rate[n1s4m0], latency[n1s4m0], 'o-',
             linewidth=2, label="AI MAC - without rho")
    plt.xlabel("Interval Time(packet/slot)")
    plt.ylabel("Latency(s)")
    plt.title('Latency - Constant Packet Arrival')
    plt.grid()
    plt.legend(loc='upper left')
    plt.show()
    # plt.savefig(fig_dir + 'latency.png')
    plt.clf()

    # Draw Latency - Poisson
    plt.figure(1)
    plt.plot(tf_rate[n1s0m1], latency[n1s0m1], 'o-',
             linewidth=2, label="IEEE 802.15.6 CSMA")
    plt.plot(tf_rate[n1s1m1], latency[n1s1m1], 'o-',
             linewidth=2, label="IEEE 802.15.6 TDMA")
    # plt.plot(tf_rate[n1s2m1], latency[n1s2m1], 'o-',
    #          linewidth=2, label="IEEE 802.15.6 Hybrid")
    plt.plot(tf_rate[n1s3m1], latency[n1s3m1], 'o-',
             linewidth=2, label="AI MAC")
    plt.plot(tf_rate[n1s4m1], latency[n1s4m1], 'o-',
             linewidth=2, label="AI MAC - without rho")
    plt.xlabel("Interval Time(packet/slot)")
    plt.ylabel("Latency(s)")
    plt.title('Latency - Poisson Packet Arrival')
    plt.grid()
    plt.legend(loc='upper left')
    plt.show()
    # plt.savefig(fig_dir + 'latency.png')
    plt.clf()

    # Energy - Constant
    plt.figure(1)
    plt.plot(tf_rate[n1s0m0], energy_total[n1s0m0], 'o-',
             linewidth=2, label="IEEE 802.15.6 CSMA")
    plt.plot(tf_rate[n1s1m0], energy_total[n1s1m0], 'o-',
             linewidth=2, label="IEEE 802.15.6 TDMA")
    # plt.plot(tf_rate[n1s2m0], energy_total[n1s2m0], 'o-',
    #          linewidth=2, label="IEEE 802.15.6 Hybrid")
    plt.plot(tf_rate[n1s3m0], energy_total[n1s3m0], 'o-',
             linewidth=2, label="AI MAC")
    plt.plot(tf_rate[n1s4m0], energy_total[n1s4m0], 'o-',
             linewidth=2, label="AI MAC - without rho")
    plt.xlabel("Interval Time(packet/slot)")
    plt.ylabel("Energy(Joule)")
    plt.title('Energy - Constant')
    plt.grid()
    plt.legend(loc='lower right')
    plt.show()
    # plt.savefig(fig_dir + 'latency.png')
    plt.clf()

    # Energy - Poisson
    plt.figure(1)
    plt.plot(tf_rate[n1s0m1], energy_total[n1s0m1], 'o-',
             linewidth=2, label="IEEE 802.15.6 CSMA")
    plt.plot(tf_rate[n1s1m1], energy_total[n1s1m1], 'o-',
             linewidth=2, label="IEEE 802.15.6 TDMA")
    # plt.plot(tf_rate[n1s2m1], energy_total[n1s2m1], 'o-',
    #          linewidth=2, label="IEEE 802.15.6 Hybrid")
    plt.plot(tf_rate[n1s3m1], energy_total[n1s3m1], 'o-',
             linewidth=2, label="AI MAC")
    plt.plot(tf_rate[n1s4m1], energy_total[n1s4m1], 'o-',
             linewidth=2, label="AI MAC - without rho")
    plt.xlabel("Interval Time(packet/slot)")
    plt.ylabel("Energy(Joule)")
    plt.title('Energy - Poisson')
    plt.grid()
    plt.legend(loc='lower right')
    plt.show()
    # plt.savefig(fig_dir + 'latency.png')
    plt.clf()


def make_dir(path_in):
    if not os.path.exists(path_in):
        print("mkdir %s...") % path_in
        os.makedirs(path_in)

if __name__ == "__main__":
    if not 2 <= len(sys.argv) <= 3:
        print("Usage: %s stat_log[dir or file] [figure_dir]") % sys.argv[0]
        sys.exit(1)
    stat_log = sys.argv[1]
    fig_dir = None
    if len(sys.argv) == 3:
        fig_dir = sys.argv[2]
    load_stat_log(stat_log, fig_dir)
