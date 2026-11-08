#include "head.hpp"

using namespace geode::prelude;

// One-shot hand-off from a clone action to the editor layer it opens, used to
// copy the source level/list background onto the clone.
struct PendingCloneBackground {
    std::string sourceKey;
};

static PendingCloneBackground g_pendingLevelClone;
static PendingCloneBackground g_pendingListClone;

NineSlice* replaceNode(CCScale9Sprite* target, const char* sprite, GLubyte alpha) {
	auto pos = target->getPosition();
	auto size = target->getContentSize();
	auto z = target->getZOrder();
	auto id = target->getID();
	auto tag = target->getTag();

	target->removeFromParentAndCleanup(true);
	auto slice = NineSlice::create(sprite);

	slice->setPosition(pos);
	slice->setContentSize(size);
	slice->setZOrder(z);
	slice->setID(id);
	slice->setTag(tag);
	slice->setOpacity(alpha);
	slice->setColor(ccc3(0, 0, 0));

	return slice;

}

#include <Geode/modify/MenuLayer.hpp>
class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("menu-layer")) {
			auto bg = geode::createLayerBG();
			tryReplace(bg, "layers", "MenuLayer");
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-19);
			bg->setID("background"_spr);
			this->addChild(bg);
			// Keep the animated menu background underneath instead of hiding
			// it: hiding could leave the menu buttons unresponsive.
			coverMenuGameLayer(this);
		}
		return true;
    }
};

#include <Geode/modify/MenuGameLayer.hpp>
class $modify(MenuGameLayer) {
    void update(float dt) {
        MenuGameLayer::update(dt);

        // The game can add the animated menu background *after*
        // MenuLayer::init, when the hook above found m_menuGameLayer null.
        // Keep it covered while our background is on screen: below the
        // wallpaper (z = -19) and without touch input, otherwise it sits
        // above/between the menu's children and can swallow clicks. The check
        // also re-applies the cover if the game re-adds or re-shows the layer.
        auto menu = typeinfo_cast<MenuLayer*>(this->getParent());
        if (!menu) {
            menu = MenuLayer::get();
            if (menu && menu->m_menuGameLayer != this)
                menu = nullptr;
        }
        if (!menu || !menu->getChildByID("background"_spr))
            return;

        bool covered = this->getUserObject("menu-game-z"_spr)
            && this->getZOrder() < -19 && !this->isTouchEnabled();
        if (!covered)
            coverMenuGameLayer(menu);
    }
};

#include <Geode/modify/LoadingLayer.hpp>
class $modify(LoadingLayer) {
    bool init(bool p) {
        if (!LoadingLayer::init(p)) return false;
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("loading-layer")) {
			if (auto bg0 = this->getChildByID("bg-texture"))
				bg0->setVisible(false);
			auto bg = geode::createLayerBG();
			tryReplace(bg, "layers", "LoadingLayer");
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-19);
			bg->setID("background"_spr);
			this->addChild(bg);
		}
		return true;
    }
};

#include <Geode/modify/LevelSelectLayer.hpp>
class $modify(LevelSelectLayer) {
    bool init(int p) {
        if (!LevelSelectLayer::init(p)) return false;
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("level-select-layer")) {
			if (auto bg0 = this->getChildByID("background"))
				bg0->setVisible(false);
			auto bg = geode::createLayerBG();
			tryReplace(bg, "layers", "LevelSelectLayer");
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-19);
			bg->setID("background"_spr);
			this->addChild(bg);
		}
		return true;
    }
};

#include <Geode/modify/CreatorLayer.hpp>
class $modify(CreatorLayer) {
    bool init() {
        if (!CreatorLayer::init()) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				tryReplace(bg, "layers", "CreatorLayer");
				bg->setColor(ccc3(255, 255, 255));
			}
		return true;
    }
};

#include <Geode/modify/GJGarageLayer.hpp>
class $modify(GJGarageLayer) {
    bool init() {
        if (!GJGarageLayer::init()) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				tryReplace(bg, "layers", "GJGarageLayer");
				bg->setColor(ccc3(255, 255, 255));
			}
		return true;
    }
};

