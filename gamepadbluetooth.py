import serial
from struct import pack
from time import sleep, time
from inputs import get_gamepad


def ser_conn(): #connect to the arduino over serial. runs once at the beginning
    ser = serial.Serial("COM8", 115200, timeout=0.01)
    ser.reset_input_buffer()
    sleep(3)
    return ser

gamepad_map = { # what does each event code mean?
    "ABS_Y":'Ljoy_y',
    "ABS_RY":'Rjoy_y'
}

'''
Unused buttons

BTN_WEST 1 or 0
BTN_EAST 1 or 0
BTN_NORTH 1 or 0
BTN_SOUTH 1 or 0
BTN_TL 1 or 0
BTN_TR 1 or 0
ABs_Z 0 to 255
ABS_RZ 0 to 255

L = 1/2 x + y
R = -1/2 x + y 
'''

gamepad_read = { # mapping each input name to its most recent reading. values are floats from -1 to 1. dict order dictates setpoint send order
    'Ljoy_y':128,
    'Rjoy_y':128,
    'trigger':0
}


def input_loop():
    ser = ser_conn()
    while True:
        try:
            gamepad = get_gamepad()
            for event in gamepad:
                #print(event.ev_type, event.code, event.state)
                event_code = event.code
                if event_code in {"BTN_TL", "BTN_TR", "ABS_Y", "ABS_RY"}:
                    if event.code == "ABS_Y" or event.code == "ABS_RY":
                        gamepad_read[gamepad_map[event.code]] = max(min(255, round((event.state / 32768 + 1) * 128 )), 0) # store the value to whatever input is corresponds with
                    else:
                        gamepad_read['trigger'] = event.state
                    print("Sending " + str(list(gamepad_read.values())))
                    ser.write(pack("ccc", *[pt.to_bytes(1, 'little') for pt in gamepad_read.values()])) #send the message over serial
                
        except:
            print('no gamepad found')

input_loop()