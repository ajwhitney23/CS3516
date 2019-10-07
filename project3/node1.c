#include <stdio.h>
#include "project3.h"

extern int TraceLevel;
extern float clocktime;

struct distance_table
{
    int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt1;
struct NeighborCosts *neighbor1;
struct RoutePacket pkt1;

void setTable1();
void initPKT1();
void sendPKT1(int source, int dest);
void update1(struct RoutePacket *rpkt);
int findMin1(int num);

int isUpdate1;
int min0b;
int min1b;
int min2b;
int min3b;

/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt1(int MyNodeNumber, struct NeighborCosts *neighbor,
              struct distance_table *dtptr)
{
    int i, j;
    int TotalNodes = neighbor->NodesInNetwork; // Total nodes in network
    int NumberOfNeighbors = 0;                 // How many neighbors
    int Neighbors[MAX_NODES];                  // Who are the neighbors

    // Determine our neighbors
    for (i = 0; i < TotalNodes; i++)
    {
        if ((neighbor->NodeCosts[i] != INFINITY) && i != MyNodeNumber)
        {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber);
    for (i = 0; i < NumberOfNeighbors; i++)
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for (i = 0; i < TotalNodes; i++)
    {
        if (i != MyNodeNumber)
        {
            printf("dest %d|", i);
            for (j = 0; j < NumberOfNeighbors; j++)
            {
                printf("  %4d", dtptr->costs[i][Neighbors[j]]);
            }
            printf("\n");
        }
    }
    printf("\n");
} // End of printdt1

void setTable1()
{
    int i = 0;
    int j = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        for (j = 0; j < MAX_NODES; j++)
        {
            if (i == j)
            {
                dt1.costs[i][j] = neighbor1->NodeCosts[i];
            }
            else
            {
                dt1.costs[i][j] = INFINITY;
            }
        }
    }
}

void sendPKT1(int source, int dest)
{
    pkt1.sourceid = source;
    pkt1.destid = dest;
    toLayer2(pkt1);
    printf("At time=%f, node %d sends packet to node %d with: %i %i %i %i\n", clocktime, source, dest, pkt1.mincost[0], pkt1.mincost[1], pkt1.mincost[2], pkt1.mincost[3]);
}

void initPKT1()
{
    int i = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        pkt1.mincost[i] = INFINITY;
    }
    for (i = 0; i < MAX_NODES; i++)
    {
        if (pkt1.mincost[i] > neighbor1->NodeCosts[i])
        {
            pkt1.mincost[i] = neighbor1->NodeCosts[i];
        }
    }
}

void update1(struct RoutePacket *rpkt)
{
    int i = 0;
    int j = 0;
    int k = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        pkt1.mincost[i] = INFINITY;
    }

    for (i = 0; i < MAX_NODES; i++)
    {
        if ((dt1.costs[i][rpkt->sourceid]) > (dt1.costs[rpkt->sourceid][rpkt->sourceid] + rpkt->mincost[i]))
        {
            dt1.costs[i][rpkt->sourceid] = (dt1.costs[rpkt->sourceid][rpkt->sourceid] + rpkt->mincost[i]);
            isUpdate1 = 1;
            for (k = 0; k < MAX_NODES; k++)
            {
                if (pkt1.mincost[i] > dt1.costs[i][k])
                {
                    pkt1.mincost[i] = dt1.costs[i][k];
                }
            }
        }
    }
}

int findMin1(int num)
{
    int i = 0;
    int min = INFINITY;
    for (i = 0; i < MAX_NODES; i++)
    {
        if (dt1.costs[num][i] < min)
        {
            min = dt1.costs[num][i];
        }
    }
    return min;
}

void calcMin1(int num)
{
    int i = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        if (pkt1.mincost[i] > dt1.costs[num][i])
        {
            pkt1.mincost[i] = dt1.costs[num][i];
        }
    }
}


/* students to write the following two routines, and maybe some others */

void rtinit1()
{
    int i = 0;
    neighbor1 = getNeighborCosts(1);
    printf("At time t=%f, rinit1() called.\n", clocktime);
    setTable1();
    printdt1(1, neighbor1, &dt1);
    initPKT1();
    for (i = 0; i < MAX_NODES; i++)
    {
        if ((neighbor1->NodeCosts[i] != INFINITY) && i != 1)
        {
            sendPKT1(1, i);
        }
    }
    printf("\n");
}

void rtupdate1(struct RoutePacket *rcvdpkt)
{
    printf("At time t=%f, rtupdate1() called, by a pkt received from Sender id: %i;\n", clocktime, rcvdpkt->sourceid);
    isUpdate1 = 0;
    int i = 0;
    update1(rcvdpkt);
    /*
    for (i = 0; i < MAX_NODES; i++)
    {
        
        if((neighbor1->NodeCosts[i] != INFINITY) && i != 1)
        if ((pkt1.mincost[i] != INFINITY) && i != 1)
        {
            sendPKT1(1, i);
        }
        
    }
    */
    for (i = 0; i < MAX_NODES; i++)
    {
        calcMin1(i);
    }
    if (isUpdate1)
    {
        for(i = 0; i < MAX_NODES; i++){
            if((neighbor1->NodeCosts[i] != INFINITY) && (i != 1))
            {
                sendPKT1(1, i);
            }
        }
        printdt1(1, neighbor1, &dt1);
        printf("\n");
    }
    min0b = findMin1(0);
    min1b = 0;
    min2b = findMin1(2);
    min3b = findMin1(3);
    printf("At time t=%f, node 1 current distance vector: %d %d %d %d\n", clocktime, min0b, min1b, min2b, min3b);
}
