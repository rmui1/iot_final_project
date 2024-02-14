import base64
from awsiot import mqtt5_client_builder
from awscrt import mqtt5, http
import threading
from concurrent.futures import Future
import time
import json

# based on https://github.com/aws/aws-iot-device-sdk-python-v2/blob/main/samples/mqtt5_pubsub.py

TIMEOUT = 100

sub_topic = 'esp32/info'
pub_topic = 'esp32/req'

received_all_event = threading.Event()
future_stopped = Future()
future_connection_success = Future()

image, weight = None, None
received_bad_info = False

def on_publish_received(publish_packet_data):
    publish_packet = publish_packet_data.publish_packet
    assert isinstance(publish_packet, mqtt5.PublishPacket)

    global image, weight, received_bad_info
    
    try:
        response = json.loads(publish_packet.payload) # receiving weight info
        if weight == None:
            try:
                weight = float(response['weight'])
            except:
                weight = None
                received_bad_info = True
    except: # receiving image info
        if image == None:
            try:
                image = publish_packet.payload
                image = base64.b64decode(image)
                with open('static/captured_image.jpg', 'wb') as f:
                    f.write(image)
            except:
                received_bad_info = True
                image = None

        if received_bad_info:
            time.sleep(1)

def on_lifecycle_stopped(lifecycle_stopped_data: mqtt5.LifecycleStoppedData):
    print('connection stopped...')
    global future_stopped
    future_stopped.set_result(lifecycle_stopped_data)

def on_lifecycle_connection_success(lifecycle_connect_success_data: mqtt5.LifecycleConnectSuccessData):
    print('connected!')
    global future_connection_success
    future_connection_success.set_result(lifecycle_connect_success_data)

def on_lifecycle_connection_failure(lifecycle_connection_failure: mqtt5.LifecycleConnectFailureData):
    print('could not connect:{}'.format(lifecycle_connection_failure.exception))

def get_pantry_info():
    client = mqtt5_client_builder.mtls_from_path(
            endpoint='a2ezfc6atnz7fy-ats.iot.us-east-2.amazonaws.com',
            cert_filepath='certificate.pem.crt',
            pri_key_filepath='private.pem.key',
            ca_filepath='AmazonRootCA1.pem',
            on_publish_received=on_publish_received,
            on_lifecycle_stopped=on_lifecycle_stopped,
            on_lifecycle_connection_success=on_lifecycle_connection_success,
            on_lifecycle_connection_failure=on_lifecycle_connection_failure,
            client_id='Flask_App')
    print("MQTT5 Client Created")

    client.start()
    lifecycle_connect_success_data = future_connection_success.result(TIMEOUT)
    connack_packet = lifecycle_connect_success_data.connack_packet
    negotiated_settings = lifecycle_connect_success_data.negotiated_settings

    print("Subscribing to topic '{}'...".format(sub_topic))
    subscribe_future = client.subscribe(subscribe_packet=mqtt5.SubscribePacket(
        subscriptions=[mqtt5.Subscription(
            topic_filter=sub_topic,
            qos=mqtt5.QoS.AT_LEAST_ONCE)]
    ))
    suback = subscribe_future.result(TIMEOUT)
    print("Subscribed with {}".format(suback.reason_codes))

    message = "send info please"
    print("Publishing message to topic '{}': {}".format(pub_topic, message))
    publish_future = client.publish(mqtt5.PublishPacket(
        topic=pub_topic,
        payload=json.dumps({"message":message}),
        qos=mqtt5.QoS.AT_LEAST_ONCE
    ))

    publish_completion_data = publish_future.result(TIMEOUT)
    print("PubAck received with {}".format(repr(publish_completion_data.puback.reason_code)))

    global image, weight, received_bad_info
    while image == None or weight == None or received_bad_info:
        if received_bad_info:
            message = "send info please"
            print("Publishing message to topic '{}': {}".format(pub_topic, message))
            publish_future = client.publish(mqtt5.PublishPacket(
                topic=pub_topic,
                payload=json.dumps({"message":message}),
                qos=mqtt5.QoS.AT_LEAST_ONCE
            ))

            publish_completion_data = publish_future.result(TIMEOUT)
            print("PubAck received with {}".format(repr(publish_completion_data.puback.reason_code)))
            received_bad_info = False

    print("Unsubscribing from topics")
    unsubscribe_future = client.unsubscribe(unsubscribe_packet=mqtt5.UnsubscribePacket(
        topic_filters=[pub_topic, sub_topic]))
    unsuback = unsubscribe_future.result(TIMEOUT)
    print("Unsubscribed with {}".format(unsuback.reason_codes))

    print("Stopping Client")
    client.stop()

    future_stopped.result(TIMEOUT)
    print("Client Stopped!")

    ret_image = image
    ret_weight = weight

    image = weight = None

    return ret_weight