#include <Geode/modify/EditLevelLayer.hpp>
class $modify(EditLevelLayer) {
    bool init(GJGameLevel* level) {
		if (!EditLevelLayer::init(level)) return false;

		// Copy the level background when this editor was opened by a clone.
		// The clone may have been given a different name, so do not match
		// names; the source key was recorded by GJGameLevel::copyLevelInfo.
		if (!g_pendingLevelClone.sourceKey.empty()) {
			auto pending = g_pendingLevelClone;
			g_pendingLevelClone = {};
			if (Mod::get()->getSettingValue<bool>("copy-background-on-clone") && level) {
				auto sourceSaved = getSavedBackground(pending.sourceKey);
				auto targetKey = levelBackgroundKey(level);
				if (!sourceSaved.empty() && !targetKey.empty()) {
					setSavedBackground(targetKey, sourceSaved);
					(void)Mod::get()->saveData();
				}
			}
		}

		// frame
		GLubyte alpha = Mod::get()->getSavedValue<bool>("replace-notify", false) ? Mod::get()->getSettingValue<int64_t>("frame-opacity") : 144;

		if (Mod::get()->getSettingValue<bool>("replace-frames") || !Mod::get()->getSavedValue<bool>("replace-notify", false)) {
			if (auto n = this->getChildByID("level-name-background"))
				this->addChild(replaceNode(static_cast<CCScale9Sprite*>(n), "square02b_001.png", alpha));

			if (auto n = this->getChildByID("description-background"))
				this->addChild(replaceNode(static_cast<CCScale9Sprite*>(n), "square02b_001.png", alpha));			
		} else {
			if (auto n = this->getChildByID("level-name-background")) {
				static_cast<CCScale9Sprite*>(n)->setOpacity(alpha);
				static_cast<CCScale9Sprite*>(n)->setColor(ccc3(0, 0, 0));
			}

			if (auto n = this->getChildByID("description-background")) {
				static_cast<CCScale9Sprite*>(n)->setOpacity(alpha);
				static_cast<CCScale9Sprite*>(n)->setColor(ccc3(0, 0, 0));
			}			
		}

		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				// Editor levels are keyed by their editor ID ("le-..."); the
				// level name is only used for the legacy editor_levels files.
				auto levelKey = levelBackgroundKey(level);
				bool applied = !levelKey.empty()
					&& tryReplace(bg, "editor_levels", level->m_levelName, levelKey);
				if (!applied)
					tryReplace(bg, "layers", "EditLevelLayer", "EditLevelLayer");
				bg->setColor(ccc3(255, 255, 255));
			}
			
		return true;
    }

	void onBack(CCObject* sender) {
		if (!Mod::get()->setSavedValue("replace-notify", true))
			geode::createQuickPopup(
				"Where comes these black frames?",
				"Your Transparent Background mod removes the option of setting those frames invisible but instead, "
				"their colors are now black, and it's now optional to replace each of them with a NineSlice "
				"to avoid them from looking haunted, but it's slightly crash risky if you load mods that confict with this in the future. "
				"Their opacity values are also optional, here is 144/255 opacity as showcase,\n"
				"<cg>Click Yes please to keep your frames like such in case you are satisfied with such effect;</c>\n"
				"<cr>Or click No for now in case you hate game crash all the time (will directly hide frames).</c>",
				"No for now", "Yes please", 450.f,
				[this, sender] (FLAlertLayer*, bool yes) {
					Mod::get()->setSettingValue("replace-frames", yes);
					Mod::get()->setSettingValue("frame-opacity", 144 * yes);

					EditLevelLayer::onBack(sender);
				}
			);
		else
			EditLevelLayer::onBack(sender);
	}

};

#include <Geode/modify/LevelBrowserLayer.hpp>

// The Safe (LevelBrowserLayer in a safe search mode) re-colors its background
// sprite whenever a tab is switched. Keep that tint under our control: white
// by default, or the dark gray for the top two (Daily/Weekly) tabs when the
// dark option is enabled.
static bool isSafeSearchType(SearchType type) {
    return type == SearchType::DailySafe
        || type == SearchType::WeeklySafe
        || type == SearchType::EventSafe;
}

