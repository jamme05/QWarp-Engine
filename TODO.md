# TODO

## Editor:
Editor: (low priority, Editor, requires: Windows Core)

Blender type object movement (Locking to axis movement)

Display actual size units (t.ex meters)

LOD visualization tools

Landscape

Heightmaps for terrain

Easy to reimport asset

Asset by reference, option, allowing external sources

Maya+Blender controls

Mouse speed from scroll wheel

Easily customizable grid/rotate snapping

Easy snap with offset or with grid toggle

Customizable views with front, top, side and perspective

Default view modes, debug and such

Ghosting view mode

Unreal home, move object to floor

Tenporary movable pivots

Multi select with mouse

Code graph ifs: Will be created as a If with a single exit and a Boolean input, will then have the option to either change/add a condition as well as changing/adding a else/else if which get their own exits.

Code graph namings:

Entry: Going into a node

Exit: Going out from a node

## Backend:
Backend:

Materials, vector of pointers to meshes, meshes. MaterialManager that increases a tick each frame and when a model registers for rendering their own tick is set to the current tick. If the tick isn't the same, set current to null, swap it with the last element, as well as swapping the corresponding instance matrix element and then decrease the number of elements. It'll auto instance. (Infrastructure, high priority)

Add a lock to the dynamic buffer, when locking it sets a member called m_locked_ptr to the current ptr. If data would be manipulated during this it'll create a second instance of data, and once the buffer is unlocked it'll free the locked pointer IF a second instance exists. (Safety, medium priority)

Rename the dynamic buffer to cBuffer and add a new dynamic buffer with easy runtime type size manipulation. (Infrastructure/convenience/safety, high priority)

Event manager and global event listener class. Will replace the current component event system and each class will subscribe to their own required events (check components). This will allow the first intended optimization. Use a similar system as the material. (Infrastructure, high priority) Event manager added, uses weak ptrs for validation checks.

Make it easier to fetch assets? Due to gltf names are practically hidden. Loading a file returns an asset bundle which acts as a self contained asset manager search? Aka you can only seatch between each asset created. (Convenience, Low priority) Added asset list.

Create an asset_ref class, mimicking most of the weak ptrs logic with the exception of the data keeping track of each reference, hence being able to push a modify or delete the asset for all. This will replace the usage of shared/weak ptr in terms of assets. (Convenience, low priority)

Smart ptr swap, or is std::swap enough? (Convenience, low priority)

Move stuff to more relevant places (infrastructure/Diverse, medium priority, WIP)

Make helpers for importing assets?
These helpers would exist in the Engine which would allow the assets to be moved to the Framework (infrastructure, high priority, WIP)

Add kEditorRender to premade events

Add
template< class Ty, class Ty2, Ty2 Value >
constexpr qw::hash< Ty > CalcHash( Value );

Something like unreals user struct
When created, creates a read-only header for the struct, or just a class type, such as unreal.
When removing a member referenced within c++ it'll be marked as deprecated with 0 as it's default value, until project is rebuilt.
(Infrastructure/editor, medium priority, mixed with argument reflections?)

Create some kind of argument reflection system, template support? Standard type support? Use own classes? Errors in case the type isn't supported? (infrastructure/Editor, High priority)

Add ImGui as a dependency (Editor/debugging, Medium priority)

Have ptr base in the shared ptr return a ptr reference, or allow direct access to the variables (as std::ptrs) due to copy costing performance (performance, experimental)

Figure out how to handle ptr and references, new types or type modifiers? Will the nodes become c++ files or compiled by themselves? (Pointing towards raw c++ files)

Make a grid shader (editor/convince, low priority)

Check out modules, could be convenient when making more stuff (infrastructure, experimental)

Make graphics and os cores different selections, allow more reusability. (Infrastructure, high priority)

Plugins containing a lib and header? Should probably be made once the engine is functional within itself (Infrastructure, medium priority)

Something similar to std::function class, but holds the logic for validating args as well as the class itself weak ptr? qw::callback

## Ensure Engine Structure:
Base: Barebones requirements to get the Engine running as well as includes for both the Engine and Framework. Everything will be headers.

Core: The part that does all OS and graphics api dependent work. Will be formed from the Base

Engine: Will hold most assets, ECS as well as the pipelines.

Framework: Empty environment for the user to do stuff. The point at which the user customizes the engine.

Game: All the logic for the game as well as the assets. (infrastructure, High priority, WIP)

## NOTE:
These are mainly copy and pasted for now and will be replaced with something better in the future.
