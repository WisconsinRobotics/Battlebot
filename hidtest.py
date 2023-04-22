import hid  # remember to install the hid library from https://trezor.github.io/cython-hidapi/index.html
from time import sleep
import serial
from struct import pack

gamepads = {} # mapping of gamepad objects to serial connection objects

def ser_conn(name): #connect to the arduino over serial. runs once at the beginning
    ser = serial.Serial(name, 115200, timeout=0.01)
    ser.reset_input_buffer()
    sleep(3)
    return ser

for hid_device in hid.enumerate(): # look at all connected hid devices and save the ones that match
    if hid_device['product_string'] == 'Pro Controller':
        gamepad = hid.device()
        gamepad.open_path(hid_device['path'])
        gamepad.set_nonblocking(True)
        gamepads[gamepad] = None


'''
========== Change these COM ports to reflect the outgoing bluetoothserial COM ports on your machine =========
'''
names = ["COM4", "COM8"]


for i, gamepad in enumerate(gamepads.keys()): # populate the dictionary with serial connections
    try:
        gamepads[gamepad] = ser_conn(names[i])
    except:
        print(f'Serial connection failed on {names[i]}')
        sleep(1)

fields = [8, 11] # relevant fields

while 1:
    for gamepad in gamepads.keys(): # alternate between gamepads
        try:
            reading = gamepad.read(255) # read 255 bytes from the hid api
            if reading: # sometimes no reading is available and none will be returned
                positions = [] # stores the positions of the analog sticks from 0 to 255
                for i in fields: # read the positions of the analog sticks into a list
                    positions.append(reading[i])
                try:
                    gamepads[gamepad].write(pack("ccc", *[pt.to_bytes(1, 'little') for pt in positions + [0]])) #send the message over serial
                    print(f'Sending {positions}')
                except Exception as e:
                    print(e)
        except Exception as e:
            print(e)




'''
0x057e:0x2009 Pro Controller 8, 11
'''