static cocos2d::ccColor3B levelBrowserTint(GJSearchObject* search) {
    auto type = search ? search->m_searchType : SearchType::Search;

    bool darkTab = type == SearchType::DailySafe
        || type == SearchType::WeeklySafe;
    if (darkTab && Mod::get()->getSettingValue<bool>("dark"))
        return ccc3(100, 100, 100);

    return ccc3(255, 255, 255);
}

// Applies our tint unless the sprite is already showing exactly that color.
// When the game starts a tint animation on the background, stopping it here
// lets our color stay in place.
static void applySafeTintTo(cocos2d::CCSprite* background, cocos2d::ccColor3B tint) {
    if (!background)
        return;

    auto current = background->getColor();
    if (current.r == tint.r && current.g == tint.g && current.b == tint.b)
        return;

    background->stopAllActions();
    background->setColor(tint);
}

// Whether this level browser is The Safe. The search type can lag behind the
// selected tab, so the safe-mode tab bar is also accepted as a signal.
static bool isSafeBrowser(LevelBrowserLayer* layer) {
    if (!layer)
        return false;
    if (layer->m_safeModeNode)
        return true;
    auto search = layer->m_searchObject;
    return search && isSafeSearchType(search->m_searchType);
}

// Re-assert our tint on The Safe's background. The game can re-color it (and
// animate that color) on tab switches and after loading the page.
static void forceSafeTint(LevelBrowserLayer* layer) {
    if (!layer || !Mod::get()->getSettingValue<bool>("BG"))
        return;
    if (!isSafeBrowser(layer))
        return;

    auto tint = levelBrowserTint(layer->m_searchObject);
    applySafeTintTo(layer->m_background, tint);

    if (auto node = layer->getChildByID("background"))
        if (auto background = typeinfo_cast<cocos2d::CCSprite*>(node);
            background && background != layer->m_background) {
            applySafeTintTo(background, tint);
        }
}

class $modify(SafeLevelBrowserLayer, LevelBrowserLayer) {
    // Per-frame enforcement, scheduled at the end of init. The game re-tints
    // the background on tab switches and after page loads, sometimes with a
    // color action that we cannot catch inside a single hook.
    void keepSafeTint(float) {
        forceSafeTint(this);
    }

    bool init(GJSearchObject* search) {
		if (!LevelBrowserLayer::init(search)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				tryReplace(bg, "layers", "LevelBrowwserLayer");
				if (search && isSafeSearchType(search->m_searchType))
					bg->stopAllActions();
				bg->setColor(levelBrowserTint(search));
			}
		forceSafeTint(this);
		// Tab switches can re-color the background at any later point, so keep
		// re-asserting the tint every frame while on The Safe.
		this->schedule(schedule_selector(SafeLevelBrowserLayer::keepSafeTint));
		return true;
    }

    // The known places where the game may re-color the background; each hook
    // calls the original first so the game's own tint is then overwritten.
    void updateBackgroundColor() {
        LevelBrowserLayer::updateBackgroundColor();
        forceSafeTint(this);
    }

    void onSafeMode(cocos2d::CCObject* sender) {
        LevelBrowserLayer::onSafeMode(sender);
        forceSafeTint(this);
    }

