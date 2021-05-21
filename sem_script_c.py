import sem
import numpy as np
import pandas as pd
import copy
import matplotlib.pyplot as plt
import tikzplotlib as tkz

APP_START = 800
UDP_APP_TRACEFILE = 'UdpServerRxAdress.txt'
FTP_UL_APP_TRACEFILE = 'ftp-ul-rx-app-trace.txt'
FTP_DL_APP_TRACEFILE = 'ftp-dl-rx-app-trace.txt'

ns_path = './'
ns_script = 'mmwave-central-iab-test'
ns_res_path = './sem-tests-res'

# Create the  simulation campaigns
campaign = sem.CampaignManager.new(ns_path, ns_script, ns_res_path, optimized=True,
                                   runner_type='ParallelRunner', check_repo=False)

app_run_time = 3000 # [ms]
runs = 25
 
params_grid = {
    'run' : 1,
    'RngRun': list(range(runs)),
    'centralizedSched': True,
    'appRunTime': app_run_time,
    'cooldownPeriod': 500, # [ms]
    'numIabs': 5,
    'harqOn': True,
    'uesPerBs': 8,
    'weightPolicy': [1, 2, 3],
    'packetSize': 100,
    'allocationPeriod': [2, 4], #[ms]
    'eta': 50,
    'k': 2,
    'muThreshold': 10
}

# app_run_time = 200
# runs = 5
# params_grid = {
#     'run' : 1,
#     'RngRun': list(range(runs)),
#     'centralizedSched': True,
#     'appRunTime': app_run_time,
#     'cooldownPeriod': 500, # [ms]
#     'numIabs': 5,
#     'harqOn': True,
#     'uesPerBs': 3,
#     'weightPolicy': [1, 2, 3],
#     'packetSize': [50, 100, 200, 500],
#     'eta': 50,
#     'k': 2,
#     'muThreshold': 10
# }

overall_list = sem.list_param_combinations(params_grid)
#params_grid.update(centralizedSched=False, weightPolicy=1)
#overall_list.extend (sem.list_param_combinations(params_grid))

#params_grid.update(centralizedSched=True, packetSize=100, weightPolicy=3, eta=[20, 50, 100], k=[2], muThreshold = [10])
#overall_list.extend (sem.list_param_combinations(params_grid))

#params_grid.update(centralizedSched=True, packetSize=100, weightPolicy=3, eta=[50], k=[1, 2, 3], muThreshold = [10])
#overall_list.extend (sem.list_param_combinations(params_grid))

#params_grid.update(centralizedSched=True, packetSize=100, weightPolicy=3, eta=[50], k=[2], muThreshold = [5, 10, 20])
#overall_list.extend (sem.list_param_combinations(params_grid))

# Temporarily limit number of max cores used
#sem.parallelrunner.MAX_PARALLEL_PROCESSES=19
campaign.run_missing_simulations(overall_list)

# Data analysis


def get_e2e_avg_throughput(results, tracename):
    thr_avg = []
    empty = 0
    for result in results:
        if(check_errors(result) != 0):
            continue
        result_filenames = campaign.db.get_result_files(result['meta']['id'])
        result_filename = result_filenames[tracename]
        rx_df = pd.read_csv(result_filename, sep=' ', lineterminator='\n', skiprows=1,
                            names=['packet_size', 'address', 'rx_time', 'delay'], skip_blank_lines=True)
        if(rx_df.size == 0):
            #print('Warning: empty trace!')
            empty = empty + 1
            continue
        # Str to int
        rx_df = rx_df.astype(
            {'packet_size': 'int32', 'address': 'int32', 'rx_time': 'int64', 'delay': 'int64'})
        # Keep just data received before end of TX time
        #rx_df = rx_df[rx_df['rx_time'] < (APP_START + app_run_time)*1e6]
        thr_entry = rx_df['packet_size'].sum(
        )/(rx_df['rx_time'].iat[-1] - APP_START*1e6)*1e9*8/1e6  # [Mbit/s]
        thr_avg.append(thr_entry)

    print(f"Warning: {empty} empty traces")

    return thr_avg


def get_e2e_ue_throughput(results, tracename):
    thr_ue = []
    empty = 0
    for result in results:
        if(check_errors(result) != 0):
            continue
        result_filenames = campaign.db.get_result_files(result['meta']['id'])
        result_filename = result_filenames[tracename]
        rx_df = pd.read_csv(result_filename, sep=' ', lineterminator='\n', skiprows=1,
                            names=['packet_size', 'address', 'rx_time', 'delay'], skip_blank_lines=True)
        if(rx_df.size == 0):
            #print('Warning: empty trace!')
            empty = empty + 1
            continue
        # Str to int
        rx_df = rx_df.astype(
            {'packet_size': 'int32', 'address': 'int32', 'rx_time': 'int64', 'delay': 'int64'})
        # Keep just data received before end of TX time
        #rx_df = rx_df[rx_df['rx_time'] < (APP_START + app_run_time)*1e6]
        # Per UE throughput
        ue_addresses = set(rx_df['address'])
        for ue_address in ue_addresses:
            ue_df = rx_df[rx_df['address'] == ue_address]
            # If only 1 packet: skip
            time_diff = ue_df['rx_time'].max() - APP_START*1e6
            if(time_diff == 0):
                continue
            thr_entry_ue = ue_df['packet_size'].sum(
            )/(time_diff)*1e9*8/1e6  # [Mbit/s]
            thr_ue.append(thr_entry_ue)
        
    print(f"Warning: {empty} empty traces")

    return thr_ue


