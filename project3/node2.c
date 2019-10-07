#include <stdio.h>
#include "project3.h"

extern int TraceLevel;
extern float clocktime;

struct distance_table
{
    int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt2;
struct NeighborCosts *neighbor2;
struct RoutePacket pkt2;

void setTable2();
void initPKT2();
void sendPKT1(int source, int dest);
void update2(struct RoutePacket *rpkt);
int findMin2(int num);

int isUpdate2;
int min0c;
int min1c;
int min2c;
int min3c;

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
void printdt2(int MyNodeNumber, struct NeighborCosts *neighbor,
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
} // End of printdt2

void setTable2()
{
    int i = 0;
    int j = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        for (j = 0; j < MAX_NODES; j++)
        {
            if (i == j)
            {
                dt2.costs[i][j] = neighbor2->NodeCosts[i];
            }
            else
            {
                dt2.costs[i][j] = INFINITY;
            }
        }
    }
}

void sendPKT2(int source, int dest)
{
    pkt2.sourceid = source;
    pkt2.destid = dest;
    toLayer2(pkt2);
    printf("At time=%f, node %d sends packet to node %d with: %i %i %i %i\n", clocktime, source, dest, pkt2.mincost[0], pkt2.mincost[1], pkt2.mincost[2], pkt2.mincost[3]);
}

void initPKT2()
{
    int i = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        pkt2.mincost[i] = INFINITY;
    }
    for (i = 0; i < MAX_NODES; i++)
    {
        if (pkt2.mincost[i] > neighbor2->NodeCosts[i])
        {
            pkt2.mincost[i] = neighbor2->NodeCosts[i];
        }
    }
}

void update2(struct RoutePacket *rpkt)
{
    int i = 0;
    int j = 0;
    int k = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        pkt2.mincost[i] = INFINITY;
    }

    for (i = 0; i < MAX_NODES; i++)
    {
        if ((dt2.costs[i][rpkt->sourceid]) > (dt2.costs[rpkt->sourceid][rpkt->sourceid] + rpkt->mincost[i]))
        {
            dt2.costs[i][rpkt->sourceid] = (dt2.costs[rpkt->sourceid][rpkt->sourceid] + rpkt->mincost[i]);
            isUpdate2 = 1;

            for (k = 0; k < MAX_NODES; k++)
            {
                if (pkt2.mincost[i] > dt2.costs[i][k])
                {
                    pkt2.mincost[i] = dt2.costs[i][k];
                }
            }
        }
    }
}

int findMin2(int num)
{
    int i = 0;
    int min = INFINITY;
    for (i = 0; i < MAX_NODES; i++)
    {
        if (dt2.costs[num][i] < min)
        {
            min = dt2.costs[num][i];
        }
    }
    return min;
}

void calcMin2(int num)
{
    int i = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        if (pkt2.mincost[i] > dt2.costs[num][i])
        {
            pkt2.mincost[i] = dt2.costs[num][i];
        }
    }
}



/* students to write the following two routines, and maybe some others */

void rtinit2()
{
    int i = 0;
    neighbor2 = getNeighborCosts(2);
    printf("At time t=%f, rinit2() called.\n", clocktime);
    setTable2();
    printdt2(2, neighbor2, &dt2);
    initPKT2();
    for (i = 0; i < MAX_NODES; i++)
    {
        if ((neighbor2->NodeCosts[i] != INFINITY) && i != 2)
        {
            sendPKT2(2, i);
        }
    }
    printf("\n");
}

void rtupdate2(struct RoutePacket *rcvdpkt)
{
    printf("At time t=%f, rtupdate2() called, by a pkt received from Sender id: %i;\n", clocktime, rcvdpkt->sourceid);
    isUpdate2 = 0;
    int i = 0;
    update2(rcvdpkt);
    /*
    for( i = 0; i < MAX_NODES; i++)
    {
        //if ((neighbor2->NodeCosts[i] != INFINITY) && i != 2)
        if ((pkt2.mincost[i] != INFINITY) && i != 2)
        {
            sendPKT2(2, i);
        }
    }
    */
    for (i = 0; i < MAX_NODES; i++)
    {
        calcMin2(i);
    }
    if (isUpdate2)
    {
        for(i = 0; i < MAX_NODES; i++){
            if((neighbor2->NodeCosts[i] != INFINITY) && (i != 2))
            {
                sendPKT2(2, i);
            }
        }
        printdt2(2, neighbor2, &dt2);
        printf("\n");
    }
    min0c = findMin2(0);
    min1c = findMin2(1);
    min2c = 0;
    min3c = findMin2(3);
    printf("At time t=%f, node 2 current distance vector: %d %d %d %d\n", clocktime, min0c, min1c, min2c, min3c);
}
