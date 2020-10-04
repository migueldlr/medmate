from firebase_admin import auth
import paho.mqtt.client as mqtt
import json
from mmglobals import active_users, mqttclient
import fbClient
from msgProcess import on_message

def on_connect(client, userdata, flags, rc):
    print("Connected to mqtt server. Result code: "+str(rc))
    client.subscribe("medmate/c/+/c2s")
    client.subscribe("medmate/dev/+/d2s")

def mqtt_bind():
    mqttclient.on_connect = on_connect
    mqttclient.on_message = on_message
    mqttclient.connect("mqtt.eclipse.org", 1883, 60)

def mqtt_loop():
    mqttclient.loop_forever()