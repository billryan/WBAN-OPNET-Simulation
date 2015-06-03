#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import glob
import json
import datetime
from wban_pram import *


def make_dir(path_in):
    if not os.path.exists(path_in):
        print("mkdir %s...") % path_in
        os.makedirs(path_in)


def load_trace(raw_json):
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


def output_stat(raw_json, stat_path=None):
    if not stat_path:
        stat_path = os.path.abspath(os.path.join(raw_json, os.pardir))
    if not os.path.isdir(stat_path):
        print("!!!stat_path %s is not a valid dir!!!") % stat_path
        sys.exit(1)
    stat_path = os.path.join(stat_path, 'stat_log')
    trace_infos = load_trace(raw_json)
    today = str(datetime.date.today())
    stat_fn = "stat_" + today + ".log"
    make_dir(stat_path)
    stat_path = os.path.join(stat_path, stat_fn)
    with open(stat_path, 'w') as f:
        f.write('# network ' + 'version ' + 'MAC ' + 'traffic_mode ' +
                'traffic_rate ' + 'latency ' +
                'pkt_gen ' + 'pkt_qsucc ' + 'pkt_qfail ' + 'pkt_sent ' +
                'pkt_rcv ' + 'pkt_subq ' + 'pkt_fail ' +
                'energy_tx ' + 'energy_rx ' + 'energy_cca ' +
                'energy_idle ' + 'energy_sleep ' + 'energy_total ' +
                'thr_msdu ' + 'thr_ppdu' + '\n')


def valid_bid(bids):
    bids = [int(bid) for bid in bids]
    for bid in bids:
        if bid < 2 or bid > 15:
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
