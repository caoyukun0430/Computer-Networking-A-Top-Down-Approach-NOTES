#import socket module
from socket import *
MAX_NUM_OF_REQUEST = 3
serverSocket = socket(AF_INET, SOCK_STREAM)
#Prepare a sever socket 
serverPort = 6789
# server begins listening for incoming TCP requests
# on defined port
serverSocket.bind(('',serverPort))
serverSocket.listen(1)
numReq = 0
while numReq <= MAX_NUM_OF_REQUEST:     
    #Establish the connection    
    print('Ready to serve...')
    # addr is ip:port of the client
    connectionSocket, addr = serverSocket.accept()
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
        # for i in range(0, len(outputdata)):
        #     connectionSocket.send(outputdata[i].encode())
        connectionSocket.close()
    except IOError:
        #Send response message for file not found
        connectionSocket.send(('HTTP/1.1 404 Not Found\r\n\r\n').encode())
        # Send the content HTML data for 404
        data404 = "<html><head></head><body><h1>404 Not found</h1></body></html>\r\n"
        connectionSocket.send(data404.encode())

        #Close client socket
        connectionSocket.close()
    numReq += 1
serverSocket.close()