from socket import *
import os
import sys
import struct
import time
import select
import binascii

ICMP_ECHO_REQUEST = 8
ICMP_ECHO_REPLY = 0
PING_NUMBER = 4

def checksum(str):
    csum = 0
    countTo = (len(str) / 2) * 2
    count = 0
    while count < countTo:
        thisVal = str[count+1] * 256 + str[count]
        csum = csum + thisVal
        csum = csum & 0xffffffff
        count = count + 2
        
    if countTo < len(str):
        csum = csum + str[len(str) - 1].decode()
        csum = csum & 0xffffffff

    csum = (csum >> 16) + (csum & 0xffff)
    csum = csum + (csum >> 16)
    answer = ~csum
    answer = answer & 0xffff
    answer = answer >> 8 | (answer << 8 & 0xff00)
    return answer

def receiveOnePing(mySocket, ID, timeout, destAddr):
    timeLeft = timeout

    while 1:
        startedSelect = time.time()
        whatReady = select.select([mySocket], [], [], timeLeft)
        howLongInSelect = (time.time() - startedSelect)
        if whatReady[0] == []: # Timeout
            return None

        timeReceived = time.time()
        recPacket, addr = mySocket.recvfrom(1024)

        #Fetch the ICMP header from the IP packet
        # ICMP is in the 20 to 28 byte of the header
        header = recPacket[20: 28]
        type, code, checksum, packetid, sequence = struct.unpack("bbHHh", header)
        # Type and code must be set to 0.
        # identifier should be same for request and reply
        # dstaddr should match
        if addr[0] == str(destAddr) and type == ICMP_ECHO_REPLY and code == 0 and packetid == ID:
            # calculate the data size
            byte_in_double = struct.calcsize("d")
            timeSent = struct.unpack("d", recPacket[28: 28 + byte_in_double])[0]
            rtt = timeReceived - timeSent
            # TTL is in header 8-9 byte, has format recPacket[8:9] (b'-',)
            # ASCII characters - is the TTL
            ttl = ord(struct.unpack("c", recPacket[8: 9])[0].decode())
            return (byte_in_double, rtt, ttl)

        timeLeft = timeLeft - howLongInSelect
        if timeLeft <= 0:
        	return None


def sendOnePing(mySocket, destAddr, ID):
    # Header is type (8), code (8), checksum (16), id (16), sequence (16)
    
    myChecksum = 0
    # Make a dummy header with a 0 checksum.
    # struct -- Interpret strings as packed binary data, "bbHHh" is format
    header = struct.pack("bbHHh", ICMP_ECHO_REQUEST, 0, myChecksum, ID, 1)
    data = struct.pack("d", time.time())
    # Calculate the checksum on the data and the dummy header.
    myChecksum = checksum(header + data)

    # Get the right checksum, and put in the header
    # htons() function converts a 16 bit positive integer from host byte order to network byte order.
    # https://pythontic.com/modules/socket/byteordering-coversion-functions
    # so we can either use htons() or pack using (struct.pack("!bbHHh")
    if sys.platform == 'darwin':
        myChecksum = htons(myChecksum) & 0xffff
        #Convert 16-bit integers from host to network byte order.
    else:
        myChecksum = htons(myChecksum)
    
    header = struct.pack("bbHHh", ICMP_ECHO_REQUEST, 0, myChecksum, ID, 1)
    packet = header + data
    
    mySocket.sendto(packet, (destAddr, 1)) # AF_INET address must be tuple, not str
    #Both LISTS and TUPLES consist of a number of objects
    #which can be referenced by their position number within the object

def doOnePing(destAddr, timeout):
    icmp = getprotobyname("icmp")

    #SOCK_RAW is a powerful socket type. For more details see: http://sock-raw.org/papers/sock_raw
    
    #Create Socket here
    # mySocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)
    mySocket = socket(AF_INET, SOCK_RAW, icmp)

    myID = os.getpid() & 0xFFFF #Return the current process i
    sendOnePing(mySocket, destAddr, myID)
    res = receiveOnePing(mySocket, myID, timeout, destAddr)

    mySocket.close()
    return res

def ping(host, timeout=1):
    #timeout=1 means: If one second goes by without a reply from the server,
    #the client assumes that either the client’s ping or the server’s pong is lost
    dest = gethostbyname(host)
    print("Pinging " + dest + " using Python:")
    print("")
    loss = 0
    rtt_arr = []
    #Send ping requests to a server separated by approximately one second
    # while 1 :
    # instead of keeping pinging, we run default PING_NUMBER=4 times
    for i in range(0, PING_NUMBER):
        res = doOnePing(dest, timeout)
        if not res:
            print("Request timed out.")
            loss += 1
        else:
            byte_in_double = res[0]
            rtt = int(res[1]*1000)
            rtt_arr.append(rtt)
            ttl = res[2]
            print("Received from %s: byte(s) = %d delay = %dms TTL = %d" % (dest, byte_in_double, rtt, ttl))
        time.sleep(1)# one second
    print("Packet: sent = %d received = %d lost = %d (%.0f%%)" % (PING_NUMBER, PING_NUMBER - loss, loss, loss/PING_NUMBER*100))
    print("Round Trip Time (rtt): min = %dms max = %dms avg = %dms" % (min(rtt_arr), max(rtt_arr), int(sum(rtt_arr)/len(rtt_arr))))
    return

ping("www.google.com")