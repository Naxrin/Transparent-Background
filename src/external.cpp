#include <alphalaneous.alphas_geode_utils/include/NodeModding.h>

using namespace geode::prelude;

// Texture Loader
class $nodeModify(PackSelectLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

// Better Info
class $nodeModify(CustomCreatorLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("better-info")) return;
		auto bg = geode::createLayerBG();
		bg->setColor(ccc3(255, 255, 255));
		bg->setZOrder(-2);
		this->addChild(bg);
		if (auto oribg = this->getChildByID("cvolton.betterinfo/background"))
			oribg->setVisible(false);
	}
};

class $nodeModify(DailyViewLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("better-info")) return;
		auto bg = geode::createLayerBG();
		bg->setColor(ccc3(255, 255, 255));
		bg->setZOrder(-2);
		this->addChild(bg);
		if (auto oribg = this->getChildByID("cvolton.betterinfo/background"))
			oribg->setVisible(false);
	}
};

class $nodeModify(LevelSearchViewLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("better-info")) return;
		auto bg = geode::createLayerBG();
		bg->setColor(ccc3(255, 255, 255));
		bg->setZOrder(-2);
		this->addChild(bg);
		if (auto oribg = this->getChildByID("cvolton.betterinfo/background"))
			oribg->setVisible(false);
	}
};

class $nodeModify(LeaderboardViewLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("better-info")) return;
		auto bg = geode::createLayerBG();
		bg->setColor(ccc3(255, 255, 255));
		bg->setZOrder(-2);
		this->addChild(bg);
		if (auto oribg = this->getChildByID("cvolton.betterinfo/background"))
			oribg->setVisible(false);
	}
};

class $nodeModify(RewardGroupLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("better-info")) return;
		auto bg = geode::createLayerBG();
		bg->setColor(ccc3(255, 255, 255));
		bg->setZOrder(-2);
		this->addChild(bg);
		if (auto oribg = this->getChildByID("cvolton.betterinfo/background"))
			oribg->setVisible(false);
	}
};

class $nodeModify(RewardViewLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("better-info")) return;
		auto bg = geode::createLayerBG();
		bg->setColor(ccc3(255, 255, 255));
		bg->setZOrder(-2);
		this->addChild(bg);
		if (auto oribg = this->getChildByID("cvolton.betterinfo/background"))
			oribg->setVisible(false);
	}
};

// GD Utils
class $nodeModify(MoreLeaderboards) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("gd-utils")) return;
		if (auto bg = this->getChildByType<CCSprite>(0))
			bg->setColor(ccc3(255, 255, 255));
	}
};

// Globed
class $nodeModify(GlobedLevelListLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto bg = this->getChildByType<CCSprite>(1))
			bg->setColor(ccc3(255, 255, 255));
	}
};

class $nodeModify(GlobedMenuLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("globed")) return;
		auto bg = geode::createLayerBG();
		bg->setColor(ccc3(255, 255, 255));
		bg->setZOrder(-2);
		this->addChild(bg);
		if (auto oribg = this->getChildByID("background"))
			oribg->setVisible(false);
	}
};

class $nodeModify(GlobedServersLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("globed")) return;
		auto bg = geode::createLayerBG();
		bg->setColor(ccc3(255, 255, 255));
		bg->setZOrder(-2);
		this->addChild(bg);
		if (auto oribg = this->getChildByID("background"))
			oribg->setVisible(false);
	}
};

class $nodeModify(GlobedSettingsLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

// GDPS switcher
class $nodeModify(ServerSwitchLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

// GDPS Hub
class $nodeModify(GDPSHubLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("gdps-hub")) return;
        auto bg = geode::createLayerBG();
        bg->setColor(ccc3(255, 255, 255));
        bg->setZOrder(-2);
        this->addChild(bg);
        if (auto oribg = this->getChildByID("background"))
            oribg->setVisible(false);
		if (auto oribg = this->getChildByID("swelvy-node"))
            oribg->setVisible(false);
	}
};

// GDDP integration
class $nodeModify(DPLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("bg"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

class $nodeModify(DPListLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("bg"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

class $nodeModify(DPSearchLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("bg"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

class $nodeModify(RecommendedLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("bg"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

class $nodeModify(RouletteSafeLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("bg"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

// Garage Plus
class $nodeModify(GPFeedbackLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("garage-plus")) return;
        auto bg = geode::createLayerBG();
        bg->setColor(ccc3(255, 255, 255));
        bg->setZOrder(-2);
        this->addChild(bg);
        if (auto oribg = this->getChildByID("menu")->getChildByID("background"))
            oribg->setVisible(false);
	}
};

class $nodeModify(GPKofiLayer) {
	void modify() {
		if (Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("garage-plus")) return;
        auto bg = geode::createLayerBG();
        bg->setColor(ccc3(255, 255, 255));
        bg->setZOrder(-2);
        this->addChild(bg);
        if (auto oribg = this->getChildByID("background"))
            oribg->setVisible(false);
	}
};

// Texture Workshop
class $nodeModify(TextureWorkshopLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG") || !Mod::get()->getSettingValue<bool>("texture-workshop")) return;
        auto bg = geode::createLayerBG();
        bg->setColor(ccc3(255, 255, 255));
        bg->setZOrder(-2);
        this->addChild(bg);
        if (auto oribg = this->getChildByID("background"))
            oribg->setVisible(false);
	}
};

// Integrated Demonlist
class $nodeModify(IDListLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

class $nodeModify(IDPackLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

// GDCP challenge
class $nodeModify(GDCPListLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

// Better Achievements
class $nodeModify(BetterAchievementLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("content-background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

// GD Stream

// Geometry Dash Odyssey
class $nodeModify(NewgroundsBrowserLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("teamtcm.geometry-dash-odyssey/background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

// Newgrounds Explorer
class $nodeModify(NewgroundsBrowserLayer) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto node = this->getChildByID("background"))
			if (auto bg = typeinfo_cast<CCSprite*>(node))
				bg->setColor(ccc3(255, 255, 255));
	}
};

// Secret Layer 6
class $nodeModify(SecretLayer6R) {
	void modify() {
		if (!Mod::get()->getSettingValue<bool>("BG")) return;
		if (auto bg = this->getChildByType<CCSprite>(0))
			bg->setColor(ccc3(255, 255, 255));
	}
};