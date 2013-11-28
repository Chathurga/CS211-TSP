# Introduction

The [Travelling Salesman Problem](http://en.wikipedia.org/wiki/Travelling_salesman_problem) (TSP) is a [NP-hard](http://www.quora.com/What-are-P-NP-NP-complete-and-NP-hard) where you have N towns (or points) and you have to find he shortest route between all towns given that:

*   You have to enter and leave every town once
*   You have to form a complete route, the "last" town must link back to the "first" town

A quick reference of some terminology I'll use:
*   **Distance matrix**: A data structure that contains the distance between any 2 towns. Every method of solving the TSP will obviously require this.
*   **Journey**: Going from town A straight to town B.

If a certain method of solving the TSP always gives you the best answer we'll call that a guaranteed optimal solution. The method I detail here will generate the guaranteed optimal solution if it produces a final answer. For certain instances of the problem it will fail to end in a reasonable amount of time either due to there being too many points or the layout of the points makes it hard to solve.

Most methods produce an approximation, it might be the best answer but you have no guarantee (and often it isn't the optimal solution). When set the TSP as a class challenge most people will use an approximation technique. Usually the technique will initially be: start at a point, go to the nearest neighbor that hasn't been used yet, repeat. That gives an answer, almost certainly not the best but it is a valid solution. You can improve this method by doing various refinements like:

*   Starting at every point do the nearest neighbor approach and return the route that gave the best answer
*   Jitter every value in the distance matrix with random offsets. This can make the nearest neighbor approach choose a route that it otherwise wouldn't but is actually closer to the optimal solution. You can tune the jitter range to get better results.
*   The telltale sign of a non-optimal solution is *crossovers* where the journeys between 2 pairs of towns intersect. Uncrossing these journeys will yield a better solution.

Some of the approximation techniques are quick clever (like the [Ant colony method](http://en.wikipedia.org/wiki/Travelling_salesman_problem#Ant_colony_optimization)) but it's always nicer to completely solve something so let's do that.

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

**Note**: The equation solver will be an external program that you feed input to and read output from. There are many available e.g. lp_solve (free), GLPK (free), IBM's CPLEX (proprietary) and many others. The completionist in you might want to create one from scratch and that's certainly a great exercise but I didn't feel the need here because I lacked the motivation to undertake such a large side project and CPLEX (the solver I used for this project) is just so incredibly fast.

Now for the real problem. We'll represent the distance matrix with many dist(i,j) variables where dist(i,j) represents the distance between town i and j. Since the distance between town i and j is the same as the distance between town j and i we won't include duplicates. So we'd have many variables like so (or some data structure that encapsulated these):

    dist(1,2)   = 46
    dist(1,3)   = 23
    ...
    dist(1,N)   = 125
    dist(2,3)   = 33
    ...
    dist(N-1,N) = 12

Now we'll define the solutions variables for our problem. These will mark whether a particular journey between towns was taken. We'll force these variables to be binary so they can be either 0 or 1 where 0 = *wasn't taken* and 1 = *taken*.

With these binary variables and distance calculations we can formulate the TSP like so:

    Minimize:
    dist(1,2) * x(1,2) + dist(1,3) * x(1,3) + ...
    
    Binary Vars:
    for i in (1:N): for j in (1:N): x(i,j)

Where x(i,j) indicates whether the journey between town i and j was taken. The syntax I'm using here is made up but it resembles the syntax used in the various problem formats read by different solvers.

If a journey is set to 1 then its distance gets added to the equation, if the journey is set to 0 then the distance gets cancelled out. We'll make the solver flip certain journeys on or off with the goal of minimizing the total distance used.

If a solver wanted to minimize the result of that equation then all it'd have to do is set every x(i,j) to zero.

    dist(1,2) * 0 + dist(1,3) * 0 + ... = 0

Now we need constraints to make the solver turn certain journeys on.

How many solution variables should the solver output? Well there should be exactly N journeys so our first constraint will force this condition.

    Minimize: ...
    
    Where:
    x(1,2) + x(1,3) + ... + x(N-1,N) = N
    
    Binary Vars: ...

We'll definitely get N distinct journeys outputted but there's no guarantee we'll get cyclical routes. The solver will just choose the N shortest journeys with no regard for the "enter and leave every town once" rule. It might use x(1,2), x(1,3) and x(1,4) because the corresponding dist(i,j) variables where smaller than other distances even though it should be illegal to use town 1 more than twice. We can phrase this rule as a constraint:

    Minimize: ...
    
    Where: ...
    x(1,2) + x(1,3) + x(1,4) + ... + x(1,N) = 2
    x(1,2) + x(2,3) + x(2,4) + ... + x(2,N) = 2
    x(1,3) + x(2,3) + x(3,4) + ... + x(3,N) = 2
    ...
    x(1,N) + x(2,N) + x(3,N) + ... + x(N-1,N) = 2
    
    Binary Vars: ...

Since each town has to be entered and left once this forces cyclic routes to form, there's no way to get invalid routes. The problem now is that several complete *subtours* can form. The solution might not be one total tour containing all towns but several distinct tours involving a subset of the towns. This is because having several subtours will almost certainly give a total shorter distance travelled, it would be very, very rare that 1 tour would be the first solution found. There is no easy way to word the "1 tour only" rule as a constraint upfront.

Up until now all the user program was doing was wording the problem for the linear equation solver to solve. You could write the input file by hand even (for small problems). Now programming comes back into it as we'll have to dynamically add constraints to get the final answer. What these constraints will do is tell the solver that certain subtours are illegal and to not allow it to form specific subtours when generating a solution. We keep generating solutions and eliminating subtours until a solution is formed that contains only 1 tour. This would be our final answer.

After the solver does a pass we must detect and collect the subtours. I won't detail how to that yet but the writeup may be expanded later to include an explanation. Then we explicitly eliminate a certain amount of subtours (in this project I eliminate the smallest half). There's no point in eliminating all subtours found, if you had 2 subtours and you eliminate the first then it implicitly eliminates the second anyway. The general form of the elimination constraint is:

    sum (for (i,j) in tour; x(i,j)) <= length(tour) - 1

Let's say we detect a subtour with 3 points: x(3,7), x(7,18), x(3,18) (which is the route 3 -> 7 -> 18 -> 3). The constraint we would set is:

   x(3,7) + x(7,18) + x(3,18) <= 2

This stops all 3 journeys being active at the same time and thus eliminates the subtour. Rinse and repeat.

That's all for now. Later I'll add notes about the implementation and how to compile the project.
