from machine import Pin
from machine import PWM
from machine import Timer
from machine import SoftI2C
from machine import ADC
# import machine
import esp32
import time
import urequests
import socket

# import hts221

wifi_ssid = 'ASUS'
wifi_psswrd = 'rickroll362'

# IFTTT api key
api_key = "cRY9n1jJnl-fCLuPYsZZ-8"
session_id = 100000


def internet_connect():
    import network
    import struct
    time.sleep(1)
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    time.sleep(3)
    while not wlan.isconnected():
        wlan.connect(wifi_ssid, wifi_psswrd)
        time.sleep(5)
    if wlan.isconnected():
        print('Oh Yes! Get connected')
        print('Connected to ASUS')
        mac = "%x:%x:%x:%x:%x:%x" % struct.unpack("BBBBBB", wlan.config('mac'))
        print('MAC Address: ' + mac)
        print('IP Address: ' + str(wlan.ifconfig()[0]))
        # time.sleep(.1)
    else:
        # if still not connected
        print('Not connected :-(')
    return wlan


def http_get(url):
    _, _, host, path = url.split('/', 3)
    # print(host)
    addr = socket.getaddrinfo(host, 80)[0][-1]
    # print(addr)
    s = socket.socket()
    s.connect(addr)
    s.send(bytes('GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n' % (path, host), 'utf8'))
    # print('Sent')
    # while True:
    #     data = s.recv(500)
    #     if data:
    #         print('Response')
    #         print(str(data, 'utf8'), end='')
    #         # comment out after debugging
    #     else:
    #         break
    s.close()
    # print('Closed socket')


# TODO update this
def update_sensors_IFTTT(_):
    # ISR for 2 second timer
    global session_id, hts, CO2_adc, methane_adc

    # hts.get_temp_humid_measurements()

    # temp = hts.__celsius_to_fahrenheit(hts.temperature)
    # humid = hts.relative_humidity

    CO2 = CO2_adc.read()
    methane = methane_adc.read()

    print('Methane:' + str(methane))
    print('CO2: ' + str(CO2))
    # print('Humid: ' + str(humid))

    session_str = "{:06d}".format(session_id)
    sensor_readings = {'value1': session_str,
                       'value2': str(CO2),
                       'value3': str(methane)
                       }
    print(sensor_readings)  # TODO comment this out
    session_id += 1
    try:
        request_headers = {'Content-Type': 'application/json'}
        request = urequests.post(
            'http://maker.ifttt.com/trigger/ece477_sensor_comp/with/key/' + api_key,
            json=sensor_readings,
            headers=request_headers)
        print(
            request.text)  # TODO comment this response request out - should be 'Congratulations! You've fired the lab6_reading event'
        request.close()
    except Exception as e:
        print('Failed to send readings: {}'.format(e))

    # send to thinkspeak as well

    # # send to thingspeak via socket api + http get request
    # url1 = "https://api.thingspeak.com/update?api_key=CRPM5S9C06JVXFRT&field1=" + str(temp)
    # # print("url1: " + url1)
    # http_get(url1)
    #
    # url2 = "https://api.thingspeak.com/update?api_key=CRPM5S9C06JVXFRT&field2=" + str(humid)
    # # print("url2: " + url2)
    # http_get(url2)
    #
    # url3 = "https://api.thingspeak.com/update?api_key=CRPM5S9C06JVXFRT&field3=" + str(methane)
    # # print("url3: " + url3)
    # http_get(url3)


def init_IFTTT_timer():
    sensor_update_timer = Timer(1)
    # callback every min.
    sensor_update_timer.init(period=3600000, mode=Timer.PERIODIC, callback=update_sensors_IFTTT)


# main

# ADC pin - A2 - ADC1
methane_adc = ADC(Pin(34))

# ADC pin - A3 - ADC1
CO2_adc = ADC(Pin(39))

# set to measure up to 3.3v
methane_adc.atten(ADC.ATTN_11DB)
methane_adc.width(ADC.WIDTH_12BIT)

CO2_adc.atten(ADC.ATTN_11DB)
CO2_adc.width(ADC.WIDTH_12BIT)

# i2c = SoftI2C(scl=Pin(22), sda=Pin(23))
# hts = hts221.HTS(i2c)

internet_connect()
init_IFTTT_timer()