def get_e2e_avg_latency(results, tracename):
    avg_lat = []
    for result in results:
        if(check_errors(result) != 0):
            continue
        result_filenames = campaign.db.get_result_files(result['meta']['id'])
        result_filename = result_filenames[tracename]
        rx_df = pd.read_csv(result_filename, sep=' ', lineterminator='\n', skiprows=1,
                            names=['packet_size', 'address', 'rx_time', 'delay'])
        if(rx_df.size == 0):
            print('Warning: empty trace!')
            continue
        # Str to int
        rx_df = rx_df.astype(
            {'packet_size': 'int32', 'address': 'int32', 'rx_time': 'int64', 'delay': 'int64'})
        lat_entry = rx_df['delay'].mean()/1e6  # [ms]
        avg_lat.append(lat_entry)
    return avg_lat


def get_e2e_ue_latency(results, tracename):
    ovr_lat = []
    for result in results:
        if(check_errors(result) != 0):
            continue
        result_filenames = campaign.db.get_result_files(result['meta']['id'])
        result_filename = result_filenames[tracename]
        rx_df = pd.read_csv(result_filename, sep=' ', lineterminator='\n', skiprows=1,
                            names=['packet_size', 'address', 'rx_time', 'delay'])
        if(rx_df.size == 0):
            print('Warning: empty trace!')
            continue
        # Str to int
        rx_df = rx_df.astype(
            {'packet_size': 'int32', 'address': 'int32', 'rx_time': 'int64', 'delay': 'int64'})
        # Try to shrink size
        shrinked = [v for i, v in enumerate(
            np.sort(rx_df['delay']/1e6)) if i % 8 == 0]
        del rx_df
        ovr_lat.extend(shrinked)  # [ms]
    return ovr_lat

# Measures troughput at lower layers, not E2E


def get_throughput(results):
    thr = []
    for result in results:
        if(check_errors(result) != 0):
            continue
        result_filenames = campaign.db.get_result_files(result['meta']['id'])
        result_filename = result_filenames['RxPacketTrace.txt']
        rx_df = pd.read_csv(result_filename, sep='\t', lineterminator='\n', skiprows=1,
                            names=['frame', 'subf', 'firstSim', 'simNum', 'cid', 'rnti', 'tb', 'mcs',
                                   'rv', 'sinr', 'corr', 'tbler'])
        if(rx_df.size == 0):
            print('Warning: empty trace!')
            continue
        # Str to int
        rx_df = rx_df.astype({'frame': 'int32', 'tb': 'int32'})
        # Keep just data received before end of TX time
        rx_df = rx_df[rx_df['frame'] > APP_START]
        rx_df = rx_df[rx_df['frame'] < app_run_time + APP_START]
        data_tx_cut = rx_df['tb'].sum()
        thr.append(data_tx_cut/(app_run_time/1000))  # [ms]-->[s]
    return thr


def check_errors(result):
    result_filenames = campaign.db.get_result_files(result['meta']['id'])
    result_filename = result_filenames['stderr']
    with open(result_filename, 'r') as result_file:
        error_file = result_file.read()
        if (len(error_file) != 0):
            return 1
        else:
            return 0


def plot_cqi(result):
    for result in results:
        if(check_errors(result) != 0):
            continue
    result_filenames = campaign.db.get_result_files(result['meta']['id'])
    result_filename = result_filenames['CqiStatsTrace.txt']
    rx_df = pd.read_csv(result_filename, sep=' ',
                        lineterminator='\n', header=0, skiprows=0)
    # Keep just interesting data
    rx_df = rx_df[rx_df['time'] > 1e6*APP_START]
    rx_df = rx_df[rx_df['targetImsi'] > 200]  # Just UEs
    # Merge src and dst imsi to same col
    rx_df = rx_df.astype({'srcImsi': 'str', 'targetImsi': 'str'})
    rx_df['srcImsi'] = rx_df['srcImsi'] + ' - ' + rx_df['targetImsi']

    edges = set(rx_df['srcImsi'])
    fig, ax = plt.subplots()
    for edge in edges:
        temp_df = rx_df[rx_df['srcImsi'] == edge]
        temp_df.plot(x='time', y='cqi', ax=ax)
    plt.show()


def get_bsr_stats(results):
    ue_bsr = []
    node_bsr = []

    for result in results:
        if(check_errors(result) != 0):
            continue
        result_filenames = campaign.db.get_result_files(result['meta']['id'])
        result_filename = result_filenames['BsrStatsTrace.txt']
        rx_df = pd.read_csv(result_filename, sep=' ',
                            lineterminator='\n', header=0, skiprows=0)
        # Keep just interesting data
        rx_df = rx_df[rx_df['time'] > 1e6*APP_START]
        # rx_df = rx_df[rx_df['targetImsi'] > 200] # Just UEs
        # Split UEs and IABs links
        ue_df = rx_df[rx_df['targetImsi'] > 200]
        node_df = rx_df[rx_df['targetImsi'] < 200]
        # Merge src and dst imsi to same col
        #rx_df = rx_df.astype ({'srcImsi': 'str', 'targetImsi':'str'})
        #rx_df['srcImsi'] =  rx_df['srcImsi'] + ' - ' + rx_df['targetImsi']

        shrinked_ue = [v for i, v in enumerate(
            np.sort(ue_df['bsr'])) if i % 64 == 0]
        shrinked_node = [v for i, v in enumerate(
            np.sort(node_df['bsr'])) if i % 64 == 0]
        ue_bsr.extend(shrinked_ue)
        node_bsr.extend(shrinked_node)

    return node_bsr, ue_bsr