    void loadLevelsFinished(cocos2d::CCArray* levels, char const* key, int type) {
        LevelBrowserLayer::loadLevelsFinished(levels, key, type);
        forceSafeTint(this);
    }
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* level, bool p1) {
		if (!LevelInfoLayer::init(level, p1)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				bool isGauntlet = level->m_gauntletLevel || level->m_gauntletLevel2;
				bool isTimely = level->m_dailyID.value() > 0;

				// Dedicated level background: saved under "le-<id>", looked up
				// through the 4-argument tryReplace override. The file name
				// argument stays the raw ID so legacy online_levels/<id>.png
				// files keep working.
				auto levelKey = levelBackgroundKey(level);
				auto legacyName = numToString(level->m_levelID.value());
				bool levelApplied = !levelKey.empty()
					&& tryReplace(bg, "online_levels", legacyName, levelKey);
				if (!levelApplied) {
					bool categoryApplied = false;
					if (isGauntlet)
						categoryApplied = tryReplace(bg, "entry", "gauntlet", "gauntlet");
					else if (isTimely) {
						auto categoryKey = level->m_dailyID.value() > 2000000 ? "event"
							: (level->m_dailyID.value() > 1000000 ? "weekly" : "daily");
						categoryApplied = tryReplace(
							bg, "entry", categoryKey, categoryKey
						);
					}

					// Layer is the lowest-priority fallback, so it must also be
					// tried for ordinary levels and when the category is unset.
					if (!categoryApplied)
						tryReplace(bg, "layers", "LevelInfoLayer", "LevelInfoLayer");
				}
				bg->setColor(Mod::get()->getSettingValue<bool>("dark") && (isGauntlet || isTimely) ? ccc3(100, 100, 100): ccc3(255, 255, 255));
			}

		return true;
    }

    void onPlay(cocos2d::CCObject* sender) {
        LevelInfoLayer::onPlay(sender);
		if (!Mod::get()->getSettingValue<bool>("Circles")) return;

		if (m_playSprite->getChildrenCount() >= 3) {
			auto sp0 = static_cast<CCSprite *>(m_playSprite->getChildren()->objectAtIndex(0));
			auto sp1 = static_cast<CCSprite *>(m_playSprite->getChildren()->objectAtIndex(1));
			auto sp2 = static_cast<CCSprite *>(m_playSprite->getChildren()->objectAtIndex(2));
			
			ccColor3B black = Mod::get()->getSettingValue<ccColor3B>("plate");
			ccColor3B white = Mod::get()->getSettingValue<ccColor3B>("circle");

			sp0->setColor(black);
			sp1->setColor(black);
			sp2->setColor(black);
			m_progressTimer->setColor(white);
		}
    }
};

#include <Geode/modify/LevelSearchLayer.hpp>
class $modify(LevelSearchLayer) {
    bool init(int p) {
		if (!LevelSearchLayer::init(p)) return false;

		// frames
		GLubyte alpha = Mod::get()->getSavedValue<bool>("replace-notify", false) ? Mod::get()->getSettingValue<int64_t>("frame-opacity") : 144;

		if (Mod::get()->getSettingValue<bool>("replace-frames") || !Mod::get()->getSavedValue<bool>("replace-notify", false)) {
			if (auto n = this->getChildByID("level-search-bg"))
				this->addChild(replaceNode(static_cast<CCScale9Sprite*>(n), "square02b_001.png", alpha));

			if (auto n = this->getChildByID("level-search-bar-bg"))
				this->addChild(replaceNode(static_cast<CCScale9Sprite*>(n), "square02b_small.png", alpha));

			if (auto n = this->getChildByID("quick-search-bg"))
				this->addChild(replaceNode(static_cast<CCScale9Sprite*>(n), "square02b_001.png", alpha));

			if (auto n = this->getChildByID("difficulty-filters-bg"))
				this->addChild(replaceNode(static_cast<CCScale9Sprite*>(n), "square02b_001.png", alpha));

			if (auto n = this->getChildByID("length-filters-bg"))
				this->addChild(replaceNode(static_cast<CCScale9Sprite*>(n), "square02b_001.png", alpha));			
		} else {
			if (auto n = this->getChildByID("level-search-bg")) {
				static_cast<CCScale9Sprite*>(n)->setOpacity(alpha);
				static_cast<CCScale9Sprite*>(n)->setColor(ccc3(0, 0, 0));
			}
			if (auto n = this->getChildByID("level-search-bar-bg")) {
				static_cast<CCScale9Sprite*>(n)->setOpacity(alpha);
				static_cast<CCScale9Sprite*>(n)->setColor(ccc3(0, 0, 0));
			}
			if (auto n = this->getChildByID("quick-search-bg")) {
				static_cast<CCScale9Sprite*>(n)->setOpacity(alpha);
				static_cast<CCScale9Sprite*>(n)->setColor(ccc3(0, 0, 0));
			}
			if (auto n = this->getChildByID("difficulty-filters-bg")) {
				static_cast<CCScale9Sprite*>(n)->setOpacity(alpha);
				static_cast<CCScale9Sprite*>(n)->setColor(ccc3(0, 0, 0));
			}
			if (auto n = this->getChildByID("length-filters-bg")) {
				static_cast<CCScale9Sprite*>(n)->setOpacity(alpha);
				static_cast<CCScale9Sprite*>(n)->setColor(ccc3(0, 0, 0));
			}
		}

		
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				tryReplace(bg, "layers", "LevelSearchLayer");
				bg->setColor(ccc3(255, 255, 255));				
			}

