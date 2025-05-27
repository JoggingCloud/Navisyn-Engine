Personel engine made in C++ that supports DirectX11 API.
--------------------------------------------------------

It features the following major subsystems:
-------------------------------------------
[AI Systems](Engine/Code/Engine/AI)

- Obstacle Avoidance algorithms
- Markov System

[Pathfinding](Engine/Code/Engine/AI/Pathfinding)

- Grid based pathing
- Nav mesh pathing

[Job System](Engine/Code/Engine/Core) - For Multi-Threading

[Renderer](Engine/Code/Engine/Renderer) - For Drawing and Displaying everything on screen
 
 
 > 2D and 3D Debug visualization system
 > Bitmap fonts
 > OBJ Loader


[Math Libraries](Engine/Code/Engine/Math) - Includes 2D, 3D & 4D vectors (vec2, vec3, vec4), Matrix4x4 and more
 > Primitives - Includes 2D & 3D Axis-Aligned bounding box (AABB2, AABB3), 2D & 3D Oriented bounding box(OBB2, OBB3) and more
 > Splines - Includes Cubic Bezier curve, Cubic Hermite curve and Catmull Rom curve
 > *Raycast system - Includes raycasts vs Discs, LineSegment2D, AABB2D, OBB2D, AABB3D, OBB3D, Plane3D, Sphere3D and ZCylinder3D

[Event System](Engine/Code/Engine/Core) - Pub-sub system

[Dev Console](Engine/Code/Engine/Core) - Triggers events when commands are enter in the given syntax

[Input System](Engine/Code/Engine/Input) - Supports Keyboard, mouse and upto 4 Xbox Controllers

[Network System](Engine/Code/Engine/Net) - Supports TCP messaging to a single connection

[Audio system using FMOD](Engine/Code/Engine/Audio)

Third party libraries:
--------------------------
[FMOD](Engine/Code/ThirdParty/fmod) - For audio features

[tinyXML2](Engine/Code/ThirdParty/TinyXML2) - For parsing XML files

[stb_image](Engine/Code/ThirdParty/stb) - For loading image files

[Dear ImGui](Engine/Code/ThirdParty/ImGui) - For debug UI purposes and graph plotting
