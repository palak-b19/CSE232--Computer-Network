import time
import threading
from socket import *


# Function to handle individual client requests
def handle_client(connectionSocket, addr):
    try:
        print(f"Simulating a delay for request from {addr}")
        time.sleep(10)
        print(f"Connection established with {addr}")
        # Receive the client's request message
        message = connectionSocket.recv(1024).decode()

        # If the message is empty or doesn't have enough components, ignore
        if not message or len(message.split()) < 2:
            print(f"Empty or invalid request from {addr}, closing connection.")
            connectionSocket.close()
            return

        print(f"Received message from {addr}: \n{message}")
        
        # Extract the requested filename from the message
        filename = message.split()[1]  # Get the file name after the GET request

        # Open and read the requested file from the server
        try:
            with open(filename[1:], 'r') as f:
                outputdata = f.read()
            
            # Send HTTP response header
            connectionSocket.send('HTTP/1.1 200 OK\r\n'.encode())
            connectionSocket.send('Content-Type: text/html\r\n'.encode())
            connectionSocket.send('Connection: close\r\n'.encode())  # Close the connection after serving
            connectionSocket.send("\r\n".encode())  # Blank line to separate headers from content
            
            # Send the content of the requested file to the client
            connectionSocket.send(outputdata.encode())
        except FileNotFoundError:
            # Send 404 Not Found response if the file doesn't exist
            connectionSocket.send('HTTP/1.1 404 Not Found\r\n'.encode())
            connectionSocket.send('Connection: close\r\n'.encode())
            connectionSocket.send("\r\n".encode())
        
        # Close the connection socket after sending the response
        connectionSocket.close()
    except Exception as e:
        print(f"Error handling request from {addr}: {e}")
        connectionSocket.close()

# Main server function that listens for incoming connections
def start_server():
    # Create a TCP server socket
    serverSocket = socket(AF_INET, SOCK_STREAM)

    # Define the server port
    serverPort = 6789
    
    # Bind the server to the address and port
    serverSocket.bind(('', serverPort))
    
    # Enable the server to accept connections (with a backlog of 5)
    serverSocket.listen(5)
    
    print(f'Web server is up and running on port: {serverPort}')

    while True:
        # Accept connection from the client
        connectionSocket, addr = serverSocket.accept()
        
        # Start a new thread to handle the client's request
        client_thread = threading.Thread(target=handle_client, args=(connectionSocket, addr))
        client_thread.start()  # Start the thread to handle the client request

    serverSocket.close()

if __name__ == "__main__":
    start_server()
