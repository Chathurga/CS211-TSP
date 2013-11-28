# Introduction

The [Travelling Salesman Problem](http://en.wikipedia.org/wiki/Travelling_salesman_problem) (TSP) is a [NP-hard](http://www.quora.com/What-are-P-NP-NP-complete-and-NP-hard) where you have N towns (or points) and you have to find he shortest route between all towns given that:

*   You have to enter and leave every town once
*   You have to form a complete route, the "last" town must link back to the "first" town

A quick reference of some terminology I'll use:
*   **Distance matrix**: A data structure that contains the distance between any 2 towns. Every method of solving the TSP will obviously require this.
*   **Journey**: Going from town A sraight to town B.

If a certain method of solving the TSP always gives you the best answer we'll call that a guaranteed optimal solution. The method I detail here will generate the guaranteed optimal solution if it produces a final answer. For certain instances of the problem it will fail to end in a reasonable amount of time either due to there being too many points or the layout of the points makes it hard to solve.

Most methods produce an approximation, it might be the best answer but you have no guarantee (and often it isn't the optimal solution). When set the TSP as a class challenge most people will use an approximation technique. Usually the technique will initially be: start at a point, go to the nearest neighbour that hasn't been used yet, repeat. That gives an answer, almost certainly not the best but it is a valid solution. You can improve this method by doing various refinements like:

*   Starting at every point do the nearest neighbour approach and return the route that gave the best answer
*   Jitter every value in the distance matrix with random offsets. This can make the nearest neighbour approach choose a route that it otherwise wouldn't but is actually closer to the optimal solution. You can tune the jitter range to get better results.
*   The telltale sign of a non-optimal solution is **crossovers** where the journeys between 2 pairs of towns intersect. Uncrossing these journies will yield a better solution.

Some of the approximation techniques are quick clever (like the [Ant colony method](http://en.wikipedia.org/wiki/Travelling_salesman_problem#Ant_colony_optimization) but it's always nicer to completely solve something so let's do that.

# Linear Programming Technique

This method of solving the TSP works by phrasing the TSP as a linear equation then getting the minimal solution to that equation under a set of constraints.  

This is an example of a minimization problem (x(1) and x(2) are just variable names):  
    
    Minimize:
    2 * x(1) + 6 * x(2)
    
    Where:
    x(1) >= 2
    x(2) >= 4

Here we want to get the smallest solution to "2 x(1) + 6 x(2)" under the constraints that x(1) is at least 2 and x(2) is at least 4.  

If we ran that through a solver it would find that this equation is minimal simply when x(1) is 2 and x(2) is 4. The output we would read is what values the solver set for x(1) and x(2). We'll call x(1) and x(2) the solution variables.   

Now for the real problem. Given a list of towns the first thing we do is generate the distance between every pair of towns. Since the distance between town 1 to 2 is the same as town 2 to 1 we won't include duplicates.  

    dist(1,2) = 46
    dist(1,3) = 23
    ...
    dist(1,80) = 125
    dist(2,3) = 33
    ...
    dist(79,80) = 12

Where dist(i,j) represents the distance between town i and j.  

Now we'll define the solutions variables for our problem. These will mark whether a particular journey between towns was taken. Each of these solution variables can be either 0 or 1 where 0 = "wasn't taken" and 1 = "taken".

With these binary variables and distance calculations we can formulate the TSP like so:

    Minimize:
    dist(1,2) * x(1,2) + dist(1,3) * x(1,3) + ...

Where x(i,j) indicates whether the journey between i and j was taken.

If a journey is set to 1 then its distance gets added to the equation, if the journey is set to 0 then the distance gets canceled out. We'll make the solver flip certain journeys on or off with the goal of minimizing the total distance used.

If we wanted to minimize the result of that equation then all we'd have to do it set every x(i,j) to zero.

    dist(1,2) * 0 + dist(1,3) * 0 + ... = 0

This is why we need constraints to make the solver turn certain journeys on.

The first constraint is that if you take the number of towns in a problem as N then the number of journeys turned on must be N.


** Writeup Still in Progress