		return true;
    }
	void onBack(CCObject* sender) {
		if (!Mod::get()->setSavedValue("replace-notify", true))
			geode::createQuickPopup(
				"Where comes these black frames?",
				"Your Transparent Background mod removes the option of setting those frames invisible but instead, "
				"their colors are now black, and it's now optional to replace each of them with a NineSlice "
				"to avoid them from looking haunted, but it's slightly crash risky if you load mods that confict with this in the future. "
				"Their opacity values are also optional, here is 144/255 opacity as showcase,\n"
				"<cg>Click Yes please to keep your frames like such in case you are satisfied with such effect;</c>\n"
				"<cr>Or click No for now in case you hate game crash all the time (will directly hide frames).</c>",
				"No for now", "Yes please", 450.f,
				[this, sender] (FLAlertLayer*, bool yes) {
					Mod::get()->setSettingValue("replace-frames", yes);
					Mod::get()->setSettingValue("frame-opacity", 144 * yes);

					LevelSearchLayer::onBack(sender);
				}
			);
		else
			LevelSearchLayer::onBack(sender);
	}
};

#include <Geode/modify/LeaderboardsLayer.hpp>
class $modify(LeaderboardsLayer) {
    bool init(LeaderboardType type, LeaderboardStat stat) {
		if (!LeaderboardsLayer::init(type, stat)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				tryReplace(bg, "layers", "LeaderboardsLayer");
				bg->setColor(ccc3(255, 255, 255));				
			}

		return true;		
    }
};

#include <Geode/modify/LevelListLayer.hpp>
class $modify(LevelListLayer) {
    bool init(GJLevelList* list) {
		if (!LevelListLayer::init(list)) return false;

		// Copy the list background when this layer was opened by a clone.
		// The clone may have been given a different name, so do not match
		// names; the source key was recorded by cloneList().
		if (!g_pendingListClone.sourceKey.empty()) {
			auto pending = g_pendingListClone;
			g_pendingListClone = {};
			if (Mod::get()->getSettingValue<bool>("copy-background-on-clone")
				&& list && list->m_isEditable) {
				auto sourceSaved = getSavedBackground(pending.sourceKey);
				auto targetKey = listBackgroundKey(list);
				if (!sourceSaved.empty() && !targetKey.empty()) {
					setSavedBackground(targetKey, sourceSaved);
					(void)Mod::get()->saveData();
				}
			}
		}

		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				// List backgrounds use "li-<id>" keys; the legacy folder files
				// still use the list name / raw ID.
				auto legacyName = list->m_isEditable
					? std::string(list->m_listName)
					: numToString(list->m_listID);
				auto listKey = listBackgroundKey(list);
				bool applied = !listKey.empty()
					&& tryReplace(
						bg,
						list->m_isEditable ? "editor-lists" : "online-lists",
						legacyName,
						listKey
					);
				if (!applied)
					tryReplace(bg, "layers", "LevelListLayer", "LevelListLayer");
				bg->setColor(ccc3(255, 255, 255));
			}
		return true;
    }

	void cloneList() {
		g_pendingListClone = {};
		if (Mod::get()->getSettingValue<bool>("copy-background-on-clone") && m_levelList) {
			auto sourceKey = listBackgroundKey(m_levelList);
			if (!sourceKey.empty() && !getSavedBackground(sourceKey).empty())
				g_pendingListClone.sourceKey = sourceKey;
		}
		LevelListLayer::cloneList();
	}
};

#include <Geode/modify/GauntletSelectLayer.hpp>
class $modify(GauntletSelectLayer) {
    bool init(int p) {
		if (!GauntletSelectLayer::init(p)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				tryReplace(bg, "layers", "GauntletSelectLayer");
				bg->setColor(ccc3(255, 255, 255));				
			}

		return true;
    }
};

