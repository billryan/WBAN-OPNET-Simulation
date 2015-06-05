#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import glob
import json
import datetime
from collections import defaultdict
from operator import itemgetter
import numpy as np
from wban_pram import *


def make_dir(path_in):
    if not os.path.exists(path_in):
        print("mkdir %s...") % path_in
        os.makedirs(path_in)


def is_hub(nid):
    """Hub: 17 <= nid <= 31"""
    if 17 <= int(nid) <= 31:
        return True
    else:
        return False


def load_trace_infos(raw_json):
    stat_json = raw_json
    if os.path.isdir(raw_json):
        json_files = raw_json + '/*.json'
        stat_json = max(glob.iglob(json_files), key=os.path.getctime)
    if os.path.isfile(stat_json):
        if not stat_json.endswith('.json'):
            print("!!!%s is not a valid json file!!!") % stat_json
            sys.exit(1)
    with open(stat_json, 'r') as f:
        trace_infos = json.load(f)
        return trace_infos


def get_stat_info(trace_info):
    """get stat info dict from trace_info"""
    stat_info = {}
    # get net from trace_info
    # net = get_net(trace_info)
    scene = trace_info['scene']
    stat_info['net'] = scene['net']
    stat_info['scene'] = scene['ver'] + '_' + scene['mac']
    stat_info['tf_mode'] = scene['tf']
    return stat_info


def get_tf_rate(trace_info):
    # v1: trace_infos[0]['2']
    for (bid, v1) in trace_info.items():
        if bid == 'scene':
            continue
        # v2: trace_infs[0]['2']['33']
        for (nid, v2) in v1.items():
            if is_hub(nid):
                continue
            # get UP0 traffic rate
            tf_interval = v2['src']['0'][0]['mean']
            return tf_interval


def get_lat(trace_info):
    latency = []
    # v1: trace_infos[0]['2']
    for (bid, v1) in trace_info.items():
        if bid == 'scene':
            continue
        # v2: trace_infs[0]['2']['33']
        for (nid, v2) in v1.items():
            # get UP8 as latency
            if is_hub(nid):
                latency.append(v2['lat']['8'])
    return float("{0:.6f}".format(np.mean(latency)))


def get_pkt_info(trace_info):
    pkt_info = {}
    # v1: trace_infos[0]['2']
    for (bid, v1) in trace_info.items():
        if bid == 'scene':
            continue
        # v2: trace_infs[0]['2']['33']
        for (nid, v2) in v1.items():
            # get hb_data as pkt_info
            if is_hub(nid):
                pkt_gen_num = v2['hb_data'][UP0][GEN][0]
                pkt_rcv_num = v2['hb_data'][UP0][RCV][0]
                pkt_info['gen'] = int(pkt_gen_num)
                pkt_info['rcv'] = int(pkt_rcv_num)
                return pkt_info


def get_energy(trace_info):
    energy = 0
    # v1: trace_infos[0]['2']
    for (bid, v1) in trace_info.items():
        if bid == 'scene':
            continue
        # v2: trace_infs[0]['2']['33']
        for (nid, v2) in v1.items():
            # get total as energy
            energy += v2['energy'][-1]
    return float("{0:.6f}".format(energy))


def get_throughput(trace_info):
    throughput = {'msdu': [], 'ppdu': []}
    # v1: trace_infos[0]['2']
    for (bid, v1) in trace_info.items():
        if bid == 'scene':
            continue
        # v2: trace_infs[0]['2']['33']
        for (nid, v2) in v1.items():
            # get hb_data as pkt_info
            if not is_hub(nid):
                continue
            throughput['msdu'].append(v2['throughput'][0])
            throughput['ppdu'].append(v2['throughput'][1])
    msdu_kb = float("{0:.6f}".format(np.mean(throughput['msdu'])))
    ppdu_kb = float("{0:.6f}".format(np.mean(throughput['ppdu'])))
    return (msdu_kb, ppdu_kb)


def get_stat_lines(trace_infos):
    stat_lines = []
    for trace_info in trace_infos:
        stat_line = {}
        stat_info = get_stat_info(trace_info)
        network = sim_net[stat_info['net']]
        scene = sim_scene[stat_info['scene']]
        tf_mode = sim_tf_mode[stat_info['tf_mode']]
        prefix = network + ' ' + scene + ' ' + tf_mode
        stat_line['prefix'] = prefix
        tf_rate = get_tf_rate(trace_info)
        stat_line['tf_rate'] = float("{0:.6f}".format(0.008 / tf_rate))
        latency = get_lat(trace_info)
        stat_line['latency'] = latency
        pkt_info = get_pkt_info(trace_info)
        pkt_gen = pkt_info['gen']
        pkt_rcv = pkt_info['rcv']
        pkt_eta = 1.0 * pkt_rcv / pkt_gen
        stat_line['pkt_gen'] = pkt_gen
        stat_line['pkt_rcv'] = pkt_rcv
        stat_line['pkt_eta'] = pkt_eta
        energy_total = get_energy(trace_info)
        stat_line['energy_total'] = energy_total
        thr_msdu = get_throughput(trace_info)[0]
        thr_ppdu = get_throughput(trace_info)[1]
        stat_line['thr_msdu'] = thr_msdu
        stat_line['thr_ppdu'] = thr_ppdu
        stat_lines.append(stat_line)
    return stat_lines


