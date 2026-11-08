#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Dispatch.hpp>

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace geode::prelude;

inline asp::fs::Path const& configDir() {
    static asp::fs::Path const dir = Mod::get()->getConfigDir();
    return dir;
}

inline bool already_has_menu = false; // the menu is launched

static std::string layerName;
static std::string levelName;
static std::string listName;
static std::string timelyName; // one of : daily weekly event gauntlet
static std::string gauntletName; // gauntlet map

static const std::vector<std::string> gauntletIndex = {
    "Fire", "Ice", "Poison", "Shadow", "Lava", "Bonus", "Chaos", "Demon", "Time",
    "Crystal", "Magic", "Spike", "Monster", "Doom", "Death", "Forest", "Rune", "Force",
    "Spooky", "Dragon", "Water", "Haunted", "Acid", "Witch", "Power", "Potion", "Snake",
    "Toxic", "Halloween", "Treasure", "Ghost", "Spider", "Gem", "Inferno", "Portal", "Strange",
    "Fantasy", "Christmas", "Surprise", "Mystery", "Cursed", "Cyborg", "Castle", "Grave", "Temple",
    "World", "Galaxy", "Universe", "Discord", "Split", "NCS I", "NCS II", "Space", "Cosmos",
    "Random", "Chance", "Future", "Utopia", "Cinema", "Love", "Duality", "Paradox" // addition
};

// ---------------------------------------------------------------------------
// saved.json helpers
// ---------------------------------------------------------------------------

inline std::string normalizePathString(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
}

// Paths are stored relative to the config folder without a root marker,
// e.g. "layers/MenuLayer-uhd.png". Older saves used "~/" or "config/" and are
// still accepted by savedToPath.
inline std::string pathToSaved(asp::fs::Path const& path) {
    if (path.empty())
        return "";

    if (auto rel = path.lexicallyRelative(configDir()); rel && !rel->empty()) {
        auto relString = normalizePathString(rel->string());
        if (!relString.starts_with(".."))
            return relString;
    }

    return normalizePathString(path.string());
}

inline std::string canonicalSavedPath(std::string const& saved) {
    auto normalized = normalizePathString(saved);
    if (normalized.starts_with("config/"))
        return normalized.substr(7);
    if (normalized.starts_with("~/"))
        return normalized.substr(2);
    return normalized;
}

inline asp::fs::Path savedToPath(std::string const& saved) {
    if (saved.empty())
        return {};

    auto normalized = normalizePathString(saved);
    if (normalized.starts_with("config/"))
        return configDir() / normalized.substr(7);
    if (normalized.starts_with("~/"))
        return configDir() / normalized.substr(2);

    asp::fs::Path path{normalized};
    if (path.isRelative())
        return configDir() / normalized;
    return path;
}

inline std::string pathForDisplay(std::string const& saved) {
    return canonicalSavedPath(saved);
}

// Kept for call sites that predate the switch to plain relative paths; the
// canonical form no longer has a root marker.
inline std::string displayPathWithoutRoot(std::string const& saved) {
    return pathForDisplay(saved);
}

// Background names are limited to 30 characters; keep the beginning and cut
// the tail.
inline std::string truncateName(std::string name, size_t maxLength = 30) {
    if (name.size() > maxLength)
        name.resize(maxLength);
    return name;
}

