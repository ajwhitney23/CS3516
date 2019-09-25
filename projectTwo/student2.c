#include <stdio.h>
#include <stdlib.h>
#include "project2.h"

/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */
/*
Flow:
A sends pkt0
B rcv pkt0
B sends ack0
A rcv ack0
A send pkt1
B rcv pkt1
B send ack1
A rcv ack1
... and loop
A must be able to timeout, resend;    B detect when timeout happens

ORDER:
a_init
b_init
a_output
b_input
a_input
...

*/

//prototypes
int createChecksum(struct pkt packet);
int checkPacket(struct pkt packet);

/* Global variables to be used for the project */
//A variables

//B variables
int correctSeqNum;
int correctPacket;

//Global Variables
struct pkt prevPKT;
struct pkt ackPTK;

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message)
{

    struct pkt send; //packet to be sent
    send.acknum = 0;
    send.seqnum = !(prevPKT.seqnum);
    for (int i = 0; i < MESSAGE_LENGTH; i++)
    {
        send.payload[i] = message.data[i];
    }
    send.checksum = createChecksum(send);

    //check if ack is the same...
    if (ackPTK.acknum == send.seqnum)
    {
        return;
    }

    //store
    prevPKT.seqnum = send.seqnum;
    prevPKT.acknum = send.acknum;
    for (int i = 0; i < MESSAGE_LENGTH; i++)
    {
        prevPKT.payload[i] = message.data[i];
    }
    prevPKT.checksum = send.checksum;

    tolayer3(AEntity, send);
    startTimer(AEntity, 1000);
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 * 
 * if((isCorruptedPacket(packet) == FALSE) && (strncmp(packet.payload, "ACK", strlen("ACK")) == 0) &&
 *  (packet.acknum == lastPacket.seqnum)){
    if(TraceLevel >= 4)	printf("AEntity got a valid expected ACK!\n");
 */
void A_input(struct pkt packet)
{
    /*
    //check if acknum is correct, check packet corruption
        //stop timer
        //set ackPKT seq and ack num to packet's
        //flip seq
    if((checkPacket(packet) == 0) && (packet.acknum == prevPKT.seqnum))
    {
        stopTimer(AEntity);
        ackPTK.acknum = packet.acknum;
        ackPTK.seqnum = packet.seqnum;
        for(int i = 0; i < MESSAGE_LENGTH; i++)
        {
            ackPTK.payload[i] = packet.payload[i];
        }
        ackPTK.checksum = createChecksum(ackPTK);
    }
    //else
        //was error
        //send to layer3 again
        //start timer
    else{
        //print was error, resending
        tolayer3(AEntity, prevPKT);
        startTimer(AEntity, 1000);
    }
    */
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt()
{
    if (prevPKT.seqnum != ackPTK.acknum)
    {
        //trace print resending, and time out
        tolayer3(AEntity, prevPKT);
        startTimer(AEntity, 1000);
    }
    else
    {
        //packet was acknowledged
    }
}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    //initialize last received back and last ack packet
    prevPKT.seqnum = 1;
    prevPKT.acknum = 0;
    prevPKT.checksum = 0;
    //set ackPKT seq and ack to invalid 0 to ensure first packet goes through...
    ackPTK.seqnum = -1;
    ackPTK.acknum = -1;
    ackPTK.checksum = 0;
    for (int i = 0; i < MESSAGE_LENGTH; i++)
    {
        prevPKT.payload[i] = 0;
        ackPTK.payload[i] = 0;
    }
}

/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */
void B_output(struct msg message)
{
    //not needed
}

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet)
{
    struct pkt resend;
    //if isn't corrupted & correct seqnum
    if ((checkPacket(packet) == 0) && packet.seqnum == correctSeqNum)
    {
        struct pkt reply;   //for layer3 reply
        struct msg sendMSG; //for layer5 send

        correctSeqNum = !correctSeqNum; //flip correct seq num
        correctPacket = 1;              //notify that correct packet was found
        //set reply seqnum and acknum to packet seqnum
        reply.seqnum = packet.seqnum;
        reply.acknum = packet.seqnum;
        //copy packet payload into response
        for (int i = 0; i < MESSAGE_LENGTH; i++)
        {
            reply.payload[i] = packet.payload[i];
        }
        //calculate reply checksum
        reply.checksum = createChecksum(reply);
        //send reply to layer3
        tolayer3(BEntity, reply);
        //copy packet payload into message for layer 5
        for (int i = 0; i < MESSAGE_LENGTH; i++)
        {
            sendMSG.data[i] = packet.payload[i];
        }
        //add 0 to end of message
        sendMSG.data[20] = 0;
        //send message to layer5
        tolayer5(BEntity, sendMSG);
        resend = reply;
    }

    else
    { //wrong seq num or packet corrupt

        if (correctPacket == 1)
        {
            tolayer3(BEntity, resend);
        }
    }
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void B_timerinterrupt()
{
    //not needed
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init()
{
    //initialize anticipated seqnum opposite of prevPKT.seqnum
    correctSeqNum = 0;
    correctPacket = 0;
}

/*
*   This will be called to create the checksum
*/
//still need to do
int createChecksum(struct pkt packet)
{
    int a = 0;
    for (int i = 0; i < MESSAGE_LENGTH; i++)
    {
        a = (a + (i * ((int)(packet.payload[i]))));
    }
    return a;
}
/*
 * This will be called the check the check sum
 */
int checkPacket(struct pkt packet)
{
    int check = createChecksum(packet);

    if (check != packet.checksum)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
