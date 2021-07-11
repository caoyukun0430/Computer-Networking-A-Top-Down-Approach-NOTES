# Source from https://www.positronx.io/create-socket-server-with-multiple-clients-in-python/
# Multi thread https://www.tutorialspoint.com/python3/python_multithreading.htm

from socket import *
from _thread import *
import threading

serverSocket = socket(AF_INET, SOCK_STREAM)
#Prepare a sever socket 
serverPort = 6789
# server begins listening for incoming TCP requests
# on defined port
serverSocket.bind(('',serverPort))
serverSocket.listen(5)
ThreadCount = 0

# Each thread/client has its own connectionSocket, server
# send 200 OK / 404 to each connectionSocket based on request
def handle_multi_thread(connectionSocket):
    try:
        message = connectionSocket.recv(1024)
        print(message)
        filename = message.split()[1] 
        f = open(filename[1:])
        outputdata = f.read()
        #Send one HTTP header line into socket
        header = 'HTTP/1.1 200 OK \nConnection: close\n' + \
                 'Content0Length: {}\n'.format(len(outputdata)) + \
                 'Content-Type: text/html\r\n\r\n'
        connectionSocket.send(header.encode())

        #Send the content of the requested file to the client
        connectionSocket.send(outputdata.encode())
        connectionSocket.close()
    except IOError:
        #Send response message for file not found
        connectionSocket.send(('HTTP/1.1 404 Not Found\r\n\r\n').encode())
        # Send the content HTML data for 404
        data404 = "<html><head></head><body><h1>404 Not found</h1></body></html>\r\n"
        connectionSocket.send(data404.encode())
        #Close client socket
        connectionSocket.close()

while True:
    # addr is ip:port of the client
    print("Ready to serve ...")
    connectionSocket, addr = serverSocket.accept()
    print("Connected to client on: " + str(addr[0]) + ":" +str(addr[1]))
    start_new_thread(handle_multi_thread, (connectionSocket, ))
    ThreadCount += 1
    print('Thread Number: ' + str(ThreadCount))
    pass
serverSocket.close()
