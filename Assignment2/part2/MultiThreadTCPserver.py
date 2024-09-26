#import socket module
from socket import *
import sys
import threading  # Import threading module


def handle_client(connectionSocket):
    try:
        # Receive the client's request message
        message = connectionSocket.recv(1024)
        print("Received message: ", message.decode())
        
        # Extract the requested filename from the message
        filename = message.split()[1]
        
        # Open and read the requested file from the server
        f = open(filename[1:])
        outputdata = f.read()
        
        # Send HTTP response header
        connectionSocket.send('HTTP/1.1 200 OK\r\n'.encode())
        connectionSocket.send('Content-Type: text/html\r\n'.encode())
        connectionSocket.send("\r\n".encode())  # Blank line to separate headers from content
        
        # Send the content of the requested file to the client
        for i in range(0, len(outputdata)):
            connectionSocket.send(outputdata[i].encode())
        
        connectionSocket.send("\r\n".encode())
        connectionSocket.close()
    except IOError:
        # Send response message for file not found
        connectionSocket.send('HTTP/1.1 404 Not Found\r\n'.encode())
        connectionSocket.send("\r\n".encode())
        connectionSocket.close()
    except IndexError:
        # If the request is malformed
        connectionSocket.send('HTTP/1.1 400 Bad Request\r\n'.encode())
        connectionSocket.send("\r\n".encode())
        connectionSocket.close()



def start_server():
    # Create a TCP server socket
    serverSocket = socket(AF_INET, SOCK_STREAM)

    # Define the server port
    serverPort = 6789
    
    # Bind the server to the address and port
    serverSocket.bind(('', serverPort))
    
    # Enable the server to accept connections (with a backlog of 5)
    serverSocket.listen(5)
    
    print('Web server is up on port:', serverPort)

    while True:
        # Accept connection from the client
        connectionSocket, addr = serverSocket.accept()
        print(f"Connection established with {addr}")
        
        # Start a new thread to handle the client's request
        client_thread = threading.Thread(target=handle_client, args=(connectionSocket,))
        client_thread.start()  # Run the thread

    serverSocket.close()

if __name__ == "__main__":
    start_server()        