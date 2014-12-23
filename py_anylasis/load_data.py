#!/usr/bin/env python2
#-*-coding:utf-8 -*-
import os
import sys
import re
import read_large_file as rlf
import numpy as np
from enum import Enum

raw_data_dir = 'raw_data/'
stat_data_dir = 'stat_data/'

# user priority for packet
UP = Enum('0', '1', '2', '3', '4', '5', '6', '7')
# data State - Generate, Sent, Receive
DataState = Enum('GEN', 'QUEUE_SUCC', 'QUEUE_FAIL', 'SENT', 'RCV', 'SUBQ')

if len(sys.argv) > 1:
    print "Usage: %s" %(sys.argv[0])
    sys.exit(1)

# get the file list recursively
def get_file_list_recur(path_in):
    file_list = []
    subdir_file_list = os.listdir(path_in)
    for file_name in subdir_file_list:
        full_file_name = os.path.join(path_in, file_name)
        if os.path.isdir(full_file_name):
            get_file_list_recur(full_file_name)
        elif (None != re.search('trace$', file_name, re.IGNORECASE)):
            file_list.append(full_file_name)

    return file_list

# split single collection to multiple files
def split_raw_file(path_in):
    subdir_list = os.listdir(path_in)
    for dir_name in subdir_list:
        dir_path = os.path.join(path_in, dir_name)
        if os.path.isdir(dir_path):
            if (None != re.search('^(exp|con)', dir_name, re.IGNORECASE)):
                raw_file_list = get_file_list_recur(dir_path)
                rate = dir_name[3:dir_name.index('_')]
                i = 0
                for raw_file in raw_file_list:
                    pre_file_name = os.path.basename(raw_file)
                    orig_fp = open(raw_file)
                    for line in orig_fp:
                        if line.find('t=0.000000,') >= 0:
                            i += 1
                            post_file_name = os.path.join(dir_path, rate+'-'+str(i)+'-'+pre_file_name)
                            post_fp = open(post_file_name, 'w')
                            post_fp.write(line)
                        else:
                            post_fp.write(line)
                    orig_fp.close()
                    os.remove(raw_file)

#get raw data file from raw_data
def get_raw_file_dic(path_in):
    raw_file_dic = {}
    subdir_file_list = os.listdir(path_in)
    for file_name in subdir_file_list:
        full_dir_name = os.path.join(path_in, file_name)
        if os.path.isdir(full_dir_name):
            if (None != re.search('^(exp|con)', file_name, re.IGNORECASE)):
                raw_file_list= get_file_list_recur(full_dir_name)
                rate = file_name[3:file_name.index('_')]
                version = file_name[-1]
                if raw_file_dic.has_key(version):
                    if raw_file_dic[version].has_key(rate):
                        raw_file_dic[version][rate].append(raw_file_list)
                    else:
                        raw_file_dic[version][rate] = []
                        raw_file_dic[version][rate] = raw_file_list
                else:
                    raw_file_dic[version] = {}
                    raw_file_dic[version][rate] = []
                    raw_file_dic[version][rate] = raw_file_list
    return raw_file_dic

def ensure_dir(f):
    d = os.path.dirname(f)
    if not os.path.exists(d):
        os.makedirs(d)
    return 0

def load_init(in_file):
    init_list = []
    init_dict = {}
    fp = open(in_file)
    i = 0
    for line in fp:
        i = i + 1
        match = re.findall('INIT', line)
        if match != []:
            init_list.append(line[0:-2])
        elif i > 500:
            break

    for txt in init_list:
        init_bkd = re.split('[=,]', txt)
        node_name = init_bkd[3]
        node_id = init_bkd[5]
        nid = int(init_bkd[8])
        if init_dict.has_key(node_id):
            init_dict[node_id]['NODE_NAME'] = node_name
            init_dict[node_id]['NID'] = nid
        else:
            init_dict[node_id] = {}
            init_dict[node_id]['NODE_NAME'] = node_name
            init_dict[node_id]['NID'] = nid

    return init_dict

def load_latency(in_file):
    #latency_list = []
    stat_info_dic = load_init(in_file)
    fp = open(in_file)
    for line in fp:
        #if re.findall('FRAME_TYPE=3.*ETE_DELAY', line) != []:
        #    latency = float(re.split('[=,]', line[0:-2])[-1])
        #    latency_list.append(latency)
        if re.findall('STAT,LATENCY,', line) != []:
            latency_avg = float(re.split('[=,]', line[0:-2])[-1])
            up = re.split('[=,]', line[0:-2])[-3]
            if stat_info_dic.has_key('LATENCY'):
                stat_info_dic['LATENCY'][up] = latency_avg
            else:
                stat_info_dic['LATENCY'] = {}
                stat_info_dic['LATENCY'][up] = latency_avg
    #stat_info_dic['LATENCY']['9'] = latency_list
    return stat_info_dic

