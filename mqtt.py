# python3.6

import base64
import random
import json
from paho.mqtt import client as mqtt_client
from mqtt_send import publish



port = 1883
# topic = "/ext/ntp/h5n2m9JLbea/showMeCode3/response"
topic = "/h5n2m9JLbea/showMeCode3/user/CCO"
# generate client ID with pub prefix randomly
# client_id = 'h5n2m9JLbea.showMeCode3|securemode=2,signmethod=hmacsha256,timestamp=1680314308616|'
# username = 'showMeCode3&h5n2m9JLbea'
# password = '2b9bdf67a6a142e94572b2d817d31c4464f70c405919d5d5dc6d0f2c7225fa61'
# broker = 'iot-06z00jfn2k9djbj.mqtt.iothub.aliyuncs.com'

client_id = 'Client4444'
broker = 'qlmsmart.com'
username = 'guest'
password = 'guest'

def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client
file = ""
fileName = ""

def subscribe(client: mqtt_client):
    global file, fileName
    def on_message(client, userdata, msg):
        global file, fileName
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        print(msg.payload.decode())
        # body = msg.payload.decode()
        # if body[0] == "4":
        #     file += body[16:-4]
        #     print(body) 
        # elif body == "end":
        #     with open(fileName, "wb") as f:
        #         f.write(bytes.fromhex(file))
        #     file = ""
        #     fileName = ""
        # elif body[:5] == "start":
        #     fileName = body.split(",")[1]


            
    


    client.subscribe(topic)
    client.on_message = on_message

def on_unsubscribe(client, userdata, mid):
    # 取消订阅成功后的回调函数
    print("Unsubscribed from topic")
    print(userdata)
    print(mid)

def publish(client):
    msg_count = 0
    while msg_count <1 or 0:
        msg = f"messages: {msg_count}"
        result = client.publish("/ext/ntp/h5n2m9JLbea/showMeCode3/request",json.dumps( {"deviceSendTime":"1571724098000"})) # calculate_checksum("5A0400290071008807"+generate_hex_string() )
        # result: [0, 1]
        status = result[0]
        if status == 0:
            print(f"Send `{msg}` to topic `{topic}`")
        else:
            print(f"Failed to send message to topic {topic}")
        msg_count += 1

def run():
    client = connect_mqtt()
    client.on_unsubscribe = on_unsubscribe
    subscribe(client)
    # publish(client)
    # result, mid = client.unsubscribe("/ext/ntp/h5n2m9JLbea/showMeCode3/request")
    # print("(~~~~~~)")
    # print(result)
    # print(mid)

    client.loop_forever()


if __name__ == '__main__':
    run()