def get_bsr_quartile_vs_depth(results, quartile):
    bsr_map = {
        0: [],
        1: [],
        2: [],
        3: [] # Should be empty
        }
    quartile_vec = []
    depth_vec = []

    for result in results:
        if(check_errors(result) != 0):
            continue
        result_filenames = campaign.db.get_result_files(result['meta']['id'])
        result_filename = result_filenames['BsrStatsTrace.txt']
        rx_df = pd.read_csv(result_filename, sep=' ',
                            lineterminator='\n', header=0, skiprows=0)
        # Keep just interesting data
        rx_df = rx_df[rx_df['time'] > 1e6*APP_START]

        for depth in set(rx_df['depth']):
            # rx_df = rx_df[rx_df['targetImsi'] > 200] # Just UEs
            # Split UEs and IABs links
            bsr_df = rx_df[rx_df['depth'] == depth]
            temp = [v for i, v in enumerate(np.sort(bsr_df['bsr'])) if i % 64 == 0]
            bsr_map[depth].extend(temp)
    
    for depth in bsr_map.keys():
        if len(bsr_map[depth]) != 0:
            quartile_vec.append(np.percentile(bsr_map[depth], quartile))
            depth_vec.append(depth)

    return quartile_vec, depth_vec

# Compare distributed vs central allocation strategies


params_grid.update(centralizedSched=[False, True])
params_grid.update(weightPolicy=[1, 2, 3])
#params_grid.update(eta=[0, 5, 10, 20, 50, 100])
#params_grid.update(k=[1, 2, 3, 5])
#params_grid.update(muThreshold=[5, 10, 20])
#params_grid.update(packetSize=[50, 100, 200, 500])
params_orig = copy.deepcopy(params_grid)

results = campaign.db.get_results()
# Check for errors
errors = []
for result_entry in results:
    errors.append(check_errors(result_entry))
    #plot_cqi (result_entry)

num_errors = sum(errors)
print("Overall, we have " + str(num_errors) + " errors out of " + str(len(results)) + " simulations!")

#  # Get average e2e rates
# print('Per UE E2E throughput stats:')
# for size in params_orig['packetSize']:
#     plt.clf()
#     fig_cdf, ax_cdf = plt.subplots(constrained_layout=True)
#     ax_cdf.title.set_text(f'UE e2e throughput ECDF')
#     print(f'Packet size: {size} ')
#     # Non centralized scheduler results
#     params_grid.update (centralizedSched=[False], packetSize=size, weightPolicy=1)
#     results = campaign.db.get_results(params_grid)
#     thr_non_cen_ue = get_e2e_ue_throughput (results, UDP_APP_TRACEFILE)
#     data_count = len(thr_non_cen_ue)
#     ax_cdf.plot(np.sort(thr_non_cen_ue), np.array(range(data_count))/float(data_count), label='Non cen')
#     # Centralized scheduler results
#     for policy in params_orig['weightPolicy']:
#         params_grid.update (centralizedSched=[True], packetSize=size, weightPolicy=policy)
#         results = campaign.db.get_results(params_grid)
#         thr_cen_ue = get_e2e_ue_throughput (results, UDP_APP_TRACEFILE)
#         data_count = len(thr_cen_ue)
#         ax_cdf.plot(np.sort(thr_cen_ue), np.array(range(data_count))/float(data_count), label=f'Policy {policy}')

#     ax_cdf.set(xlabel='Per UE system throughput [Mbit/s]')
#     fig_cdf.legend(loc='lower right')
#     fig_cdf.savefig(f'Figures/Pdf/E2E_throughput_ECDF_packet_size_{size}.pdf')
#     tkz.clean_figure(fig=fig_cdf)
#     tkz.save(f'Figures/Tex/E2E_throughput_ECDF_packet_size_{size}.tex', figure=fig_cdf)
#     plt.close(fig_cdf)

# print('------------------')
# print('Average E2E throughput stats (& quartiles):')
# plt.clf()
# fig_avg, ax_avg = plt.subplots(constrained_layout=True)
# fig_first_quart, ax_first_quart = plt.subplots(constrained_layout=True)
# fig_third_quart, ax_third_quart = plt.subplots(constrained_layout=True)
# ax_avg.title.set_text(f'Overall e2e throughput')
# ax_first_quart.title.set_text(f'UE e2e throughput first quartile')
# ax_third_quart.title.set_text(f'UE e2e throughput third quartile')
# mean_thr = []
# first_quart = []
# third_quart = []
# # Non centralized scheduler results
# for size in params_orig['packetSize']:
#     params_grid.update (centralizedSched=[False], packetSize=size, weightPolicy=1, eta=[50], k=[2], muThreshold=[10])
#     results = campaign.db.get_results(params_grid) 
#     thr_avg = get_e2e_avg_throughput(results, UDP_APP_TRACEFILE)
#     thr_ue = get_e2e_ue_throughput(results, UDP_APP_TRACEFILE)
#     mean_thr.append(np.mean(thr_avg)) 
#     first_quart.append(np.percentile(thr_ue, 25))
#     third_quart.append(np.percentile(thr_ue, 75))
# ax_avg.plot(params_orig['packetSize'], mean_thr, label='Non cen')  
# ax_first_quart.plot(params_orig['packetSize'], first_quart, label='Non cen')
# ax_third_quart.plot(params_orig['packetSize'], third_quart, label='Non cen')
# # Centralized scheduler results
# for policy in params_orig['weightPolicy']:
#     mean_thr = []
#     first_quart = []
#     third_quart = []
#     for size in params_orig['packetSize']:
#         print(f'Packet size: {size} ')
#         params_grid.update (centralizedSched=[True], packetSize=size, weightPolicy=policy, eta=[50], k=[2], muThreshold=[10])
#         results = campaign.db.get_results(params_grid) 
#         thr_avg = get_e2e_avg_throughput (results, UDP_APP_TRACEFILE)
#         thr_ue = get_e2e_ue_throughput(results, UDP_APP_TRACEFILE)
#         mean_thr.append(np.mean(thr_avg)) 
#         first_quart.append(np.percentile(thr_ue, 25))
#         third_quart.append(np.percentile(thr_ue, 75))