def load_energy(in_file):
    energy_list = []
    i = 0
    stat_info_dic = load_init(in_file)
    fp = open(in_file)
    for line in rlf.reversed_lines(fp):
        i = i + 1
        if re.findall('STAT,ENERGY,', line) != []:
            tx = float(re.split('[=,]', line[0:-2])[-11])
            rx = float(re.split('[=,]', line[0:-2])[-9])
            cca = float(re.split('[=,]', line[0:-2])[-7])
            idle = float(re.split('[=,]', line[0:-2])[-5])
            sleep = float(re.split('[=,]', line[0:-2])[-3])
            total = float(re.split('[=,]', line[0:-2])[-1])
            node_id = re.split('[=,]', line[0:-2])[5]
            if stat_info_dic[node_id].has_key('ENERGY'):
                stat_info_dic[node_id]['ENERGY']['TX'] = tx
                stat_info_dic[node_id]['ENERGY']['RX'] = rx
                stat_info_dic[node_id]['ENERGY']['CCA'] = cca
                stat_info_dic[node_id]['ENERGY']['IDLE'] = idle
                stat_info_dic[node_id]['ENERGY']['SLEEP'] = sleep
                stat_info_dic[node_id]['ENERGY']['TOTAL'] = total
            else:
                stat_info_dic[node_id]['ENERGY'] = {}
                stat_info_dic[node_id]['ENERGY']['TX'] = tx
                stat_info_dic[node_id]['ENERGY']['RX'] = rx
                stat_info_dic[node_id]['ENERGY']['CCA'] = cca
                stat_info_dic[node_id]['ENERGY']['IDLE'] = idle
                stat_info_dic[node_id]['ENERGY']['SLEEP'] = sleep
                stat_info_dic[node_id]['ENERGY']['TOTAL'] = total
        elif i > 800:
            break
    return stat_info_dic

def load_throughput(in_file):
    stat_info_dic = load_init(in_file)
    #rcv_msdu_kbps_list = []
    fp = open(in_file)
    for line in fp:
        if re.findall('STAT,THROU.*RCV_MSDU', line) != []:
            rcv_msdu_kbps = float(re.split('[=,]', line[0:-2])[-1])
            #rcv_msdu_kbps_list.append(rcv_msdu_kbps)
    if stat_info_dic.has_key('THROUGHPUT'):
        stat_info_dic['THROUGHPUT']['RCV_MSDU'] = rcv_msdu_kbps
    else:
        stat_info_dic['THROUGHPUT'] = {}
        stat_info_dic['THROUGHPUT']['RCV_MSDU'] = rcv_msdu_kbps
    return stat_info_dic

def load_pkt_stat(in_file):
    stat_info_dic = load_init(in_file)
    fp = open(in_file)
    for line in fp:
        if re.findall('STAT,DATA,UP', line) != []:
            node_id = re.split('[=,]', line[0:-2])[3]
            up = re.split('[=,]', line[0:-2])[7]
            data_state = re.split('[=,]', line[0:-2])[9]
            number = float(re.split('[=,]', line[0:-2])[11])
            ppdu_kbits = float(re.split('[=,]', line[0:-2])[13])
            if stat_info_dic[node_id].has_key('DATA') == False:
                stat_info_dic[node_id]['DATA'] = {}
                stat_info_dic[node_id]['DATA'][up] = {}
                stat_info_dic[node_id]['DATA'][up][data_state] = {}
                stat_info_dic[node_id]['DATA'][up][data_state]['NUMBER'] = number
                stat_info_dic[node_id]['DATA'][up][data_state]['PPDU_KBITS'] = ppdu_kbits
            elif stat_info_dic[node_id]['DATA'].has_key(up):
                if stat_info_dic[node_id]['DATA'][up].has_key(data_state) == False:
                    stat_info_dic[node_id]['DATA'][up][data_state] = {}
                    stat_info_dic[node_id]['DATA'][up][data_state]['NUMBER'] = number
                    stat_info_dic[node_id]['DATA'][up][data_state]['PPDU_KBITS'] = ppdu_kbits
                else:
                    stat_info_dic[node_id]['DATA'][up][data_state]['NUMBER'] = number
                    stat_info_dic[node_id]['DATA'][up][data_state]['PPDU_KBITS'] = ppdu_kbits

    return stat_info_dic

