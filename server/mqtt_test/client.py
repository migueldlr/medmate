import paho.mqtt.client as mqtt
import json

topic='medmate/dev/ESP32Test/+'
mode='json' #set to raw or json

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe(topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print('message received.')
    print(f'topic: {msg.topic}')
    if mode == 'json':
        try:
            msgDict = json.loads(msg.payload.decode('utf-8'))
            for key in msgDict:
                print(f'{key}: {msgDict[key]}')
        except Exception as e:
            print(e)
            print(msg.payload.decode('utf-8'))
    elif mode == 'raw':
        print(str(msg.payload))

    print()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("mqtt.eclipse.org", 1883, 60)

while True:
    client.loop()
