# Transparent Background
This mod removes the stupid blue blending of background image in several menu pages and let us enjoy our anime ~~porns~~ backgrounds more
Check settings menu for more perhaps favoured features  
  
**Turn OFF the Replace Frames option if your game crashes at level search menu (LevelSearchLayer) or edit level entrance menu (EditLevelLayer), and report it to me.**

### Pick background separately
- Firstly, all following paths start from this mod's config folder. $CONFIG refers to this config folder;
- you can (had better) add game texture quality note to these files based on their names, such as *$CONFIG/layers/MenuLayer-uhd.png*;
- Never pick a realistic image file *(e.g. rename a text file to bg.png and try to let my mod load it in game)*. Otherwise, I believe your game will definitely crash and I take no responsibility for your being rediculous.
#### Layers (by default)
If you do not know about layer class names, install DevTools or ask me directly.
- For raw game menu by RobTop: <cy>$CONFIG/layers/{CLASSNAME}.png</c>
- For mod added menu: <cy>$CONFIG/layers/{modID}-{CLASSNAME}.png</c>
#### Timely/Gauntlet Level Entrance
Images here has higher priority than $CONFIG/layers/LevelInfoLayer.png mentioned above
- For daily levels: <cy>$CONFIG/entry/daily.png</c>
- For weekly demons: <cy>$CONFIG/entry/weekly.png</c>
- For event levels: <cy>$CONFIG/entry/event.png</c>
- For gauntlet levels: <cy>$CONFIG/entry/gauntlet.png</c>
#### Gauntlet Maps
Here GauntletName is Fire/Water/Time/... (without the common "Gauntlet" suffix)
Here has higher priority than $CONFIG/entry/GauntletLayer.png mentioned above
- For any gauntlet entry: <cy>$CONFIG/gauntlet/{GauntletName}.png</c>
#### Online Levels/Lists Specialize
Here saved/online level backgrounds work for general online levels, timely and gauntlet, but will never work for your editor copies. For editor levels, use their names instead, rather than IDs. Also images here have the MAX priority, higher than Timely/Gauntlet Level Entrance and layers folders mentioned above.
- For saved/online levels: <cy>$CONFIG/online_levels/{levelID}.png</c>
- For saved/online lists: <cy>$CONFIG/online_lists/{listID}.png</c>
- For editor levels: <cy>$CONFIG/editor_levels/{levelName}.png</c>
- For editor lists: <cy>$CONFIG/editor_lists/{listName}.png</c>

### Special Thanks:
~~So this is my first mod and the initial motivation I learn geode modding~~
- WEGFan (Github help in early time)
- Also dev-helpers in Geode Discord Server