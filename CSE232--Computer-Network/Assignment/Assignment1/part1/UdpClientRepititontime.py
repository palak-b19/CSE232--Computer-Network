import random
import time  # For repetition time and sleep
from socket import *
from time import perf_counter, ctime

client_socket = socket(AF_INET, SOCK_DGRAM)

server_addr = ("localhost", 12000)
client_socket.settimeout(1)
ping_count = 0
RTT_list = []
lost_package = 0
repetition_time = 2  # Set repetition time to 1 second (can be adjusted)

try:
    while ping_count < 10:
        try:
            start_time = perf_counter()
            ping_number = str(ping_count)
            current_time = ctime()
            message_sent = "Ping_Number: " + ping_number + " Time: " + str(current_time)
            
            # Print timestamp before sending
            print(f"[{ctime()}] Sending message: {message_sent}")
            client_socket.sendto(message_sent.encode(), server_addr)
            
            # Receive the message from the server
            message_received, address = client_socket.recvfrom(4096)
            print(f"[{ctime()}] Message received from server: {message_received.decode()}")
            
            end_time = perf_counter()
            Round_Trip_Time = end_time - start_time
            print("Round Trip Time: ", Round_Trip_Time)
            RTT_list.append(Round_Trip_Time)
            
            print(f"Start Time: {start_time}")
            print(f"End Time: {end_time}")

        except timeout:
            print(f"[{ctime()}] Requested timeout for Ping_Number: {ping_number}")
            lost_package += 1

        print("-" * 50)
        ping_count += 1

        # Wait for the specified repetition time before sending the next ping
        time.sleep(repetition_time)

finally:
    print(f"[{ctime()}] Closing socket")
    client_socket.close()

# Calculate statistics after all pings
number_of_datagrams_sent = len(RTT_list)
if RTT_list:
    sum_RTT = sum(RTT_list)
    avg_RTT = sum_RTT / len(RTT_list)
    max_RTT = max(RTT_list)
    min_RTT = min(RTT_list)
    perc_RTT_loss = (lost_package / 10) * 100
else:
    avg_RTT = 0
    max_RTT = 0
    min_RTT = 0
    perc_RTT_loss = 100

# Print final statistics
print('Average RTT: ', avg_RTT)
print('Max RTT: ', max_RTT)
print('Min RTT: ', min_RTT)
print("Packet Loss Percentage: ", perc_RTT_loss)