#     ax_avg.plot (params_orig['packetSize'], mean_thr, label=f'Policy {policy}')  
#     ax_first_quart.plot(params_orig['packetSize'], first_quart, label=f'Policy {policy}')
#     ax_third_quart.plot(params_orig['packetSize'], third_quart, label=f'Policy {policy}')

# ax_avg.set(ylabel='System throughput [Mbit/s]', xlabel='Packet size')
# ax_first_quart.set(ylabel='System throughput [Mbit/s]', xlabel='Packet size')
# ax_third_quart.set(ylabel='System throughput [Mbit/s]', xlabel='Packet size')
# fig_avg.legend(loc='lower right')
# fig_avg.savefig('Figures/Pdf/E2E_avg_throughput.pdf')
# fig_first_quart.legend(loc='lower right')
# fig_first_quart.savefig('Figures/Pdf/E2E_first_quartile_throughput.pdf')
# fig_third_quart.legend(loc='lower right')
# fig_third_quart.savefig('Figures/Pdf/E2E_third_quartile_throughput.pdf')
# tkz.save('Figures/Tex/E2E_avg_throughput.tex', figure=fig_avg)
# tkz.save('Figures/Tex/E2E_first_quartile_throughput.tex', figure=fig_first_quart)
# tkz.save('Figures/Tex/E2E_third_quartile_throughput.tex', figure=fig_third_quart)
# plt.close(fig_avg)
# plt.close(fig_first_quart)
# plt.close(fig_third_quart)

# """
# # Get average rates
# print('------------------')
# print('Average overall throughput (@PHY layer):')
# for size in params_orig['packetSize']:
#     print(f'Packet size: {size} ')
#     # Non centralized scheduler results
#     params_grid.update (centralizedSched=[False], packetSize=size, weightPolicy=1)
#     results = campaign.db.get_results(params_grid) 
#     thr_non_cen_avg = get_throughput (results)
#     # Centralized scheduler results
#     for policy in params_orig['weightPolicy']:
#         params_grid.update (centralizedSched=[True], packetSize=size, weightPolicy=policy)
#         results = campaign.db.get_results(params_grid) 
#         thr_cen_avg = get_throughput (results)
#         print(f'\tPolicy: {policy}  throughput:  {np.mean(thr_cen_avg)/np.mean(thr_non_cen_avg)*100:.2f} %')
# """

# # Get average e2e latencies`
# print('------------------')
# print('Average E2E delays stats (& quartiles):')
# plt.clf()
# fig_avg, ax_avg = plt.subplots(constrained_layout=True)
# fig_first_quart, ax_first_quart = plt.subplots(constrained_layout=True)
# fig_third_quart, ax_third_quart = plt.subplots(constrained_layout=True)
# ax_first_quart.title.set_text(f'UE e2e delay first quartile')
# ax_third_quart.title.set_text(f'UE e2e delay third quartile')
# ax_avg.title.set_text(f'Overall e2e delay')
# # Non centralized scheduler results
# mean_del = []
# first_quart = []
# third_quart = []
# for size in params_orig['packetSize']:
#     params_grid.update (centralizedSched=[False], packetSize=size, weightPolicy=1, eta=[50], k=[2], muThreshold=[10])
#     results = campaign.db.get_results(params_grid) 
#     delay_avg = get_e2e_avg_latency (results, UDP_APP_TRACEFILE)
#     mean_del.append (np.mean(delay_avg)) # [ms]
#     del_ue = get_e2e_ue_latency(results, UDP_APP_TRACEFILE)
#     first_quart.append(np.percentile(del_ue, 25))
#     third_quart.append(np.percentile(del_ue, 75))
# ax_avg.plot (params_orig['packetSize'], mean_del, label='Non cen')  
# ax_first_quart.plot(params_orig['packetSize'], first_quart, label='Non cen')
# ax_third_quart.plot(params_orig['packetSize'], third_quart, label='Non cen')
# # Centralized scheduler results
# for policy in params_orig['weightPolicy']:
#     mean_del = []
#     first_quart = []
#     third_quart = []
#     for size in params_orig['packetSize']:
#         print(f'Packet size: {size} ')
#         params_grid.update (centralizedSched=[True], packetSize=size, weightPolicy=policy, eta=[50], k=[2], muThreshold=[10])
#         results = campaign.db.get_results(params_grid) 
#         delay_avg = get_e2e_avg_latency (results, UDP_APP_TRACEFILE)
#         mean_del.append (np.mean(delay_avg)) # [ms]
#         del_ue = get_e2e_ue_latency(results, UDP_APP_TRACEFILE)
#         first_quart.append(np.percentile(del_ue, 25))
#         third_quart.append(np.percentile(del_ue, 75))
#     ax_avg.plot (params_orig['packetSize'], mean_del, label=f'Policy {policy}')
#     ax_first_quart.plot(params_orig['packetSize'], first_quart, label=f'Policy {policy}')
#     ax_third_quart.plot(params_orig['packetSize'], third_quart, label=f'Policy {policy}')
      
