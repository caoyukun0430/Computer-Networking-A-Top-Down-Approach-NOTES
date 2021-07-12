from socket import *
import time

# Packet loss is depended on the time difference in sec between client
# sends the packet and server receives.
SERVER_TIMEOUT = 0.1

# SERVER_KEEPALIVE is the time server keeps the idle connection
SERVER_KEEPALIVE = 5

# Create a UDP socket
# Notice the use of SOCK_DGRAM for UDP packets
serverSocket = socket(AF_INET, SOCK_DGRAM)
# Assign IP address and port number to socket
serverSocket.bind(('', 12000))
print("ready to serve ...")
# serverSocket.settimeout(SERVER_TIMEOUT)

serverStart = time.time()

while True:
    # Receive the client packet along with the address it is coming from
    message, address = serverSocket.recvfrom(1024)
    # if the time between last packet and current packet received is larger
    # than SERVER_KEEPALIVE, we assume client is stopped.
    if time.time() - serverStart > SERVER_KEEPALIVE:
        print(time.time() - serverStart)
        print("Client stopped, heartbeat finished")
        break
    message = message.decode()
    serverReceive = message.split()[2]
    sequenceNum = int(message.split()[1])
    STT = time.time() - float(serverReceive)
    print("message: ", message)
    # If rand is less is than 4, we consider the packet lost and do not respond
    if STT >= SERVER_TIMEOUT:
        print("Sequence Number: %d, STT: %.4f, packet loss" % (sequenceNum, STT))
        continue
    # Otherwise, the server responds and reset the time server starts
    message = 'Ping %d %.4f' % (sequenceNum, STT)
    serverSocket.sendto(message.encode(), address)
    serverStart = time.time()
