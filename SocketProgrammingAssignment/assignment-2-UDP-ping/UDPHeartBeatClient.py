from socket import *
import time

serverName = 'localhost'
serverPort = 12000

clientSocket = socket(AF_INET, SOCK_DGRAM)
# set client socket timeout
clientSocket.settimeout(1)
print("ready to sent ...")

for i in range(1, 11):
    currentTime = time.time()
    # Ping message format
    message = 'Ping %d %s' % (i, currentTime)
    try:
        # we can add intentionally client sleep to trigger heartbeat
        # time.sleep(6)
        clientSocket.sendto(message.encode(), (serverName, serverPort))
        modifiedMessage, serverAddress = clientSocket.recvfrom(1024)
        # calculate the RTT time
        rtt = time.time() - currentTime
        print("Sequence: %d, replied from %s, with RTT %.4fs, message is %s: " % (i, serverAddress[0], rtt, modifiedMessage.decode()))
    except Exception as e:
        if e is socket.timeout:
            print("Request timed out")
        else:
            print("HeartBeat stopped")
clientSocket.close()