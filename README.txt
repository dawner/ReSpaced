To Run My Code on a linux system:
COMPILING:
- Navigate to the COMP308_A4 directory, then execute
% make clean
and then
% make

EXECUTING
- To run the code, execute the following line
% ./Ass4


CORE:
1. Running the line given above will bring up a window, which reads the 6 object files and renders them on screen.

2. The objects are positioned, scaled and rotated according to the provided picture (Figure 1) as closely as possible. 

3. The objects were all coloured according to a predefined colour, using glColorf, which would mimic the required material as closely as possible. The two objects requiring 2D textures use a base colour of white and the texture is then applied as well. Each objects various materials were also set to match the required appearance as closely as possible. All properties declarations can be found in the method G308_Object_Details() in main.cpp. 

4. Four light sources are rendered on the scene. These include a weak point light [LIGHT0], a directional light [LIGHT1], pointing (0.5, 1, 1), a spotlight [LIGHT2] positioned in front of the camera and pointing directly at the bunny and an all round ambient light [LIGHT3].


EXTENSION:
1. The spotlight has been positioned in front of the camera so the icon representing it is clearly visible. The light position is shown by a white/grey sphere. The direction is represented by a white/grey arrow, made up of a cylinder and cone, pointing in the direction of the spotlight. Finally, the cutoff angle is shown by 6 cylinders, where the angle from the direction arrow corresponds to the spotlights cutoff angle. 

2. Interactive lighting is represented through the use of the spotlight icon and will update its display in real time when used. The following commands can be used to change position, direction and cutof angle:
- Position: Pressing the 'p' key will bring up 3 axis (X=Red, Y=Green, Z=Blue). The currently selected axis will be yellow. To toggle the selected axis, use the 'x' key. The position of the spotlight, along the selected axis, can then be changed by left click and drag anywhere on the screen. Dragging right will increase the axis position value, while dragging left will decrease it. (Keep this in mind when moving along the 'Y' axis as it is tempting to click and drag up and down instead).
- Direction: Pressing the 'd' key will bring up 3 axis (X=Red, Y=Green, Z=Blue). The currently selected axis will be yellow. To toggle the selected axis, use the 'x' key. The direction of the spotlight, around the selected axis, can then be changed by left click and drag anywhere on the screen. 
- Cutoff angle: 'Pressing the 'c' key will highlight the cutoff-angle cylinders on the spotlight to show it is selected. You can then left click and drag anywhere on the screen. Dragging right will increase the cutoff-angle, while dragging left will decrease it.

3. The Teapot and sphere reflect the environment around them to emphasis their metallic quality, which reflects the provided cubemap.jpg. I have added to this illusion by also adding a skybox around the entire scene to act as the background.

4. Pressing the 't' key at any time will rotate the entire table 360 degrees. Pressing 't' again while rotating will stop the rotation. It can be resumed again by pressing the 't' key once more. When the table has completed 360 degrees it will stop rotating until the 't' key is pressed again

5. The camera has some additional controls to allow the user to see the scene efectively. These are handled by the keyboard arrow keys:
- Left/Right arrow keys will rotate the table left or right when held
- Up/Down arrow keys will zoom in or out when held

CHALLENGE:
1. My bunny is slightly translucent to make it look more crystal!

Additional:
- hitting the enter key will toggle between wireframe and solid rendering