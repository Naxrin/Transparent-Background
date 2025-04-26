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

#include <Geode/modify/CreatorLayer.hpp>
class $modify(CreatorLayer) {
    bool init() {
        if (!CreatorLayer::init()) return false;
		if (Mod::get()->getSettingValue<bool>("BG"))
			if (auto bg = static_cast<CCSprite*>(getChildByID("background")))
				bg->setColor(ccc3(255, 255, 255));
		return true;
    }
};

#include <Geode/modify/GJGarageLayer.hpp>
class $modify(GJGarageLayer) {
    bool init() {
        if (!GJGarageLayer::init()) return false;
		if (Mod::get()->getSettingValue<bool>("BG"))
			if (auto bg = static_cast<CCSprite*>(getChildByID("background")))
				bg->setColor(ccc3(255, 255, 255));
		return true;		
    }
};

#include <Geode/modify/EditLevelLayer.hpp>
class $modify(EditLevelLayer) {
    bool init(GJGameLevel* level) {
		if (!EditLevelLayer::init(level)) return false;
		if (Mod::get()->getSettingValue<bool>("BG"))
			if (auto bg = static_cast<CCSprite*>(getChildByID("background")))
				bg->setColor(ccc3(255, 255, 255));
		// frame
		if (Mod::get()->getSavedValue<bool>("frame")) {
			getChildByID("level-name-background")->setVisible(false);
			getChildByID("description-background")->setVisible(false);
		}
		return true;
    }
};

#include <Geode/modify/LevelBrowserLayer.hpp>
class $modify(LevelBrowserLayer) {
    bool init(GJSearchObject* search) {
		if (!LevelBrowserLayer::init(search)) return false;
		if (Mod::get()->getSettingValue<bool>("BG"))
			if (auto bg = static_cast<CCSprite*>(getChildByID("background")))
				bg->setColor(ccc3(255, 255, 255));
		return true;
    }
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* level, bool p1) {
		if (!LevelInfoLayer::init(level, p1)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			if (auto bg = static_cast<CCSprite*>(getChildByID("background"))) {
				bool gauntlet = Mod::get()->getSavedValue<bool>("gauntlet-dark") && (level->m_gauntletLevel || level->m_gauntletLevel2);
				bool timely = Mod::get()->getSavedValue<bool>("timely-dark") || getChildByID("daily-label");
				bg->setColor(gauntlet || timely ? ccc3(100, 100, 100): ccc3(255, 255, 255));
			}
		}
		return true;
    }

    void onPlay(cocos2d::CCObject* sender) {
        LevelInfoLayer::onPlay(sender);
		// test will change
		bool circle = Mod::get()->getSavedValue<bool>("Circles", true);
		if (circle){
			auto playMenu = getChildByID("play-menu");
			auto btn = playMenu->getChildByID("play-button");
			auto sprt = static_cast<CCNode *>(btn->getChildren()->objectAtIndex(0));
			
			if (sprt->getChildrenCount()){
				auto sp0 = static_cast<CCSprite *>(sprt->getChildren()->objectAtIndex(0));
				auto sp1 = static_cast<CCSprite *>(sprt->getChildren()->objectAtIndex(1));
				auto sp2 = static_cast<CCProgressTimer *>(sprt->getChildren()->objectAtIndex(2));
				auto sp3 = static_cast<CCSprite *>(sprt->getChildren()->objectAtIndex(3));
				
				ccColor3B black = Mod::get()->getSavedValue<ccColor3B>("plate");
				ccColor3B white = Mod::get()->getSavedValue<ccColor3B>("circle", {255, 255, 255});

				sp0->setColor(black);
				sp1->setColor(black);
				sp2->setColor(black);
				sp3->setColor(white);
			}
		}
    }
};

#include <Geode/modify/LevelSearchLayer.hpp>
class $modify(LevelSearchLayer) {
    bool init(int p) {
		if (!LevelSearchLayer::init(p)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			if (auto bg = static_cast<CCSprite*>(getChildByID("background")))
				bg->setColor(ccc3(255, 255, 255));
		}
		// to edit
		if (Mod::get()->getSavedValue<bool>("frame"), true){
			getChildByID("level-search-bg")->setVisible(false);
			getChildByID("level-search-bar-bg")->setVisible(false);
			getChildByID("quick-search-bg")->setVisible(false);
			getChildByID("difficulty-filters-bg")->setVisible(false);
			getChildByID("length-filters-bg")->setVisible(false);
		}
	return true;
    }
};

#include <Geode/modify/LeaderboardsLayer.hpp>
class $modify(LeaderboardsLayer) {
    bool init(LeaderboardState state) {
		if (!LeaderboardsLayer::init(state)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			if (auto bg = static_cast<CCSprite*>(getChildByID("background")))
				bg->setColor(ccc3(255, 255, 255));
		}
		return true;		
    }
};

#include <Geode/modify/LevelListLayer.hpp>
class $modify(LevelListLayer) {
    bool init(GJLevelList* list) {
		if (!LevelListLayer::init(list)) return false;
		if (Mod::get()->getSettingValue<bool>("BG"))
			static_cast<CCSprite *>(this->getChildren()->objectAtIndex(0))->setColor(
				Mod::get()->getSavedValue<bool>("gauntlet-dark") ? ccc3(100, 100, 100): ccc3(255, 255, 255));
		return true;		
    }
};

#include <Geode/modify/GauntletSelectLayer.hpp>
class $modify(GauntletSelectLayer) {
    bool init(int p) {
		if (!GauntletSelectLayer::init(p)) return false;
		if (Mod::get()->getSettingValue<bool>("BG"))
			static_cast<CCSprite *>(this->getChildren()->objectAtIndex(0))->setColor(
				Mod::get()->getSavedValue<bool>("gauntlet-dark") ? ccc3(100, 100, 100): ccc3(255, 255, 255));
		return true;
    }
};

