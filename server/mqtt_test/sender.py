import paho.mqtt.client as mqtt
import json
import sys
import datetime as dt




def isoFormatParser(isoStr):
    return isoStr[0:4], isoStr[5:7], isoStr[8:10], isoStr[11:13], isoStr[14:16]

msgtype = sys.argv[1]
s2d_msgs = {'rtt'}
d2s_msgs = {'r', 'e'}
# Set topic
if msgtype in s2d_msgs:
    topic='medmate/dev/ESP32Test/s2d'
elif msgtype in d2s_msgs:
    topic='medmate/dev/ESP32Test/d2s'
else:
    print('bad message type')
    exit(0)


r2tJson = {'type': 'readyToTake', 'prescription': True, 'prescriptionID': 'kIKBEJUn0bD3tANWnG3G', 'readyToTake': True}

registerJson = {
    'type': 'register',
    'deviceId': 'ESP32test'
}

now = dt.datetime.now().isoformat()
y, m, d, hr, minute = isoFormatParser(now)

sEventJson = {
    'type': 's_event',
    'userID': '70odLXE4nfNCmghCKxNRImmeRMl2',
    'prescription': True,
    'prescriptionID': 'kIKBEJUn0bD3tANWnG3G',
    'date': int(y + m + d),
    'hour': int(hr),
    'min': int(minute)
}

client = mqtt.Client()
client.connect("mqtt.eclipse.org", 1883, 60)

if msgtype == 'r':
    msgJson = registerJson
if msgtype == 'e':
    msgJson = sEventJson

if msgtype == 'rtt':
    msgJson = r2tJson
    
msg = json.dumps(msgJson)

print(f'Sending {msg}\non topic {topic}')
client.publish(topic, msg)
