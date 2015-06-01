#!/usr/bin/env python

import os
from sys import argv
import re

# obtain packet arrival rate list
pkt_rate = [x / 10.0 for x in xrange(400, 1201) if x % 25 == 0]
msdu_size = 960
msdu_interval = ["{0:.6f}".format(0.001 * msdu_size / x) for x in pkt_rate]

# different seed
random_seed = ['233', '666', '123']

runsim_header = '''#!/usr/bin/sh
# opnet simulations scripts

'''


def runsim_from_ef(ef, seed):
    ef_real = os.path.basename(ef)
    net_name = ef_real[:ef_real.find('-DES-')]
    ef_prefix = net_name + '-' + seed + '-DES-'
    runsim_dir = "runsim_src/"
    runsim_fn = runsim_dir + "runsim_" + net_name + '-' + seed + '.sh'
    with open(runsim_fn, 'w') as runsim_fp:
        runsim_fp.writelines(runsim_header)
    for i, item in enumerate(msdu_interval):
        ef_fn = 'p' + ef_prefix + format(i + 1, '02d') + '.ef'
        ef_fn = os.path.join('ef_src', ef_fn)
        runsim_line = 'op_runsim -net_name ' + net_name + \
            ' -noprompt -m32 -ef ' + ef_fn[:-3] + \
            ' -DESinfo ' + ef_fn[:-3] + \
            ' -exec_id ' + str(i + 1) + '\n'
        with open(runsim_fn, 'a') as runsim_fp:
            runsim_fp.writelines(runsim_line)
        # write ef_new with ef_tml
        with open(ef, 'U') as ef_tml, open(ef_fn, 'w') as ef_new:
            for line in ef_tml:
                # replace log_file, ot_file, ov_file
                if re.search('^\".*_file\"', line):
                    file_line = line[:line.find(':') + 3] + ef_fn[:-3] + \
                        '\"' + '\n'
                    ef_new.writelines(file_line)
                    continue
                # update simulation duration
                if re.search('^\"duration\"', line):
                    dura = line[:line.find(':') + 3] + '82' + '\"' + '\n'
                    ef_new.writelines(dura)
                    continue
                # update seed value
                if re.search('^\"seed\"', line):
                    seed_val = line[:line.find(':') + 3] + seed + '\"' + '\n'
                    ef_new.writelines(seed_val)
                    continue
                # update Interval Time
                if re.search('Interval Time', line):
                    interval = line[:line.find('(') + 1] + item + ')\"' + '\n'
                    ef_new.writelines(interval)
                    continue
                # write normal lines
                ef_new.writelines(line)

if __name__ == "__main__":
    ef_files = argv[1:]
    for ef_file in ef_files:
        for seed in random_seed:
            runsim_from_ef(ef_file, seed)