# ax_avg.set(ylabel='E2E delay [ms]', xlabel='Packet size')
# ax_first_quart.set(ylabel='E2E delay [ms]', xlabel='Packet size')
# ax_third_quart.set(ylabel='E2E delay [ms]', xlabel='Packet size')
# fig_avg.legend(loc='lower right')
# fig_avg.savefig(f'Figures/Pdf/E2E_avg_delay.pdf')
# fig_first_quart.legend(loc='lower right')
# fig_first_quart.savefig('Figures/Pdf/E2E_first_quartile_delay.pdf')
# fig_third_quart.legend(loc='lower right')
# fig_third_quart.savefig('Figures/Pdf/E2E_third_quartile_delay.pdf')
# tkz.save('Figures/Tex/E2E_avg_delay.tex', figure=fig_avg)
# tkz.save('Figures/Tex/E2E_first_quartile_delay.tex', figure=fig_first_quart)
# tkz.save('Figures/Tex/E2E_third_quartile_delay.tex', figure=fig_third_quart)
# plt.close(fig_avg)
# plt.close(fig_first_quart)
# plt.close(fig_third_quart) 

# print('------------------')
# print('Per UE E2E delay stats:')
# for size in params_orig['packetSize']:
#     plt.clf()
#     fig_cdf, ax_cdf = plt.subplots(constrained_layout=True)
#     ax_cdf.title.set_text(f'UE e2e delay ECDF')
#     print(f'Packet size: {size} ')
#     # Non centralized scheduler results
#     params_grid.update (centralizedSched=[False], packetSize=size, weightPolicy=1)
#     results = campaign.db.get_results(params_grid)
#     del_non_cen_ue = get_e2e_ue_latency (results, UDP_APP_TRACEFILE)
#     data_count = len(del_non_cen_ue)
#     ax_cdf.plot(np.sort(del_non_cen_ue), np.array(range(data_count))/float(data_count), label='Non cen')
#     # Centralized scheduler results
#     for policy in params_orig['weightPolicy']:
#         params_grid.update (centralizedSched=[True], packetSize=size, weightPolicy=policy)
#         results = campaign.db.get_results(params_grid)
#         del_cen_ue = get_e2e_ue_latency (results, UDP_APP_TRACEFILE)
#         data_count = len(del_cen_ue)
#         ax_cdf.plot(np.sort(del_cen_ue), np.array(range(data_count))/float(data_count), label=f'Policy {policy}')

#     ax_cdf.set(xlabel='Per UE E2E delay [ms]')
#     fig_cdf.legend(loc='lower right')
#     tkz.clean_figure(fig=fig_cdf)
#     fig_cdf.savefig(f'Figures/Pdf/E2E_delay_ECDF_packet_size_{size}.pdf')
#     tkz.save(f'Figures/Tex/E2E_delay_ECDF_packet_size_{size}.tex', figure=fig_cdf)
#     plt.close(fig_cdf)

# print('BSR quartiles:')
# plt.clf()
# fig_first_quart, ax_first_quart = plt.subplots(constrained_layout=True)
# fig_third_quart, ax_third_quart = plt.subplots(constrained_layout=True)
# ax_first_quart.title.set_text(f'BSRs median')
# ax_third_quart.title.set_text(f'BSRs third quartile')
# # Non centralized scheduler results
# first_quart_ue = []
# third_quart_ue = []
# first_quart_node = []
# third_quart_node = []
# for size in params_orig['packetSize']:
#     params_grid.update (centralizedSched=[False], packetSize=size, weightPolicy=1, eta=[50], k=[2], muThreshold=[10])
#     results = campaign.db.get_results(params_grid)
#     bsr_node, bsr_ue = get_bsr_stats(results)
#     first_quart_ue.append(np.percentile(bsr_ue, 50))
#     third_quart_ue.append(np.percentile(bsr_ue, 75))
#     first_quart_node.append(np.percentile(bsr_node, 50))
#     third_quart_node.append(np.percentile(bsr_node, 75))
# ax_first_quart.plot(params_orig['packetSize'], first_quart_ue, label='Non cen - UE', linestyle='dashed')
# ax_third_quart.plot(params_orig['packetSize'], third_quart_ue, label='Non cen - UE', linestyle='dashed')
# ax_first_quart.plot(params_orig['packetSize'], first_quart_node, label='Non cen')
# ax_third_quart.plot(params_orig['packetSize'], third_quart_node, label='Non cen')
# # Centralized scheduler results
# for policy in params_orig['weightPolicy']:
#     first_quart_ue = []
#     third_quart_ue = []
#     first_quart_node = []
#     third_quart_node = []
#     for size in params_orig['packetSize']:
#         print(f'Packet size: {size} and policy: {policy}')
#         params_grid.update (centralizedSched=[True], packetSize=size, weightPolicy=policy, eta=[50], k=[2], muThreshold=[10])
#         results = campaign.db.get_results(params_grid)
#         bsr_node, bsr_ue = get_bsr_stats(results)
#         first_quart_ue.append(np.percentile(bsr_ue, 50))
#         third_quart_ue.append(np.percentile(bsr_ue, 75))
#         first_quart_node.append(np.percentile(bsr_node, 50))
#         third_quart_node.append(np.percentile(bsr_node, 75))
#     ax_first_quart.plot(params_orig['packetSize'], first_quart_ue, label=f'Policy {policy} - UE', linestyle='dashed')
#     ax_third_quart.plot(params_orig['packetSize'], third_quart_ue, label=f'Policy {policy} - UE', linestyle='dashed')
#     ax_first_quart.plot(params_orig['packetSize'], first_quart_node, label=f'Policy {policy}')
#     ax_third_quart.plot(params_orig['packetSize'], third_quart_node, label=f'Policy {policy}')