#include <Geode/modify/GauntletLayer.hpp>
class $modify(GauntletLayer) {
    bool init(GauntletType p) {
		if (!GauntletLayer::init(p)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("gauntlet-map")) return true;

		if (auto bg0 = this->getChildByID("background"))
			bg0->setVisible(false);
		auto bg = geode::createLayerBG();

		// out of index error escape
		if ((int)p - 1 >= gauntletIndex.size())
			return true;
		// Map-specific backgrounds live under "g-<lowercase map name>"; the
		// legacy gauntlet/<MapName>.png lookup keeps the original map name.
		auto const& mapName = gauntletIndex[(int)p - 1];
		if (!tryReplace(bg, "gauntlet", mapName, gauntletBackgroundKey(mapName)))
			tryReplace(bg, "layers", "GauntletLayer", "GauntletLayer");
		bg->setColor({255, 255, 255});
		bg->setZOrder(-19);
		bg->setID("background"_spr);
		this->addChild(bg);

		return true;
    }
};

// Both clone actions (the level info page and the editor settings) create a
// new local level and call copyLevelInfo with the source level. This is the
// reliable place to detect a clone: the menu callbacks themselves are inlined
// on Windows and cannot be hooked. Record the source background key here; the
// new editor consumes it in EditLevelLayer::init.
//
// autoAssign=false: never create new editor IDs just for a lookup.
#include <Geode/modify/GJGameLevel.hpp>
class $modify(GJGameLevel) {
    void copyLevelInfo(GJGameLevel* source) {
        GJGameLevel::copyLevelInfo(source);

        g_pendingLevelClone = {};
        if (!source || !Mod::get()->getSettingValue<bool>("copy-background-on-clone"))
            return;

        auto sourceKey = levelBackgroundKey(source, false);
        if (sourceKey.empty() || getSavedBackground(sourceKey).empty())
            return;

        g_pendingLevelClone.sourceKey = sourceKey;
    }
};

#include <Geode/modify/GameLevelManager.hpp>
class $modify(GameLevelManager) {
    void deleteLevel(GJGameLevel* level) {
        // Editor levels always drop their dedicated background; saved levels
        // only when the option is on. Compute the key before deletion.
        auto key = std::string();
        if (level && (isEditorLevel(level)
            || Mod::get()->getSettingValue<bool>("clear-saved-backgrounds"))) {
            key = levelBackgroundKey(level, false);
        }

        GameLevelManager::deleteLevel(level);

        if (!key.empty()) {
            removeSavedBackground(key);
            (void)Mod::get()->saveData();
        }
    }

    void deleteLevelList(GJLevelList* list) {
        // Same rules as deleteLevel, but for lists. The key is computed before
        // deletion because the game may free or detach the list afterwards.
        auto key = std::string();
        if (list && (list->m_isEditable
            || Mod::get()->getSettingValue<bool>("clear-saved-backgrounds"))) {
            key = listBackgroundKey(list, false);
        }

        GameLevelManager::deleteLevelList(list);

        if (!key.empty()) {
            // Erase the entry entirely instead of leaving "":"" behind.
            removeSavedBackground(key);
            (void)Mod::get()->saveData();
        }
    }
};

$execute {
	listenForKeybindSettingPresses("menu", [](Keybind const& keybind, bool down, bool repeat, double dt) {
		// mute the event in play layer and level editor layer or in case the menu is already here
		if (down || already_has_menu || PlayLayer::get() || LevelEditorLayer::get())
			return;
		// The loading screen cannot be previewed in place; point the user at
		// the platformer level areas where the same setting is editable.
		if (isLoadingScreen()) {
			Notification::create(
				"Go to plat levels spire to modify loading page!",
				NotificationIcon::Info
			)->show();
			return;
		}
		// Also ignore screens that do not have an editable background at all.
		if (!canOpenBackgroundPopup())
			return;

		already_has_menu = true;
		if (auto popup = BackgroundPopup::create())
			popup->show();
		else
			already_has_menu = false;
	});
}