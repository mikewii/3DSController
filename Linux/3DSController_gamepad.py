#!/usr/bin/env python

from __future__ import print_function
import socket, struct, time
import uinput

##########################
##### configuration #####
port = 8889

# 0 = japanese:  X    1 = western:  Y
#              Y   A              X   B
#                B                  A
layout_type = 1

# 0 = ThumbL/ThumbR on touch, 1 = R1/R2 on touch
touch_buttons = 0

# 0 = vertical split: [1 2] 1 = horizontal split: [1 1] 2 = cross split: [1 2]
#                     [1 2]                       [2 2]                  [2 1]
layout_touch = 2

# 0 = default, 1 = inverted
touch_buttons_inverted = 0

##### /configuration #####
##########################

##### controller with on/off shoulder buttons, no axes
device = uinput.Device((
	# left stick
	uinput.ABS_X + (-159, 159, 0, 10),
	uinput.ABS_Y + (-159, 159, 0, 10),	
	
	# right stick
	uinput.ABS_RX + (-146, 146, 0, 16),
	uinput.ABS_RY + (-146, 146, 0, 16),

	# dpad
	uinput.ABS_HAT0X + (-1, 1, 0, 0),
	uinput.ABS_HAT0Y + (-1, 1, 0, 0),

	uinput.BTN_A,
	uinput.BTN_B,
	uinput.BTN_X,
	uinput.BTN_Y,
	uinput.BTN_TL,
	uinput.BTN_TR,
	uinput.BTN_TL2,
	uinput.BTN_TR2,
	uinput.BTN_SELECT,
	uinput.BTN_START,
	uinput.BTN_MODE,
	uinput.BTN_THUMBL,
	uinput.BTN_THUMBR,
	), bustype=0x1
)
##### /controller

btn_map = {
	"A": uinput.BTN_A,
	"B": uinput.BTN_B,
	"X": uinput.BTN_X,
	"Y": uinput.BTN_Y,
	"L": uinput.BTN_TL,
	"R": uinput.BTN_TR,
	"ZL": uinput.BTN_TL2,
	"ZR": uinput.BTN_TR2,
	"Start": uinput.BTN_START,
	"Select": uinput.BTN_SELECT,
	"Touch0": uinput.BTN_THUMBL,
	"Touch1": uinput.BTN_THUMBR
}

keys = {
	0:"A", 1:"B", 2:"Select", 3:"Start",
	8:"R", 9:"L", 10:"X", 11:"Y",
	14:"ZL", 15:"ZR",
	# 32:"Touch0", 33:"Touch1"
}

def set_layout_type():
	btn_map["A"] = uinput.BTN_B
	btn_map["B"] = uinput.BTN_A
	btn_map["X"] = uinput.BTN_Y
	btn_map["Y"] = uinput.BTN_X

def set_touch_buttons():
	btn_map["ZL"] = uinput.BTN_THUMBL
	btn_map["ZR"] = uinput.BTN_THUMBR
	btn_map["Touch0"] = uinput.BTN_TL2
	btn_map["Touch1"] = uinput.BTN_TR2
	
def press_key(key):
	device.emit(key, 1, syn=False)
        
def release_key(key):
	device.emit(key, 0, syn=False)

def handle_buttons(data):
	for id in keys:
		if data & 1<<id:
			press_key(btn_map[keys[id]])
		else: release_key(btn_map[keys[id]])
		

########## touch ##########
touch0 = "Touch0"
touch1 = "Touch1"
def touch_invert():
	global touch0
	global touch1
	touch0 = "Touch1"
	touch1 = "Touch0"

def touch_vertical(touchX):
	if touchX < 160: # left half
		press_key(btn_map[touch0])
	else: release_key(btn_map[touch0])
	
	if touchX > 160: # right half
		press_key(btn_map[touch1])
	else: release_key(btn_map[touch1])

def touch_horizontal(touchY):
	if touchY < 120: # top half
		press_key(btn_map[touch0])
	else: release_key(btn_map[touch0])
	
	if touchY > 120: # bottom half
		press_key(btn_map[touch1])
	else: release_key(btn_map[touch1])

def touch_cross(touchX, touchY):
	if touchX < 160 and touchY < 120: # top left
		press_key(btn_map[touch0])
	elif touchX > 160 and touchY > 120: # bottom right
		press_key(btn_map[touch0])
	else: release_key(btn_map[touch0])

	if touchX > 160 and touchY < 120: # top right
		press_key(btn_map[touch1])
	elif touchX < 160 and touchY > 120: # bottom left
		press_key(btn_map[touch1])
	else: release_key(btn_map[touch1])
	

def handle_touch_buttons(touchX, touchY):
	if touchX > 0 and touchY > 0:
		if layout_touch == 0: touch_vertical(touchX)
		elif layout_touch == 1: touch_horizontal(touchY)
		elif layout_touch == 2: touch_cross(touchX, touchY)
	else:
		release_key(btn_map[touch0])
		release_key(btn_map[touch1])
########## /touch ##########

def handle_dpad(keys):
	if keys & 1<<4:	# right
		device.emit(uinput.ABS_HAT0X, 1, syn=False)
	elif keys & 1<<5:	# left
		device.emit(uinput.ABS_HAT0X, -1, syn=False)
	else: device.emit(uinput.ABS_HAT0X, 0, syn=False)

	if keys & 1<<6:	# up
		device.emit(uinput.ABS_HAT0Y, -1, syn=False)
	elif keys & 1<<7:	# down
		device.emit(uinput.ABS_HAT0Y, 1, syn=False)
	else: device.emit(uinput.ABS_HAT0Y, 0, syn=False)

if __name__ == "__main__":
	# configure controller:
	if layout_type == 1: set_layout_type()
	if layout_touch == 1: set_touch_buttons()
	if touch_buttons_inverted == 1: touch_invert()

	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.bind(("", port))

	print("Running on port: %d" % port)

	while True:
		rawdata, addr = sock.recvfrom(20)
		rawdata = bytearray(rawdata)
		
		#print("received message", rawdata, "from", addr)	
		
		if rawdata[0] == 3:
			fields = struct.unpack("<BBxxIhhHHhh", rawdata)
			
			data = {
				"command": fields[0],
				"keyboardActive": fields[1],
				"keys": fields[2],
				"circleX": fields[3],
				"circleY": fields[4],
				"touchX": fields[5],
				"touchY": fields[6],
				"cstickX": fields[7],
				"cstickY": fields[8],
			}
			
			handle_buttons(data["keys"])
			handle_dpad(data["keys"])
			handle_touch_buttons(data["touchX"], data["touchY"])

			device.emit(uinput.ABS_X, data["circleX"], syn=False)
			device.emit(uinput.ABS_Y, 0-data["circleY"], syn=False)
			device.emit(uinput.ABS_RX, data["cstickX"], syn=False)
			device.emit(uinput.ABS_RY, data["cstickY"])