# ax_first_quart.set(ylabel='BSR [ms]', xlabel='Packet size')
# ax_third_quart.set(ylabel='BSR [ms]', xlabel='Packet size')
# fig_first_quart.legend(loc='lower right')
# fig_first_quart.savefig('Figures/Pdf/Median_BSR.pdf')
# fig_third_quart.legend(loc='lower right')
# fig_third_quart.savefig('Figures/Pdf/Third_quartile_BSR.pdf')
# tkz.save('Figures/Tex/Median_BSR.tex', figure=fig_first_quart)
# tkz.save('Figures/Tex/Third_quartile_BSR.tex', figure=fig_third_quart)
# plt.close(fig_first_quart)
# plt.close(fig_third_quart)

# print('BSR quartiles vs depth:')
# plt.clf()
# fig_third_quart, ax_third_quart = plt.subplots(constrained_layout=True)
# ax_third_quart.title.set_text(f'BSRs third quartile vs depth')
# # Non centralized scheduler results
# params_grid.update (centralizedSched=[False], packetSize=200, weightPolicy=1)
# results = campaign.db.get_results(params_grid)
# bsr, depth = get_bsr_quartile_vs_depth(results, 75)
# ax_third_quart.plot(depth, bsr, label='Non cen')
# # Centralized scheduler results
# for policy in params_orig['weightPolicy']:
#     params_grid.update (centralizedSched=[True], packetSize=200, weightPolicy=policy)
#     results = campaign.db.get_results(params_grid)
#     bsr, depth = get_bsr_quartile_vs_depth(results, 75)
#     ax_third_quart.plot(depth, bsr, label=f'Policy {policy}')

# ax_third_quart.set(ylabel='BSR [B]', xlabel='Packet size')
# fig_third_quart.legend(loc='lower right')
# fig_third_quart.savefig('Figures/Pdf/Third_quartile_BSR_vs_depth.pdf')
# tkz.save('Figures/Tex/Third_quartile_BSR_vs_depth.tex', figure=fig_third_quart)
# plt.close(fig_third_quart)

print('Metrics quartiles vs allocation period:')
fig_first_quart_thr, ax_first_quart_thr = plt.subplots(constrained_layout=True)
fig_first_quart_del, ax_first_quart_del = plt.subplots(constrained_layout=True)
first_quart_thr = []
third_quart_del = []

# Centralized scheduler results
for policy in params_orig['weightPolicy']:
    first_quart_thr = []
    third_quart_del = []
    for period in params_orig['allocationPeriod']:
        print(f'Allocation period: {period} - Policy: {policy}')
        params_grid.update (weightPolicy=policy, allocationPeriod=period)
        results = campaign.db.get_results(params_grid)
        thr_ue = get_e2e_ue_throughput(results, UDP_APP_TRACEFILE)
        del_ue = get_e2e_ue_latency(results, UDP_APP_TRACEFILE)
        first_quart_thr.append(np.percentile(thr_ue, 25))
        third_quart_del.append(np.percentile(del_ue, 75))

    ax_first_quart_thr.plot(params_orig['allocationPeriod'], first_quart_thr, label=f'Policy {policy}')
    ax_first_quart_del.plot(params_orig['allocationPeriod'], third_quart_del, label=f'Policy {policy}')

ax_first_quart_thr.set(ylabel='System throughput [Mbit/s]', xlabel='Allocation period')
fig_first_quart_thr.legend(loc='lower right')
fig_first_quart_thr.savefig('Figures/Pdf/Thr_first_quartile_vs_period.pdf')
tkz.save('Figures/Tex/Thr_first_quartile_vs_period.tex', figure=fig_first_quart_thr)
plt.close(fig_first_quart_thr)

ax_first_quart_del.set(ylabel='Delay [ms]', xlabel='Allocation period')
fig_first_quart_del.legend(loc='lower right')
fig_first_quart_del.savefig('Figures/Pdf/Del_first_quartile_vs_period.pdf')
tkz.save('Figures/Tex/Del_first_quartile_vs_period.tex', figure=fig_first_quart_del)
plt.close(fig_first_quart_del)

# print('Metrics quartiles vs policy 3 parameter K:')
# fig_quart_thr, ax_quart_thr = plt.subplots(constrained_layout=True)
# fig_quart_del, ax_quart_del = plt.subplots(constrained_layout=True)
# quart_thr = []
# quart_del = []

# for kVal in params_orig['k']:
#     print(f'K: {kVal} policy 3')
#     params_grid.update (centralizedSched=[True], packetSize=[100], weightPolicy=3, eta=[50], k=kVal, muThreshold=[10], numIabs=[5])
#     results = campaign.db.get_results(params_grid)
#     thr_ue = get_e2e_ue_throughput(results, UDP_APP_TRACEFILE)
#     del_ue = get_e2e_ue_latency(results, UDP_APP_TRACEFILE)
#     quart_thr.append(np.percentile(thr_ue, 25))
#     quart_del.append(np.percentile(del_ue, 75))
# ax_quart_thr.plot(params_orig['k'], quart_thr)
# ax_quart_del.plot(params_orig['k'], quart_del)

