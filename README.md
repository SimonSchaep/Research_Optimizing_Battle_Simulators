# Optimizing Battle Simulators

This is a small research project on how to optimize a large amount of ai agents in a battle simulator.
It can be applied to any game or simulation that requires a lot of agents that need to find a target.
In it's simplest form, agents loop over all enemy agents on the battlefield, calculate the distance and check if it is the closest enemy it has encountered.
This makes the search algorithm for agents be exponential (O(n^2)), since for every agent, it will loop over every agent.

In this project I try to find different ways of optimizing this process.

Techniques explored (placeholders for now):
1. Simple partitioning
2. Some kind of advanced partitioning
3. Agents moving towards closest grid cell with enemy agents until within attack range, only then look for closest agent in that cell.
