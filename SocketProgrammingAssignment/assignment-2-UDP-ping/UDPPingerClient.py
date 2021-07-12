from socket import *
import time

serverName = 'localhost'
serverPort = 12000

clientSocket = socket(AF_INET, SOCK_DGRAM)
# set client socket timeout
clientSocket.settimeout(1)
print("ready to sent ...")

rttArray = []
for i in range(1, 11):
    currentTime = time.time()
    # Ping message format
    message = 'Ping %d %s' % (i, currentTime)
    try:
        clientSocket.sendto(message.encode(), (serverName, serverPort))
        modifiedMessage, serverAddress = clientSocket.recvfrom(1024)
        # calculate the RTT time
        rtt = time.time() - currentTime
        rttArray.append(rtt)
        print("Sequence: %d, replied from %s, with RTT %.4fs, message is %s: " % (i, serverAddress[0], rtt, modifiedMessage.decode()))
    except:
        print("Request timed out")
avgRTT = sum(rttArray) / len(rttArray)
lostRate = 100 - len(rttArray) * 10
print("Max RTT: %.4f, min RTT: %.4f, average RTT: %.4f, lost rate: {:.1%}".format(max(rttArray), min(rttArray), avgRTT, lostRate))
clientSocket.close()