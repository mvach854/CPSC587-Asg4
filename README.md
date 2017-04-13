# CPSC587-Asg4
Behavioral Animation based on Particle Systems 

Starting/sample code credit to: Andrew Owens
All other edits/additions made by: Manorie Vachon

Command line
Run: make (if you want to remake it)
Run: ./ParticleSystem - to run the program (an executable has been provided)

== Controls ==
*Same camera controls as original (See other README for these)*
*As well as pause/play being enabled (space bar)*


== File format ==
File format must look like the boids1.txt file.
'A character' 'Value'
ex. N 200
    C 40

Character values inputs are as follows:
N : Number of boids

A : Avoidance radius
C : Cohesion radius
G : Gathering radius

F : Max force to clamp at
V : Max velocity to clamp at

D : Weight of avoidance function
H : Weight of cohesion function
T : Weight of gathering function

E : Size of the bounding box which the boids must stay within
    (The box will extend the length of this value from all sides of the origin)
    In other words, if you give it an edge size of 50, the box will be 100x100x100

Note1: Putting too many boids won't work, but even 1000 isn't really laggy.
Note2: In the favoid function, a couple different functions were tried, including 1/x^2.
       The current one being used is pow((1-r), 3) * (3*r + 1).

Note3: I attempted switching to instancing, but in the end couldn't get it to work properly. I've left in most 
       of the code for it, just commented out.

