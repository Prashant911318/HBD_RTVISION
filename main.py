from nicegui import ui
import paho.mqtt.client as mqtt
import time 
import threading
import json
import nicegui

current_zmq = 0
power_zmq = 0
pressure_zmq = 0


class NanoMQClient:
    def __init__(self):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
        self.client.on_connect = self.on_connect
        self.client.on_disconnect = self.on_disconnect
        self.client.message_callback_add("sensor_sub", self.on_message_topic1)
        self.connected = False
        self.stop_event = threading.Event()
        self.retry_interval = 5  # Retry every 5 seconds
        self.init_motor_position = -1
    def publish(self, topic, payload):
        topic = "async-mqtt/Portenta_H7_Ethernet_Pub"
        payload = json.dumps(payload)
        try:
            self.client.publish(topic, payload)
            print(f"Published to {topic}: {payload}")
        except Exception as e:
            print(f"Failed to publish: {e}")

    def gpio(self, pin, value):
        payload = {}
        args = {}
        args["GPIO"] = pin
        args["VALUE"] = value
        payload["CMD"] = "test"
        payload["ARGS"] = args
        self.publish("async-mqtt/Portenta_H7_Ethernet_Pub", payload)

    def motor(self, value):
        payload = {}
        args = {}
        args["VALUE"] = value
        payload["CMD"] = "motor"
        payload["ARGS"] = args
        self.publish("async-mqtt/Portenta_H7_Ethernet_Pub", payload)

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print("Connected to NanoMQ broker.")
            self.connected = True
            client.subscribe("sensor_sub")
        else:
            print(f"Connection failed with code {rc}")

    def on_disconnect(self, client, userdata, rc):
        print("Disconnected from NanoMQ broker.")
        self.connected = False
        if not self.stop_event.is_set():
            print("Attempting reconnection...")
            self.reconnect()

    def on_message_topic1(self, client, userdata, msg):
        #print(f"Received on {msg.topic}: {msg.payload.decode()}")
        global current_zmq, power_zmq, pressure_zmq
        data = json.loads(msg.payload.decode())
        current_zmq = data.get("Current", 0)
        power_zmq = data.get("Power", 0)
        temp = data.get("Temperature", 0)

        Heater_temp.set_text(str(temp) + ' °C')
        current_value.set_text(str(current_zmq) + ' A')
        power_value.set_text(str(power_zmq) + ' W')
        Power_value_slide.set_value(power_zmq)

        motor_position = data.get("Motor_Position", 0)
        motor_position = round(motor_position, 1)
        if self.init_motor_position <=0:
            #print('Update motor position')
            #print(motor_position)
            self.init_motor_position = motor_position
            motor_knob.set_value(motor_position)
        #round to 1 decimal
        
        Motor_pos.set_text("Motor Position: " + str(motor_position) + ' °')
        # Process data and publish acknowledgment to topic2
        ack_msg = f"Ack: {msg.payload.decode().upper()}"
        #client.publish("topic2", ack_msg)
        #print(f"Published to topic2: {ack_msg}")

    def reconnect(self):
        while not self.connected and not self.stop_event.is_set():
            try:
                print(f"Retrying connection in {self.retry_interval} seconds...")
                time.sleep(self.retry_interval)
                self.client.reconnect()
                break
            except Exception as e:
                print(f"Reconnection failed: {e}")

    def start(self):
        self.thread = threading.Thread(target=self.run)
        self.thread.start()

    def run(self):
        while not self.stop_event.is_set():
            try:
                if not self.connected:
                    print("Connecting to NanoMQ broker...")
                    self.client.connect("mqtt.eclipseprojects.io", 1883, 60)
                    #self.client.connect("192.168.29.120", 1883, 60)
                    #self.client.connect("97.74.86.232", 1883, 60)
                    self.client.loop_forever()
                time.sleep(1)  # Prevent tight loop
            except Exception as e:
                print(f"Initial connection failed: {e}. Retrying...")
                time.sleep(self.retry_interval)

    def stop(self):
        self.stop_event.set()
        self.client.disconnect()
        self.client.loop_stop()
        self.thread.join()
#Add a card 

nano_client = NanoMQClient()
nano_client.start()



#Add a knob
with ui.card():
    with ui.row():
        with ui.column():
            with ui.card():
                with ui.row():
                    ui.label('Motor Control').style('font-size: 20px; font-weight: bold; color: blue-5;')

                with ui.row():
                #Add a knob to show current value
                    motor_knob = ui.knob(0, show_value=True, track_color='grey-2', size='200px', color='blue-5', max=300, min=0, step=1).on_value_change(lambda e: nano_client.motor(e.value))
                    Motor_switch = ui.switch('Motor').on_value_change(lambda e: nano_client.gpio(1, e.value))
                with ui.row():
                    Motor_pos = ui.label('Motor Position').style('font-size: 20px; font-weight: bold; color: blue-5;')
        with ui.column():
            with ui.row():
                with ui.card():
                    with ui.row():
                        ui.label('Heater Control').style('font-size: 20px; font-weight: bold; color: blue-5;')
                        Heater_switch = ui.switch('Heater').on_value_change(lambda e: nano_client.gpio(2, e.value))
                        Heater_temp = ui.label('Temperature').style('font-size: 20px; color: blue-5;')
                        Heater_temp.set_text('0 °C')
            with ui.row():
                with ui.card().style('width: 330px;'):
                    with ui.row():
                        ui.label('Blower Control').style('font-size: 20px; font-weight: bold; color: blue-5;')
                        Blower_switch = ui.switch('Blower').on_value_change(lambda e: nano_client.gpio(3, e.value))
                        

        
 

        #Add a slider to show current value
    with ui.row():
        with ui.card():
            with ui.row():
                ui.label('General Diag').style('font-size: 20px; font-weight: bold; color: blue-5;')
            with ui.row():    
                ui.label('Current Consumption').style('font-size: 20px; font-weight: bold; color: blue-5;')
                current_value = ui.label( str(current_zmq) + ' A').style('font-size: 20px;')

            with ui.row():
                ui.label('Power').style('font-size: 20px; font-weight: bold; color: blue-5;')
                Power_value_slide = ui.slider(min=0, max=100, value=0).props('label-always')
                Power_value_slide.style('width: 440px;')
                power_value = ui.label(str(power_zmq) + ' W').style('font-size: 20px;')
            with ui.row():
                ui.label('Pressure').style('font-size: 20px; font-weight: bold; color: blue-5;')
                pressure_value = ui.label('0 psi').style('font-size: 20px;')




ui.run()