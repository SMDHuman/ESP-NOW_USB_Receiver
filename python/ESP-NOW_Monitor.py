from serialcom_task import SerialCOM
from time import sleep

def message_received(msg: bytearray):
	msg_list = list(msg)
	
	msg_tag = ""
	msg_start: int = 0
	for i, byte in enumerate(msg_list):
		if(byte == 0):
			msg_start = i+1
			break
		else:
			msg_tag += chr(byte)

	if(msg_tag == "DEBUG_STR"):
		print(f"[{msg_tag}]", msg[msg_start:].decode())
	else:
		print(msg_tag, msg_list[msg_start:])


esp = SerialCOM("COM11")
esp.set_receive_callback(message_received)
#esp.open()
sleep(0.1)
esp.send_slip_str("ADD_PEER")
esp.send_slip(bytearray([0, 112, 4, 29, 168, 130, 88]))
esp.end_slip()

while(True):
	esp.send_slip_str("PING")
	esp.end_slip()
	#print("-")
	esp.send_slip_str("SEND")
	esp.send_slip(bytearray([0, 112, 4, 29, 168, 130, 88]))
	esp.send_slip_str("HELLO")
	esp.end_slip()

	#esp.end_slip()


	sleep(1)