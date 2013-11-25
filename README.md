This solver works by phrasing the TSP as a linear equation then getting the minimal solution to that equation under a set of constraints.  

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