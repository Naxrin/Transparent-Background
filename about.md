# Wallpaper Wizard

Long long ago this mod just removes the blue blending that RobTop applies to backgrounds, now it's also an integrated menu background manager where we can more easily deal with your own ~~anime porns~~ wallpaper images.

## How to Use

Press the keybind (**F7** by default) anywhere may contains a wallpaper.  
It's always muted in-level and in-editor.

All paths shown in the picker are relative to the mod's config folder (use the folder button to open it). Images are loaded from any subfolder (except non-ascii-only dir name). Adding a texture quality suffix such as `-uhd` or `-hd` is **not necessary**.

## Priority

This mod allows to customize wallpers for timely or gauntlet level entrance page, and is even able to set a dedicated wallpaper to a single level, or list or gauntlet map. The priority order of them is like:

`Level / Map / List > Daily / Weekly / Event / Gauntlet > Layer`

The scope with the highest priority that has set the wallpaper wins. Applying on a lower-priority scope keeps the higher-priority image on screen and only records your choice.

## Extra Layers

Some interfaces have no background of their own and are controlled by the *Extra Layers* settings (main menu, loading page, level select, gauntlet maps, the Geode mod browser, and several mod menus). When one of these is off and you apply an image from the picker, the choice is saved first and you are asked whether to enable the background display for that interface. Disabling it again from the picker fades the added background out and restores the interface's original one.

## Misc Utils

- **Dark Timely/Event/Gauntlets**: tints daily/weekly/event and gauntlet backgrounds, and The Safe's Daily/Weekly tabs, gray so they stand out from normal levels.
- **Replace Frames / Frames Opacity**: replaces the blue frames in some screens with black NineSlices. Turn *Replace Frames* off if the game crashes in the level search or editor entrance.

## Notice
- As mentioned above, non-ascii folders and files will be ignored
- No realitic images (rename a text file to xxx.png, etc), they may be ignored by the mod as expected, but I donot promise none of them would lead to game crash
- No giant images (file size), my mod is not a Cob Cannon and cannot handle your Gargantuars.
- Don't place a toxic count of images in config folder, such issue saying their SrollLayer of wallpapers getting longer than their life will just be ignored (may write a page flip later).

## Claim
- Some features and bugfixes were implemented with the help of Deepseek, I have reviewed and fully understood all the code and their meaning, and have manually revised it according to actual requirements.

## Credits

- WEGFan (early Geode help)
- Dev helpers in the Geode Discord
- Cvolton for the optional Editor Level ID API

## <cy>Contact / Report Bugs</c>

- Github issue
- Discord: [Naxrin#6957](https://discordapp.com/users/414986613962309633) 
- Twitter / X: [@Naxrin19](https://x.com/Naxrin19) (not really check it often)
- In-Game ID: Naxrin