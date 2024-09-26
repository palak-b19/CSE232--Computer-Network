#import socket module
from socket import *
import sys # In order to terminate the program
serverSocket = socket(AF_INET, SOCK_STREAM)
serverPort=6789
#Prepare a sever socket

#Fill in startt
serverSocket.bind(('',serverPort)) #? Binds to specific port
serverSocket.listen(1)  #? To ensure only one connection at a time
print('Web server is up on port: ',serverPort)
#Fill in end
while True:
    #Establish the connecƟon
    print('Ready to serve...')
    connectionSocket, addr = serverSocket.accept()#Fill in start #Fill in end
    try:
        #Fill in start 
        message = connectionSocket.recv(1024).decode()     #?Receive up to 1024 bytes 
        #Fill in end
        filename = message.split()[1]   #? Extract the requested file name
        print("Requested file:", filename[1:])  # Debugging print statement
        f = open(filename[1:])
        outputdata = f.read() #Fill in start #Fill in end
        print(outputdata)
        #Send one HTTP header line into socket
        #Fill in start
        connectionSocket.send('HTTP/1.1 200 OK\r\n'.encode())
        connectionSocket.send('Content-Type: text/html\r\n'.encode())  # Header for HTML content
        connectionSocket.send("\r\n".encode())  # Blank line to separate headers from content
        #Fill in end
        #Send the content of the requested file to the client
        for i in range(0, len(outputdata)):
            connectionSocket.send(outputdata[i].encode())
        connectionSocket.send("\r\n".encode())
        connectionSocket.close()
    except IOError:
        #Send response message for file not found
        #Fill in start
        connectionSocket.send('HTTP/1.1 404 Not Found\r\n'.encode())
        connectionSocket.send("\r\n".encode())  # Blank line

        #Fill in end
        #Close client socket
        #Fill in start
        connectionSocket.close()
        #Fill in end
serverSocket.close()        
sys.exit() #Terminate the program afer sending the corresponding data