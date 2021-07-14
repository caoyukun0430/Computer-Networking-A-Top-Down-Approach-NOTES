from socket import *
import base64
import ssl

msg = "\r\nI love computer networks!"
endmsg = "\r\n.\r\n"
# Choose a mail server (e.g. Google mail server) and call it mailserver 
mailserver = "smtp.office365.com"

# Sender and reciever
fromaddress = "xx@outlook.com"
toaddress = "xx@yeah.net"

# Create socket called clientSocket and establish a TCP connection with mailserver
clientSocket = socket(AF_INET, SOCK_STREAM) 
clientSocket.connect((mailserver, 587))

recv = clientSocket.recv(1024).decode()
print(recv)
if recv[:3] != '220':
    print('220 reply not received from server.')

# Send HELO command and print server response.
heloCommand = 'HELO mailserver\r\n'
clientSocket.send(heloCommand.encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '250':
    print('250 reply not received from server.')

#Request an TLS encrypted connection
command = 'STARTTLS\r\n'.encode()
clientSocket.send(command)
recv = clientSocket.recv(1024).decode()
print(recv)
if recv[:3] != '220':
    print ('220 reply not received from server')
#Encrypt the socket
clientSocket = ssl.wrap_socket(clientSocket)


# Authentication and login with username and password
email = "xx@outlook.com"
password = ""
print(base64.b64encode(email.encode()) + ('\r\n').encode())
print(base64.b64encode(password.encode()) + ('\r\n').encode())


# Send HELO command and print server response after encrypted.
heloCommand = 'HELO mailserver\r\n'
clientSocket.send(heloCommand.encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '250':
    print('250 reply not received from server.')

# authentication to send mail server with username and passwd
clientSocket.send('AUTH LOGIN\r\n'.encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '334':
    print ('334 reply not received from server')

clientSocket.send(base64.b64encode(email.encode()) + ('\r\n').encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '334':
    print ('334 reply not received from server')

clientSocket.send(base64.b64encode(password.encode()) + ('\r\n').encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '235':
    print ('235 reply not received from server')

# Send MAIL FROM command and print server response.
fromcmd = "MAIL FROM: <" + fromaddress + ">\r\n"
clientSocket.send(fromcmd.encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '250':
    print('250 reply not received from server.')

# Send RCPT TO command and print server response.
tocmd = "RCPT TO: <" + toaddress + ">\r\n"
clientSocket.send(tocmd.encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '250':
    print('250 reply not received from server.')

# Send DATA command and print server response.
clientSocket.send("DATA\r\n".encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '354':
    print('354 reply not received from server.')


# Send message data.
''' typical format of header
From: alice@crepes.fr
To: bob@hamburger.edu
Subject: Searching for the meaning of life.
'''
# send plain text email
# header = "From: " + fromaddress + "\r\n"
# header += "To: " + toaddress + "\r\n"
# header += "Subject: I love computer networks\r\n"
# header += 'Content-Type: text/plain\r\n'
# data = header + msg
# clientSocket.send(data.encode())

# send html content
header = "From: " + fromaddress + "\r\n"
header += "To: " + toaddress + "\r\n"
header += "Subject: hello computer networks\r\n"
header += 'Content-Type: text/html\r\n'
htmlmsg = '<html><head>Hello world!</head><body><h1>hello</h1><img src="https://pic3.zhimg.com/50/v2-29a01fdecc80b16e73160c40637a5e8c_hd.jpg"></body></html>'
data = header + '\r\n' + htmlmsg
clientSocket.send(data.encode())

# Message ends with a single period.
clientSocket.send(endmsg.encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '250':
    print('250 reply not received from server.')

# Send QUIT command and get server response.
clientSocket.send("QUIT\r\n".encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '221':
    print('354 reply not received from server.')

# Close connection
clientSocket.close()