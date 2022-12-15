# Optimizing Battle Simulators

Small research project on how to optimize a large amount of ai agents in a battle simulator.
The biggest performance sink is agents having to find the closest enemy to move towards.
In it's simplest form, agents loop over all enemy agents on the battlefield, calculate the distance and check if it is the closest enemy it has encountered.
This makes the search algorithm for agents be exponential (O(n^2)), since for every agent, it will loop over every agent.

In this project I test different common optimization techniques, like partitioning, not searching a target every frame...
I also try out some other ideas I came up with that could make a difference.

Techniques explored:
1. own thing
2. partitioning
3. conditional searching
4. lowering search interval
