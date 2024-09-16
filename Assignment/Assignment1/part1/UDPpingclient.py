
import random
from socket import *
from time import *

client_socket = socket(AF_INET,SOCK_DGRAM)

server_addr = ("localhost", 12000)
client_socket.settimeout(2)
ping_count =0
RTT_list = []
lost_package=0

try :
    while(ping_count<10):
        try:
            start_time = perf_counter()
            ping_number =  str(ping_count)
            current_time = ctime()
            message_sent = "Ping_Number: " + ping_number + " Time: " + str(current_time)
            client_socket.sendto(message_sent.encode(), server_addr)
            print("Message sent to server \n")
            print( "Message : ", message_sent, "\n")
            message_received, address = client_socket.recvfrom(4096)
            print("Message received from the server \n")
            print("Message :", message_sent,"\n")
            end_time = perf_counter()
            Round_Trip_Time = end_time - start_time
            print("Round Trip Time : " , Round_Trip_Time)
            RTT_list.append(Round_Trip_Time)

            print(f"Start Time: {start_time}")
            print(f"End Time: {end_time}")

            
        except timeout:
                print(" Requested Time out for : ", ping_number)
                lost_package+=1
        print("-"*50)
        ping_count+=1
finally:
    print("closing socket")
    client_socket.close()

sum_RTT = sum(RTT_list)
avg_RTT = sum_RTT/len(RTT_list)
max_RTT = max(RTT_list)
min_RTT = min(RTT_list)
perc_RTT_loss = (lost_package/10)*100

print('Average RTT : ', avg_RTT)
print('Max RTT : ', max_RTT)
print('Min RTT : ', min_RTT)
print("Perc Package Loss : ", perc_RTT_loss)

#Assumption - AVG RTT we are considering which we were dropped

    #buffer size 4096