# # Centralized scheduler results
# for policy in [1, 2]:
#     print(f'policy {policy}')
#     params_grid.update (centralizedSched=[True], packetSize=[100], weightPolicy=policy, eta=[50], k=[2], muThreshold=[10], numIabs=[5])
#     results = campaign.db.get_results(params_grid)
#     thr_ue = get_e2e_ue_throughput(results, UDP_APP_TRACEFILE)
#     del_ue = get_e2e_ue_latency(results, UDP_APP_TRACEFILE)
#     quart_thr_entry = np.percentile(thr_ue, 25)
#     quart_del_entry = np.percentile(del_ue, 75)
#     ax_quart_thr.hlines(y=quart_thr_entry, xmin=1, xmax=3, label=f'Policy {policy}', color='r')
#     ax_quart_del.hlines(y=quart_del_entry, xmin=1, xmax=3, label=f'Policy {policy}', color='g')

# ax_quart_thr.set(ylabel='System throughput [Mbit/s]', xlabel='Parameter k')
# fig_quart_thr.legend(loc='lower right')
# fig_quart_thr.savefig('Figures/Pdf/Thr_first_quartile_vs_k.pdf')
# tkz.save('Figures/Tex/Thr_first_quartile_vs_k.tex', figure=fig_quart_thr)
# plt.close(fig_quart_thr)

# ax_quart_del.set(ylabel='Delay [ms]', xlabel='Parameter k')
# fig_quart_del.legend(loc='lower right')
# fig_quart_del.savefig('Figures/Pdf/Del_first_quartile_vs_k.pdf')
# tkz.save('Figures/Tex/Del_first_quartile_vs_k.tex', figure=fig_quart_del)
# plt.close(fig_quart_del)

# params_grid.update(centralizedSched=[True], packetSize=[100], weightPolicy=[3], eta=[0], k=[2], muThreshold=[10], numIabs=[5])
# results_mrba = campaign.db.get_results(params_grid)
# # Check for errors
# errors_mrba = []
# for result_entry in results_mrba:
#     errors_mrba.append(check_errors(result_entry))
#     #plot_cqi (result_entry)
# num_errors_mrba = sum(errors_mrba)

# params_grid.update(centralizedSched=[True], packetSize=[100], weightPolicy=[1], eta=[50], k=[2], muThreshold=[10], numIabs=[5])
# results_msr = campaign.db.get_results(params_grid)
# # Check for errors
# errors_msr = []
# for result_entry in results_msr:
#     errors_msr.append(check_errors(result_entry))
#     #plot_cqi (result_entry)
# num_errors_msr = sum(errors_msr)

# print(f'For policy 3 and eta 0, we have {num_errors_mrba} errors out of {len(results_mrba)} simulations!')
# print(f'For policy 1, we have {num_errors_msr} errors out of {len(results_msr)} simulations!')

# thr_ue_mrba = get_e2e_ue_throughput(results_mrba, UDP_APP_TRACEFILE)
# thr_ue_msr = get_e2e_ue_throughput(results_msr, UDP_APP_TRACEFILE)
# print(f'For policy 3 and eta 0, we have the following average throughput: {np.percentile(thr_ue_mrba, 50)}')
# print(f'For policy 1, we have the following average throughput: {np.percentile(thr_ue_msr, 50)}')

# print('Metrics quartiles vs policy 3 parameter ETA:')
# fig_quart_thr, ax_quart_thr = plt.subplots(constrained_layout=True)
# fig_third_quart_thr, ax_third_quart_thr = plt.subplots(constrained_layout=True)
# fig_quart_del, ax_quart_del = plt.subplots(constrained_layout=True)
# fig_third_quart_del, ax_third_quart_del = plt.subplots(constrained_layout=True)
# quart_thr = []
# third_quart_thr = []
# quart_del = []
# third_quart_del = []

# for etaVal in params_orig['eta']:
#     print(f'ETA: {etaVal} ')
#     params_grid.update(centralizedSched=[True], packetSize=[
#                        100], weightPolicy=3, eta=etaVal, k=[2], muThreshold=[10], numIabs=[5])
#     results = campaign.db.get_results(params_grid)
#     thr_ue = get_e2e_ue_throughput(results, UDP_APP_TRACEFILE)
#     del_ue = get_e2e_ue_latency(results, UDP_APP_TRACEFILE)
#     quart_thr.append(np.percentile(thr_ue, 25))
#     third_quart_thr.append(np.percentile(thr_ue, 75))
#     quart_del.append(np.percentile(del_ue, 25))
#     third_quart_del.append(np.percentile(del_ue, 75))
# ax_quart_thr.plot(params_orig['eta'], quart_thr)
# ax_quart_del.plot(params_orig['eta'], quart_del)
# ax_third_quart_thr.plot(params_orig['eta'], third_quart_thr)
# ax_third_quart_del.plot(params_orig['eta'], third_quart_del)

