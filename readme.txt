README

To Compile: Open directory containing makefile, and use the 'make && ./a.out' command in terminal.

INPUT INSTRUCTIONS
1: Scene 1
2: Scene 2
3: Custom Scene

Space: Jump

W: Move Forward (Negative in Z-axis)
S: Move Backward (Positive in Z-axis)
A: Move Left (Negative in X-axis)
D: Move Right (Positive in X-axis)

Up Arrow: Look Up
Down Arrow: Look Down
Right Arrow: Look Right
Left Arrow: Look Left

F: Toggle Depth-of-Field (see Notes)

NOTES
1. I attempted Depth-of-Field but it doesn't work quite as I anticipated. As such, I have commented out the code related to calculating DoF in the main function of the Fragment Shader so as to decrease computation time per frame. To see the DoF as I have attempted it, you'll need to uncomment/comment the relevant code.
1.1 I've disabled DoF in my custom scene.
2. The camera was initially being used to see if shadow and reflection rays were being calculated, so it wasn't really designed to move in the direction I was facing. As such the camera can be a bit difficult to control if it is not facing 'forward'.
3. There's some aliasing on the sphere in Scene 3. This was intentional cause I liked the watery texture that gave the blue sphere.

COLLABORATORS
Scott Saunders
Camilo Talero
Timothy Blake Mealey
Ben Roberts (not in the class, but he helped with reflections)
