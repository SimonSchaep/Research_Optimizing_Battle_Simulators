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

There are of course many aspects to making a battle simulator, I will try to cover most of the important ones and implement some of them in a small battle simulator application.

All the code will be in separate branches depending on what partitioning technique I used. Since that made the most difference between implementations, all other things are implemented in all branches


explain gpu
explain crowd simulation
explain pathfinding




BASE APPLICATION:

So, for our own battle simulator that we'll be using to test some optimization techniques, we'll be working in c++ (since it's fast) and using a framework that has a camera, UI and allows us to draw. It also has some basic structs like vector2. We won't be using any in-built physics systems or ai pathfinding since those can be very expensive and I want to explore all the important aspects of optimizing a battle simulator.

We have an agent, every frame, it will try to find the closest enemy target, then calculate the velocity (which will be a vector towards the target), and moves according to that velocity, when it is in attack range, it will stop moving and try to attack.

Our agent will have a simple health component, that we use to do all the health calculations and it tells us when we're dead. It will also have a melee attack component, which we use to attack other agents.


Since this is a battle simulator there will be lots of agents dying and spawning. Constantly deallocating and reallocating memory for these agents would be very expensive, so we will eb using an object pool. This means we will allocate a lot of agents at the start of the application and set their state to disabled, and when we want to spawn an agent, we will find a disabled agent and enable it. Whenever an agent dies we will disable it.
We hold these agents in two separate vectors, since we don't want to loop over all the disabled agents every frame. This way the size of our initial pool won't impact our runtime speed. It will still impact the startup time of the application.

There are two more disadvantages:
- There might be a lot more memory used by the application than what is actually necessary, or there might not be enough memory allocated and our pool overflows. That   last one can be fixed by detecting when the pool will overflow and then create more agents during runtime.
- We need to be careful and make sure when we disable an agent, that everything is properly reset, so nothing is remembered and carried over to the next life of the agent

http://gameprogrammingpatterns.com/object-pool.html


In our main application, I added a few options to be able to test everything, you can spawn agents by clicking a button, which will spawn agents according to the values you put in.
You can also spawn agents by dragging the mouse, after selecting a team by pressing the number keys.
You can delete agents by pressing backspace or the clear agents button in the UI.
There is also a timescale slider to speed up or slow down the simulator.
And there are some more settings that will be explained further in this readme.

We will define a few benchmarks that we will use to measure how fast the application is. The fps will of course differ based on which pc you are running it on.

Benchmarks:
1000 red vs 1000 blue.
5000 red vs 5000 blue.
10000 red vs 10000 blue.

500 of each team.
1000 of each team.
3000 of each team.
6000 of each team.

We set timescale to 0 to measure, since all calculations are still done, the agents just won't move since their velocity will be multiplied by 0.



Explain how it's made
what it can do
measure multiple times
post measurements in different situations
explain smth about measurements

pause is done by setting deltatime to 0, this way framerate will be (almost) identical in paused and unpaused state
this helps with measuring





MULTITHREADING:

One of the most efficient ways of optimizing any application is using more than one thread of your cpu. The idea is that you run tasks on multiple threads simultaneously. This can multiply the speed of an applications by a lot, depending on how many cores your cpu has.
We will use multithreading on the biggest bottleneck of the battlesimulator, updating agents. This is relatively easy since we are already doing this in a for loop, we can use concurrency::parallel_for from the <ppl.h> header.
Since multiple threads are accessing some of the same resources, there are some issues that emerge.
A first issue is with the way we are disabling our agents, we can't delete our agents from the vector we are looping over so we are instead adding their index to a list of indexes of agents that need to be removed. After the agents are updated, this list is looped over and we remove the according agents.
Using multithreading you need to make sure you don't resize a vector during a parallel thread, because resizing might make the vector relocate to a different part of memory, that the other threads don't know about. Since we are doing a pushback and this can cause a vector to resize when it's size would be bigger than the capacity, we need to fix this. Luckily this is easily done by reserving enough size to the vector before starting the parallel threads. This does use more memory than necessary in most cases since there is the possibility that all agents will be disabled, so the reserve should reserve memory enough for all the enabled agents. An added benefit is that it should be faster when disabling many agents at once since there won't be constant resizing. But in all other cases there is memory 'wasted'.
Another issue happens with the way we are removing agents.
We remove agents by replacing the agent with the last agent of the vector, and then remove the last agent, this is faster than removing an agent from the middle of a vector, since all subsequent elements would have to be moved. Using a list, this would not be the case, but since we need to constantly access our agents the constant lookup time of a vector is preferred.
This means we have to start with the highest index of agents that need to be disabled and end with the first one. Otherwise we might be disabling the wrong agents.
For example we have 10 enabled agents, we need to disable agent 0, 5 and 9. doing this from 0 to 9 happens like this:
(read agent[n] as agent at index n)
start with 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
agent[0] is swapped with 9
9, 1, 2, 3, 4, 5, 6, 7, 8, 0
agent[9] is deleted
9, 1, 2, 3, 4, 5, 6, 7, 8, null
agent[5] is swapped with 8
9, 1, 2, 3, 4, 8, 6, 7, 5, null
agent[8] is deleted
9, 1, 2, 3, 4, 8, 6, 7, null, null
agent[9] is swapped with 7
9, 1, 2, 3, 4, 8, 6, null, null, 7
agent[7] is deleted
9, 1, 2, 3, 4, 8, 6, null, null, 7
so 9 did not get deleted, and 7 got put after where the vector ends.

if we sort the list of agent indexes to be removed: (9,5,0)
start with 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
agent[9] is swapped with 9
0, 1, 2, 3, 4, 5, 6, 7, 8, 9
agent[9] is deleted
0, 1, 2, 3, 4, 5, 6, 7, 8, null
agent[5] is swapped with 8
0, 1, 2, 3, 4, 8, 6, 7, 5, null
agent[8] is deleted
0, 1, 2, 3, 4, 8, 6, 7, null, null
agent[0] is swapped with 7
7, 1, 2, 3, 4, 8, 6, 0, null, null
agent[7] is deleted
7, 1, 2, 3, 4, 8, 6, null, null, null
our final enabled agents are 1, 2, 3, 4, 6, 7 and 8
this is what we want.


This issue can be fixed by sorting the list after updating the agents.
There are multiple sorting algorithms.
I tested std::qsort, std::sort and std::stableSort. They all didn't impact the framerate at all, probably because the list to sort usually isn't that big.

There are some more issues that will emerge later on when we use partitioning, but I'll explain them at the appropriate time.

It is important to also mention the gpu here. Gpus also use parallelism, but they have many more cores compared to a cpu, and also more threads per core. This allows them to run code a lot faster than a cpu, they are however more limited in memory.
Commercial battle simulators (like UEBS 2) are often running a lot of code on the gpu which drastically improves performance and allows for many more agents to be simulated. It is however a lot more complicated than cpu programming, so I won't implement it in this project.

https://www.pgroup.com/blogs/posts/cuda-threading-model.htm


would be better on gpu
but cpu is quicker to implement
quite easy to multithread our most costly processes since already in for loop
use parallel for instead
only issue is that todelete is no longer sorted
quickly sort using algorithm 
tested sort, qsort and stable sort. All three gave almost the exact same fps(5-6) when benchmarking with 5000 agents per team. They probably have a very small difference but not enough to make an impact in our case

multithreading gives a very high performance boost







PARTITIONING:

One of the biggest bottlenecks in our simulator is still finding the closest target for each of our agents, we are checking all of the enabled agents in every agent update, which means increasing the amount of agents exponentially increases the time spent searching for the closest target.
A solution would be to make sure the agents only check agents that are reasonably close to them. How can we know which agents should be checked? By using partitioning.
Partitioning is a design pattern used to divide up agents into different 'groups' so we can check if that group is close enough, and only then check all the agents inside. This pattern is often used for collision detection, where we store points in the partitions. It is also used to find nearest neighbors, which is very close to what we want to do.
There are many different kinds of partitioning. The simplest is using a grid to divide the world in many cells, each cell represents a group of agents.
I will first explain how I implemented gridpartitioning.

Archtecture:
We have a grid class and a cell class.
The grid contains a vector of cells, it also contains functions to get specific cells (based on position and row/column).
A cell contains a vector that holds the agents in that cell.

Our agent now also holds a pointer to the cell it is currently in. To make sure the cells have the correct agents inside, we will need to check every frame, for every agent, if the cell at the agent's position is still the same cell as last frame, if it changed we have to update the current cell in the agent and add the agent to the new cell and remove it from the old cell.

This works perfectly, you can see it in action by ticking the 'render grid' option in the UI of the application.
But there is an issue when we do this while multithreading. While we are finding a target in a cell in one thread, another might be modifying that same cell's vector of agents, this causes issues since we might be removing from a vector while looping over it. To fix this we need to update the agents' cells in a synchronous for loop after the normal update. This will make the multithreading a lot less efficient but is still a good solution. Another solution would be buffering the agents in every cell. So instead of modifying the active vector of agents when removing or adding agents, we would do it on a separate vector, and then update the active vector based on the modified one after looping over our agents. That way we will need a lot more memory and won't necessarily be faster since updating the vector would happen twice (on the buffer, and after that on the active vector).

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




