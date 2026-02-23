# Editor Log
The development of the editor is part of an assignment hence I'm required to make a log :(

---

Day \*:
- Bug fixes and cleaning

---

Day 1 (2/2):
- A bit sick but started work towards the end of the day.
- Found a couple of other bugs which surprisingly didn't cause any issues on Nvidia but would've been really annoying to find out of nowhere.
- Started with the cmake requirements for adding multiple runtime projects so the editor can be its own runtime.
- DLL copying TODO.
- Ran into issues with the Intel Arc B580s OpenGL drivers behaving slightly different compared to the Nvidia ones.
- Found an issue that I thought was the Intel Arc treating the entire program as the only block when calling glGetActiveUniformBlockiv. But it ended up being nvidia that ignored an argument where I provided the wrong information. Never letting me catch the bug previously.

---

Day 2 (3/2):
- Changed the target dir for the binaries to [Project Root]/bin to make dlls easier to manage.
- For the future I will attempt to use install for managin this, but for now this works well enough.
- Added ImGui
- Had to fix linking and the setup for ImGui but it ended up working.
- Needed to dig into how to set up ImGui for multi-viewport and docking, but got it eventually working.
- Got a viewport with basic functionality.

---

Day 3 (4/2):
- Spent most of the day debugging and fixing the viewport.
- Created an interface named iSurface to allow a me to have "virtual" windows, making the viewport easier to control whilst running on the same render pipeline as the game would.
- I also made the Editor folder not build if the Editor runtime wasn't included in the cmake config. As it would be unecessary overhead when building the game only.
- The future plan is to have the Editor be it's own static plugin.
- Added a aTab abstract class for managin tabs in the future.
- Made a Tab for the viewport and a placeholder for the remaining.
- Made a Tab to list the active scenes and objects in said scenes.

---

Day 4 (5/2):
- Started the day fighting with CLion trying to get the class template to be how I want it to be.
- Learned some Apache Velocity and was able to achive it 1.5 hours later.
- Added a context menu helper class that allows easier creations of context menus.
- Added Load/Unload and Unregister buttons to the scene. Planning to make the scene into an actual loadable asset the next week. Due to creating it during the runtime it will destroy itself as quick as it's been unloaded for the first time. The unloading is standard behaviour and will stay as the standard behaviour.
- Added an Editor Internal Component that can be used in the future.
- Added a selection manager to keep track of selections between tabs.
- Added the logic to the ObjectListTab to actually select and unselect things.

---

Day 5 (6/2):
- Fixed the LoadFolder function in my asset manager to fully load in the project folder.
- With this I also removed the manual loadFile calls.
- Made sure that all paths area treated the same.
- Added a quickly made AssetViewTab to show the assets in the project.
- Added support for branching to the context menu as well as support for disabling/enabling the items.

---

Day 6 (9/2):
- Started fixing the serialization system I had begun before this assignment.
- Realized I was unable to get my own runtime type info from reflected class instances. So I fixed it.
- Made a template for how the serialized object and worked on getting the json exporter for it correct.

---

Day 7 (10/2):
- Continued working on designing and implementing the scene -> json converting.
- Got a basic scene saved and tweaked the outputs to better fix my needs.
- Added a simple way to with runtime types be able to construct a class using a serialization object.
- Started working on the logic for loading a serialized object as well as the logic for loading the scene.
- Got most of the stored scene to be turned back into a SerializationObject.
- Will not push anything today as it's still in progress and there's plenty of testing code included.
- Trust me bro there's changes

---

Day 8 (11/2):
- Fixed a way to serialize asset references. But it needs more fallbacks to make sure that it always gets the asset.
- Made all the objects and components serializable.
- SCENE IS SAVEABLE AND LOADABLE YIPPIE
- Started testing serialization on a scene with a bit more than 40k objects and trying to optimize around it.
- Did not push for the end of the day as it got late and there seems to be some memory corruptions.

---

Day 9 (12/2):
- Spent the early part of the day trying to track down the memory corruption. Ended up finding out it was from the scene getting recreated.
- When trying to dig deeper into the reason for the corruption I found out that I was leaking memory so started working on fixing that.
- Did decrease the number of memory leaks and only have a few remaining.
- Will not push todays progress either as the memory leaks are still lurking.

---

Day 10 (13/2):
- Spent the first part of the day getting the scene to work as a valid asset. As well as for storing the metadata.
- Spent the next part of the day trying to get scenes to safely open and close which I've made progress on.

---

Day 11 (16/2):
- Continued work on getting Jolt integrated as it'll be required for raycasts in the editor.
- Whilst working on the Jolt integration I realized a way to optimize the JSON so I moved to that for the rest of the day.
- Due to switching from simdjson::dom to simdjson::ondemand which both work in completely different ways. The rewrite will take longer than initially expected.

---

Day 12 (17/2):
- Continued with the rewrite which is continuing to be a headache but it is progressing forwards.

---

Day 13 (18/2):
- Was exhausted most of the day but changed how you read and write from the `SerializedObject` which should end up feeling better.

---

Day 14 (19/2):
- Completed the rewrite in theory but did not test it. (Ofc bugs were lurking that I didn't know about)

---

Day 15 (20/2):
- Completed the rewrite (excluding missed cases) and was able to save and load from a scene once more.
- Forgot to push so the content will be combined with the pushes on day 16.

---

Day 16 (23/2):
- Started testing a bunch of cases to make sure that the saving and loading works correctly.
- When testing I found out that the `SerializedObject` didn't find the assets correctly. So I fixed that.
- Also realized that the asset metadata isn't able to handle files with multiple assets (such as glb) where it would only save the metadata for a single one of the assets.
- Added a quick fix that will have to be replaced later to protect against this.
- Reworked the `MakeShared` as I didn't like the macro solution and ended up having the function return a `MakeSharedContext` which the shared pointer could construct itself using. This allows the access of the users `std::source_location`.
- Wrote the log from day 13-15 as I missed it earlier.

---