def load_latency_avg(file_list):
    latency_stat_list = []
    for file_name in file_list:
        stat_info_dic = load_latency(file_name)
        latency_stat_list.append(stat_info_dic['LATENCY']['8'])
    return np.mean(latency_stat_list)

def load_energy_avg(file_list):
    energy_stat_list = []
    for file_name in file_list:
        stat_info_dic = load_energy(file_name)
        energy_sum = 0.0
        for (k,v) in stat_info_dic.items():
            energy_sum += stat_info_dic[k]['ENERGY']['TOTAL']
        energy_stat_list.append(energy_sum)
    return np.mean(energy_stat_list)

def load_throughput_avg(file_list):
    throughput_stat_list = []
    for file_name in file_list:
        stat_info_dic = load_throughput(file_name)
        # get the last element(avg) of throughput list
        throughput_avg = stat_info_dic['THROUGHPUT']['RCV_MSDU']
        throughput_stat_list.append(throughput_avg)
    return np.mean(throughput_stat_list)

def load_pkt_loss_avg(file_list):
    pkt_loss_dic = {}
    pkt_loss_rate_list = []
    pkt_queue_rate_list = []
    for file_name in file_list:
        stat_info_dic = load_pkt_stat(file_name)
        pkt_gen_num = 0.0
        pkt_rcv_num = 0.0
        pkt_fail_num = 0.0
        pkt_subq_num = 0.0
        for (k,v) in stat_info_dic.items():
            if 16 < stat_info_dic[k]['NID'] < 32:
                for i in xrange(8):
                    for j in xrange(7):
                        if stat_info_dic[k]['DATA'].has_key(str(i)):
                            pkt_gen_num += stat_info_dic[k]['DATA'][str(i)]['0']['NUMBER']
                            pkt_rcv_num += stat_info_dic[k]['DATA'][str(i)]['4']['NUMBER']
                            pkt_fail_num += stat_info_dic[k]['DATA'][str(i)]['6']['NUMBER']
            elif stat_info_dic[k]['NID'] > 31:
                for i in xrange(8):
                    for j in xrange(7):
                        if stat_info_dic[k]['DATA'].has_key(str(i)):
                            pkt_subq_num += stat_info_dic[k]['DATA'][str(i)]['5']['NUMBER']
        #pkt_loss_num = pkt_gen_num - pkt_rcv_num - pkt_subq_num
        #pkt_loss_num = pkt_gen_num - pkt_rcv_num - pkt_subq_num
        pkt_loss_num = pkt_gen_num - pkt_rcv_num
        pkt_loss_rate = pkt_loss_num/pkt_gen_num
        pkt_queue_rate = pkt_subq_num/pkt_gen_num
        pkt_loss_rate_list.append(pkt_loss_rate)
        pkt_queue_rate_list.append(pkt_queue_rate)
    pkt_loss_dic['LOSS'] = np.mean(pkt_loss_rate_list)
    pkt_loss_dic['QUEUE'] = np.mean(pkt_queue_rate_list)
    return pkt_loss_dic

def write_stat_log(path_in):
    stat_avg_file = path_in + 'stat_avg.log'
    ensure_dir(stat_avg_file)
    fp = open(stat_avg_file, 'w')
    fp.write('#version\trate\tlatency\tenergy\tthroughput\tpacket_loss_rate\tpacket_queue_rate\n')
    raw_files_dic = get_raw_file_dic(raw_data_dir)
    # print raw_files_dic
    ver_list = sorted(raw_files_dic.keys())
    for version in ver_list:
        rate_list = sorted(raw_files_dic[version].keys())
        for rate in rate_list:
            print("Write version-%s,rate-%s stat log file" % (version, rate))
            raw_file_list = raw_files_dic[version][rate]
            lat_avg = load_latency_avg(raw_file_list)
            energy_avg = load_energy_avg(raw_file_list)
            thput_avg = load_throughput_avg(raw_file_list)
            pkt_loss_dic = load_pkt_loss_avg(raw_file_list)
            pkt_loss_avg = pkt_loss_dic['LOSS']
            pkt_queue_avg = pkt_loss_dic['QUEUE']
            fp.write('%s\t%s\t%f\t%f\t%f\t%f\t%f\n' % (version, rate, lat_avg, energy_avg, thput_avg, pkt_loss_avg, pkt_queue_avg))
