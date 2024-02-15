/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>
/**
 * Required to modify the MenuLayer class
 */
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/modify/LevelListLayer.hpp>
#include <Geode/modify/LeaderboardsLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>

/**
 * Brings cocos2d and all Geode namespaces 
 * to the current scope.
 */
using namespace geode::prelude;

/**
 * `$modify` lets you extend and modify GD's 
 * classes; to hook a function in Geode, 
 * simply $modify the class and write a new 
 * function definition with the signature of 
 * the one you want to hook.
 */

class $modify(CreatorLayer) {
    bool init() {
        if (!CreatorLayer::init()) return false;
		auto background = static_cast<CCSprite *>(this->getChildByID("background"));
		ccColor3B cb = { 255, 255, 255 };
		background->setColor(cb);
        return true;
    }
};

class $modify(EditLevelLayer) {
    bool init(GJGameLevel* p0) {
		if (!EditLevelLayer::init(p0)) return false;
		auto background = static_cast<CCSprite *>(this->getChildByID("background"));
		ccColor3B cb = { 255, 255, 255 };
		background->setColor(cb);
		return true;
    }
};

class $modify(LevelBrowserLayer) {
    bool init(GJSearchObject* p0) {
		if (!LevelBrowserLayer::init(p0)) return false;
		auto background = static_cast<CCSprite *>(this->getChildByID("background"));
		ccColor3B cb = { 255, 255, 255 };
		background->setColor(cb);
		return true;
    }
};

class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* p0, bool p1) {
		if (!LevelInfoLayer::init(p0, p1)) return false;
		auto background = static_cast<CCSprite *>(this->getChildByID("background"));
		ccColor3B cb = { 255, 255, 255 };
		background->setColor(cb);
		return true;
    }
};

class $modify(LevelSearchLayer) {
    bool init(int p0) {
		if (!LevelSearchLayer::init(p0)) return false;
		auto background = static_cast<CCSprite *>(this->getChildByID("background"));
		ccColor3B cb = { 255, 255, 255 };
		background->setColor(cb);
		return true;
    }
};

class $modify(LeaderboardsLayer) {
    bool init(LeaderboardState p0) {
		if (!LeaderboardsLayer::init(p0)) return false;
		auto background = static_cast<CCSprite *>(this->getChildByID("background"));
		ccColor3B cb = { 255, 255, 255 };
		background->setColor(cb);
		return true;
    }
};
