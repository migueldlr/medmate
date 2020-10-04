from firebase_admin import auth
import paho.mqtt.client as mqtt
import json
import fbClient
import threadservice
import datetime as dt
from mmglobals import active_users, mqttclient, connected_devices
from threadservice import date_from_ts, times_from_ts
from time import sleep

def process_dev_msg(dev_name, msgDict):
    print(f'Received message:\n{msgDict}')
    if 'type' not in msgDict:
        print('no msg type in device msg', msgDict)
        return
    if msgDict['type'] == 'hb':
        print("Received Heartbeat from " + dev_name)
        send_dev_hb(dev_name)
    elif msgDict['type'] == 'register':
        print("Received register message from " + dev_name)
        register_device(dev_name, msgDict)
    elif msgDict['type'] == 'alert':
        print("Received alert message from " + dev_name)
        dev_alert(dev_name, msgDict)
    elif msgDict['type'] == 's_event':
        print("Received s_event message from " + dev_name)
        handle_take_event(dev_name, msgDict)

def process_user_msg(uid, msgDict):
    if 'type' not in msgDict:
        print('no msg type in user msg', msgDict)
        return
    if msgDict['type'] == 'hb':
        send_clt_hb(uid)
    elif msgDict['type'] == 'login_verify':
        verify_login(uid, msgDict)
    # elif msgDict['type'] == 'new_pres'

def send_user_msg(uid, message_dict):
    mqttclient.publish('medmate/c/{}/s2c'.format(uid), json.dumps(message_dict))

def send_dev_msg(dev_id, message_dict):
    print(f'sending this to {dev_id}:\n{message_dict}')
    mqttclient.publish('medmate/dev/{}/s2d'.format(dev_id), json.dumps(message_dict))


def on_message(client, userdata, msg):
    topic = msg.topic.split('/')
    
    try:
        msgDict = json.loads(msg.payload.decode('utf-8'))
    except Exception as e:
        print(e)
        return
    
    client_type = topic[1]
    if client_type == 'c':
        process_user_msg(topic[2], msgDict)
    elif client_type == 'dev':
        process_dev_msg(topic[2], msgDict)
    else:
        print(f'bad topic {client_type}')

    # print(topic[1]+" "+str(msg.payload))

def send_clt_hb(uid):
    builder = {'type': 'hb'}
    mqttclient.publish(f'medmate/c/{uid}/s2c', json.dumps(builder))

def send_dev_hb(dev_name):
    builder = {'type': 'hb'}
    mqttclient.publish(f'medmate/dev/{dev_name}/s2d', json.dumps(builder))


def register_device(dev_name, msgDict):
    dev_dict = fbClient.get_dev_info(dev_name)
    pid = fbClient.get_pid_for_dev(dev_name, dev_dict['userId'])
    pres_dict = fbClient.get_pres_info(dev_dict['userId'], pid)
    uid = dev_dict['userId']
    if uid not in active_users:
        active_users[uid] = {}
    if 'devs' not in active_users[uid]: 
        active_users[uid]['devs'] = {}

    active_users[uid]['devs'][dev_name] = 1
    connected_devices[dev_name] = {'userId': uid}
    is_pres = pres_dict['is_pres']
    amount = pres_dict['amt']
    if is_pres:
        connected_devices[dev_name]['pres'] = True
        connected_devices[dev_name]['prescriptionId'] = pid
        connected_devices[dev_name]['readyToTake'] = False
        connected_devices[dev_name]['thread_id'] = None
        connected_devices[dev_name]['timeToTake'] = threadservice.set_time_to_take(dev_name, pres_dict)
        connected_devices[dev_name]['amount'] = amount
    
    # prescription == !readyToTake
    register_ack_builder = {
        'type': 'register_ack',
        'userID': uid,
        'prescription': is_pres,
        'prescriptionID': '' if not is_pres else dev_dict['prescriptionId'],
        'readyToTake': not is_pres
    }
    sleep(1)
    send_dev_msg(dev_name, register_ack_builder)

def verify_login(uid, msgDict):
    if 'token' not in msgDict:
        print("login attempt failed")
    authed_user = auth.verify_id_token(msgDict['token'])
    user_uid = authed_user['uid']
    if user_uid not in active_users:
        active_users[user_uid] = {'devs': {}}
    fbClient.add_user_devs(user_uid)
    ack_builder = {'type': 'login_ack', 'success': True}
    send_user_msg(user_uid, ack_builder)

def dev_alert(dev_name, msgDict):
    if  'date' not in msgDict or \
        'hour' not in msgDict or \
        'min' not in msgDict:
        print("Bad alert msg")
    today = dt.date.today()
    time = dt.time(msgDict['hour'], msgDict['min'])
    d = dt.datetime.combine(today, time)

    
    uid = connected_devices[dev_name]['userId']
    pid = connected_devices[dev_name]['prescriptionId']

    fbClient.post_alert(uid, pid, dt.datetime.now(), "putdown")

def send_ready_to_take(dev_name):
    builder = {
        'type': 'readyToTake',
        'prescription': True,
        'prescriptionID': connected_devices[dev_name]['prescriptionId'],
        'readyToTake': True,
        'amount': connected_devices[dev_name]['amount']
    }
    send_dev_msg(dev_name, builder)
    uid = connected_devices[dev_name]['userId']
    if uid is None:
        print("uid is none")
        return
    pid = connected_devices[dev_name]['prescriptionId']

    fbClient.post_alert(uid, pid, dt.datetime.now(), "rtt")


def handle_take_event(dev_name, msgDict):
    print(f"s_event received from {dev_name}")
    uid = msgDict['userID']
    if uid is None:
        print("uid is none")
        return
    today = dt.datetime.today()
    e_date = str(today.year) + str(today.month) + str(msgDict['date'])
    e_time = str(msgDict['hour']) + str(msgDict['min'])

    if msgDict['prescription']:
        # handle ReadyToTake==False, probably just alert the user
        pres_dict = fbClient.get_pres_info(uid, msgDict['prescriptionID'])
        name, amt, unit = pres_dict['name'], pres_dict['amt'], pres_dict['u']
        ttt_str = (connected_devices[dev_name]['timeToTake'] + dt.timedelta(minutes=1)).isoformat()
        pres_dict['a_date'] = ttt_str[0:4] + ttt_str[5:7] + ttt_str[8:10]
        pres_dict['a_time'] = ttt_str[11:13] + ttt_str[14:16]
        fbClient.update_a_date(dev_name, uid, msgDict['prescriptionID'], pres_dict)
        connected_devices[dev_name]['readyToTake'] = False
        connected_devices[dev_name]['timeToTake'] = threadservice.set_time_to_take(dev_name, pres_dict)
    else:
        dev_dict = fbClient.get_dev_info(dev_name)
        name = dev_dict['medName']
        amt = -1
        unit = dev_dict['u']
        
    today = dt.date.today()
    time = dt.time(msgDict['hour'], msgDict['min'])
    d = dt.datetime.combine(today, time)
    pid = msgDict['prescriptionID']

    fbClient.post_history_item(uid, pid, d)
    
    