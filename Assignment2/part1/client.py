import socket
import sys

def http_client(server_host, server_port, filename):
    try:
        # Create a TCP socket
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Establish connection to the server (server_host and server_port)
        client_socket.connect((server_host, int(server_port)))

        # Prepare the GET request to be sent to the server
        request_line = f"GET /{filename} HTTP/1.1\r\n"
        host_header = f"Host: {server_host}\r\n"
        connection_header = "Connection: close\r\n"
        blank_line = "\r\n"

        # Combine the HTTP request headers
        http_request = request_line + host_header + connection_header + blank_line

        print(f"Sending HTTP request to {server_host}:{server_port} for file {filename}...\n")
        print(http_request)  # Optional: Print the request being sent

        # Send the HTTP request to the server
        client_socket.sendall(http_request.encode())

        # Receive the server's response
        response = b""
        while True:
            chunk = client_socket.recv(1024)
            if not chunk:
                break
            response += chunk

        # Decode and print the server's response (headers + content)
        print("Server response:\n")
        print(response.decode())

        # Close the client socket after the response has been received
        client_socket.close()

    except Exception as e:
        print(f"Error: {e}")
        client_socket.close()

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python client.py <server_host> <server_port> <filename>")
        sys.exit()

    # Command-line arguments for server host, port, and filename
    server_host = sys.argv[1]
    server_port = sys.argv[2]
    filename = sys.argv[3]

    # Run the client to connect to the server and request the file
    http_client(server_host, server_port, filename)
