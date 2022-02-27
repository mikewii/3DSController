#!/usr/bin/env python
# Compatible with both Python 2.7.6 and 3.4.3

from __future__ import print_function
import socket, struct, time
import uinput


port = 8889
class x: pass

command = x()
command.CONNECT = 1
command.DISCONNECT = 2
command.KEYS = 3

# /usr/include/linux/input-event-codes.h
btn_map = {
	"A": uinput.BTN_A,
	"B": uinput.BTN_B,
	"X": uinput.BTN_X,
	"Y": uinput.BTN_Y,
	"L": uinput.BTN_TL,
	"R": uinput.BTN_TR,
	"ZL": uinput.BTN_THUMBL,
	"ZR": uinput.BTN_THUMBR,
	"Start": uinput.BTN_START,
	"Select": uinput.BTN_SELECT,
}

device = uinput.Device((
	uinput.ABS_X + (-159, 159, 0, 10),
	uinput.ABS_Y + (-159, 159, 0, 10),	
	uinput.ABS_RX + (-146, 146, 0, 16),
	uinput.ABS_RY + (-146, 146, 0, 16),

	uinput.ABS_HAT0X + (-1, 1, 0, 0),
	uinput.ABS_HAT0Y + (-1, 1, 0, 0),

	# uinput.ABS_Z + (-1, 1, 0, 0),
	# uinput.ABS_RZ + (-1, 1, 0, 0),

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
	),
	# name="Sony Interactive Entertainment Wireless Controller",
	bustype=0x3,
	# #vendor=0x054c, # doesnt work
	# product=0x09CC,
	# version=0xE,
	)



keynames = [
	"A", "B", "Select", "Start", None, None, None, None,
	"R", "L", "X", "Y", None, None, "ZL", "ZR",
	None, None, None, None, "Tap", None, None, None,
	"CSRight", "CSLeft", "CSUp", "CSDown", "CRight", "CLeft", "CUp", "CDown",
]
	
def press_key(key):
	device.emit(key, 1, syn=False)
        
def release_key(key):
	device.emit(key, 0, syn=False)

def handle_shoulder_buttons(touchX, touchY):
	if touchX > 0 and touchX < 160:	# left half
		device.emit(uinput.BTN_TL2, 1, syn=False)
	else: device.emit(uinput.BTN_TL2, 0, syn=False)

	if touchX > 0 and touchX > 160:	# right half
		device.emit(uinput.BTN_TR2, 1, syn=False)
	else: device.emit(uinput.BTN_TR2, 0, syn=False)

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


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", port))

prevkeys = 0

while True:
	rawdata, addr = sock.recvfrom(20)
	rawdata = bytearray(rawdata)
	
	#print("received message", rawdata, "from", addr)	
	if rawdata[0]==command.CONNECT:
		print("Connected to", addr)

	elif rawdata[0]==command.DISCONNECT:
		print("Disconnected from", addr)
	
	elif rawdata[0]==command.KEYS:
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

		
		#print("Touch: ", fields[5], fields[6])
		
		newkeys 	= data["keys"] & ~prevkeys
		oldkeys 	= ~data["keys"] & prevkeys
		prevkeys 	= data["keys"]

		for btnid in range(16):
			if newkeys & (1<<btnid):
				if btnid < 4 or btnid > 7:
					press_key(btn_map[keynames[btnid]])
			if oldkeys & (1<<btnid):
				if btnid < 4 or btnid > 7:
					release_key(btn_map[keynames[btnid]])

		handle_dpad(data["keys"])
		handle_shoulder_buttons(data["touchX"], data["touchY"])

		device.emit(uinput.ABS_X, data["circleX"], syn=False)
		device.emit(uinput.ABS_Y, 0-data["circleY"], syn=False)
		device.emit(uinput.ABS_RX, data["cstickX"], syn=False)
		device.emit(uinput.ABS_RY, data["cstickY"])
