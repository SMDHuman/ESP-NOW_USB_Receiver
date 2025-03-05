from typing import Callable
from serial.tools import list_ports
from threading import Thread
from serial import Serial
import struct
from slip import SLIP
import time

class SerialCOM:
	#--------------------------------------------------------------------------
	#...
	def __init__(self, port: str = "", baudrate: int = 115200):
		self.serial: Serial
		if(port != ""): 
			self.serial = Serial(port, baudrate)
		else:
			self.serial = Serial(baudrate = baudrate)
		#...
		self.slip: SLIP = SLIP()
		#...
		self.active: bool = True
		self.request_close: bool = False
		self.apply_close: bool = False
		self.disconnected_cb: Callable[[], None]
		self.receive_cb: Callable[[bytearray], None]
		self.checksum: int = 0
		#...
		Thread(target=self.rx_loop).start()
		
	#--------------------------------------------------------------------------
	#...
	def set_disconnect_callback(self, callback: Callable[[], None]):
		self.disconnected_cb = callback
	#--------------------------------------------------------------------------
	#...
	def set_receive_callback(self, callback: Callable[[bytearray], None]):
		self.receive_cb = callback
	#--------------------------------------------------------------------------
	#...
	def get_ports(self):
		return([com.name for com in list_ports.comports()])
	#--------------------------------------------------------------------------
	#...
	def set_port(self, name: str):
		self.close()
		self.serial.port = name
	#--------------------------------------------------------------------------
	#...
	def set_baudrate(self, baudrate: int):
		self.close()
		self.serial.baudrate = baudrate
	#--------------------------------------------------------------------------
	#...
	def is_open(self):
		return(self.serial.is_open)
	#--------------------------------------------------------------------------
	#...
	def close(self):
		self.request_close = True
		while(not self.apply_close): pass
		if(self.serial.is_open):
			self.serial.close()
		self.request_close = False
	#--------------------------------------------------------------------------
	#...
	def open(self):
		if(not self.is_open()):
			self.serial.open()
			self.serial.flush()
	#--------------------------------------------------------------------------
	#...
	def send_slip_str(self, text: str) -> None:
		self.send_slip(bytearray([ord(c) for c in text]))
	#...
	def send_slip(self, package: bytearray) -> None:
		#print("package:", package)
		for data in package:
			self.checksum += data
			if(data == self.slip.END):
				self.write(self.slip.ESC)
				self.write(self.slip.ESC_END)
			elif(data == self.slip.ESC):
				self.write(self.slip.ESC)
				self.write(self.slip.ESC_ESC)
			else:
				self.write(data)
	#...
	def end_slip(self):
		self.send_slip(bytearray(struct.pack("I", self.checksum)))
		self.write(self.slip.END)
		self.checksum = 0

	#--------------------------------------------------------------------------
	#...
	def write(self, data: int) -> bool:
		if(self.serial.is_open):
			self.serial.write(bytearray([data]))
			return(True)
		return(False)
	#--------------------------------------------------------------------------
	#...
	def rx_loop(self):
		while(self.active):
			#...
			if(self.request_close):
				self.apply_close = True
				while(self.request_close): pass
				self.apply_close = False
			#...
			if(self.slip.ready):
				pack = self.slip.get()
				if(self.check_checksum(pack)):
					if(self.receive_cb):
						self.receive_cb(pack[:-4])
			#...
			elif(self.slip.wait_ack):
				self.serial.write(self.slip.ESC_END)
				self.slip.wait_ack = False
			#...
			elif(self.serial.is_open):
				try:
					while(self.serial.in_waiting > 0):
						value = ord(self.serial.read(1))
						#print(chr(value), end = "")
						self.slip.push(value)
				except:
					if(self.disconnected_cb):
						self.disconnected_cb()
					self.serial.close()
			time.sleep(0.000001)	
	#--------------------------------------------------------------------------
	#...
	def check_checksum(self, package: bytearray):
		checksum = struct.unpack("I", package[-4:])[0]
		return(sum(package[:-4])%(2**64) == checksum)

