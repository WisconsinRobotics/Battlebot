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
    # Hammer Button
    # Reset Button
}

gamepad_read = { # mapping each input name to its most recent reading. values are floats from -1 to 1. dict order dictates setpoint send order
    'Ljoy_y':128,
    'Rjoy_y':128
}


def input_loop():
    ser = ser_conn()
    while True:
        try:
            gamepad = get_gamepad()
            for event in gamepad:
                #print(event.ev_type, event.code, event.state)
                if event.code == "ABS_Y" or event.code == "ABS_RY":
                    gamepad_read[gamepad_map[event.code]] = max(min(255, round((event.state / 32768 + 1) * 128 )), 0) # store the value to whatever input is corresponds with
                    print("Sending " + str(list(gamepad_read.values())))
                    ser.write(pack("cc", *[pt.to_bytes(1, 'little') for pt in gamepad_read.values()])) #send the message over serial
        except:
            print('no gamepad found')

input_loop()