# # Centralized scheduler results
# for policy in [1, 2]:
#     params_grid.update(centralizedSched=[True], packetSize=[
#                        100], weightPolicy=policy, eta=[50], k=[2], muThreshold=[10], numIabs=[5])
#     results = campaign.db.get_results(params_grid)
#     thr_ue = get_e2e_ue_throughput(results, UDP_APP_TRACEFILE)
#     del_ue = get_e2e_ue_latency(results, UDP_APP_TRACEFILE)
#     quart_thr_entry = np.percentile(thr_ue, 25)
#     third_quart_thr_entry = np.percentile(thr_ue, 75)
#     quart_del_entry = np.percentile(del_ue, 25)
#     third_quart_del_entry = np.percentile(del_ue, 75)
#     ax_quart_thr.hlines(
#         y=quart_thr_entry, xmin=params_orig['eta'][0], xmax=params_orig['eta'][-1], label=f'Policy {policy}', color='r')
#     ax_quart_del.hlines(
#         y=quart_del_entry, xmin=params_orig['eta'][0], xmax=params_orig['eta'][-1], label=f'Policy {policy}', color='r')
#     ax_third_quart_thr.hlines(
#         y=third_quart_thr_entry, xmin=params_orig['eta'][0], xmax=params_orig['eta'][-1], label=f'Policy {policy}', color='r')
#     ax_third_quart_del.hlines(
#         y=third_quart_del_entry, xmin=params_orig['eta'][0], xmax=params_orig['eta'][-1], label=f'Policy {policy}', color='r')

# ax_quart_thr.set(ylabel='System throughput [Mbit/s]', xlabel='Parameter eta')
# fig_quart_thr.legend(loc='lower right')
# fig_quart_thr.savefig('Figures/Pdf/Thr_first_quartile_vs_eta.pdf')
# tkz.save('Figures/Tex/Thr_first_quartile_vs_eta.tex', figure=fig_quart_thr)
# plt.close(fig_quart_thr)

# ax_quart_del.set(ylabel='Delay [ms]', xlabel='Parameter eta')
# fig_quart_del.legend(loc='lower right')
# fig_quart_del.savefig('Figures/Pdf/Del_first_quartile_vs_eta.pdf')
# tkz.save('Figures/Tex/Del_first_quartile_vs_eta.tex', figure=fig_quart_del)
# plt.close(fig_quart_del)

# ax_third_quart_thr.set(
#     ylabel='System throughput [Mbit/s]', xlabel='Parameter eta')
# fig_third_quart_thr.legend(loc='lower right')
# fig_third_quart_thr.savefig('Figures/Pdf/Thr_third_quartile_vs_eta.pdf')
# tkz.save('Figures/Tex/Thr_third_quartile_vs_eta.tex',
#          figure=fig_third_quart_thr)
# plt.close(fig_quart_thr)

# ax_third_quart_del.set(ylabel='Delay [ms]', xlabel='Parameter eta')
# fig_third_quart_del.legend(loc='lower right')
# fig_third_quart_del.savefig('Figures/Pdf/Del_third_quartile_vs_eta.pdf')
# tkz.save('Figures/Tex/Del_third_quartile_vs_eta.tex',
#          figure=fig_third_quart_del)
# plt.close(fig_quart_del)

# print('Metrics quartiles vs policy 3 parameter MU THRESHOLD:')
# fig_quart_thr, ax_quart_thr = plt.subplots(constrained_layout=True)
# fig_quart_del, ax_quart_del = plt.subplots(constrained_layout=True)
# quart_thr = []
# quart_del = []

# for muVal in params_orig['muThreshold']:
#     print(f'MU THRESHOLD: {muVal} ')
#     params_grid.update (centralizedSched=[True], packetSize=[100], weightPolicy=3, eta=[50], k=[2], muThreshold=muVal, numIabs=[5])
#     results = campaign.db.get_results(params_grid)
#     thr_ue = get_e2e_ue_throughput(results, UDP_APP_TRACEFILE)
#     del_ue = get_e2e_ue_latency(results, UDP_APP_TRACEFILE)
#     quart_thr.append(np.percentile(thr_ue, 25))
#     quart_del.append(np.percentile(del_ue, 75))
# ax_quart_thr.plot(params_orig['muThreshold'], quart_thr)
# ax_quart_del.plot(params_orig['muThreshold'], quart_del)

# # Centralized scheduler results
# for policy in [1, 2]:
#     params_grid.update (centralizedSched=[True], packetSize=[100], weightPolicy=policy, eta=[50], k=[2], muThreshold=[10], numIabs=[5])
#     results = campaign.db.get_results(params_grid)
#     thr_ue = get_e2e_ue_throughput(results, UDP_APP_TRACEFILE)
#     del_ue = get_e2e_ue_latency(results, UDP_APP_TRACEFILE)
#     quart_thr_entry = np.percentile(thr_ue, 25)
#     quart_del_entry = np.percentile(del_ue, 75)
#     ax_quart_thr.hlines(y=quart_thr_entry, xmin=1, xmax=3, label=f'Policy {policy}', color='r')
#     ax_quart_del.hlines(y=quart_del_entry, xmin=1, xmax=3, label=f'Policy {policy}', color='g')

# ax_quart_thr.set(ylabel='System throughput [Mbit/s]', xlabel='Parameter muThreshold')
# fig_quart_thr.legend(loc='lower right')
# fig_quart_thr.savefig('Figures/Pdf/Thr_first_quartile_vs_muThreshold.pdf')
# tkz.save('Figures/Tex/Thr_first_quartile_vs_muThreshold.tex', figure=fig_quart_thr)
# plt.close(fig_quart_thr)

# ax_quart_del.set(ylabel='Delay [ms]', xlabel='Parameter muThreshold')
# fig_quart_del.legend(loc='lower right')
# fig_quart_del.savefig('Figures/Pdf/Del_first_quartile_vs_muThreshold.pdf')
# tkz.save('Figures/Tex/Del_first_quartile_vs_muThreshold.tex', figure=fig_quart_del)
# plt.close(fig_quart_del)
