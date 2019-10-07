#include <stdio.h>
#include "project3.h"

extern int TraceLevel; 
extern float clocktime; //used to print time in simulation

struct distance_table
{
    int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt0;
struct NeighborCosts *neighbor0;
struct RoutePacket pkt;

void setTable0();
void initPKT();
void sendPKT(int source, int dest);
void update(struct RoutePacket *rpkt);
int findMin(int num);
void calcMin(int num);

int isUpdate; //to see if update has updates
int min0; 
int min1;
int min2;
int min3;

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
void printdt0(int MyNodeNumber, struct NeighborCosts *neighbor,
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
} // End of printdt0

/* students to write the following two routines, and maybe some others */
//set the distance table infinity or cost to neighbors
void setTable0()
{
    int i = 0;
    int j = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        for (j = 0; j < MAX_NODES; j++)
        {
            if (i == j)
            {
                dt0.costs[i][j] = neighbor0->NodeCosts[i];
            }
            else
            {
                dt0.costs[i][j] = INFINITY;
            }
        }
    }
}

//send pkt from source to destination
void sendPKT(int source, int dest)
{
    pkt.sourceid = source;
    pkt.destid = dest;
    toLayer2(pkt);
    printf("At time=%f, node %d sends packet to node %d with: %i %i %i %i\n", clocktime, source, dest, pkt.mincost[0], pkt.mincost[1], pkt.mincost[2], pkt.mincost[3]);
}

//intialize pkt struct in node0
void initPKT()
{
    int i = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        pkt.mincost[i] = INFINITY;
    }
    for (i = 0; i < MAX_NODES; i++)
    {
        if (pkt.mincost[i] > neighbor0->NodeCosts[i])
        {
            pkt.mincost[i] = neighbor0->NodeCosts[i];
        }
    }
}

//update packet min costs if update is true, meaning new shortest path is found
void update(struct RoutePacket *rpkt)
{
    int i = 0;
    int k = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        pkt.mincost[i] = INFINITY;
    }

    for (i = 0; i < MAX_NODES; i++)
    {
        if ((dt0.costs[i][rpkt->sourceid]) > (dt0.costs[rpkt->sourceid][rpkt->sourceid] + rpkt->mincost[i]))
        {
            dt0.costs[i][rpkt->sourceid] = (dt0.costs[rpkt->sourceid][rpkt->sourceid] + rpkt->mincost[i]);
            isUpdate = 1;

            for (k = 0; k < MAX_NODES; k++)
            {
                if (pkt.mincost[i] > dt0.costs[i][k])
                {
                    pkt.mincost[i] = dt0.costs[i][k];
                }
            }
        }
    }
}
//go through table and find min costs for printout
int findMin(int num)
{
    int i = 0;
    int min = INFINITY;
    for (i = 0; i < MAX_NODES; i++)
    {
        if (dt0.costs[num][i] < min)
        {
            min = dt0.costs[num][i];
        }
    }
    return min;
}
//calc the mincost from the table
void calcMin(int num)
{
    int i = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        if (pkt.mincost[i] > dt0.costs[num][i])
        {
            pkt.mincost[i] = dt0.costs[num][i];
        }
    }
}


void rtinit0()
{
    int i = 0;
    neighbor0 = getNeighborCosts(0); //initialize neighbor struct
    printf("At time t=%f, rinit0() called.\n", clocktime);
    setTable0(dt0); //initialize the table
    printdt0(0, neighbor0, &dt0); //print original table
    initPKT(); //initialize pkt
    for (i = 0; i < MAX_NODES; i++) //send first packets
    {
        if ((neighbor0->NodeCosts[i] != INFINITY) && i != 0)
        {
            sendPKT(0, i);
        }
    }
    printf("\n");
}

void rtupdate0(struct RoutePacket *rcvdpkt)
{
    printf("At time t=%f, rtupdate0() called, by a pkt received from Sender id: %i;\n", clocktime, rcvdpkt->sourceid);
    isUpdate = 0; //reset if there is an update to send out packets
    int i = 0;
    update(rcvdpkt); //update based on incoming pkt

    for (i = 0; i < MAX_NODES; i++) //recalc mincost after running update(rcvdpkt)
    {
        calcMin(i);
    }
    if (isUpdate) //see if isUpdate
    {
        //send packets to neighbors
        for(i = 0; i < MAX_NODES; i++){ 
            if((neighbor0->NodeCosts[i] != INFINITY) && (i != 0))
            {
                sendPKT(0, i);
            }
        }
        printdt0(0, neighbor0, &dt0); //print table
        printf("\n");
    }
    //calc and print shortest path to each node
    min0 = 0;
    min1 = findMin(1);
    min2 = findMin(2);
    min3 = findMin(3);
    printf("At time t=%f, node 0 current distance vector: %d %d %d %d\n", clocktime, min0, min1, min2, min3);
}
