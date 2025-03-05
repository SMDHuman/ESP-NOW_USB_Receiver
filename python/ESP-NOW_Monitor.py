from serialcom_task import SerialCOM
from time import sleep
import sys, os
import keyboard
import datetime
from colorama import Fore, Back, Style


# Global variables
listen_buffer_size: int = 100
listen: bool = False
listen_buffer: list[str] = []
added_peers: dict[str, bytes] = {}
#------------------------------------------------------------------------------
# Clear terminal screen
def clear_terminal():
    if(sys.platform == "win32"):
        os.system("cls")
    elif(sys.platform == "linux"):
        os.system("clear")
    else:
        os.system("clear")

#------------------------------------------------------------------------------
# Add data to listen buffer and print if listening mode is enabled
def add_to_buffer(*args: str|int):
    global listen
    data = f"[{datetime.datetime.now().strftime('%H:%M:%S.%f')}]"
    data += " ".join([str(arg) for arg in args])
    # Print data if listening mode is enabled
    if(listen == True):
        print(data)
    # Add data to buffer
    listen_buffer.append(data)
    if(len(listen_buffer) > listen_buffer_size):
        listen_buffer.pop(0)

#------------------------------------------------------------------------------
# Callback function for received messages
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
        add_to_buffer(Fore.RED, f"[{msg_tag}]", Fore.GREEN, msg[msg_start:].decode(), Style.RESET_ALL)
    elif(msg_tag == "PING_CB"):
        add_to_buffer(Fore.RED, f"[{msg_tag}]", Fore.GREEN, "PONG", Style.RESET_ALL)
    elif(msg_tag == "ESP_NOW_CB"):
        mac_addr = msg_list[msg_start:msg_start+6]
        msg_start += 6
        mac_addr_str = ":".join([f"{byte:02X}" for byte in mac_addr])
        add_to_buffer(Fore.RED, f" [{msg_tag}]", 
                      Fore.RED, f" {mac_addr_str}", "\n                 ", 
                      Fore.CYAN, msg_list[msg_start:], Style.RESET_ALL)
    else:
        add_to_buffer(Fore.RED, f"[{msg_tag}]",  Fore.GREEN, msg_list[msg_start:], Style.RESET_ALL)

#------------------------------------------------------------------------------
# Send command to ESP32
def send_command(serial: SerialCOM, command: str, data: bytes = None):
    if(serial.is_open() == False):
        print("Not connected to ESP32")
        return
    serial.send_slip_str(command)   # command tag
    serial.send_slip(bytes([0]))  # null terminator
    if data:
        serial.send_slip(data) # data
    serial.end_slip()

#------------------------------------------------------------------------------
# Print help
def print_help():
    print()
    print("Available commands:")
    print("  ping                              - Test connection with ESP32")
    print("  read_config                       - Read current configuration")
    print("  reset_config                      - Reset configuration to defaults")
    print("  add_peer <MAC> [name]             - Add peer (MAC format: 11:22:33:44:55:66)")
    print("  remove_peer <MAC|name>            - Remove peer")
    print("  list_peers                        - List added peers")
    print("  send <MAC> <data>                 - Send data to peer")
    print("  help                              - Show this help")
    print("  exit                              - Exit application")
    print("  list                              - List available ports")
    print("  connect <PORT> [BAUDRATE:115200]  - Connect to ESP32 on specified port")
    print("  clear                             - Clear terminal screen")
    print("  listen                            - Enter the listen mode\n")

#------------------------------------------------------------------------------
# Parse MAC address
def parse_mac(mac_str: str) -> bytes:
    try:
        return bytes.fromhex(mac_str.replace(':', ''))
    except:
        raise ValueError("Invalid MAC address format. Use XX:XX:XX:XX:XX:XX")

