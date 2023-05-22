# python 3.6

import json
import random
import socket
import time

from paho.mqtt import client as mqtt_client

import secrets, struct

local_ip = socket.gethostbyname(socket.gethostname())
print(local_ip)


def calculate_checksum(data):
    """
    计算数据的校验和。
    :param data: 要计算校验和的数据，以字节形式表示
    :return: 数据的校验和，以一个16进制字符串形式表示，不包含0x前缀
    """
    checksum = 0
    for b in data.encode():
        checksum += b
        checksum &= 0xFFFFFFFF  # 防止溢出，保证checksum始终是32位无符号整数
    checksum_bytes = struct.pack("!I", checksum)  # 将32位整数转换成4字节的字节串
    checksum_hex = (
        data + format(checksum_bytes[-1], "02x").upper() + "16"
    )  # 取最后一个字节作为校验和，并将其转换成16进制字符串
    return checksum_hex


len("1D008812AC454B0C02013364640005001000FFFFFFFFFFFFFFFF14FF01FF05")


def generate_hex_string():
    """
    生成长度为12的随机16进制字符串。
    :return: 生成的随机16进制字符串
    """
    return secrets.token_hex(31).upper()


port = 1883
topic = "/h5n2m9JLbea/showMeCode3/user/MCU"
# topic = "/ext/ntp/h5n2m9JLbea/showMeCode3/request"
# generate client ID with pub prefix randomly
client_id = "h5n2m9JLbea.showMeCode3|securemode=2,signmethod=hmacsha256,timestamp=1680314308616|"
username = "showMeCode3&h5n2m9JLbea"
password = "2b9bdf67a6a142e94572b2d817d31c4464f70c405919d5d5dc6d0f2c7225fa61"
broker = "iot-06z00jfn2k9djbj.mqtt.iothub.aliyuncs.com"

# client_id = "test111Client1"
# broker = "qlmsmart.com"
# username = "guest"
# password = "guest"


def connect_mqtt():
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


def publish(client):
    msg_count = 0
    while msg_count < 20 or 0:
        msg = f"messages: {msg_count}"
        result = client.publish(
            topic,
            calculate_checksum("5A0400290071008807" + generate_hex_string()),
            qos=1,
        )  # calculate_checksum("5A0400290071008807"+generate_hex_string() )
        # result: [0, 1]
        status = result[0]
        if status == 0:
            print(f"Send `{msg}` to topic `{topic}`")
        else:
            print(f"Failed to send message to topic {topic}")
        msg_count += 1
        # time.sleep(1)


def run():
    client = connect_mqtt()
    client.loop_start()
    time.sleep(4)
    publish(client)


if __name__ == "__main__":
    run()

#  json.dumps({
# "code":400,
# "deviceName":"test" ,
# "area":"00 00",
# "address":"00 00",
# "action":"stopBlink",
# "params":"",
# "identity":""
# })
