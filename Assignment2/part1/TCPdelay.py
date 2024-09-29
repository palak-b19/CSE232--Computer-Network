import time  # To simulate delay
from socket import *
import sys  # In order to terminate the program

serverSocket = socket(AF_INET, SOCK_STREAM)
serverPort = 6789

# Prepare a server socket
serverSocket.bind(('', serverPort))  # Binds to specific port
serverSocket.listen(1)  # To ensure only one connection at a time
print('Web server is up on port:', serverPort)

while True:
    # Establish the connection
    print('Ready to serve...')
    connectionSocket, addr = serverSocket.accept()

    try:
        # Simulate a long-running request (5-second delay)
        print(f"Simulating a delay for request from {addr}")
        time.sleep(10)

        # Receive the client's request message
        message = connectionSocket.recv(1024)

        # Check if the message is valid (non-empty) before processing
        if not message:
            print("Empty request received, closing connection.")
            connectionSocket.close()
            continue

        # Print the received message for debugging
        print(f"Received message: {message.decode()}")

        # Split the message and extract the requested file name (if valid)
        try:
            filename = message.split()[1].decode()
            print("Requested file:", filename[1:])  # Debugging print statement
        except IndexError:
            # Handle case where the request is malformed
            print("Invalid request received, closing connection.")
            connectionSocket.send('HTTP/1.1 400 Bad Request\r\n'.encode())
            connectionSocket.send("\r\n".encode())
            connectionSocket.close()
            continue

        # Try to open and read the requested file
        try:
            f = open(filename[1:])  # Open the file, excluding the leading '/'
            outputdata = f.read()
            print(outputdata)

            # Send HTTP response header
            connectionSocket.send('HTTP/1.1 200 OK\r\n'.encode())
            connectionSocket.send('Content-Type: text/html\r\n'.encode())  # Header for HTML content
            connectionSocket.send("\r\n".encode())  # Blank line to separate headers from content

            # Send the content of the requested file to the client
            connectionSocket.send(outputdata.encode())
            connectionSocket.send("\r\n".encode())
        except FileNotFoundError:
            # Send a 404 response if the file is not found
            connectionSocket.send('HTTP/1.1 404 Not Found\r\n'.encode())
            connectionSocket.send("\r\n".encode())

        # Close the client connection
        connectionSocket.close()

    except IOError:
        # Handle generic I/O errors
        connectionSocket.send('HTTP/1.1 500 Internal Server Error\r\n'.encode())
        connectionSocket.send("\r\n".encode())
        connectionSocket.close()

# Close the server socket when done (though this point won't be reached)
serverSocket.close()
sys.exit()  # Terminate the program after sending the corresponding data
