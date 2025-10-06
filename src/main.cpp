#include "head.hpp"

using namespace geode::prelude;

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
			// HIDE BASEGAMELAYER
			if (this->m_menuGameLayer) this->m_menuGameLayer->setVisible(false);
		}
		return true;
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
		// frame
		if (Mod::get()->getSettingValue<bool>("frame")) {
			if (auto n = this->getChildByID("level-name-background"))
				n->setVisible(false);
			if (auto n = this->getChildByID("description-background"))
				n->setVisible(false);
		}
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				if (!tryReplace(bg, "editor_levels", level->m_levelName))
				tryReplace(bg, "layers", "EditLevelLayer");
				bg->setColor(ccc3(255, 255, 255));
			}
		return true;
    }
};

#include <Geode/modify/LevelBrowserLayer.hpp>
class $modify(LevelBrowserLayer) {
    bool init(GJSearchObject* search) {
		if (!LevelBrowserLayer::init(search)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				tryReplace(bg, "layers", "LevelBrowwserLayer");
				bg->setColor(ccc3(255, 255, 255));
			}
		return true;
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
				if (!tryReplace(bg, "online_levels", level->m_levelID)) {
					if (isGauntlet) {
						if (!tryReplace(bg, "entry", "gauntlet"))
							tryReplace(bg, "layers", "LevelInfoLayer");						
					}
					else if (isTimely)
						if (!tryReplace(bg, "entry", level->m_dailyID.value() > 2000000 ? "event" : (level->m_dailyID.value() > 1000000 ? "weekly" : "daily")))
							tryReplace(bg, "layers", "LevelInfoLayer");					
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

		// to edit
		if (Mod::get()->getSettingValue<bool>("frame")) {
			if (auto n = this->getChildByID("level-search-bg"))
				n->setVisible(false);
			if (auto n = this->getChildByID("level-search-bar-bg"))
				n->setVisible(false);
			if (auto n = this->getChildByID("quick-search-bg"))
				n->setVisible(false);
			if (auto n = this->getChildByID("difficulty-filters-bg"))
				n->setVisible(false);
			if (auto n = this->getChildByID("length-filters-bg"))
				n->setVisible(false);
		}
		
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				tryReplace(bg, "layers", "LevelSearchLayer");
				bg->setColor(ccc3(255, 255, 255));				
			}

		return true;
    }
};

#include <Geode/modify/LeaderboardsLayer.hpp>
class $modify(LeaderboardsLayer) {
    bool init(LeaderboardState state) {
		if (!LeaderboardsLayer::init(state)) return false;
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
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node)) {
				if (!tryReplace(bg, list->m_isEditable ? "editor-lists" : "online-lists", list->m_isEditable ? fmt::format("{}", list->m_listName) : numToString(list->m_listID)))
					tryReplace(bg, "layers", "LevelListLayer");
				bg->setColor(ccc3(255, 255, 255));				
			}
		return true;
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
		if (!tryReplace(bg, "gauntlet", gauntletIndex[(int)p - 1]))
			tryReplace(bg, "layers", "GauntletLayer");
		bg->setColor({255, 255, 255});
		bg->setZOrder(-19);
		bg->setID("background"_spr);
		this->addChild(bg);

		return true;
    }
};