#include <Geode/modify/GauntletLayer.hpp>
class $modify(GauntletLayer) {
    bool init(GauntletType p) {
		if (!GauntletLayer::init(p)) return false;
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("gauntlet")){
			static_cast<CCSprite*>(getChildren()->objectAtIndex(0))->setVisible(false);
			auto bg = geode::createLayerBG();
			bg->setColor({255, 255, 255});
			bg->setZOrder(-19);
			addChild(bg);

		}
		return true;
    }
};

// MODS ZONE
// All of these codes below (along with the hooker part) are inspired (copied) from Geodify
#include "Hooker.hpp"
// Texture Loader
class PackSelectLayer : public Betterhook::HookBetter { 
	
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG"))
			if (auto bg = static_cast<CCSprite*>(_This->getChildByID("background"))) 
				bg->setColor({255, 255, 255});
    }
    const char* PutLayer() const override { return "PackSelectLayer"; }
};
REGISTER_HookBetter(PackSelectLayer);

// Better Info
class CustomCreatorLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("better-info")) {
			auto bg = geode::createLayerBG();
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-2);
			_This->addChild(bg);
			if (auto oribg = _This->getChildByID("cvolton.betterinfo/background"))
				oribg->setVisible(false);
		}
    }
    const char* PutLayer() const override { return "CustomCreatorLayer"; }
};
REGISTER_HookBetter(CustomCreatorLayer);

// Globed
class GlobedLevelListLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("globed"))
			if (auto bg = typeinfo_cast<CCSprite*>(_This->getChildren()->objectAtIndex(1)))
				bg->setColor({255, 255, 255});
    }
    const char* PutLayer() const override { return "GlobedLevelListLayer"; }
};
REGISTER_HookBetter(GlobedLevelListLayer);

class GlobedMenuLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("globed")) {
			auto bg = geode::createLayerBG();
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-2);
			_This->addChild(bg);
			if (auto oribg = _This->getChildByID("background"))
				oribg->setVisible(false);
		}

    }
    const char* PutLayer() const override { return "GlobedMenuLayer"; }
};
REGISTER_HookBetter(GlobedMenuLayer);

class GlobedServersLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("globed")) {
			auto bg = geode::createLayerBG();
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-2);
			_This->addChild(bg);
			if (auto oribg = _This->getChildByID("background"))
				oribg->setVisible(false);			
		}

	}
    const char* PutLayer() const override { return "GlobedServersLayer"; }
};
REGISTER_HookBetter(GlobedServersLayer);

class GlobedSettingsLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("globed"))
			if (auto bg = static_cast<CCSprite*>(_This->getChildByID("background")))
				bg->setColor({255, 255, 255});
    }
    const char* PutLayer() const override { return "GlobedSettingsLayer"; }
};
REGISTER_HookBetter(GlobedSettingsLayer);

// GDPS switcher
class ServerSwitchLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG"))
			if (auto bg = static_cast<CCSprite*>(_This->getChildByID("background")))
				bg->setColor({255, 255, 255});
    }
    const char* PutLayer() const override { return "ServerSwitchLayer"; }
};
REGISTER_HookBetter(ServerSwitchLayer);

// GDDP integration
class DPLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG"))
			if (auto bg = static_cast<CCSprite*>(_This->getChildByID("bg")))
				bg->setColor({255, 255, 255});
    }
    const char* PutLayer() const override { return "DPLayer"; }
};
REGISTER_HookBetter(DPLayer);

class DPListLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG"))
			if (auto bg = static_cast<CCSprite*>(_This->getChildByID("bg")))
				bg->setColor({255, 255, 255});
    }
    const char* PutLayer() const override { return "DPListLayer"; }
};
REGISTER_HookBetter(DPListLayer);

// Garage Plus
class GPFeedbackLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("garage-plus")) {
			auto bg = geode::createLayerBG();
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-2);
			_This->addChild(bg);
			if (auto oribg = _This->getChildByID("menu")->getChildByID("background"))
				oribg->setVisible(false);
		}
    }

    const char* PutLayer() const override { return "GPFeedbackLayer"; }
};
REGISTER_HookBetter(GPFeedbackLayer);

class GPKofiLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("garage-plus")) {
			auto bg = geode::createLayerBG();
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-2);
			_This->addChild(bg);
			if (auto oribg = _This->getChildByID("background"))
				oribg->setVisible(false);
		}
    }
    const char* PutLayer() const override { return "GPKofiLayer"; }
};
REGISTER_HookBetter(GPKofiLayer);

// Newgrounds Explorer
/*
class NewgroundsBrowserLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG"))
			if (auto bg = static_cast<CCSprite*>(_This->getChildByID("background")))
				bg->setColor({255, 255, 255});
    }
    const char* PutLayer() const override { return "NewgroundsBrowserLayer"; }
};

REGISTER_HookBetter(NewgroundsBrowserLayer);*/

// Texture Workshop
class TextureWorkshopLayer : public Betterhook::HookBetter { 
    void init(CCNode* _This) override {
		if (Mod::get()->getSettingValue<bool>("BG") && Mod::get()->getSettingValue<bool>("texture-workshop")) {
			auto bg = geode::createLayerBG();
			bg->setColor(ccc3(255, 255, 255));
			bg->setZOrder(-2);
			_This->addChild(bg);
			if (auto oribg = _This->getChildByID("background"))
				oribg->setVisible(false);
		}
    }
    const char* PutLayer() const override { return "TextureWorkshopLayer"; }
};
REGISTER_HookBetter(TextureWorkshopLayer);