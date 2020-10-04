
import paho.mqtt.client as mqtt
import json
import mqttservice
import threadservice
import fbClient
import queue

def main():
    threadservice.init_threads()
    fbClient.init_fb()
    mqttservice.mqtt_bind()
    mqttservice.mqtt_loop()

    pass



if __name__ == "__main__":
    main()