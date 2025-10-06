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
    "World", "Galaxy", "Universe", "Discord", "Split", "NCS I", "NCS II", "Space", "Cosmos"
};
/*
const static std::map<TextureQuality, std::string> qualityNotes = {
    {TextureQuality::kTextureQualityLow, ""},
    {TextureQuality::kTextureQualityMedium, "-hd"},
    {TextureQuality::kTextureQualityHigh, "-uhd"}
};*/

static bool tryReplace(CCSprite* spr, std::string subFolder, auto fileName) {
    auto q = CCDirector::sharedDirector()->getLoadedTextureQuality();
    auto p = CONFIG / subFolder / fmt::format("{}{}.png", fileName, q == TextureQuality::kTextureQualityLow ? "" : (q == TextureQuality::kTextureQualityHigh ? "-uhd" : "-hd"));
    if (!std::filesystem::exists(p))
        return false;
    auto ref = CCSprite::create(p.string().c_str());
    if (!ref)
        return false;
    spr->setDisplayFrame(ref->displayFrame());
    return true;
}