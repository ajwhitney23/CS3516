#include <stdio.h>
#include "project3.h"

extern int TraceLevel;
extern float clocktime;

struct distance_table
{
    int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt3;
struct NeighborCosts *neighbor3;
struct RoutePacket pkt3;

void setTable3();
void initPKT3();
void sendPKT3(int source, int dest);
void update3(struct RoutePacket *rpkt);
int findMin3(int num);

int isUpdate3;
int min0d;
int min1d;
int min2d;
int min3d;

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
void printdt3(int MyNodeNumber, struct NeighborCosts *neighbor,
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
} // End of printdt3

void setTable3()
{
    int i = 0;
    int j = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        for (j = 0; j < MAX_NODES; j++)
        {
            if (i == j)
            {
                dt3.costs[i][j] = neighbor3->NodeCosts[i];
            }
            else
            {
                dt3.costs[i][j] = INFINITY;
            }
        }
    }
}

void sendPKT3(int source, int dest)
{
    pkt3.sourceid = source;
    pkt3.destid = dest;
    toLayer2(pkt3);
    printf("At time=%f, node %d sends packet to node %d with: %i %i %i %i\n", clocktime, source, dest,  pkt3.mincost[0], pkt3.mincost[1], pkt3.mincost[2], pkt3.mincost[3]);
}

void initPKT3()
{
    int i = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        pkt3.mincost[i] = INFINITY;
    }
    for (i = 0; i < MAX_NODES; i++)
    {
        if (pkt3.mincost[i] > neighbor3->NodeCosts[i])
        {
            pkt3.mincost[i] = neighbor3->NodeCosts[i];
        }
    }
}

void update3(struct RoutePacket *rpkt)
{
    int i = 0;
    int j = 0;
    int k = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        pkt3.mincost[i] = INFINITY;
    }

    for (i = 0; i < MAX_NODES; i++)
    {
        if ((dt3.costs[i][rpkt->sourceid]) > (dt3.costs[rpkt->sourceid][rpkt->sourceid] + rpkt->mincost[i]))
        {
            dt3.costs[i][rpkt->sourceid] = (dt3.costs[rpkt->sourceid][rpkt->sourceid] + rpkt->mincost[i]);
            isUpdate3 = 1;
            for (k = 0; k < MAX_NODES; k++)
            {
                if (pkt3.mincost[i] > dt3.costs[i][k])
                {
                    pkt3.mincost[i] = dt3.costs[i][k];
                }
            }
        }
    }
}

int findMin3(int num)
{
    int i = 0;
    int min = INFINITY;
    for(i = 0; i < MAX_NODES; i++)
    {
        if(dt3.costs[num][i] < min)
        {
            min = dt3.costs[num][i];
        }
    }
    return min;
}

void calcMin3(int num)
{
    int i = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        if (pkt3.mincost[i] > dt3.costs[num][i])
        {
            pkt3.mincost[i] = dt3.costs[num][i];
        }
    }
}


/* students to write the following two routines, and maybe some others */

void rtinit3()
{
    int i = 0;
    neighbor3 = getNeighborCosts(3);
    printf("At time t=%f, rinit3() called.\n", clocktime);
    setTable3();
    printdt3(3, neighbor3, &dt3);
    initPKT3();
    for (i = 0; i < MAX_NODES; i++)
    {
        if ((neighbor3->NodeCosts[i] != INFINITY) && i != 3)
        {
            sendPKT3(3, i);
        }
    }
    printf("\n");
}

void rtupdate3(struct RoutePacket *rcvdpkt)
{
    printf("At time t=%f, rtupdate3() called, by a pkt received from Sender id: %i;\n", clocktime, rcvdpkt->sourceid);
    isUpdate3 = 0;
    int i = 0;
    update3(rcvdpkt);
    /*
    for(i = 0; i < MAX_NODES; i++)
    {
        //if((neighbor3->NodeCosts[i] != INFINITY) && i != 3)
        if ((pkt3.mincost[i] != INFINITY) && i != 3)
        {
            sendPKT3(3, i);
        }
    }
    */
    for (i = 0; i < MAX_NODES; i++)
    {
        calcMin3(i);
    }
    if (isUpdate3)
    {

        for(i = 0; i < MAX_NODES; i++){
            if((neighbor3->NodeCosts[i] != INFINITY) && (i != 3))
            {
                sendPKT3(3, i);
            }
        }
        printdt3(3, neighbor3, &dt3);
        printf("\n");
    }
    min0d = findMin3(0);
    min1d = findMin3(1);
    min2d = findMin3(2);
    min3d = 0;
    printf("At time t=%f, node 3 current distance vector: %d %d %d %d\n", clocktime, min0d, min1d, min2d, min3d);

}
