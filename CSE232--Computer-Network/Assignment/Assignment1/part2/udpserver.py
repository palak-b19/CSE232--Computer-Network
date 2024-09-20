# UDPPingerServer.py
# We will need the following module to generate randomized lost packets
import random
from time import *
from socket import *
from datetime import *

# Create a UDP socket
# Notice the use of SOCK_DGRAM for UDP packets
serverSocket = socket(AF_INET, SOCK_DGRAM)
print("Setting up the socket")
# Assign IP address and port number to socket
serverSocket.bind(('', 12000))
print ('Socket Set up complete')

while True:
    # Generate random number in the range of 0 to 10
    rand = random.randint(0, 10)
    print(" number received", rand)
    # Receive the client packet along with the address it is coming from
    message, address = serverSocket.recvfrom(1024)
    print(" Data received :" + str(len(message)) + " bytes from address" + str(address))
    print("Message : ", message)
    time_received = message[- 26:].decode()
    date_format = '%Y-%m-%d %H:%M:%S.%f'
    time_enrypted = datetime.strptime(time_received,date_format)
    time_difference = datetime.now() - time_enrypted
    print(" Current time ", datetime.now())
    print("Time difference ", time_difference)
    
    # Capitalize the message from the client 
    # If rand is less than 4, we consider the packet lost and do not respond
    if rand < 4:
        continue
    message_sent = "Time difference is " + str(time_difference)
    # Otherwise, the server responds
    serverSocket.sendto(message_sent.encode(), address)
    print("Sent : " + str(len(message)) + " bytes to " + str(address) + " address " + "Time Difference :" + message_sent)
    print("Message : ",message_sent)
    print (  "-"*50 )