// Texture names are shown without their .png suffix in the picker.
inline std::string displayFileName(std::string const& fileName) {
    if (fileName.size() > 4) {
        auto tail = fileName.substr(fileName.size() - 4);
        std::transform(tail.begin(), tail.end(), tail.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (tail == ".png")
            return truncateName(fileName.substr(0, fileName.size() - 4));
    }
    return truncateName(fileName);
}

// ---------------------------------------------------------------------------
// dedicated background keys for levels / lists / gauntlet maps
// ---------------------------------------------------------------------------
// Editor levels and lists have no IDs in the base game, so "cvolton.level-id-api"
// optionally provides stable ones. It exports its API through Geode events, so
// no build-time dependency is required: the first call looks the exported
// function pointer up in the global event center and caches it. If the API mod
// is not installed (or the lookup fails), 0 is returned and the callers below
// fall back to a prefixed name key.
//
// `autoAssign` asks the API to create an ID for a level/list that does not have
// one yet; pass false when only looking up existing data (e.g. during deletion).
inline int getEditorLevelID(GJGameLevel* level, bool autoAssign) {
    if (!level)
        return 0;

    static auto api = geode::geode_internal::callEventExportListener<
        int (*)(GJGameLevel*, bool)
    >("cvolton.level-id-api/getID-level-autoAssign");
    if (!api)
        return 0;
    return api(level, autoAssign);
}

inline int getEditorListID(GJLevelList* list, bool autoAssign) {
    if (!list)
        return 0;

    static auto api = geode::geode_internal::callEventExportListener<
        int (*)(GJLevelList*, bool)
    >("cvolton.level-id-api/getID-list-autoAssign");
    if (!api)
        return 0;
    return api(list, autoAssign);
}

// An editor level is either flagged editable or registered in LocalLevelManager.
// Online / saved levels are not in that manager, so they keep using their
// online level ID instead of an editor ID.
inline bool isEditorLevel(GJGameLevel* level) {
    if (!level)
        return false;
    if (level->m_isEditable)
        return true;

    auto manager = LocalLevelManager::sharedState();
    if (!manager || !manager->m_localLevels)
        return false;

    for (auto entry : CCArrayExt<GJGameLevel*>(manager->m_localLevels)) {
        if (entry == level)
            return true;
    }
    return false;
}

// Saved background key of a level:
//   online / saved level -> "le-<onlineID>"
//   editor level         -> "le-<editorID>" (or "le-n:<name>" without the API)
// The prefix keeps level, list and gauntlet keys apart in the flat saved map.
inline std::string levelBackgroundKey(GJGameLevel* level, bool autoAssign = true) {
    if (!level)
        return "";

    if (isEditorLevel(level)) {
        int id = getEditorLevelID(level, autoAssign);
        if (id != 0)
            return "le-" + numToString(id);
        if (!level->m_levelName.empty())
            return "le-n:" + level->m_levelName;
        return "";
    }

    auto id = level->m_levelID.value();
    return id > 0 ? "le-" + numToString(id) : "";
}

// Saved background key of a list: "li-<listID>" for online lists and
// "li-<editorID>" for editable lists. Same fallback rules as levels.
inline std::string listBackgroundKey(GJLevelList* list, bool autoAssign = true) {
    if (!list)
        return "";

    if (list->m_isEditable) {
        int id = getEditorListID(list, autoAssign);
        if (id != 0)
            return "li-" + numToString(id);
        if (!list->m_listName.empty())
            return "li-n:" + list->m_listName;
        return "";
    }

    return list->m_listID != 0 ? "li-" + numToString(list->m_listID) : "";
}

// Gauntlet map keys are stored as "g-<lowercase map name>", e.g. "g-fire".
inline std::string gauntletBackgroundKey(std::string mapName) {
    std::transform(mapName.begin(), mapName.end(), mapName.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return "g-" + mapName;
}

inline std::map<std::string, std::string> getSavedBackgrounds() {
    return Mod::get()->getSavedValue<std::map<std::string, std::string>>("backgrounds");
}

inline std::string getSavedBackground(std::string_view key) {
    auto backgrounds = getSavedBackgrounds();
    if (auto it = backgrounds.find(std::string(key)); it != backgrounds.end())
        return it->second;
    return "";
}

inline void setSavedBackground(std::string_view key, std::string const& path) {
    auto backgrounds = getSavedBackgrounds();
    backgrounds[std::string(key)] = path;
    Mod::get()->setSavedValue("backgrounds", backgrounds);
}

// Erases the entry entirely instead of leaving an empty value behind.
inline void removeSavedBackground(std::string_view key) {
    auto backgrounds = getSavedBackgrounds();
    if (backgrounds.erase(std::string(key)) > 0)
        Mod::get()->setSavedValue("backgrounds", backgrounds);
}

// User-assigned display names. The mapping is stored in saved.json only; the
// file on disk is never renamed. Keys are canonicalized so older saves that
// still contain a "~/" prefix keep resolving.
inline std::map<std::string, std::string> getDisplayNames() {
    auto stored = Mod::get()->getSavedValue<std::map<std::string, std::string>>(
        "display-names"
    );
    std::map<std::string, std::string> names;
    for (auto const& [key, value] : stored)
        names[canonicalSavedPath(key)] = value;
    return names;
}

inline std::string getDisplayName(std::string const& savedPath) {
    auto canonical = canonicalSavedPath(savedPath);
    auto names = getDisplayNames();
    if (auto it = names.find(canonical); it != names.end())
        return truncateName(it->second);
    return "";
}

inline void setDisplayName(std::string const& savedPath, std::string const& name) {
    auto canonical = canonicalSavedPath(savedPath);
    auto names = getDisplayNames();
    if (name.empty())
        names.erase(canonical);
    else
        names[canonical] = truncateName(name);
    Mod::get()->setSavedValue("display-names", names);
}

// Favorites are canonicalized on read for the same backwards-compatibility
// reason as display names.
inline std::vector<std::string> getFavorites() {
    auto list = Mod::get()->getSavedValue<std::vector<std::string>>("favorites");
    for (auto& entry : list)
        entry = canonicalSavedPath(entry);
    return list;
}

inline bool isFavorite(std::string const& savedPath) {
    auto canonical = canonicalSavedPath(savedPath);
    auto list = getFavorites();
    return std::any_of(list.begin(), list.end(), [&](auto const& entry) {
        return canonicalSavedPath(entry) == canonical;
    });
}

inline void setFavorite(std::string const& savedPath, bool favorite) {
    auto canonical = canonicalSavedPath(savedPath);
    auto list = getFavorites();
    auto it = std::find_if(list.begin(), list.end(), [&](auto const& entry) {
        return canonicalSavedPath(entry) == canonical;
    });

    if (favorite && it == list.end()) {
        list.push_back(canonical);
        Mod::get()->setSavedValue("favorites", list);
    }
    else if (!favorite && it != list.end()) {
        list.erase(it);
        Mod::get()->setSavedValue("favorites", list);
    }
}

inline bool toggleFavorite(std::string const& savedPath) {
    bool favorite = !isFavorite(savedPath);
    setFavorite(savedPath, favorite);
    return favorite;
}

// ---------------------------------------------------------------------------
// background helpers
// ---------------------------------------------------------------------------

inline std::string textureQualitySuffix() {
    switch (CCDirector::sharedDirector()->getLoadedTextureQuality()) {
    case TextureQuality::kTextureQualityHigh:
        return "-uhd";
    case TextureQuality::kTextureQualityMedium:
        return "-hd";
    default:
        return "";
    }
}

inline bool isImagePath(asp::fs::Path const& path) {
    auto ext = path.extension();
    if (!ext)
        return false;

    std::string lowered(ext->data(), ext->size());
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return lowered == "png" || lowered == "jpg" || lowered == "jpeg" ||
           lowered == "bmp" || lowered == "webp" || lowered == "gif";
}

// Remembers which interface filename / class a background sprite belongs to,
// so the picker can save the choice under the same key the hooks look up.
inline std::string backgroundTag(CCSprite* spr) {
    if (!spr)
        return "";
    if (auto obj = spr->getUserObject("layer-key"_spr))
        if (auto str = typeinfo_cast<CCString*>(obj))
            return str->getCString();
    return "";
}

inline void setBackgroundTag(CCSprite* spr, std::string const& key) {
    if (spr)
        spr->setUserObject("layer-key"_spr, CCString::create(key));
}

inline std::string backgroundKeyFor(CCSprite* spr, std::string const& fallback) {
    if (auto tag = backgroundTag(spr); !tag.empty())
        return tag;

    if (spr) {
        // walk up to the topmost node that is still inside the running scene
        CCNode* node = spr->getParent();
        while (node && node->getParent() && !typeinfo_cast<CCScene*>(node->getParent()))
            node = node->getParent();

        if (node && typeinfo_cast<CCLayer*>(node)) {
            std::string name{geode::cocos::getObjectName(node)};
            if (!name.empty() && !name.starts_with("CC"))
                return name;
        }
    }

    return fallback;
}

// Remembers how a background sprite looked before this mod replaced its frame,
// so the picker can switch back to the default when the applied one is toggled
// off.
class BackgroundSnapshot final : public cocos2d::CCObject {
public:
    Ref<cocos2d::CCSpriteFrame> frame;
    float scaleX = 1.f;
    float scaleY = 1.f;
    cocos2d::CCPoint position;
    cocos2d::ccColor3B color = ccWHITE;

    static BackgroundSnapshot* capture(cocos2d::CCSprite* spr) {
        auto snapshot = new BackgroundSnapshot();
        if (spr) {
            snapshot->frame = spr->displayFrame();
            snapshot->scaleX = spr->getScaleX();
            snapshot->scaleY = spr->getScaleY();
            snapshot->position = spr->getPosition();
            snapshot->color = spr->getColor();
        }
        snapshot->autorelease();
        return snapshot;
    }
};

inline void rememberDefaultBackground(cocos2d::CCSprite* spr) {
    if (!spr || spr->getUserObject("default-background"_spr))
        return;
    spr->setUserObject("default-background"_spr, BackgroundSnapshot::capture(spr));
}

// ---------------------------------------------------------------------------
// main menu animated background
// ---------------------------------------------------------------------------

// MenuGameLayer draws the animated main-menu background and also handles taps
// on it (making the icon jump). Simply hiding it is not enough: the main menu
// can end up with visible but unresponsive buttons. Keep the layer alive
// underneath our covering background and cut its touch input instead; the
// original z-order is remembered for revealMenuGameLayer.
inline void coverMenuGameLayer(cocos2d::CCNode* layer) {
    auto menu = typeinfo_cast<MenuLayer*>(layer);
    if (!menu || !menu->m_menuGameLayer)
        return;

    auto gameLayer = menu->m_menuGameLayer;
    // Store the original z-order once; its presence also marks the layer as
    // covered for revealMenuGameLayer.
    if (!gameLayer->getUserObject("menu-game-z"_spr)) {
        gameLayer->setUserObject(
            "menu-game-z"_spr, cocos2d::CCInteger::create(gameLayer->getZOrder())
        );
    }

    gameLayer->setVisible(true);
    gameLayer->setTouchEnabled(false);
    // MenuLayer's covering background is added at z = -19.
    gameLayer->setZOrder(-20);
}

// Undoes coverMenuGameLayer once our covering background is removed: the
// animated background gets its original z-order and touch input back.
inline void revealMenuGameLayer(cocos2d::CCNode* layer) {
    auto menu = typeinfo_cast<MenuLayer*>(layer);
    if (!menu || !menu->m_menuGameLayer)
        return;

    auto gameLayer = menu->m_menuGameLayer;
    auto stored = gameLayer->getUserObject("menu-game-z"_spr);
    if (!stored)
        return;

    if (auto number = typeinfo_cast<cocos2d::CCInteger*>(stored))
        gameLayer->setZOrder(number->getValue());
    gameLayer->setUserObject("menu-game-z"_spr, nullptr);

    gameLayer->setVisible(true);
    gameLayer->setTouchEnabled(true);
}

// Background switches fade out, swap, then fade back in. Both halves take
// 0.2 seconds.
inline constexpr float BACKGROUND_FADE_TIME = 0.2f;
inline constexpr int BACKGROUND_FADE_TAG = 0x4247;

// Runs `apply` on the sprite halfway through a fade-out / fade-in pair. The
// final opacity is remembered on the sprite so two quick switches do not end
// with an invisible background.
inline void runBackgroundFade(cocos2d::CCSprite* spr, std::function<void()> apply) {
    if (!spr) {
        if (apply)
            apply();
        return;
    }

    GLubyte targetOpacity = spr->getOpacity();
    if (auto stored = spr->getUserObject("fade-opacity"_spr))
        if (auto number = typeinfo_cast<cocos2d::CCInteger*>(stored))
            targetOpacity = static_cast<GLubyte>(number->getValue());

    if (!spr->isRunning() || !apply) {
        if (apply)
            apply();
        return;
    }

    spr->setUserObject("fade-opacity"_spr, cocos2d::CCInteger::create(targetOpacity));
    spr->stopActionByTag(BACKGROUND_FADE_TAG);

    auto sequence = cocos2d::CCSequence::create(
        cocos2d::CCFadeTo::create(BACKGROUND_FADE_TIME, 0),
        geode::cocos::CallFuncExt::create(std::move(apply)),
        cocos2d::CCFadeTo::create(BACKGROUND_FADE_TIME, targetOpacity),
        nullptr
    );
    sequence->setTag(BACKGROUND_FADE_TAG);
    spr->runAction(sequence);
}

inline bool restoreDefaultBackground(cocos2d::CCSprite* spr) {
    if (!spr)
        return false;

    auto snapshot = static_cast<BackgroundSnapshot*>(
        spr->getUserObject("default-background"_spr)
    );
    if (!snapshot)
        return false;

    runBackgroundFade(spr, [spr, snapshot]() {
        if (snapshot->frame)
            spr->setDisplayFrame(snapshot->frame);
        spr->setScaleX(snapshot->scaleX);
        spr->setScaleY(snapshot->scaleY);
        spr->setPosition(snapshot->position);
        // Restored backgrounds always show at full white, like applied ones.
        spr->setColor(ccc3(255, 255, 255));
    });
    return true;
}

// How an applied image is mapped onto the screen.
enum class ImagePlacement : int64_t {
    Stretch = 0, // stretch both axes independently to fill
    Fit = 1,     // long edge fits the window, short edge letterboxed
    Fill = 2,    // short edge fills the window, long edge overflows
};

// Placement is stored per image, keyed by the canonical relative path, e.g.
// {"Background/foo.png": 2}. The old single "image-placement" scalar is kept
// only as the default for images that have no entry yet, so existing installs
// keep their previous fit mode until each image is changed individually.
inline std::map<std::string, int64_t> getImagePlacements() {
    return Mod::get()->getSavedValue<std::map<std::string, int64_t>>("image-placements");
}

inline ImagePlacement getDefaultImagePlacement() {
    return static_cast<ImagePlacement>(
        Mod::get()->getSavedValue<int64_t>("image-placement", 0)
    );
}

// Looks up the fit mode of one image; falls back to the legacy default.
inline ImagePlacement getImagePlacementFor(std::string const& savedPath) {
    auto canonical = canonicalSavedPath(savedPath);
    auto placements = getImagePlacements();
    if (auto it = placements.find(canonical); it != placements.end())
        return static_cast<ImagePlacement>(it->second);
    return getDefaultImagePlacement();
}

inline ImagePlacement getImagePlacementForFile(asp::fs::Path const& file) {
    return getImagePlacementFor(pathToSaved(file));
}

inline void setImagePlacementFor(std::string const& savedPath, ImagePlacement placement) {
    auto canonical = canonicalSavedPath(savedPath);
    auto placements = getImagePlacements();
    placements[canonical] = static_cast<int64_t>(placement);
    Mod::get()->setSavedValue("image-placements", placements);
}

inline void setImagePlacementForFile(asp::fs::Path const& file, ImagePlacement placement) {
    setImagePlacementFor(pathToSaved(file), placement);
}

inline char const* imagePlacementName(ImagePlacement placement) {
    switch (placement) {
        case ImagePlacement::Fit: return "Fit";
        case ImagePlacement::Fill: return "Fill";
        default: return "Stretch";
    }
}

// Applies `path` to `spr` using that image's own placement mode.
// `animate == true`  : fade the old frame out, swap at the midpoint, fade in.
// `animate == false` : swap immediately (used when a covering background is
//                      created while invisible, then faded in by the caller,
//                      so the default material never flashes).
inline bool applyImageToSprite(
    CCSprite* spr,
    asp::fs::Path const& path,
    bool animate = true
) {
    if (!spr || path.empty() || !path.exists())
        return false;

    rememberDefaultBackground(spr);

    auto image = CCSprite::create(path.string().c_str());
    if (!image)
        return false;

    // A sprite that belongs to the interface itself (i.e. not a covering
    // background created by this mod) is tagged with the game's conventional
    // background ID once an image is applied to it, so the applied wallpaper
    // stays discoverable as "background". Mod-created coverings keep the
    // namespaced "background"_spr ID assigned by their callers.
    std::string ourID = "background"_spr;
    if (spr->getID() != ourID)
        spr->setID("background");

    // Keep the decoded sprite alive until the swap happens halfway through
    // the fade.
    geode::Ref<CCSprite> imageRef = image;
    auto placement = getImagePlacementForFile(path);

    auto applyNow = [spr, imageRef, placement]() {
        if (!imageRef)
            return;
        if (auto frame = imageRef->displayFrame())
            spr->setDisplayFrame(frame);

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto size = spr->getContentSize();
        if (size.width > 0.f && size.height > 0.f) {
            float scaleX = winSize.width / size.width;
            float scaleY = winSize.height / size.height;

            switch (placement) {
                case ImagePlacement::Fit: {
                    float scale = std::min(scaleX, scaleY);
                    scaleX = scaleY = scale;
                    break;
                }
                case ImagePlacement::Fill: {
                    float scale = std::max(scaleX, scaleY);
                    scaleX = scaleY = scale;
                    break;
                }
                default:
                    break; // Stretch: per-axis scale already computed
            }

            spr->setScaleX(scaleX);
            spr->setScaleY(scaleY);

            // Center the image on screen for every mode and anchor.
            auto anchor = spr->getAnchorPoint();
            spr->setPosition(ccp(
                winSize.width * 0.5f + (anchor.x - 0.5f) * size.width * scaleX,
                winSize.height * 0.5f + (anchor.y - 0.5f) * size.height * scaleY
            ));
        }
    };

    // A newly created covering background applies its first image instantly
    // while invisible; the caller fades it in afterwards. Fading the swap
    // here would briefly show the default material.
    if (!animate) {
        applyNow();
        return true;
    }

    runBackgroundFade(spr, applyNow);
    return true;
}

// ---------------------------------------------------------------------------
// background replacement used by all hooks
// ---------------------------------------------------------------------------

// `keyOverride` lets scoped call sites (per-level id, daily/weekly/event,
// gauntlet map, list name) use their real saved key instead of the layer's
// class name, which backgroundKeyFor would return for existing sprites.
inline bool tryReplace(
    CCSprite* spr,
    std::string subFolder,
    std::string fileName,
    std::string const& keyOverride = std::string()
) {
    if (!spr)
        return false;

    const std::string key = keyOverride.empty()
        ? backgroundKeyFor(spr, fileName)
        : keyOverride;
    const std::string quality = textureQualitySuffix();

    // A path explicitly chosen in the picker always wins.
    if (auto savedPath = savedToPath(getSavedBackground(key)); !savedPath.empty()) {
        if (applyImageToSprite(spr, savedPath)) {
            setBackgroundTag(spr, key);
            return true;
        }
    }

    // Otherwise fall back to the legacy per-folder layout, then flat files in
    // the config root (e.g. config/MenuLayer.png).
    const std::vector<asp::fs::Path> candidates = {
        configDir() / subFolder / (fileName + quality + ".png"),
        configDir() / subFolder / (fileName + ".png"),
        configDir() / (fileName + quality + ".png"),
        configDir() / (fileName + ".png"),
        configDir() / (key + quality + ".png"),
        configDir() / (key + ".png"),
    };

    for (auto const& path : candidates) {
        if (!path.exists())
            continue;
        if (applyImageToSprite(spr, path)) {
            setBackgroundTag(spr, key);
            return true;
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// in-game background picker
// ---------------------------------------------------------------------------

// Returns whether the shortcut should open the picker on the current screen.
bool canOpenBackgroundPopup();
// Returns whether the game is currently showing its loading screen.
bool isLoadingScreen();

class BackgroundPopup;

class BackgroundPreviewPopup : public Popup {
protected:
    asp::fs::Path m_file;
    BackgroundPopup* m_owner = nullptr;
    cocos2d::CCSprite* m_favoriteIcon = nullptr;
    cocos2d::CCSprite* m_applyIcon = nullptr;
    geode::LazySprite* m_image = nullptr;
    cocos2d::CCClippingNode* m_imageClip = nullptr;
    cocos2d::CCNode* m_imageFrame = nullptr;
    cocos2d::CCSize m_frameSize;
    cocos2d::CCMenu* m_buttonMenu = nullptr;
    cocos2d::CCMenu* m_renameMenu = nullptr;
    geode::TextInput* m_renameInput = nullptr;
    geode::Label* m_renameLabel = nullptr;
    geode::NineSlice* m_renameBg = nullptr;

    bool init(asp::fs::Path const& file, BackgroundPopup* owner);
    void loadImage();
    void updateImageLayout();
    bool isApplied() const;
    void updateApplyIcon();
    void enterRenameMode();
    void exitRenameMode();
    void updateRenameButtons();
    void onRename(cocos2d::CCObject* sender);
    void onRenameConfirm(cocos2d::CCObject* sender);
    void onExpand(cocos2d::CCObject* sender);
    void onCyclePlacement(cocos2d::CCObject* sender);
    void onApply(cocos2d::CCObject* sender);
    void onDelete(cocos2d::CCObject* sender);
    void onFavorite(cocos2d::CCObject* sender);
    void onRefresh(cocos2d::CCObject* sender);

public:
    static BackgroundPreviewPopup* create(asp::fs::Path const& file, BackgroundPopup* owner);
};

// One editable background scope on screens that support more than the plain
// interface background (LevelInfoLayer, GauntletLayer, LevelListLayer, ...).
struct BackgroundScope {
    std::string label; // text shown on the scope toggle
    std::string key;   // key used in saved.json
    int tier = 0;      // 0 = Layer, 1 = Daily/Weekly/Event/Gauntlet, 2 = specific
};

class BackgroundPopup : public Popup {
protected:
    cocos2d::CCNode* m_targetLayer = nullptr;
    cocos2d::CCSprite* m_bgSprite = nullptr;
    cocos2d::CCSprite* m_currentThumb = nullptr;
    geode::Label* m_currentNameLabel = nullptr;
    geode::Label* m_scopeDescription = nullptr;
    geode::Label* m_currentPathLabel = nullptr;
    geode::TextInput* m_filterInput = nullptr;
    cocos2d::CCNode* m_filterToggle = nullptr;
    cocos2d::CCNode* m_favoriteToggle = nullptr;
    geode::Label* m_scopeLabel = nullptr;
    geode::NineSlice* m_scopeBg = nullptr;
    std::vector<BackgroundScope> m_scopes;
    size_t m_scopeIndex = 0;
    geode::ScrollLayer* m_list = nullptr;
    geode::Label* m_emptyLabel = nullptr;
    cocos2d::CCNode* m_spinner = nullptr;

    std::string m_layerKey;     // key used in saved.json
    std::string m_layerDisplay; // name shown in the header
    std::vector<asp::fs::Path> m_files;        // every scanned image
    std::vector<asp::fs::Path> m_visibleFiles; // files currently in the list
    std::string m_filterText;
    bool m_pathLabelGold = false;
    bool m_loadingSettings = false;
    bool m_scanning = false;
    bool m_favoritesOnly = false;
    bool m_filterEnabled = false;

    bool init() override;
    void startScan();
    void onScanFinished(std::vector<asp::fs::Path> files);
    void rebuildList(bool preserveScroll = false);
    cocos2d::CCNode* createCell(asp::fs::Path const& file, int index);
    void updateCurrentPreview();
    void updateScopeUi();
    asp::fs::Path fileForSender(cocos2d::CCObject* sender);
    void askEnableExtraLayer(asp::fs::Path const& file, char const* setting);
    void askDisableExtraLayer(char const* setting);

public:
    static BackgroundPopup* create();

    void rescan();
    void forgetFile(asp::fs::Path const& file);
    void applyFile(asp::fs::Path const& file);
    void refreshFile(asp::fs::Path const& file);

    void onClose(cocos2d::CCObject* sender) override;
    void onPreview(cocos2d::CCObject* sender);
    void onApply(cocos2d::CCObject* sender);
    void onDelete(cocos2d::CCObject* sender);
    void onRefresh(cocos2d::CCObject* sender);
    void onFavorite(cocos2d::CCObject* sender);
    void onToggleFavorites(cocos2d::CCObject* sender);
    void onCycleScope(cocos2d::CCObject* sender);
    void onToggleFilter(cocos2d::CCObject* sender);
    void onFilterChanged(std::string const& text);
    void onRescan(cocos2d::CCObject* sender);
    void onOpenConfigFolder(cocos2d::CCObject* sender);
    void clearBackground();

    friend class BackgroundPreviewPopup;
};
