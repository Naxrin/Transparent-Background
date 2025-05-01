#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/MenuLayer.hpp>
class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("menu-layer")) {
			auto bg = geode::createLayerBG();
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-19);
			this->addChild(bg);
			// DELETE BASEGAMELAYER
			for (const auto& child : CCArrayExt<CCNode*>(this->getChildren())) {
				auto layer = typeinfo_cast<MenuGameLayer*>(child);
				if (!layer)
					continue;
				layer->removeFromParentAndCleanup(true);
				break;
        	}
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
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-19);
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
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-19);
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
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
		return true;
    }
};

#include <Geode/modify/GJGarageLayer.hpp>
class $modify(GJGarageLayer) {
    bool init() {
        if (!GJGarageLayer::init()) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
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
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));

		return true;
    }
};

#include <Geode/modify/LevelBrowserLayer.hpp>
class $modify(LevelBrowserLayer) {
    bool init(GJSearchObject* search) {
		if (!LevelBrowserLayer::init(search)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return false;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
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
				bool gauntlet = Mod::get()->getSettingValue<bool>("gauntlet-dark") && (level->m_gauntletLevel || level->m_gauntletLevel2);
				bool timely = Mod::get()->getSettingValue<bool>("timely-dark") ||  level->m_dailyID.value(); // this->getChildByID("daily-label")
				bg->setColor(gauntlet || timely ? ccc3(100, 100, 100): ccc3(255, 255, 255));				
			}
		
		return true;
    }

    void onPlay(cocos2d::CCObject* sender) {
        LevelInfoLayer::onPlay(sender);
		// test will change
		if (!Mod::get()->getSettingValue<bool>("Circles")) return;

		auto playMenu = getChildByID("play-menu");
		if (!playMenu) return;
		auto btn = playMenu->getChildByID("play-button");
		if (!btn) return;
		auto sprt = static_cast<CCNode *>(btn->getChildren()->objectAtIndex(0));
		
		if (sprt->getChildrenCount()){
			auto sp0 = static_cast<CCSprite *>(sprt->getChildren()->objectAtIndex(0));
			auto sp1 = static_cast<CCSprite *>(sprt->getChildren()->objectAtIndex(1));
			auto sp2 = static_cast<CCProgressTimer *>(sprt->getChildren()->objectAtIndex(2));
			auto sp3 = static_cast<CCSprite *>(sprt->getChildren()->objectAtIndex(3));
			
			ccColor3B black = Mod::get()->getSettingValue<ccColor3B>("plate");
			ccColor3B white = Mod::get()->getSettingValue<ccColor3B>("circle");

			sp0->setColor(black);
			sp1->setColor(black);
			sp2->setColor(black);
			sp3->setColor(white);
			
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
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
		return true;
    }
};

#include <Geode/modify/LeaderboardsLayer.hpp>
class $modify(LeaderboardsLayer) {
    bool init(LeaderboardState state) {
		if (!LeaderboardsLayer::init(state)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
		return true;		
    }
};

#include <Geode/modify/LevelListLayer.hpp>
class $modify(LevelListLayer) {
    bool init(GJLevelList* list) {
		if (!LevelListLayer::init(list)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto bg = this->getChildByType<CCSprite>(0))
			bg->setColor(ccc3(255, 255, 255));
		return true;
    }
};

#include <Geode/modify/GauntletSelectLayer.hpp>
class $modify(GauntletSelectLayer) {
    bool init(int p) {
		if (!GauntletSelectLayer::init(p)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG")) return true;
		if (auto bg = this->getChildByType<CCSprite>(0))
			bg->setColor(ccc3(255, 255, 255));
		return true;
    }
};

#include <Geode/modify/GauntletLayer.hpp>
class $modify(GauntletLayer) {
    bool init(GauntletType p) {
		if (!GauntletLayer::init(p)) return false;
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("gauntlet")) return true;

		this->getChildByType<CCSprite>(0)->setVisible(false);
		auto bg = geode::createLayerBG();
		bg->setColor({255, 255, 255});
		bg->setZOrder(-19);
		this->addChild(bg);

		return true;
    }
};