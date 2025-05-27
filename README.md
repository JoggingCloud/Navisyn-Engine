Personel engine made in C++ that supports DirectX11 API.

It features the following major subsystems:

[AI Systems](Engine/Code/Engine/AI)
* Obstacle Avoidance algorithms
* Markov System

  [Pathfinding](Engine/Code/Engine/AI/Pathfinding)
    * Grid based pathing
    * Nav mesh pathing

1.1. Bitmap fonts
1.2. 2D sprite sheets
1.3. 2D sprite animation support
1.4. OBJ loader
1.5. 2D and 3D Debug visualization system
Audio system using FMOD
Math libraries - includes 2D, 3D & 4D vectors (vec2, vec3, vec4), Matrix4x4 and more
3.1. Primitives - includes 2D & 3D Axis-Aligned bounding box (AABB2, AABB3), 2D & 3D Oriented bounding box(OBB2, OBB3) and more
3.2. Splines - includes Cubic Bezier curve, Cubic Hermite curve and Catmull Rom curve
3.3. Raycast system - includes raycasts vs Discs, LineSegment2D, AABB2D, OBB2D, AABB3D, OBB3D, Plane3D, Sphere3D and ZCylinder3D
Job System - for multi threading
Input System - supports Keyboard, mouse and upto 4 Xbox Controllers
Event System - basic event system
Dev console - triggers events when commands are enter in the given syntax
2D Physics system - Discs, LineSegment2D, AABB2D, OBB2D, AABB3D, OBB3D, Plane3D, Sphere3D and ZCylinder3D Colliders
Network System - supports TCP and UDP messaging to a single connection
Clocks and Timers - for more explicit control over timing
Third party libraries: -
FMOD - for audio features
tinyXML2 - for parsing XML files
stb_image - for loading image files
Dear ImGui - for debug UI purposes and graph plotting
