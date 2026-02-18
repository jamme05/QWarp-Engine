# Editor Log
The development of the editor is part of an assignment hence I'm required to make a log :(

---

Day \*:
- Bug fixes and cleaning

---

Day 1:
- A bit sick but started work towards the end of the day.
- Found a couple of other bugs which surprisingly didn't cause any issues on Nvidia but would've been really annoying to find out of nowhere.
- Started with the cmake requirements for adding multiple runtime projects so the editor can be its own runtime.
- DLL copying TODO.
- Ran into issues with the Intel Arc B580s OpenGL drivers behaving slightly different compared to the Nvidia ones.
- Found an issue that I thought was the Intel Arc treating the entire program as the only block when calling glGetActiveUniformBlockiv. But it ended up being nvidia that ignored an argument where I provided the wrong information. Never letting me catch the bug previously.

---

Day 2:
- Changed the target dir for the binaries to [Project Root]/bin to make dlls easier to manage.
- For the future I will attempt to use install for managin this, but for now this works well enough.
- Added ImGui
- Had to fix linking and the setup for ImGui but it ended up working.
- Needed to dig into how to set up ImGui for multi-viewport and docking, but got it eventually working.
- Got a viewport with basic functionality.

---

Day 3:
- Spent most of the day debugging and fixing the viewport.
- Created an interface named iSurface to allow a me to have "virtual" windows, making the viewport easier to control whilst running on the same render pipeline as the game would.
- I also made the Editor folder not build if the Editor runtime wasn't included in the cmake config. As it would be unecessary overhead when building the game only.
- The future plan is to have the Editor be it's own static plugin.
- Added a aTab abstract class for managin tabs in the future.
- Made a Tab for the viewport and a placeholder for the remaining.
- Made a Tab to list the active scenes and objects in said scenes.

---

Day 4:
- Started the day fighting with CLion trying to get the class template to be how I want it to be.
- Learned some Apache Velocity and was able to achive it 1.5 hours later.
- Added a context menu helper class that allows easier creations of context menus.
- Added Load/Unload and Unregister buttons to the scene. Planning to make the scene into an actual loadable asset the next week. Due to creating it during the runtime it will destroy itself as quick as it's been unloaded for the first time. The unloading is standard behaviour and will stay as the standard behaviour.
- Added an Editor Internal Component that can be used in the future.
- Added a selection manager to keep track of selections between tabs.
- Added the logic to the ObjectListTab to actually select and unselect things.

---

Day 5:
- Fixed the LoadFolder function in my asset manager to fully load in the project folder.
- With this I also removed the manual loadFile calls.
- Made sure that all paths area treated the same.
- Added a quickly made AssetViewTab to show the assets in the project.
- Added support for branching to the context menu as well as support for disabling/enabling the items.

---

Day 6:
- Started fixing the serialization system I had begun before this assignment.
- Realized I was unable to get my own runtime type info from reflected class instances. So I fixed it.
- Made a template for how the serialized object and worked on getting the json exporter for it correct.

---

Day 7:
- Continued working on designing and implementing the scene -> json converting.
- Got a basic scene saved and tweaked the outputs to better fix my needs.
- Added a simple way to with runtime types be able to construct a class using a serialization object.
- Started working on the logic for loading a serialized object as well as the logic for loading the scene.
- Got most of the stored scene to be turned back into a SerializationObject.
- Will not push anything today as it's still in progress and there's plenty of testing code included.
- Trust me bro there's changes

---

Day 8:
- Fixed a way to serialize asset references. But it needs more fallbacks to make sure that it always gets the asset.
- Made all the objects and components serializable.
- SCENE IS SAVEABLE AND LOADABLE YIPPIE
- Started testing serialization on a scene with a bit more than 40k objects and trying to optimize around it.
- Did not push for the end of the day as it got late and there seems to be some memory corruptions.

---

Day 9:
- Spent the early part of the day trying to track down the memory corruption. Ended up finding out it was from the scene getting recreated.
- When trying to dig deeper into the reason for the corruption I found out that I was leaking memory so started working on fixing that.
- Did decrease the number of memory leaks and only have a few remaining.
- Will not push todays progress either as the memory leaks are still lurking.

---

Day 10:
- Spent the first part of the day getting the scene to work as a valid asset. As well as for storing the metadata.
- Spent the next part of the day trying to get scenes to safely open and close which I've made progress on.

---

Day 11:
- Continued work on getting Jolt integrated as it'll be required for raycasts in the editor.
- Whilst working on the Jolt integration I realized a way to optimize the JSON so I moved to that for the rest of the day.
- Due to switching from simdjson::dom to simdjson::ondemand which both work in completely different ways. The rewrite will take longer than initially expected.

---

Day 12:
- Continued with the rewrite which is continuing to be a headache but it is progressing forwards.