def merge_stat_group(stat_lines_group):
    stat_line_merge = stat_lines_group[0]
    groups = len(stat_lines_group)
    for i, stat_line in enumerate(stat_lines_group):
        # ignore self with init
        if i == 1:
            continue
        for (k, v) in stat_line.items():
            if k == 'prefix' or k == 'tf_rate':
                continue
            stat_line_merge[k] += v
    # average merge results
    for (k, v) in stat_line_merge.items():
        if k == 'prefix' or k == 'tf_rate':
            continue
        stat_line_merge[k] = v / groups
    return stat_line_merge


def merge_stat_lines(stat_lines):
    stat_lines_merged = []
    stat_lines_group = defaultdict(list)
    for stat_line in stat_lines:
        prefix = stat_line['prefix']
        tf_rate = str(stat_line['tf_rate'])
        group = prefix + '_' + tf_rate
        stat_lines_group[group].append(stat_line)
    stat_lines_new = []
    for (group, v) in stat_lines_group.items():
        stat_line_merge = merge_stat_group(v)
        stat_lines_merged.append(stat_line_merge)
    # sort by key group
    # stat_lines_merged = sorted(stat_lines_merged, key=itemgetter('prefix'))
    stat_lines_sorted = sorted(
        stat_lines_merged, key=lambda k: k['prefix'] + str(k['tf_rate']))
    return stat_lines_sorted


def format_stat_lines(stat_lines):
    stat_lines_formated = []
    for stat_line in stat_lines:
        prefix = stat_line['prefix']
        network, scene, tf_mode = prefix.split(' ')
        network = network.ljust(10)
        scene = scene.ljust(6)
        tf_mode = tf_mode.ljust(8)
        tf_rate = stat_line['tf_rate']
        tf_rate = "{0:.6f}".format(tf_rate).ljust(10)
        latency = stat_line['latency']
        latency = "{0:.6f}".format(latency).ljust(10)
        pkt_gen = stat_line['pkt_gen']
        pkt_gen = str(pkt_gen).ljust(10)
        pkt_rcv = stat_line['pkt_rcv']
        pkt_rcv = str(pkt_rcv).ljust(10)
        pkt_eta = "{0:.4f}".format(stat_line['pkt_eta']).ljust(8)
        energy_total = stat_line['energy_total']
        energy_total = "{0:.6f}".format(energy_total).ljust(12)
        thr_msdu = stat_line['thr_msdu']
        thr_msdu = "{0:.6f}".format(thr_msdu).ljust(12)
        thr_ppdu = stat_line['thr_ppdu']
        thr_ppdu = "{0:.6f}".format(thr_ppdu).ljust(12)
        line = network + scene + tf_mode + tf_rate + latency + pkt_gen + \
            pkt_rcv + pkt_eta + energy_total + thr_msdu + thr_ppdu + '\n'
        stat_lines_formated.append(line)
    return stat_lines_formated


def output_stat(raw_json, stat_path=None):
    if not stat_path:
        stat_path = os.path.abspath(os.path.join(raw_json, os.pardir))
    if not os.path.isdir(stat_path):
        print("!!!stat_path %s is not a valid dir!!!") % stat_path
        sys.exit(1)
    stat_path = os.path.join(stat_path, 'stat_log')
    today = str(datetime.date.today())
    stat_fn = "stat_" + today + ".log"
    make_dir(stat_path)
    stat_path = os.path.join(stat_path, stat_fn)
    with open(stat_path, 'w') as f:
        f.write(
            '# network'.ljust(10) + 'scene'.ljust(6) +
            'tf_mode'.ljust(8) + 'tf_rate'.ljust(10) +
            'latency'.ljust(10) +
            'pkt_gen'.ljust(10) + 'pkt_rcv'.ljust(10) + 'pkt_eta'.ljust(8) +
            'engy_total'.ljust(12) +
            'thr_msdu'.ljust(12) + 'thr_ppdu'.ljust(12) + '\n')
        trace_infos = load_trace_infos(raw_json)
        stat_lines = get_stat_lines(trace_infos)
        stat_lines_sorted = merge_stat_lines(stat_lines)
        stat_lines_formated = format_stat_lines(stat_lines_sorted)
        for line in stat_lines_formated:
            f.writelines(line)


def valid_bid(bids):
    bids = [int(bid) for bid in bids]
    for bid in bids:
        if not 2 <= bid <= 15:
            print("!!!Invalid bid settings!!!")
            sys.exit(1)
    return True


def valid_ban(nids):
    nids = [int(nid) for nid in nids]
    nids.sort()
    if len(nids) < 2:
        print("!!!Only one node!!!")
        sys.exit(1)
    if nids[0] < 17 or nids[0] > 31:
        print("!!!Error Hub settings!!!")
        sys.exit(1)
    if nids[1] > 16 and nids[1] < 32:
        print("!!!Error Node settings!!!")
        sys.exit(1)
    return True


def get_net(trace_info):
    bids = trace_info.keys()
    bids = bids.remove('scene')
    ban_num = len(bids)
    node_num = 0
    bid_nids = []
    valid_bid(bids)
    for bid in bids:
        nids = trace_info[bid].keys()
        valid_ban(nids)
        node_num += len(nids) - 1
        bid_nids.append(nids)
    sym_status = "sym"
    for bid_nid in bid_nids:
        if len(bid_nid) != len(bid_nids[0]):
            sym_status = "asym"
            break
    return "B" + str(ban_num) + "N" + str(node_num) + "_" + sym_status


if __name__ == "__main__":
    if not 2 <= len(sys.argv) <= 3:
        print("Usage: %s json_dir [stat_path]") % sys.argv[0]
        sys.exit(1)
    raw_json = sys.argv[1]
    stat_path = None
    if len(sys.argv) == 3:
        stat_path = sys.argv[2]
    output_stat(raw_json, stat_path)
