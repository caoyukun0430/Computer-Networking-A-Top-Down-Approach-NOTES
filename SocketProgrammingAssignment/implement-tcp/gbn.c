#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
   
   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
       are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
       or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
       (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0 /* change to 1 if you're doing extra credit */
                        /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
/*  This defines which side (entity) is making a call to the simulator */
#define  AEntity         0
#define  BEntity         1

// A "msg" is the data unit passed from layer 5 (Application code) to layer 
// 4 (student's code).  It contains the data (characters) to be delivered 
// to the remote layer 5 via the students transport level protocol entities.  

#define  MESSAGE_LENGTH  20
#define WINDOW_SIZE 8
#define BUFFER_SIZE 64 // important to seperate BUFFER_SIZE from WINDOW_SIZE, we can 
                       // buffer more than we send to LAYER3
// Two more structs are needed to define A and B states independently

int nsimmax = 0; /* number of msgs to generate, then stop */

struct msg
{
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt
{
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

void starttimer(int AorB, float increment);
void stoptimer(int AorB);
void tolayer3(int AorB, struct pkt packet);
void tolayer5(int AorB, char datasent[20]);

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * Yukun: Below is the GO-BACK-N TCP implementation following
 * book page 262 + Fast Retransmission
 * KEY TAKEAWAY:
 *  Only undiretional is implemented
 *  Timeout is restarted once an expected ACK arrived at A
 *  BUFFER_SIZE should be larger than the WINDOW_SIZE
 *  send_buffer() needs to be called in both A_output() and A_input()
 */

struct sender
{
  int send_base;
  int next_seq;
  int next_buffer; // the num of messages sent from A's LAYER5, next_seq must <= next_buffer
  int window_size;
  struct pkt send_buffer[BUFFER_SIZE];
  float estimate_rtt;
  int fast_retransmit;
} A;

struct receiver
{
  int expect_seq; // expected received seq
} B;

// Checksumming as suggest by add all ints in pkt together
int getCheckSum(struct pkt packet) {
  int checksum = 0;
  checksum += packet.seqnum;
  checksum += packet.acknum;
  for (int i = 0; i < MESSAGE_LENGTH; i++) {
    checksum += packet.payload[i];
  }
  return checksum;
}



// Used in 2 places, in A_output() before LAYER5 finishes send message. And in
// A_input() in case all needed message are in buffer and A_output() is not called
// any more.
void send_buffer() {
    // send the buffer packet until next_buffer_index and less than window size
    // IMPORTANT to have A.next_seq < A.next_buffer, otherwise it exceeds at the end
    while (A.next_seq < A.next_buffer && A.next_seq < A.send_base + A.window_size) {
        struct pkt packet = A.send_buffer[A.next_seq % BUFFER_SIZE];
        printf("A_output: sent packet (seq=%d): %s\n", packet.seqnum, packet.payload);
        tolayer3(AEntity, packet);
        if (A.next_seq == A.send_base) {
            starttimer(AEntity, A.estimate_rtt);
            printf("  A_output: start timer for seq=%d\n", A.send_base);
        }
        A.next_seq++;
        printf("  A_output: A.send_base: %d, A.next_seq: %d\n", A.send_base, A.next_seq);
    }

}

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
    if (A.next_buffer < A.send_base) {
        printf("  A_output: Wrong seq number in GBN, aborted");
        return;
    }
    if (A.next_buffer >= A.send_base + BUFFER_SIZE) {
        printf("  A_output: buffer full. drop the message: %s\n", message.data);
        return;
    }

    // valid next_seq will be buffered
    printf("  A_output: bufferred packet (seq=%d): %s\n", A.next_buffer, message.data);
    struct pkt *packet = &A.send_buffer[A.next_buffer % BUFFER_SIZE];
    packet->seqnum = A.next_buffer;
    memmove(packet->payload, message.data, 20);
    packet->checksum = getCheckSum(*packet);
    A.next_buffer++;
    // IMPORTANT: add first then call next_buffer
    printf("A buffer_next: %d\n", A.next_buffer);
    send_buffer();
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {
  printf("  B_output: USED only when bi-directional.\n");
}

// retranmit packet [base, nextseq -1]
void retransmit_pkt() {
    for (int i = A.send_base; i < A.next_seq; i++) {
      struct pkt packet = A.send_buffer[i % BUFFER_SIZE];
      printf("  A_timerinterrupt: resend packet (seq=%d): %s\n", packet.seqnum, packet.payload);
      tolayer3(AEntity, packet);
  }
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
  if (packet.checksum != getCheckSum(packet)) {
    printf("  A_input: corrupted NAK(ack=%d) drop packet.\n", packet.acknum);
    return;
  }
  // A.seq last sent seq should equal ack from B, stop fast_retransmit once
  // we completely received all pkts
  if (packet.acknum < A.send_base) {
      if (A.send_base == nsimmax) {
          return;
      }
    printf("  A_input: duplicated ACK(ack=%d), drop packet.\n", packet.acknum);
    // use fast retransmit to handle pkt lost from A side! Since if pkt never
    // reaches B, it always lands in this loop
    if (A.fast_retransmit == 3) {
          printf("A_input: fast retransmit ACK(ack=%d) \n", packet.acknum);
          retransmit_pkt();
          A.fast_retransmit = 0;
    }
    A.fast_retransmit++;
    return;
  }
  // if ACK is correct, we stop the timer and update A struct
  printf("A_input: ACKed(ack=%d) from B.\n", packet.acknum);
  A.send_base = packet.acknum + 1;
  printf("  A.send_base: %d, A.next_seq: %d\n", A.send_base, A.next_seq);
  stoptimer(AEntity);
  // Follow page 263:
  // If an ACK is received but there are still additional transmitted but
  // not yet acknowledged packets, the timer is restarted.
  // If there are no outstanding, unacknowledged packets, the timer is stopped
  if (A.send_base == A.next_seq) {
      printf("  A_input: stop timer\n");
      // once all LAYER5 msg are stored in send_buffer, A_output() will not be
      // called, we need to continuing sending remaining msg in buffer!
      send_buffer();
  } else {
      starttimer(AEntity, A.estimate_rtt);
      printf("  A_input: restart timer + %f\n", A.estimate_rtt);
  }
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
  // when timeout reached, we retransmit all buffered packet [base, nextseqnum-1]
  retransmit_pkt();
  starttimer(AEntity, A.estimate_rtt);
  printf("  A_timerinterrupt: restart timer + %f\n", A.estimate_rtt);
}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
  // Initialize the first pkt to send
  A.send_base = 0;
  A.next_seq = 0;
  A.next_buffer = 0;
  A.estimate_rtt = 15;
  A.window_size = WINDOW_SIZE;
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

// General function for receiver B to send ACK to A
void send_ack(int AorB, int ack) {
  struct pkt rpacket;
  rpacket.acknum = ack;
  rpacket.checksum = getCheckSum(rpacket);
  tolayer3(AorB, rpacket);
}

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
  // Two error condtions to consider as in rdt2.2/rdt3.0, we send duplicate
  // ACK(i.e. wrong ACK number) for corrupted or wrong seq packet
  if (packet.checksum != getCheckSum(packet)) {
    printf("  B_input: packet from A is corrupted, send NAK(ack=%d)\n", B.expect_seq - 1);
    send_ack(BEntity, B.expect_seq - 1);
    return;
  }
  // we don't need to ack again for some retransmitted packets since B.expect_seq increasing
  // indicates they are received already before!
  if (packet.seqnum != B.expect_seq) {
    printf("  B_input: packet seq:%d, send duplicated ACK(ack=%d)\n", packet.seqnum, B.expect_seq - 1);
    send_ack(BEntity, B.expect_seq - 1);
    return;
  }
  printf("  B_input: recv packet (seq=%d): %s\n", packet.seqnum, packet.payload);
  printf("  B_input: send ACK(ack=%d)\n", B.expect_seq);
  send_ack(BEntity, B.expect_seq);
  tolayer5(BEntity, packet.payload);
  // Update the expected seq number in B
  B.expect_seq++;
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
  printf("B_timerinterrupt: USED only when bi-directional.\n");
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
  B.expect_seq = 0;
}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
    - emulates the tranmission and delivery (possibly with bit-level corruption
        and packet loss) of packets across the layer 3/4 interface
    - handles the starting/stopping of a timer, and generates timer
        interrupts (resulting in calling students timer handler).
    - generates message to be sent (passed from later 5 to 4)
THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event
{
    float evtime;       /* event time */
    int evtype;         /* event type code */
    int eventity;       /* entity where event occurs */
    struct pkt *pktptr; /* ptr to packet (if any) assoc w/ this event */
    struct event *prev;
    struct event *next;
};
struct event *evlist = NULL; /* the event list */

/* possible events: */
#define TIMER_INTERRUPT 0
#define FROM_LAYER5 1
#define FROM_LAYER3 2

#define OFF 0
#define ON 1
#define A 0
#define B 1

int TRACE = 1;   /* for my debugging */
int nsim = 0;    /* number of messages from 5 to 4 so far */
float time = 0.000;
float lossprob;    /* probability that a packet is dropped  */
float corruptprob; /* probability that one bit is packet is flipped */
float lambda;      /* arrival rate of messages from layer 5 */
int ntolayer3;     /* number sent into layer 3 */
int nlost;         /* number lost in media */
int ncorrupt;      /* number corrupted by media*/

void init(int argc, char **argv);
void generate_next_arrival(void);
void insertevent(struct event *p);

int main(int argc, char **argv)
{
    struct event *eventptr;
    struct msg msg2give;
    struct pkt pkt2give;

    int i, j;
    char c;

    init(argc, argv);
    A_init();
    B_init();

    while (1)
    {
        eventptr = evlist; /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next; /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2)
        {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime; /* update time to next event time */
        if (eventptr->evtype == FROM_LAYER5)
        {
            if (nsim < nsimmax)
            {
                if (nsim + 1 < nsimmax)
                    generate_next_arrival(); /* set up future arrival */
                /* fill in msg to give with string of same letter */
                j = nsim % 26;
                for (i = 0; i < 20; i++)
                    msg2give.data[i] = 97 + j;
                msg2give.data[19] = 0;
                if (TRACE > 2)
                {
                    printf("          MAINLOOP: data given to student: ");
                    for (i = 0; i < 20; i++)
                        printf("%c", msg2give.data[i]);
                    printf("\n");
                }
                nsim++;
                if (eventptr->eventity == A)
                    A_output(msg2give);
                else
                    B_output(msg2give);
            }
        }
        else if (eventptr->evtype == FROM_LAYER3)
        {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A) /* deliver packet by calling */
                A_input(pkt2give);       /* appropriate entity */
            else
                B_input(pkt2give);
            free(eventptr->pktptr); /* free the memory for packet */
        }
        else if (eventptr->evtype == TIMER_INTERRUPT)
        {
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else
        {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(
        " Simulator terminated at time %f\n after sending %d msgs from layer5\n",
        time, nsim);
}

void init(int argc, char **argv) /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();

    if (argc != 6)
    {
        printf("usage: %s  num_sim  prob_loss  prob_corrupt  interval  debug_level\n", argv[0]);
        exit(1);
    }

    nsimmax = atoi(argv[1]);
    lossprob = atof(argv[2]);
    corruptprob = atof(argv[3]);
    lambda = atof(argv[4]);
    TRACE = atoi(argv[5]);
    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("the number of messages to simulate: %d\n", nsimmax);
    printf("packet loss probability: %f\n", lossprob);
    printf("packet corruption probability: %f\n", corruptprob);
    printf("average time between messages from sender's layer5: %f\n", lambda);
    printf("TRACE: %d\n", TRACE);

    srand(9999); /* init random number generator */
    sum = 0.0;   /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75)
    {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(1);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;              /* initialize time to 0.0 */
    generate_next_arrival(); /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand(void)
{
    double mmm = RAND_MAX;
    float x;          /* individual students may need to change mmm */
    x = rand() / mmm; /* x should be uniform in [0,1] */
    return (x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival(void)
{
    double x, log(), ceil();
    struct event *evptr;
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2; /* x is uniform on [0,2*lambda] */
                                 /* having mean of lambda        */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}

void insertevent(struct event *p)
{
    struct event *q, *qold;

    if (TRACE > 2)
    {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist; /* q points to header of list in which p struct inserted */
    if (q == NULL)
    { /* list is empty */
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else
    {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL)
        { /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist)
        { /* front of list */
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else
        { /* middle of list */
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

void printevlist(void)
{
    struct event *q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next)
    {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype,
               q->eventity);
    }
    printf("--------------\n");
}

/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB /* A or B is trying to stop timer */)
{
    struct event *q, *qold;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f\n", time);
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;        /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist)
            { /* front of list - there must be event after */
                q->next->prev = NULL;
                evlist = q->next;
            }
            else
            { /* middle of list */
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

void starttimer(int AorB /* A or B is trying to stop timer */, float increment)
{
    struct event *q;
    struct event *evptr;

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then  warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}

/************************** TOLAYER3 ***************/
void tolayer3(int AorB /* A or B is trying to stop timer */, struct pkt packet)
{
    struct pkt *mypktptr;
    struct event *evptr, *q;
    float lastime, x;
    int i;

    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob)
    {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being lost\n");
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i = 0; i < 20; i++)
        mypktptr->payload[i] = packet.payload[i];
    if (TRACE > 2)
    {
        printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
               mypktptr->acknum, mypktptr->checksum);
        for (i = 0; i < 20; i++)
            printf("%c", mypktptr->payload[i]);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;      /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;         /* save ptr to my copy of packet */
                                      /* finally, compute the arrival time of packet at the other end.
                                         medium can not reorder, so make sure packet arrives between 1 and 10
                                         time units after the latest arrival time of packets
                                         currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob)
    {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            mypktptr->payload[0] = 'Z'; /* corrupt payload */
        else if (x < .875)
            mypktptr->seqnum = 999999;
        else
            mypktptr->acknum = 999999;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being corrupted\n");
    }

    if (TRACE > 2)
        printf("          TOLAYER3: scheduling arrival on other side\n");
    insertevent(evptr);
}

void tolayer5(int AorB, char datasent[20])
{
    int i;
    if (TRACE > 2)
    {
        printf("          TOLAYER5: data received: ");
        for (i = 0; i < 20; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }
}