#------------------------------------------------------------------------------
# Main function
def main():
    global listen
    # Initialize serial connection
    esp = SerialCOM()
    esp.set_receive_callback(message_received)
    
    print("ESP-NOW Monitor Console")
    print("Type 'help' for available commands")
    
    while True:
        try:
            cmd = input(">>> ").strip().split()
            if not cmd:
                continue
            
            # --- Exit command ---
            if cmd[0] == "exit":
                break
            # --- List available ports ---
            elif cmd[0] == "list":
                print("Available ports:")
                for i, port in enumerate(esp.get_ports()):
                    print(f" ({i+1}) {port}")
            # --- Connect to ESP32 with specified port ---
            elif cmd[0] == "connect":
                if(len(cmd) == 1):
                    if(esp.is_open()):
                        print("Already connected to ESP32 on ", esp.serial.port)
                    else:
                        print("Usage: connect <PORT> [BAUDRATE:115200]")
                    continue
                elif(len(cmd) > 1):
                    esp.set_port(cmd[1])
                elif(len(cmd) > 2):
                    esp.set_baudrate(int(cmd[2]))
                esp.open()
                print("Connected to ESP32 on", cmd[1])
            # --- Disconnect from ESP32 ---
            elif cmd[0] == "disconnect":
                if(esp.is_open() == False):
                    print("Not connected to ESP32")
                    continue
                esp.close()
                print("Disconnected from ESP32")
            # --- Print help ---
            elif cmd[0] == "help":
                print_help()
            # --- Ping ESP32, it will pong it back on listen ---
            elif cmd[0] == "ping":
                if(esp.is_open() == False):
                    print("Not connected to ESP32")
                    continue
                send_command(esp, "PING")
            # --- Read ESP32 configuration ---
            elif cmd[0] == "read_config":
                if(esp.is_open() == False):
                    print("Not connected to ESP32")
                    continue
                send_command(esp, "READ_CONFIG")
            # --- Reset ESP32 configuration to default ---
            elif cmd[0] == "reset_config":
                if(esp.is_open() == False):
                    print("Not connected to ESP32")
                    continue
                send_command(esp, "RESET_CONFIG")
            # --- Add peer with specified MAC address ---
            elif cmd[0] == "add_peer":
                if(esp.is_open() == False):
                    print("Not connected to ESP32")
                    continue
                if len(cmd) < 2:
                    print("Usage: add_peer <MAC> [name]")
                    continue
                mac = parse_mac(cmd[1])
                for name, mac_addr in added_peers.items():
                    if(mac == mac_addr):
                        print(f"Peer with MAC {':'.join([f'{byte:02X}' for byte in mac])} already added")
                        continue
                if(len(cmd) == 3):
                    added_peers[cmd[2]] = mac
                else:
                    added_peers[str(len(added_peers)+1)] = mac
                send_command(esp, "ADD_PEER", mac)
            # --- Remove peer with specified MAC address ---
            elif cmd[0] == "remove_peer":
                if(esp.is_open() == False):
                    print("Not connected to ESP32")
                    continue
                if len(cmd) < 2:
                    print("Usage: remove_peer <MAC|name>")
                    continue
                if cmd[1] in added_peers:
                    mac = added_peers[cmd[1]]
                else:
                    mac = parse_mac(cmd[1])
                send_command(esp, "REMOVE_PEER", mac)
            # --- List peers ---
            elif cmd[0] == "list_peers":
                print("Added peers:")
                if(len(added_peers) == 0):
                    print(" No peers added")
                    continue
                for name, mac in added_peers.items():
                    print(f" {name}: {':'.join([f'{byte:02X}' for byte in mac])}")
            # --- Send data to peer with specified MAC address ---
            elif cmd[0] == "send":
                if(esp.is_open() == False):
                    print("Not connected to ESP32")
                    continue
                if len(cmd) < 3:
                    print("Usage: send MAC data...")
                    continue
                if cmd[1] in added_peers:
                    mac = added_peers[cmd[1]]
                else:
                    mac = parse_mac(cmd[1])
                data = ' '.join(cmd[2:]).encode()
                payload = mac + data
                send_command(esp, "SEND", payload)
            # --- Clear console ---
            elif cmd[0] == "clear":
                clear_terminal()
            # --- Enter the listen mode ---
            elif cmd[0] == "listen":
                clear_terminal()
                for line in listen_buffer:
                    print(line)
                print("---- Listen mode ---- Press 'left' arrow key to exit")
                listen = True
                while(keyboard.is_pressed('left') == False):
                    sleep(0.01)
                listen = False
            else:
                print(f"Unknown command: {cmd[0]}")
                print("Type 'help' for available commands")

        except ValueError as e:
            print(f"Error: {e}")
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"Error: {e}")

    esp.close()
    esp.active = False
    print("Application closed")

if __name__ == "__main__":
    main()