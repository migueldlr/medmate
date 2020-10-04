import paho.mqtt.client as mqtt

active_users = {}
mqttclient = mqtt.Client()
connected_devices = {}