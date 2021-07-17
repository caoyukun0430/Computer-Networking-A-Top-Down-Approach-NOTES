from socket import *
import sys
import ssl

# if len(sys.argv) <= 1:
#     print('Usage : "python ProxyServer.py server_ip"\n[server_ip : It is the IP Address Of Proxy Server')
#     sys.exit(2)
# Create a server socket, bind it to a port and start listening
tcpSerSock = socket(AF_INET, SOCK_STREAM)
serverPort = 6789
tcpSerSock.bind(('',serverPort))
tcpSerSock.listen(1)
while True:
    print('Ready to serve...')
    tcpCliSock, addr = tcpSerSock.accept()
    print('Received a connection from:', addr)
    message = tcpCliSock.recv(4096).decode()
    print(message)
    # Extract the filename from the given message
    filename = message.split()[1].partition("//")[2].replace('/', '_')
    print(filename)
    fileExist = "false"
    try:
        # Check whether the file exist in the cache
        f = open(filename, "r")
        fileExist = "true"
        print('File Exist: ', fileExist)
        outputdata = f.readlines()
        print(outputdata)
        # it starts from \n before the body since we only need the body info
        # ['\n', '<html>\n', "Congratulations!  You've downloaded the first Wireshark lab file!\n", '</html>\n']
        outputdata = outputdata[outputdata.index("<html>\n")-1:]
        fileExist = "true"
        # ProxyServer finds a cache hit and generates a response message
        tcpCliSock.send("HTTP/1.1 200 OK\r\n".encode())
        tcpCliSock.send("Content-Type:application/json\r\n".encode())
        print(outputdata)
        for line in outputdata:
            tcpCliSock.send(line.encode())
        print('Read from cache')
    # Error handling for file not found in cache
    except IOError:
        if fileExist == "false":
            print('File Exist: ', fileExist)
            # Create a socket on the proxy server
            c = socket(AF_INET, SOCK_STREAM)
            # message is GET http://gaia.cs.umass.edu/wireshark-labs/INTRO-wireshark-file1.html HTTP/1.1
            # partition("/") catches the first /
            # host is gaia.cs.umass.edu

            # optional: we also support POST e.g. https://bugs.python.org/?@number=12524&@type=issue&@action=show
            # http://postman-echo.com/post?user=yukun
            hostname = message.split()[1].partition("//")[2].partition("/")[0].replace("www.", "", 1)
            print("Host name:", hostname)
            try:
                # Connect to the socket to port 80 and forward the original clientsocket message to upstream
                # should also contains request body if needed
                c.connect((hostname, 80))
                c.send(message.encode())
                buff = c.recv(4096).decode()
                print("buffer from server 80", buff)
                print("buffer from server 80", buff.encode())
                # Create a new file in the cache for the requested file.
                # Also send the response in the buffer to client socket and the corresponding file in the cache
                tcpCliSock.send(buff.encode())
                tmpFile = open("./" + filename, "w")
                tmpFile.writelines(buff)
                print("write")
                tmpFile.close()
            except:
                print("Illegal request")
        else:
            # HTTP response message for file not found
            tcpCliSock.send("HTTP/1.1 404 Not Found\r\n\r\n".encode())
    tcpCliSock.close()
tcpSerSock.close()