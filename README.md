# 2D-Vulkan-Environment
<h1>WORK IN PROGRESS</h1>
<h5>A 2D renderer for vulkan, to load textures and display them on freely movable quads. </h5>

At the momement quads can be drawn per frame in arbitrary poisitions. 
I use uniform buffers to send a view and projection matrix per frame, and I use push constants for per-draw model matricies and solid colour data.

Now I am working on a texture loader to load all the textures before drawing, then I plan to fill out a uniform buffer with an array of textures which can be picked from via
an index sent through a push constant.
