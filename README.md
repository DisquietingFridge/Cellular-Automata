# Cellular-Automata

The initial purpose of this project was to implement the popular automata system ["Conway's Game of Life"](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) in Unreal Engine 4. I have since expanded this goal to essentially implement as many automata systems as I find interesting. Currently, rule-agnostic ["Life-like"](https://en.wikipedia.org/wiki/Life-like_cellular_automaton) and ["Langton's Ant"](https://en.wikipedia.org/wiki/Langton%27s_ant) systems are supported, and I plan to implement ["Turmites"](https://en.wikipedia.org/wiki/Langton%27s_ant#Extension_to_multiple_states) in the future.

Visually, the automata use a color-fading effect, resulting in beautiful, cascading clouds of activity. From within the engine level editor, the user can define the dimensions of the grid, the cell shapes (square or hexagonal), the ["wraparound" behavior of the grid](https://conwaylife.com/wiki/Bounded_grids), and various visual properties.

<img src = "https://user-images.githubusercontent.com/30654622/219524565-d508d7a4-d76b-40ec-8137-837ebcbeb51b.png" width="20%" height="20%">

By itself, the implementation of a cellular-automata system is fairly trivial. I recall making a Langton's Ant system as part of an undergrad computer-science course, for instance. Nevertheless, this project has posed some interesting challenges:

* __Implementing multiple kinds__ of cellular-automata implies code-reuse and extendability, and so the codebase needs to be developed with this in mind. This project has served as a kind of test-bed for me to apply concepts of software architecture, and I believe it will challenge me even more as I implement more kinds of automata.

* __I've strived to make the automata as performant as possible__ for a vast number of cells and very high step frequencies, by identifying bottlenecks and profiling performance. Through use of parallelism and outputting to a GPU-based particle system, the "Life-like" automata is capable of displaying close to a million cells at 100 steps-per-second before the framerate begins to dip below 120fps. This is more cells than is nice to look at, but I'm always on the lookout for ways to make it faster.

* __Using an existing development platform (UE4) has imposed constraints__ that I've had to work within. It is one (impressive!) thing to develop a system from scratch to achieve a desired goal, but often a software engineer needs to work within an already-established code-base. For example, many early iterations of this project involved finding ways to display the automata cell grid in a way that was performant and simple, within the context this particular engine. In the future, I may try to port over the system to other kinds of engines to see what I'd need to change.

A nice addition at this point in the project might be a UI for the user to control the automata with- currently an "Automata Factory" is placed within the level-editor, and exposed parameters are set there. But, this raises a higher-level design question of what the program is "for": a simple screensaver-like application that takes very little input? Or more of an educational program where a user defines various rules to see what happens? It could even be used as a loading-screen for some larger project.
