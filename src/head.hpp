#include <Geode/Geode.hpp>
//#include <Geode/loader/Dispatch.hpp>

using namespace geode::prelude;

const auto CONFIG = Mod::get()->getConfigDir();

const static std::vector<std::string> gauntletIndex = {
    "Fire", "Ice", "Poison", "Shadow", "Lava", "Bonus", "Chaos", "Demon", "Time",
    "Crystal", "Magic", "Spike", "Monster", "Doom", "Death", "Forest", "Rune", "Force",
    "Spooky", "Dragon", "Water", "Haunted", "Acid", "Witch", "Power", "Potion", "Snake",
    "Toxic", "Halloween", "Treasure", "Ghost", "Spider", "Gem", "Inferno", "Portal", "Strange",
    "Fantasy", "Christmas", "Surprise", "Mystery", "Cursed", "Cyborg", "Castle", "Grave", "Temple",
    "World", "Galaxy", "Universe", "Discord", "Split", "NCS I", "NCS II", "Space", "Cosmos",
    "Random", "Chance", "Future", "Utopia", "Cinema", "Love" // 2.208 addition
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
    if (!std::filesystem::exists(p))
        return false;
    auto ref = CCSprite::create(p.string().c_str());
    if (!ref)
        return false;
    spr->setDisplayFrame(ref->displayFrame());
    return true;
}