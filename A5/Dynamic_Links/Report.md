## Changes Made

* Previously a program execution activates a single router.
* Now the program reads the input file and forks all the routers to run and the main program runs as a parent process.
* In the parent process, we introduce a client socket to send messages to the routers.
* On command : *q*; Parent client socket sends a kill signal to all routers and exits.
* On command : *r id1 id2*; Parent client send message to node id1 and id2 to remove the other node from their neighbour list.
* On command : *a id1 id2 min_cost max_cost*; Parent client send message to node id1 and id2 to add the other node to their neighbour list with min and max cost.
* Topology table must be refreshed before computing Shortest Path each time to acknowledge the changes made.