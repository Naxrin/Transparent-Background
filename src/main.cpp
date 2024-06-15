#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/CreatorLayer.hpp>
class $modify(CreatorLayer) {
    bool init() {
        if (!CreatorLayer::init()) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			auto background = static_cast<CCSprite *>(this->getChildByID("background"));
			if (background){
				ccColor3B cb = { 255, 255, 255 };
				background->setColor(cb);
			}
		}
		return true;		
    }
};

#include <Geode/modify/GJGarageLayer.hpp>
class $modify(GJGarageLayer) {
    bool init() {
        if (!GJGarageLayer::init()) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			auto background = static_cast<CCSprite *>(this->getChildByID("background"));
			if (background){
				ccColor3B cb = { 255, 255, 255 };
				background->setColor(cb);
			}
		}
		return true;		
    }
};

#include <Geode/modify/EditLevelLayer.hpp>
class $modify(EditLevelLayer) {
    bool init(GJGameLevel* level) {
		if (!EditLevelLayer::init(level)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			auto background = static_cast<CCSprite *>(this->getChildByID("background"));
			if (background){
				ccColor3B cb = { 255, 255, 255 };
				background->setColor(cb);
			}
		}
		if (Mod::get()->getSettingValue<bool>("frame")){
			this->getChildByID("level-name-background")->setVisible(false);
			this->getChildByID("description-background")->setVisible(false);
		}
		return true;
    }
};

#include <Geode/modify/LevelBrowserLayer.hpp>
class $modify(LevelBrowserLayer) {
    bool init(GJSearchObject* search) {
		if (!LevelBrowserLayer::init(search)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			auto background = static_cast<CCSprite *>(this->getChildByID("background"));
			if (background){
				ccColor3B cb = { 255, 255, 255 };
				background->setColor(cb);
			}
		}
		return true;
    }
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* level, bool p1) {
		if (!LevelInfoLayer::init(level, p1)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			
			auto background = static_cast<CCSprite *>(this->getChildByID("background"));
			ccColor3B cb;
			if (Mod::get()->getSettingValue<bool>("dark") && (level->m_gauntletLevel || level->m_gauntletLevel2 || this->getChildByID("daily-label")))
				cb = { 100, 100, 100 };
			else
				cb = { 255, 255, 255 };
			if (background){
				background->setColor(cb);
			}
		}
		return true;
    }
#ifndef GEODE_IS_MACOS
    void onPlay(cocos2d::CCObject* sender) {
        LevelInfoLayer::onPlay(sender);
	bool color = Mod::get()->getSettingValue<bool>("Circles");
	if (color&&Mod::get()->getSettingValue<bool>("BG")){
		auto PlayMenu = this->getChildByID("play-menu");
		auto btn = PlayMenu->getChildByID("play-button");
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
    }
#endif	
};

#include <Geode/modify/LevelSearchLayer.hpp>
class $modify(LevelSearchLayer) {
    bool init(int p) {
		if (!LevelSearchLayer::init(p)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			auto background = static_cast<CCSprite *>(this->getChildByID("background"));
			if (background){
				ccColor3B cb = { 255, 255, 255 };
				background->setColor(cb);
			}
		}
		if (Mod::get()->getSettingValue<bool>("frame")){
			this->getChildByID("level-search-bg")->setVisible(false);
			this->getChildByID("level-search-bar-bg")->setVisible(false);
			this->getChildByID("quick-search-bg")->setVisible(false);
			this->getChildByID("difficulty-filters-bg")->setVisible(false);
			this->getChildByID("length-filters-bg")->setVisible(false);
		}
	return true;
    }
};

#include <Geode/modify/LeaderboardsLayer.hpp>
class $modify(LeaderboardsLayer) {
    bool init(LeaderboardState state) {
		if (!LeaderboardsLayer::init(state)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			auto background = static_cast<CCSprite *>(this->getChildByID("background"));
			if (background){
				ccColor3B cb = { 255, 255, 255 };
				background->setColor(cb);
			}
		}
		return true;		
    }
};

#ifndef GEODE_IS_MACOS
#include <Geode/modify/LevelListLayer.hpp>
class $modify(LevelListLayer) {
    bool init(GJLevelList* list) {
		if (!LevelListLayer::init(list)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			auto background = static_cast<CCSprite *>(this->getChildren()->objectAtIndex(0));
			if (background){
				ccColor3B cb = { 255, 255, 255 };
				background->setColor(cb);
			}
		}
		return true;		
    }
};
#endif

