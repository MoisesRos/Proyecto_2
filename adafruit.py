import sys
import time
import serial
from Adafruit_IO import MQTTClient

# CREDENCIALES DE ADAFRUIT IO
ADAFRUIT_IO_USERNAME = "MoisesRos"
ADAFRUIT_IO_KEY = "borrado por seguridad"

# NOMBRES DE LOS FEEDS DE ADAFRUIT IO
feedservo1 = "Servo_1"
feedservo2 = "Servo_2"
feedservo3 = "Servo_3"
feedservo4 = "Servo_4"

# DEFINICIÓN DE FUNCIONES DE CALLBACK

def connected(client):
    # FUNCIÓN QUE SE LLAMA CUANDO EL CLIENTE SE CONECTA A ADAFRUIT IO
    print('Esperando datos...')
    # SUSCRIPCIÓN A LOS FEEDS DE LOS SERVOS
    client.subscribe(feedservo1)
    client.subscribe(feedservo2)
    client.subscribe(feedservo3)
    client.subscribe(feedservo4)

def disconnected(client):
    # FUNCIÓN QUE SE LLAMA CUANDO EL CLIENTE SE DESCONECTA DE ADAFRUIT IO
    print('Disconnected from Adafruit IO!')
    sys.exit(1)

def message(client, feed_id, payload):
    # FUNCIÓN QUE SE LLAMA CUANDO SE RECIBE UN NUEVO MENSAJE EN UN FEED SUSCRITO
    print('Feed {0} received new value: {1}'.format(feed_id, payload))

    if feed_id == feedservo1:
        print("Servo 1")
        arduino.write(bytes('6' + payload + 'F', 'utf-8'))
    if feed_id == feedservo2:
        print("Servo 2")
        arduino.write(bytes('7' + payload + 'F', 'utf-8'))
    if feed_id == feedservo3:
        print("Servo 3")
        arduino.write(bytes('8' + payload + 'F', 'utf-8'))
    if feed_id == feedservo4:
        print("Servo 4")
        arduino.write(bytes('9' + payload + 'F', 'utf-8'))

try:
    # CREACIÓN DE UNA INSTANCIA DEL CLIENTE MQTT
    client = MQTTClient(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)

    # CONFIGURACIÓN DE LAS FUNCIONES DE CALLBACK
    client.on_connect = connected
    client.on_disconnect = disconnected
    client.on_message = message

    # CONEXIÓN AL SERVIDOR DE ADAFRUIT IO
    client.connect()

    # USO DEL LOOP EN SEGUNDO PLANO PARA GESTIONAR MENSAJES
    client.loop_background()

    # CONFIGURACIÓN DEL PUERTO SERIE PARA LA COMUNICACIÓN CON EL ATMEGA
    arduino = serial.Serial(port='COM10', baudrate=9600, timeout=0.1)

    # BUCLE PRINCIPAL
    while True:
        # LECTURA DE MENSAJES DEL ATMEGA
        mensaje = arduino.readline().decode('utf-8')
        time.sleep(3)

except KeyboardInterrupt:
    # MANEJO DE LA INTERRUPCIÓN POR TECLADO (CTRL+C)
    print("Programa terminado.")
    if arduino.is_open:
        arduino.close()
    sys.exit(1)
