#------------------------------------------------------------------------------
class SLIP:
	END = 0xC0
	ESC = 0xDB
	ESC_END = 0xDC
	ESC_ESC = 0xDD
	def __init__(self):
		self.buffer: list[int] = []
		self.packages: list[bytearray] = []
		self.esc_flag: bool= False
		self.ready: int= 0
		self.wait_ack: bool = False
	#...
	def push(self, value: int):
		if(self.esc_flag):
			if(value == self.ESC_END):
				self.buffer.append(self.END)
			elif(value == self.ESC_ESC):
				self.buffer.append(self.ESC)
			elif(value == self.END):
				self.wait_ack = True
			self.esc_flag = False
		elif(value == self.ESC):
			self.esc_flag = True
		elif(value == self.END):
			self.ready += 1
			self.packages.append(bytearray(self.buffer))
			self.buffer.clear()
		else:
			self.buffer.append(value)
	#...
	def get(self) -> bytearray:
		if(self.ready):
			self.ready -= 1
			return(self.packages.pop(0))
		return(bytearray())
