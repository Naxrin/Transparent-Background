# Transparent Background
This mod removes stupid blue blending of background image and shitty yellow list background in several menu pages and let us able to enjoy the raw anime background we pick.
In addition, you can customize loading circle colors to fix your background theme.
Also you can switch to dark theme for gauntlet/daily/weekly levels.

### Pick background separately
- Firstly, all following paths starts from mod's config folder. $CONFIG refers to this config folder;
- you can (had better) add game texture quality note to these files based on their names, such as *$CONFIG/layers/MenuLayer-uhd.png*;
- Never pick a realistic png file *(e.g. rename a text file to bg.png and pick it as your image)*. Otherwise, I believe your game will definitely crash and here I take no responsibility for your being rediculous.
#### Layers (by default)
If you do not know about layer class names, install DevTools or ask me directly (take it easy).
- For raw game menu by RobTop: <cy>$CONFIG/layers/{CLASSNAME}.png</c>
- For mod added menu: <cy>$CONFIG/layers/{modID}-{CLASSNAME}.png</c>
#### Timely/Gauntlet Level Entrance
Images here has higher priority than $CONFIG/0/LevelInfoLayer.png mentioned above
- For daily levels: <cy>$CONFIG/entry/daily.png</c>
- For weekly demons: <cy>$CONFIG/entry/weekly.png</c>
- For event levels: <cy>$CONFIG/entry/event.png</c>
- For gauntlet levels: <cy>$CONFIG/entry/gauntlet.png</c>
#### Gauntlet Maps
Here GauntletName refers to Fire/Water/Time/... (without the common "Gauntlet" part)
- For any gauntlet entry: <cy>$CONFIG/gauntlet/{GauntletName}.png</c>
#### Online Levels/Lists Specialize
Here saved/online level backgrounds work for general online levels, timely and gauntlet, but will never work for your editor copies. For editor levels, use ttheir names instead, rather than IDs. Also images here have the MAX priority, higher than Timely/Gauntlet Level Entrance and layers folders mentioned above.
- For saved/online levels: <cy>$CONFIG/online_levels/{levelID}.png</c>
- For saved/online lists: <cy>$CONFIG/online_lists/{listID}.png</c>
- For editor levels: <cy>$CONFIG/editor_levels/{levelName}.png</c>
- For editor lists: <cy>$CONFIG/editor_lists/{listName}.png</c>

So this is my first mod and the initial motivation I learn geode modding, ~~idk why Absolute aren't gonna port Transparent BG to MH8 until Feb 2024 anyway.~~
### Special Thanks:
WEGFan (Github support)
Also dev-helpers in Geode Discord Server