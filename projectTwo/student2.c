#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"
#include <crypt.h>
#include <unistd.h>

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
//prototypes
int calcChecksum(struct pkt packet);
int checkChecksum(struct pkt packet);

//for loop counter; ccc servers do not allow it to be created in for loop
int i;

//packets
struct pkt prevPKT; //stores the previous packet (for restranmission and setting next packet seqnum)
struct pkt prevACK; //stores the previous ack packet
struct pkt resend;  //stores b side for resend if packet is corrupted...

//B-side variables
int seqNum;    //used for the expected seqnum... will alt between 0 and 1
int hasResend; //used to make sure pkt resend has been filled

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message)
{

    struct pkt toSend;                 //create packet to send
    toSend.acknum = 0;                 //set NULL?; 0 for checksum calculation
    toSend.seqnum = !(prevPKT.seqnum); //set seqnum to opposite of previous packet sent out
    for (i = 0; i < MESSAGE_LENGTH; i++)
    {
        toSend.payload[i] = message.data[i]; //put message into payload
    }
    toSend.checksum = calcChecksum(toSend); //calculate checksum

    //store the packet created into the prevPKT pkt
    prevPKT.seqnum = toSend.seqnum;
    prevPKT.acknum = toSend.acknum;
    for (i = 0; i < MESSAGE_LENGTH; i++)
    {
        prevPKT.payload[i] = toSend.payload[i];
    }
    prevPKT.checksum = toSend.checksum;

    //send the packet finally & start timer
    tolayer3(AEntity, toSend);
    startTimer(AEntity, 1000);
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)
{
    //not used in this project
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet)
{
    //make sure packet is not corrupt and the acknum is correct (should be that of last packet A sent out's seqnum)
    if ((checkChecksum(packet) == 1) && (packet.acknum == prevPKT.seqnum))
    {
        stopTimer(AEntity); //stop timer to prevent timeout
        //now that packet has passed check, store in the last acked packet struct
        prevACK.acknum = packet.acknum;
        prevACK.seqnum = packet.seqnum;
        for (i = 0; i < MESSAGE_LENGTH; i++)
        {
            prevACK.payload[i] = packet.payload[i];
        }
        prevACK.checksum = calcChecksum(prevACK);
    }

    else
    {
        //resend last A packet since acknum is not correct
        tolayer3(AEntity, prevPKT);
        startTimer(AEntity, 1000);
    }
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt()
{
    //resend packet, but ensure it is the right one
    if (prevPKT.seqnum != prevACK.acknum) //see if packet's seqnum is opposite of last ACK
    {
        tolayer3(AEntity, prevPKT);
        startTimer(AEntity, 1000);
    }
}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    prevPKT.seqnum = 1;                  //initialize so first seqnum is 0
    prevPKT.acknum = 0;                  //will be overwritten
    prevPKT.checksum = 0;                //will be overwritten
    prevACK.seqnum = -1;                 //will be overwritten
    prevACK.acknum = -1;                 //will be overwritten
    prevACK.checksum = 0;                //will be overwritten
    for (i = 0; i < MESSAGE_LENGTH; i++) //will be overwritten
    {
        prevPKT.payload[i] = 0;
        prevACK.payload[i] = 0;
    }
}

/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet)
{
    //create a packet and message for response
    struct pkt replyPKT;
    struct msg replyMSG;

    //ensure packet is not corrupt and it is the correct seqnum (arrived on time, in order)
    if ((checkChecksum(packet) == 1) && packet.seqnum == seqNum)
    {
        seqNum = !seqNum; //passed check so flip seqnum

        //first create reply packet acknum and seqnum to that of incoming packet seqnum
        replyPKT.acknum = packet.seqnum;
        replyPKT.seqnum = packet.seqnum;

        //create reply packet payload and message data
        for (i = 0; i < MESSAGE_LENGTH; i++)
        {
            replyPKT.payload[i] = packet.payload[i];
            replyMSG.data[i] = packet.payload[i];
        }
        replyPKT.checksum = calcChecksum(replyPKT);

        //send reply packet and message packet
        tolayer3(BEntity, replyPKT);
        tolayer5(BEntity, replyMSG);

        //store if next seqnum is corrupted to resend.
        resend.seqnum = packet.seqnum;
        resend.acknum = packet.acknum;
        resend.checksum = packet.checksum;
        for (i = 0; i < MESSAGE_LENGTH; i++)
        {
            resend.payload[i] = packet.payload[i];
        }

        hasResend = 1; //store that there is a resend packet available
    }

    else
    {
        if (hasResend) //ensure resend has been created
        {
            tolayer3(BEntity, resend);
        }
        //else let timerA timeout....
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
    //not used for this assignment...
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init()
{
    seqNum = 0;          //first seqnum should be 0
    hasResend = 0;       //no resend has been created yet
    resend.acknum = 0;   //to be overwritten
    resend.seqnum = 0;   //to be overwritten
    resend.checksum = 0; //to be overwritten
    for (i = 0; i < MESSAGE_LENGTH; i++)
    {
        resend.payload[i] = 0; //to be overwritten
    }
}

int calcChecksum(struct pkt packet)
{
    int res = 0; //initialize result

    //use crypt to create string to be converted to random int
    char *cryptRes = crypt(packet.payload, "aa");
    //create an integer as a result of the crypt
    for (i = 0; i < strlen(cryptRes); i++)
    {
        res = res + (int)cryptRes[i];
    }
    //add seqnum and acknum to result of ack
    res = res + packet.seqnum + packet.acknum;
    //return result
    return res;
}

int checkChecksum(struct pkt packet)
{
    int check = calcChecksum(packet); //calc checksum of packet
    if (check == packet.checksum)     //if not same either acknun, seqnum, payload, or checknum is wrong in packet
    {
        return 1; //return it is not corrupt
    }
    else
    {
        return 0; //return it is corrupt
    }
}