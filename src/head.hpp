#include <Geode/Geode.hpp>
//#include <Geode/loader/Dispatch.hpp>

using namespace geode::prelude;

const auto CONFIG = Mod::get()->getConfigDir();

static bool already_has_menu; // the menu is launched
static std::string layerName;
static std::string levelName;
static std::string listName;
static std::string timelyName; // one of : daily weekly event gauntlet
static std::string gauntletName; // gauntlet map

const static std::vector<std::string> gauntletIndex = {
    "Fire", "Ice", "Poison", "Shadow", "Lava", "Bonus", "Chaos", "Demon", "Time",
    "Crystal", "Magic", "Spike", "Monster", "Doom", "Death", "Forest", "Rune", "Force",
    "Spooky", "Dragon", "Water", "Haunted", "Acid", "Witch", "Power", "Potion", "Snake",
    "Toxic", "Halloween", "Treasure", "Ghost", "Spider", "Gem", "Inferno", "Portal", "Strange",
    "Fantasy", "Christmas", "Surprise", "Mystery", "Cursed", "Cyborg", "Castle", "Grave", "Temple",
    "World", "Galaxy", "Universe", "Discord", "Split", "NCS I", "NCS II", "Space", "Cosmos",
    "Random", "Chance", "Future", "Utopia", "Cinema", "Love", "Duality", "Paradox" // addition
};

static bool tryReplace(CCSprite* spr, std::string subFolder, std::string fileName) {
    std::string qtext = "";
    switch (CCDirector::sharedDirector()->getLoadedTextureQuality()) {
    case TextureQuality::kTextureQualityHigh:
        qtext = "-uhd";
        break;
    case TextureQuality::kTextureQualityMedium:
        qtext = "-hd";
        break;
    default:
        break;
    }
    auto p = CONFIG / subFolder / (fileName + qtext + ".png");
    if (!p.exists())
        return false;
    auto ref = CCSprite::create(p.string().c_str());
    if (!ref)
        return false;
    spr->setDisplayFrame(ref->displayFrame());
    return true;
}

/*
class BackgroundPopup : public Popup {
protected:
    // sprite
    CCSprite* sprBG;
    // buttons
    CCMenuItemSpriteExtra* btnPick, *btnRemove, * btnLayer, * btnTimely, * btnLevel;
    // init
    bool init();
    // place the image
    bool placeImage();
    // switch between tabs
    void onTab(CCObject* sender);
    // pick an image
    void onPick(CCObject* sender);
    // remove an image
    void onRemove(CCObject* sender);
public:
    BackgroundPopup* create() {
        auto layer = new BackgroundPopup();

        if (layer && layer->init()) {
            layer->autorelease();
            return layer;
        };
        CC_SAFE_DELETE(layer);
        return nullptr;
    }
};*/