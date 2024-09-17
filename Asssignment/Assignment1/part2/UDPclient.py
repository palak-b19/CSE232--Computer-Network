import random
from socket import *
from time import *
from datetime import *

client_socket = socket(AF_INET,SOCK_DGRAM)

server_addr = ("localhost",12000)
client_socket.settimeout(1)
ping_count =0
lost_package=0
packet_misses=0

try :
    while(packet_misses<3):
        try:
            start_time = perf_counter()
            ping_number =  str(ping_count)
            current_time = datetime.now()
            message_sent = str(ping_count) + " "  + " Time: " + str(current_time)
            client_socket.sendto(message_sent.encode(), server_addr)
            print("Message sent to server \n")
            print( "Message : ", message_sent, "\n")
            message_received, address = client_socket.recvfrom(4096)
            print("Message received from the server",message_received,"\n")
            print("Message :", message_sent,"\n")
            end_time = perf_counter()

            print(f"Start Time: {start_time}")
            print(f"End Time: {end_time}")
            packet_misses=0

            
        except timeout:
                print(" Requested Time out for : ", ping_number)
                lost_package+=1
                packet_misses+=1
        print("-"*50)
        ping_count+=1
finally:
    print("closing socket")
    print(" Server stopped Responding")
    print ( "-"*50)
    print("Total packets sent before application stopped : ", ping_count)
    client_socket.close()

number_of_datagrams_sent = len(RTT_list)


#Assumption - AVG RTT we are considering which we were dropped

    #buffer size 4096




