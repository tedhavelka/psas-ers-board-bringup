import can

# Replace '/dev/ttyACM0' with the appropriate port for your SLCAN device (COM1, /dev/tty.usbmodem, etc) and set the desired bitrate (10, 20, 50, 100, 125, 250, 500, 800, 1000 kbps)
channel = "/dev/ttyACM1"
bitrate = 125000

# Configure the connection to the VulCAN
bus = can.interface.Bus(channel=channel, interface="slcan", bitrate=bitrate)

# Define a CAN message with an arbitrary ID and data bytes
message_id = 0x123
data_bytes = [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88]
message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=False)

# Send the CAN message
bus.send(message)
print(f"Sent message: {message}")

# Close the bus connection
bus.shutdown()
