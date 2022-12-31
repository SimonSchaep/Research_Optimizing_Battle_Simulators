# Optimizing Battle Simulators




There are generally two aspects to making a battle simulator:
1. Using the GPU instead of CPU, not only for rendering but also for pathfinding and ai behavior.
2. Efficient crowd simulation algorithms
3. Efficient partitioning algorithms


we now implement a simple separation behavior to make the agents avoid each other

this makes it very inefficient
so we'll add a spatial partitioning technique
there are many different types of spatial partitioning
grids
quadtrees
octtrees

most spatial partitioning is used when agents only need information about other NEARBY agents.
Though in our case, the closest enemy might be very far away from our agent






The focus will still be on optimizing a battle simulator.
This means the algorithms we use need to very efficient.
They also need to be used for finding the closest enemy target in a crowd of enemies
They also need to be able to be used


In our case (a battle simulator)
The crowd will need to make its way to the closest enemy crowd
Once the crowd is close enough, all agents need to find their own closest target to attack.
Once that target dies, the agent needs to find the next closest target.


we need to simulate a crowd of soldiers, attacking another crowd of soldiers. This needs to be done in an opimized way, so that there can be as many as possible





INTRODUCTION:
This is a small research project on how to optimize a large amount of ai agents in a battle simulator.
Inspired by the millions of agents that some battle simulators can simulate. A good example of this is Ultimate Epic Battle Simulator (https://store.steampowered.com/app/616560/Ultimate_Epic_Battle_Simulator/), and its sequel Ultimate Epic Battle Simulator 2 (https://store.steampowered.com/app/1468720/Ultimate_Epic_Battle_Simulator_2/).


explain gpu
explain crowd simulation
explain pathfinding




BASE APPLICATION:
Explain how it's made
what it can do
measure multiple times
post measurements in different situations
explain smth about measurements

pause is done by setting deltatime to 0, this way framerate will be (almost) identical in paused and unpaused state
this helps with measuring





MULTITHREADING:
would be better on gpu
but cpu is quicker to implement
quite easy to multithread our most costly processes since already in for loop
use parallel for instead
only issue is that todelete is no longer sorted
quickly sort using algorithm 
tested sort, qsort and stable sort. All three gave almost the exact same fps(5-6) when benchmarking with 5000 agents per team. They probably have a very small difference but not enough to make an impact in our case

multithreading gives a very high performance boost







PARTITIONING:
explain different kinds of partitioning and what they're good for
https://www.ijarnd.com/manuscripts/v3i10/V3I10-1144.pdf

explain simple grid partitioning
https://gameprogrammingpatterns.com/spatial-partition.html
explain own implementation

explain quad trees
http://homepage.divms.uiowa.edu/~kvaradar/sp2012/daa/ann.pdf

explain own implementation

explain how to get closest agents from closest cells

current is not perfect, but gets most accurate results

gridpartitioning most stable
target done in cells is much better with large number of agents far away from each other


explain issues with multithreading:
checking if an agent changed cell can't be done async, because agents will be added to cells, which might cause the vector to resize, which will give issues if another thread is accessing that same vector. could be fixed by reserving size, but size could be as much as the total enabled agents, and reserving that much memory in every cell would use way too much memory.
so we have to do the checking in sync after updating agents



AGENT SIZE:
what if very large agents are present?
without partitioning, subtract distance by own radius and radius of target
with partitioning, will not always find closest target




CROWD SIMULATION:
https://en.wikipedia.org/wiki/Crowd_simulation
explain different ways to simulate a crowd

explain how to apply it to a battle simulator

find how to do this optimized

implement
explain implementation



can decrease neighbor radius to decrease spacing between agents



FUTURE WORK:
Implement better crowd simulation
Use GPU
Implement efficient collision system
Convert to 3D
Add animation

several issues with that
adding and removing agents to cells/resizing grid, causes asynchronisation, this could be fixed by buffering the agents, but this will then cause performance to go down again