#include <Geode/modify/GauntletSelectLayer.hpp>
class $modify(GauntletSelectLayer) {
    bool init(int p) {
		if (!GauntletSelectLayer::init(p)) return false;
		if (Mod::get()->getSettingValue<bool>("BG")){
			auto background = static_cast<CCSprite *>(this->getChildren()->objectAtIndex(0));
			ccColor3B cb;
			if (Mod::get()->getSettingValue<bool>("dark"))
				cb = { 100, 100, 100 };
			else
				cb = { 255, 255, 255 };
			if (background){
				background->setColor(cb);
			}
		}
		return true;
    }
};
/*
// Transparent Lists
#include <Geode/modify/GJListLayer.hpp>
class $modify(GJListLayer) {

	static GJListLayer* create(BoomListView* p0, char const* p1, cocos2d::ccColor4B p2, float p3, float p4, int p5) {
		auto ret = GJListLayer::create(p0, p1, p2, p3, p4, p5);
		if (Mod::get()->getSettingValue<bool>("List")){
			if (ret->getColor() == ccColor3B{191,114,62}){
				ret->setColor({ 223, 222, 222 });
				ret->setOpacity(50.0f);
			}
		}
		return ret;
	}
};

#include <Geode/modify/LevelCell.hpp>
class $modify(LevelCell) {

	void updateBGColor(int p0) {

		LevelCell::updateBGColor(p0);
		
		CCLayerColor* child = getChildOfType<CCLayerColor>(this, 0);
		if (Mod::get()->getSettingValue<bool>("List")){
			if(child->getColor() == ccColor3B{161,88,44}){
				child->setColor({0,0,0});
			}
			else if(child->getColor() == ccColor3B{194,114,62}){
				child->setColor({80,80,80});
			}
			child->setOpacity(50.0f);
		}
	}
};
*/


// real
/*
#include <Geode/modify/LevelCell.hpp>
class $modify(LevelCell) {
   	TodoReturn loadFromLevel(GJGameLevel* p0) {
	LevelCell::loadFromLevel(p0);	
		if (Mod::get()->getSettingValue<bool>("List")){
			auto child = static_cast<CCLayerColor *>(this->getChildren()->objectAtIndex(0));
			if(child->getColor() == ccColor3B{161,88,44}){
				child->setColor({0,0,0});
			}
			else if(child->getColor() == ccColor3B{194,114,62}){
				child->setColor({80,80,80});
			}
			child->setOpacity(50.0f);
			//this->updateLayout();
		}
    }
};

#include <Geode/modify/GJScoreCell.hpp>
class $modify(GJScoreCell) {
    TodoReturn loadFromScore(GJUserScore* p0) {
		GJScoreCell::loadFromScore(p0);	
		if (Mod::get()->getSettingValue<bool>("List")){
			auto child = static_cast<CCLayerColor *>(this->getChildren()->objectAtIndex(0));
			if(child->getColor() == ccColor3B{161,88,44}){
				child->setColor({0,0,0});
			}
			else if(child->getColor() == ccColor3B{194,114,62}){
				child->setColor({80,80,80});
			}
			child->setOpacity(50.0f);
		}
		return true;
    }
};

#include <Geode/modify/LevelListCell.hpp>
class $modify(LevelListCell) {
   	TodoReturn loadFromList(GJLevelList* p0) {
		LevelListCell::loadFromList(p0);	
		if (Mod::get()->getSettingValue<bool>("List")){
			auto child = static_cast<CCLayerColor *>(this->getChildren()->objectAtIndex(0));
			if(child->getColor() == ccColor3B{161,88,44}){
				child->setColor({0,0,0});
			}
			else if(child->getColor() == ccColor3B{194,114,62}){
				child->setColor({80,80,80});
			}
			child->setOpacity(50.0f);
		}
		return true;
    }
};

#include <Geode/modify/MapPackCell.hpp>
class $modify(MapPackCell) {
   	TodoReturn loadFromMapPack(GJMapPack* p0) {
		MapPackCell::loadFromMapPack(p0);	
		if (Mod::get()->getSettingValue<bool>("List")){
			auto child = static_cast<CCLayerColor *>(this->getChildren()->objectAtIndex(0));
			if(child->getColor() == ccColor3B{161,88,44}){
				child->setColor({0,0,0});
			}
			else if(child->getColor() == ccColor3B{194,114,62}){
				child->setColor({80,80,80});
			}
			child->setOpacity(50.0f);
		}
		return true;
    }
};*/
