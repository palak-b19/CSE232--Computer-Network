from socket import *
#! confirming that port 12000 is a free port for my device
def check_port(port):
    with socket(AF_INET, SOCK_STREAM) as serversocket:
        result = serversocket.connect_ex(('localhost', port))
        if result == 0:
            print(f"Port {port} is already in use.")
        else:
            print(f"Port {port} is free.")

# Example usage
check_port(12000)  # Replace 5000 with the port you want to check
