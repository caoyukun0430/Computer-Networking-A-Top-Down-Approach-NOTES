# Implementing a Reliable Transport Protocol

## Introduction

This is programming assignment from Computer Networking: a Top-Down Approach 7th edition, Chapter 3. Detailed description is https://www.csee.umbc.edu/~pmundur/courses/CMSC481-03/lab3.html. For this assignment, I have referred solution from https://github.com/Ghamry0x1/reliable-transport-protocol.

As described, two solutions are implemented, first, the easy Alternating-Bit-Protocol version. Second, the Go-Back-N version.

Note: You might an "improved" version of the project in https://mathcs.clarku.edu/~jbreecher/cs280/Project2/, but when I tried it, I had wired timing prints when running my programs, so I decide not to use this source, feel free to try if you like.

## KEY TAKEAWAY

### Alternating-Bit-Protocol

As suggested, I first started with this easy one. It's important to first consider the sender and receiver states, i.e. what variables should be given. And then follow rdt 3.0 FSM graph to program.

* Note: Even if it states that we should include both ACK/NAK, in the code, I treate NAK as ACK just print it differently.

* Note: When I implement Alternating-Bit-Protocol, I didn't test loss or corruption, the main goal is to get it work and I deal the failures in the latter Go-Back-N version.

- Compile the file
  >$ gcc abp.c -o abp <br>
- Run usage: 
  >./abp num_sim prob_loss prob_corrupt time debug_level <br>

### Go-Back-N

This implementation is based on FSM from book page 262 plus  Fast Retransmission functionality. It differs from ABP because there are a few more things tp consider: 1. sequence number 2. window size 3. buffer size 4. timeout. By the way, I only implement unidirectional.

Based on these points, there are some takeaways might be beneficial.

- It's a bit conflict how to the single timer is updated for each packet when acknowledge is received. Early picture states that timeout is calculated from the time the packet is sent from sender. But it states in page 263 "If an ACK is received but there are still additional transmitted but
not yet acknowledged packets, the timer is restarted." and Figure 3.37 that timer/timeout is updated once an ACK is received. I did the implementation following the latter because it is more reasonable and easier to implement.

- It's important to note that BUFFER_SIZE should be larger than the WINDOW_SIZE. You can buffer more data than you want to send at the moment. Because A_output() will only be called the number of times you given in the argument, if some data are droped due to small buffer size, they are gone forever, which is not what we want. So give a large BUFFER_SIZE to store data but then send data according to your WINDOW_SIZE.

- It will be hard to understand in the beginning why the function send_buffer(), which used to send packet to A LAYER3, needs to be called in both A_output() and A_input(). It's because A_output() will stop when all data are buffered, but at that time, some data could be still not send yet, so
A_input() carries on to send the remaining data in the buffer.

- Compile the file
  >$ gcc gbn.c -o gbn <br>
- Run usage: 
  >./gbn num_sim prob_loss prob_corrupt time debug_level <br>

## Test cases

I tested both versions. For Alternating-Bit-Protocol I only tested no-failure version, see the results in 20_abp.log

For Go-Back-N, I tested as suggested in the document, 20 messages with loss and corruption, see the results in the remaining 4 logs.
>$ ./gbn 20 0 0 10 0 --> 20_gbn.log

>$ ./gbn 20 0.2 0 10 0 --> 20_los.log

>$ ./gbn 20 0 0.2 10 0 --> 20_cor.log

>$ ./gbn 20 0 0 10 2 --> 20_los_cor.log