from socket import *
import sys

print("Please run the file with 3 arguments in format: client.py server_host server_port filename")
assert len(sys.argv) == 4, "wrong number of arguments given"
print("Argument List: %s", str(sys.argv))
serverIP = sys.argv[1]
serverPort = sys.argv[2]
fileName = sys.argv[3]
clientSocket = socket(AF_INET, SOCK_STREAM)
# Handshake, connection setup
clientSocket.connect((serverIP, int(serverPort)))
# Request header is:
# GET /HelloWorld.html HTTP/1.1\r\nHost: 192.168.1.111:6789\r\nConnection: keep-alive\r\n
# Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n
# Accept-Encoding: gzip, deflate\r\n
# Accept-Language: en-US,en;q=0.9,zh-CN;q=0.8,zh;q=0.7,de;q=0.6\r\n\r\n
headerString = "\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: en-US,en;q=0.9,zh-CN;q=0.8,zh;q=0.7,de;q=0.6\r\n\r\n"
requestHeader = "GET /" + fileName + " HTTP/1.1\r\nHost: " + serverIP + ":" + serverPort + headerString

clientSocket.send(requestHeader.encode()) #encoding the message to binary bytes

modified = clientSocket.recv(1024)
print ("From Server:", modified.decode())
clientSocket.close()
