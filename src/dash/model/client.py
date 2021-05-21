#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import time
from network_models.dash_MLP_v2 import Agent
from os.path import exists

client_name = sys.argv[1]
temperature = float(sys.argv[2])

print("Initializing client %s with exploration temperature %f" % (client_name, temperature))


#%%############## PARAMETERS
pretrained_client_path = 'pretrain/client/pretrained_model/pretrained_model'
trained_client_path = 'trained_client/trained_client'

# Network parameters
batch_size = 2000
replay_memory_size = 5000
gamma=0.9
target_network_update_step=50
n_hidden_1=128
n_hidden_2=256
n_input=9
n_classes=8
learning_rate=0.0001
dropout_keep_prob=1

#%%

req_filename  = client_name + "_req"
req_filename_flag  = client_name + "_req_flag"
resp_filename = client_name + "_resp"
resp_filename_flag = client_name + "_resp_flag"

### Inizialize network
agent = Agent(batch_size=batch_size,
              replay_memory_size=replay_memory_size,
              gamma=gamma,
              target_network_update_step=target_network_update_step,
              n_hidden_1=n_hidden_1,
              n_hidden_2=n_hidden_2,
              n_input=n_input,
              n_classes=n_classes,
              learning_rate=learning_rate,
              dropout_keep_prob=dropout_keep_prob)

# Load model
if exists(trained_client_path+'.meta'):
    print('Client %s - Restoring TRAINED weights from %s' % (client_name, trained_client_path))
    agent.load_model(trained_client_path)
else:
    print('Client %s - Restoring PRETRAINED weights from %s' % (client_name, pretrained_client_path))
    agent.load_model(pretrained_client_path)

def get_action(state):
    action, out_layer = agent.choose_action_softmax(state, temperature)
    return action, out_layer


## Loop till the listening file exists
while exists(client_name + "_listening"):
    
    ## Check for request file
    if exists(req_filename_flag) and exists(req_filename):
        ## Read the state
        with open(req_filename, 'r') as req_file:
            req_lines = req_file.readlines()
        os.remove(req_filename)
        os.remove(req_filename_flag)
        state = [float(l) for l in req_lines]
        ## Evaluate the action
        action, out_layer = get_action(state)
        print("python - %s - Received state: %s - Action: %d" % (client_name, state, action))
        print("python - %s - Output layer: %s" % (client_name, out_layer))
        ## Write response file
        resp_file = open(resp_filename, 'w')
        resp_file.writelines(str(action))
        resp_file.close()
        ## Write response file flag
        resp_file = open(resp_filename_flag, 'w')
        resp_file.close()

    time.sleep(0.0001)

print("python - Client " + client_name + " closed")