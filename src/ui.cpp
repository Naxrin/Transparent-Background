#include "head.hpp"

#include <Geode/ui/SceneEvent.hpp>
#include <Geode/utils/ColorProvider.hpp>
#include <asp/fs/fs.hpp>
#include <cstdlib>
#include <functional>
#include <thread>

using namespace geode::prelude;

// Interfaces whose default image lives at config/<interface>.png. The keys
// match either the filename passed to tryReplace (for freshly created
// backgrounds) or the layer class name (for backgrounds owned by the game).
static const std::vector<std::string> INTERFACE_KEYS = {
    // base game
    "MenuLayer",
    "LoadingLayer",
    "LevelSelectLayer",
    "CreatorLayer",
    "GJGarageLayer",
    "EditLevelLayer",
    "LevelBrowserLayer",
    "LevelInfoLayer",
    "LevelSearchLayer",
    "LeaderboardsLayer",
    "LevelListLayer",
    "GauntletSelectLayer",
    "GauntletLayer",

    // mods, backgrounds created by createLayerBG -> key is the fileName
    "geode.loader-ModsLayer",
    "cvolton.bettterinfo-CustomCreatorLayer",
    "cvolton.bettterinfo-DailyViewLayer",
    "cvolton.bettterinfo-LevelSearchViewLayer",
    "cvolton.bettterinfo-LeaderboardViewLayer",
    "cvolton.bettterinfo-RewardGroupLayer",
    "cvolton.bettterinfo-RewardViewLayer",
    "dankmeme.globed2-GlobedMenuLayer",
    "dankmeme.globed2-GlobedServersLayer",
    "omgrod.garage_plus-GPFeedbackLayer",
    "omgrod.garage_plus-GPKofiLayer",
    "uproxide.textures-TextureWorkshopLayer",
    "lblazen.gdps_hub-GDPSHubLayer",

    // mods, backgrounds owned by the layer -> key is the class name
    "PackSelectLayer",
    "GlobedLevelListLayer",
    "GlobedSettingsLayer",
    "ServerSwitchLayer",
    "DPLayer",
    "DPListLayer",
    "DPSearchLayer",
    "RecommendedLayer",
    "RouletteSafeLayer",
    "MoreLeaderboards",
    "IDListLayer",
    "IDPackLayer",
    "GDCPListLayer",
    "BetterAchievementLayer",
    "NewgroundsBrowserLayer",
    "SecretLayer6R",
};

// On first load of a new version, remember the image each interface would use.
// Missing files are recorded as an empty string, which means "use the default".
static void initSavedBackgrounds() {
    auto backgrounds = getSavedBackgrounds();
    bool changed = false;

    for (auto const& key : INTERFACE_KEYS) {
        if (backgrounds.contains(key))
            continue;

        asp::fs::Path found;
        const std::vector<std::string> suffixes = {std::string(), textureQualitySuffix()};
        for (auto const& suffix : suffixes) {
            auto candidate = configDir() / (key + suffix + ".png");
            if (candidate.exists()) {
                found = candidate;
                break;
            }
        }

        backgrounds[key] = found.empty() ? "" : pathToSaved(found);
        changed = true;
    }

    if (changed) {
        Mod::get()->setSavedValue("backgrounds", backgrounds);
        (void)Mod::get()->saveData();
    }
}

// Shrink a TextInput's background height and font to 80% without scaling the
// TextInput node itself: scaling the node (or any of its ancestors) makes
// CCTextInputNode place the caret at the wrong spot. Resizing the background
// nine-slice and lowering the label scales only affect drawing, so the touch
// mapping used by the caret stays correct.
static void shrinkTextInputVisuals(geode::TextInput* input) {
    if (!input)
        return;

    if (auto background = input->getBGSprite()) {
        auto size = background->getContentSize();
        background->setContentSize({size.width, size.height * 0.8f});
    }

    if (auto node = input->getInputNode()) {
        node->setLabelPlaceholderScale(0.4f); // default 0.5
        node->setMaxLabelScale(0.48f);        // default 0.6
        // Re-apply the scales to the already-created placeholder label.
        node->refreshLabel();
        node->updateLabel(node->getString());
    }
}

static geode::ListenerHandle g_geodeBackgroundListener;
static geode::ListenerHandle g_sceneListener;

// Geode's theme background for the mod browser (SwelvyBG or the selected
// theme's background), remembered from the background-providing event.
static cocos2d::CCNode* g_geodeThemeBackground = nullptr;

static cocos2d::CCSprite* attachModsLayerBackground(cocos2d::CCNode* modsLayer);
static char const* extraLayerSettingKey(cocos2d::CCNode* layer);
static void migrateBackgroundStorage();

$on_mod(Loaded) {
    initSavedBackgrounds();

    // Geode provides the mod browser's theme background through this event.
    // Remember it; the scene listener below wraps it together with our image
    // directly under the ModsLayer node itself.
    g_geodeBackgroundListener = geode::NodeProvidingEvent("geode.loader/geode-background").listen(
        [](cocos2d::CCNode*& nodeOut, std::string_view) {
            g_geodeThemeBackground = nodeOut;
        },
        // Run after Geode's own "Last" listener so its theme background is
        // created first.
        geode::Priority::Last + 1
    );

    // Add the background under ModsLayer itself once the new scene is ready.
    g_sceneListener = geode::SceneEvent().listen([](cocos2d::CCScene* scene) {
        // The game managers are initialized by the time the first scene is
        // switched, so migrate old saved keys here as well.
        migrateBackgroundStorage();

        if (!scene)
            return;

        // Recursive in case this is a transition scene wrapping the real one.
        auto modsLayer = scene->getChildByIDRecursive("ModsLayer");
        if (!modsLayer) {
            g_geodeThemeBackground = nullptr;
            return;
        }

        attachModsLayerBackground(modsLayer);
    });
}

// Only accept files whose path inside the config folder is pure ASCII.
static bool isAsciiConfigPath(asp::fs::Path const& path) {
    std::string relative;
    if (auto rel = path.lexicallyRelative(configDir()); rel && !rel->empty())
        relative = rel->string();
    else
        relative = path.string();

    for (unsigned char c : relative) {
        if (c < 0x20 || c > 0x7E)
            return false;
    }
    return true;
}

// Deletes the file and removes every saved reference to it.
static bool deleteImageFile(asp::fs::Path const& file) {
    if (asp::fs::removeFile(file).isErr())
        return false;

    auto savedPath = pathToSaved(file);
    auto canonical = canonicalSavedPath(savedPath);
    setDisplayName(savedPath, "");
    setFavorite(savedPath, false);

    auto backgrounds = getSavedBackgrounds();
    auto removed = std::erase_if(backgrounds, [&](auto const& entry) {
        return canonicalSavedPath(entry.second) == canonical;
    });
    if (removed)
        Mod::get()->setSavedValue("backgrounds", backgrounds);

    auto placements = getImagePlacements();
    if (placements.erase(canonical) > 0)
        Mod::get()->setSavedValue("image-placements", placements);

    (void)Mod::get()->saveData();
    return true;
}

static NineSlice* createButtonFrame(float width, float height) {
    auto background = NineSlice::create("square02d_001.png");
    if (!background)
        background = NineSlice::create("square02b_001.png");
    if (!background)
        return nullptr;

    background->setContentSize({width, height});
    return background;
}

// Creates a square icon button using square02d_001.png as the frame.
static CCMenuItemSpriteExtra* createIconButton(
    char const* iconName,
    float size,
    cocos2d::CCObject* target,
    cocos2d::SEL_MenuHandler handler,
    cocos2d::ccColor3B iconColor = ccWHITE,
    cocos2d::CCSprite** outIcon = nullptr,
    cocos2d::ccColor3B backgroundColor = ccWHITE,
    NineSlice** outBackground = nullptr
) {
    auto container = CCNode::create();
    container->setContentSize({size, size});

    if (auto background = createButtonFrame(size, size)) {
        background->setColor(backgroundColor);
        container->addChildAtPosition(background, Anchor::Center, ccp(0.f, 0.f), ccp(0.5f, 0.5f));
        if (outBackground)
            *outBackground = background;
    }

    auto frame = CCSpriteFrameCache::get()->spriteFrameByName(iconName);
    auto icon = frame ? CCSprite::createWithSpriteFrame(frame) : nullptr;

    if (icon) {
        float width = icon->getContentSize().width;
        float height = icon->getContentSize().height;
        if (width > 0.f && height > 0.f)
            icon->setScale(std::min((size - 6.f) / width, (size - 6.f) / height));

        icon->setColor(iconColor);
        container->addChildAtPosition(icon, Anchor::Center, ccp(0.f, 0.f), ccp(0.5f, 0.5f));

        if (outIcon)
            *outIcon = icon;
    }

    return CCMenuItemSpriteExtra::create(container, target, handler);
}

static std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

static std::string trimAscii(std::string value) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

// Decodes the file on a worker thread to make sure the game can actually load
// it as an image; broken files never make it into the picker. Uses the same
// code path as LazySprite (read bytes + CCImage), which also auto-detects the
// format from the data.
static bool canDecodeImage(asp::fs::Path const& path) {
    auto result = utils::file::readBinary(path);
    if (result.isErr())
        return false;

    auto data = std::move(result).unwrap();
    if (data.empty())
        return false;

    auto image = new cocos2d::CCImage();
    bool decoded = image->initWithImageData(
        data.data(),
        static_cast<int>(data.size()),
        cocos2d::CCImage::kFmtUnKnown
    );
    image->release();
    return decoded;
}

static constexpr int TOGGLE_BG_OFF_TAG = 1;
static constexpr int TOGGLE_BG_ON_TAG = 2;

static void setToggleState(cocos2d::CCNode* toggle, bool enabled) {
    if (!toggle)
        return;
    if (auto off = toggle->getChildByTag(TOGGLE_BG_OFF_TAG))
        off->setVisible(!enabled);
    if (auto on = toggle->getChildByTag(TOGGLE_BG_ON_TAG))
        on->setVisible(enabled);
}

// Builds a fixed square toggle button out of a container: both button
// background textures are kept as siblings and toggled by visibility, so the
// background can change without altering the button's size. The container is
// the menu item's image, which keeps the click animation centered.
static CCMenuItemSpriteExtra* createToggleButton(
    char const* iconName,
    bool enabled,
    cocos2d::CCObject* target,
    cocos2d::SEL_MenuHandler handler,
    cocos2d::CCNode** outToggle
) {
    constexpr float SIZE = 24.f;
    constexpr float ICON_SIZE = 14.f;

    auto frame = CCSpriteFrameCache::get()->spriteFrameByName(iconName);
    if (!frame)
        return nullptr;

    auto icon = CCSprite::createWithSpriteFrame(frame);
    if (!icon)
        return nullptr;

    auto container = CCNode::create();
    container->setContentSize({SIZE, SIZE});

    constexpr float BG_SCALE = 0.6f;
    auto addBackground = [&](int tag, char const* frameName, bool visible) {
        auto bgFrame = CCSpriteFrameCache::get()->spriteFrameByName(frameName);
        if (!bgFrame)
            return;

        auto background = CCSprite::createWithSpriteFrame(bgFrame);
        if (!background)
            return;

        background->setScale(BG_SCALE);
        background->setAnchorPoint(ccp(0.5f, 0.5f));
        background->setPosition(ccp(SIZE / 2.f, SIZE / 2.f));
        background->setTag(tag);
        background->setVisible(visible);
        container->addChild(background);
    };

    addBackground(TOGGLE_BG_OFF_TAG, "geode.loader/baseCircle_Small_Green.png", !enabled);
    addBackground(TOGGLE_BG_ON_TAG, "geode.loader/baseCircle_Small_Blue.png", enabled);

    auto iconSize = icon->getContentSize();
    if (iconSize.width > 0.f && iconSize.height > 0.f) {
        icon->setScale(std::min(
            ICON_SIZE / iconSize.width,
            ICON_SIZE / iconSize.height
        ));
    }
    icon->setAnchorPoint(ccp(0.5f, 0.5f));
    icon->setPosition(ccp(SIZE / 2.f, SIZE / 2.f));
    container->addChild(icon);

    auto item = CCMenuItemSpriteExtra::create(container, target, handler);
    item->setContentSize({SIZE, SIZE});

    if (outToggle)
        *outToggle = container;
    return item;
}

// Fixed square button with a single green circle background.
static CCMenuItemSpriteExtra* createGreenCircleButton(
    char const* iconName,
    float iconSize,
    cocos2d::CCObject* target,
    cocos2d::SEL_MenuHandler handler
) {
    constexpr float SIZE = 24.f;

    auto frame = CCSpriteFrameCache::get()->spriteFrameByName(iconName);
    if (!frame)
        return nullptr;

    auto icon = CCSprite::createWithSpriteFrame(frame);
    if (!icon)
        return nullptr;

    auto container = CCNode::create();
    container->setContentSize({SIZE, SIZE});

    if (auto bgFrame = CCSpriteFrameCache::get()->spriteFrameByName(
            "geode.loader/baseCircle_Small_Green.png")) {
        auto background = CCSprite::createWithSpriteFrame(bgFrame);
        if (background) {
            background->setScale(0.6f);
            background->setAnchorPoint(ccp(0.5f, 0.5f));
            background->setPosition(ccp(SIZE / 2.f, SIZE / 2.f));
            container->addChild(background);
        }
    }

    auto size = icon->getContentSize();
    if (size.width > 0.f && size.height > 0.f) {
        icon->setScale(std::min(
            iconSize / size.width,
            iconSize / size.height
        ));
    }
    icon->setAnchorPoint(ccp(0.5f, 0.5f));
    icon->setPosition(ccp(SIZE / 2.f, SIZE / 2.f));
    container->addChild(icon);

    auto item = CCMenuItemSpriteExtra::create(container, target, handler);
    item->setContentSize({SIZE, SIZE});
    return item;
}

// Bare icon button without a background frame. The item keeps a square slot
// and the icon is centered inside it.
static CCMenuItemSpriteExtra* createBareButton(
    char const* iconName,
    float slotSize,
    float iconSize,
    cocos2d::CCObject* target,
    cocos2d::SEL_MenuHandler handler
) {
    auto frame = CCSpriteFrameCache::get()->spriteFrameByName(iconName);
    if (!frame)
        return nullptr;

    auto icon = CCSprite::createWithSpriteFrame(frame);
    if (!icon)
        return nullptr;

    auto size = icon->getContentSize();
    if (size.width > 0.f && size.height > 0.f) {
        icon->setScale(std::min(
            iconSize / size.width,
            iconSize / size.height
        ));
    }

    auto item = CCMenuItemSpriteExtra::create(icon, target, handler);
    item->setContentSize({slotSize, slotSize});
    icon->setAnchorPoint(ccp(0.5f, 0.5f));
    icon->setPosition(ccp(slotSize / 2.f, slotSize / 2.f));

    return item;
}

// Bare icon button with an explicit sprite scale, centered in a square slot.
static CCMenuItemSpriteExtra* createScaledIconButton(
    char const* iconName,
    float iconScale,
    float slotSize,
    cocos2d::CCObject* target,
    cocos2d::SEL_MenuHandler handler,
    cocos2d::CCSprite** outIcon = nullptr
) {
    auto frame = CCSpriteFrameCache::get()->spriteFrameByName(iconName);
    if (!frame)
        return nullptr;

    auto icon = CCSprite::createWithSpriteFrame(frame);
    if (!icon)
        return nullptr;

    icon->setScale(iconScale);

    auto item = CCMenuItemSpriteExtra::create(icon, target, handler);
    item->setContentSize({slotSize, slotSize});
    icon->setAnchorPoint(ccp(0.5f, 0.5f));
    icon->setPosition(ccp(slotSize / 2.f, slotSize / 2.f));

    if (outIcon)
        *outIcon = icon;
    return item;
}

// ---------------------------------------------------------------------------
// preview window
// ---------------------------------------------------------------------------

// Fits the largest box with the current game screen's aspect ratio inside the
// given bounds, so previews look like they would after being applied.
static cocos2d::CCSize fitToScreenAspect(float maxWidth, float maxHeight) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (winSize.width <= 0.f || winSize.height <= 0.f)
        return {maxWidth, maxHeight};

    float aspect = winSize.width / winSize.height;
    float width = maxWidth;
    float height = width / aspect;
    if (height > maxHeight) {
        height = maxHeight;
        width = height * aspect;
    }
    return {width, height};
}

bool BackgroundPreviewPopup::init(asp::fs::Path const& file, BackgroundPopup* owner) {
    if (!Popup::init(420.f, 280.f))
        return false;

    m_file = file;
    m_owner = owner;

    auto savedPath = pathToSaved(file);
    std::string fileName = file.filename() ? std::string(file.filename()->string()) : file.string();
    auto displayName = getDisplayName(savedPath);
    this->setTitle(displayName.empty() ? displayFileName(fileName) : displayName);

    loadImage();

    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setContentSize({420.f, 280.f});
    m_buttonMenu->setAnchorPoint(ccp(0.f, 0.f));
    m_buttonMenu->setPosition(ccp(0.f, 0.f));
    m_mainLayer->addChild(m_buttonMenu);

    constexpr float SLOT = 30.f;
    constexpr float GAP = 8.f;
    constexpr float ROW_Y = 25.f;

    float x = (420.f - (SLOT * 7.f + GAP * 6.f)) / 2.f;
    auto addRowButton = [&](CCMenuItemSpriteExtra* item) {
        if (!item)
            return;
        item->setPosition(ccp(x + SLOT / 2.f, ROW_Y));
        m_buttonMenu->addChild(item);
        x += SLOT + GAP;
    };

    // Favorite (bare)
    addRowButton(createScaledIconButton(
        isFavorite(savedPath) ? "gj_heartOn_001.png" : "gj_heartOff_001.png",
        0.9f, SLOT, this, menu_selector(BackgroundPreviewPopup::onFavorite),
        &m_favoriteIcon
    ));
    // Apply this image to every interface
    addRowButton(createScaledIconButton(
        "GJ_shareBtn_001.png", 0.35f, SLOT, this,
        menu_selector(BackgroundPreviewPopup::onExpand)
    ));
    // Cycle the image placement mode
    addRowButton(createScaledIconButton(
        "GJ_editModeBtn_001.png", 0.6f, SLOT, this,
        menu_selector(BackgroundPreviewPopup::onCyclePlacement)
    ));
    // Apply, using the same texture as the list cells
    addRowButton(createScaledIconButton(
        isApplied() ? "GJ_selectSongOnBtn_001.png" : "GJ_selectSongBtn_001.png",
        0.9f, SLOT, this, menu_selector(BackgroundPreviewPopup::onApply),
        &m_applyIcon
    ));
    // Rename
    addRowButton(createScaledIconButton(
        "GJ_viewLevelsBtn_001.png", 0.6f, SLOT, this,
        menu_selector(BackgroundPreviewPopup::onRename)
    ));
    // Refresh (bare)
    addRowButton(createScaledIconButton(
        "GJ_updateBtn_001.png", 0.6f, SLOT, this,
        menu_selector(BackgroundPreviewPopup::onRefresh)
    ));
    // Delete
    addRowButton(createScaledIconButton(
        "GJ_deleteBtn_001.png", 0.6f, SLOT, this,
        menu_selector(BackgroundPreviewPopup::onDelete)
    ));

    // Rename bar: input plus a single Rename/Cancel button, hidden until the
    // rename button is pressed.
    // Keep the input node unscaled: scaling it breaks the cursor placement in
    // CCTextInputNode. Shrink only the background/font visuals instead.
    m_renameInput = TextInput::create(328.f, "New name", "bigFont.fnt");
    shrinkTextInputVisuals(m_renameInput);
    m_renameInput->setMaxCharCount(30);
    {
        std::string asciiAllowed;
        for (int c = 32; c <= 126; ++c)
            asciiAllowed.push_back(static_cast<char>(c));
        m_renameInput->setFilter(std::move(asciiAllowed));
    }
    m_renameInput->setCallback([this](std::string const&) {
        this->updateRenameButtons();
    });
    m_renameInput->setVisible(false);
    m_mainLayer->addChildAtPosition(
        m_renameInput, Anchor::Center, ccp(-35.f, -116.f), ccp(0.5f, 0.5f)
    );

    m_renameMenu = CCMenu::create();
    m_renameMenu->setContentSize({420.f, 280.f});
    m_renameMenu->setAnchorPoint(ccp(0.f, 0.f));
    m_renameMenu->setPosition(ccp(0.f, 0.f));
    m_renameMenu->setVisible(false);
    m_mainLayer->addChild(m_renameMenu);

    // Single button whose label and background tint switch between
    // Rename (green) and Cancel (gray).
    constexpr float CONFIRM_WIDTH = 70.f;
    constexpr float CONFIRM_HEIGHT = 30.f;

    auto confirmNode = CCNode::create();
    confirmNode->setContentSize({CONFIRM_WIDTH, CONFIRM_HEIGHT});

    if (auto background = NineSlice::create("square02d_001.png")) {
        background->setContentSize({CONFIRM_WIDTH * 2.f, CONFIRM_HEIGHT * 2.f});
        background->setScale(0.5f);
        background->setAnchorPoint(ccp(0.5f, 0.5f));
        background->setPosition(ccp(CONFIRM_WIDTH / 2.f, CONFIRM_HEIGHT / 2.f));
        confirmNode->addChild(background);
        m_renameBg = background;
    }

    m_renameLabel = Label::create("Rename", "bigFont.fnt");
    // 0.8 is the target size; shrink only if the word would not fit the fixed
    // button width (the two captions have different lengths).
    m_renameLabel->setLimitLabelWidth(CONFIRM_WIDTH - 10.f, 0.8f, 0.4f);
    m_renameLabel->setPosition(ccp(CONFIRM_WIDTH / 2.f, CONFIRM_HEIGHT / 2.f));
    confirmNode->addChild(m_renameLabel);

    auto confirmItem = CCMenuItemSpriteExtra::create(
        confirmNode, this,
        menu_selector(BackgroundPreviewPopup::onRenameConfirm)
    );
    confirmItem->setPosition(ccp(380.f, 24.f));
    m_renameMenu->addChild(confirmItem);

    return true;
}

void BackgroundPreviewPopup::loadImage() {
    if (m_image) {
        m_image->removeFromParent();
        m_image = nullptr;
    }
    if (m_imageClip) {
        m_imageClip->removeFromParent();
        m_imageClip = nullptr;
    }
    if (m_imageFrame) {
        m_imageFrame->removeFromParent();
        m_imageFrame = nullptr;
    }

    // Screen boundary for the preview: a solid black backing extending 2
    // units past the frame on every side, shifted up 4 pixels with the frame.
    m_frameSize = fitToScreenAspect(380.f, 190.f);

    constexpr float PREVIEW_Y_OFFSET = 4.f;
    auto contentSize = m_mainLayer->getContentSize();
    auto frame = CCLayerColor::create(
        ccColor4B{0, 0, 0, 255},
        m_frameSize.width + 4.f,
        m_frameSize.height + 4.f
    );
    if (frame) {
        // CCLayerColor ignores its anchor point; position its bottom-left.
        frame->setPosition(ccp(
            contentSize.width * 0.5f - (m_frameSize.width + 4.f) * 0.5f,
            contentSize.height * 0.5f + PREVIEW_Y_OFFSET
                - (m_frameSize.height + 4.f) * 0.5f
        ));
        m_imageFrame = frame;
        m_mainLayer->addChild(frame);
    }

    // Clip the image to the screen rect so Fill-mode overflow stays hidden.
    auto stencil = CCSprite::create("geode.loader/white-square.png");
    if (stencil) {
        auto size = stencil->getContentSize();
        if (size.width > 0.f && size.height > 0.f) {
            stencil->setScaleX(m_frameSize.width / size.width);
            stencil->setScaleY(m_frameSize.height / size.height);
        }
        stencil->setAnchorPoint(ccp(0.f, 0.f));
        stencil->setPosition(ccp(0.f, 0.f));
    }

    auto clip = CCClippingNode::create(stencil);
    clip->setContentSize(m_frameSize);
    clip->setAnchorPoint(ccp(0.5f, 0.5f));
    m_imageClip = clip;
    m_mainLayer->addChildAtPosition(
        clip, Anchor::Center, ccp(0.f, PREVIEW_Y_OFFSET), ccp(0.5f, 0.5f)
    );

    auto image = LazySprite::create(m_frameSize, true);
    image->setAutoResize(false);
    image->setAnchorPoint(ccp(0.5f, 0.5f));
    image->setPosition(ccp(m_frameSize.width / 2.f, m_frameSize.height / 2.f));

    // The callback must be installed before loading: a texture-cache hit
    // finishes synchronously inside loadFromFile.
    image->setLoadCallback([this](Result<> result) {
        if (result.isErr())
            return;
        this->updateImageLayout();
    });

    m_image = image;
    clip->addChild(image);
    image->loadFromFile(m_file);
}

void BackgroundPreviewPopup::updateImageLayout() {
    if (!m_image)
        return;

    auto size = m_image->getContentSize();
    if (size.width <= 0.f || size.height <= 0.f)
        return;

    float scaleX = m_frameSize.width / size.width;
    float scaleY = m_frameSize.height / size.height;

    switch (getImagePlacementForFile(m_file)) {
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
            break;
    }

    m_image->setScaleX(scaleX);
    m_image->setScaleY(scaleY);
    m_image->setAnchorPoint(ccp(0.5f, 0.5f));
    m_image->setPosition(ccp(m_frameSize.width / 2.f, m_frameSize.height / 2.f));
}

BackgroundPreviewPopup* BackgroundPreviewPopup::create(asp::fs::Path const& file, BackgroundPopup* owner) {
    auto ret = new BackgroundPreviewPopup();
    if (ret && ret->init(file, owner)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool BackgroundPreviewPopup::isApplied() const {
    if (!m_owner)
        return false;

    // The apply button reflects the saved configuration, even while the
    // interface's background display is still turned off: applying records
    // the image first and then offers to enable the display.
    auto saved = getSavedBackground(m_owner->m_layerKey);
    return !saved.empty()
        && canonicalSavedPath(saved) == canonicalSavedPath(pathToSaved(m_file));
}

void BackgroundPreviewPopup::updateApplyIcon() {
    if (!m_applyIcon)
        return;

    auto frameName = isApplied() ? "GJ_selectSongOnBtn_001.png" : "GJ_selectSongBtn_001.png";
    if (auto frame = CCSpriteFrameCache::get()->spriteFrameByName(frameName))
        m_applyIcon->setDisplayFrame(frame);
}

void BackgroundPreviewPopup::updateRenameButtons() {
    if (!m_renameInput)
        return;

    bool hasText = !trimAscii(m_renameInput->getString()).empty();

    if (m_renameLabel)
        m_renameLabel->setString(hasText ? "Rename" : "Cancel");
    if (m_renameBg) {
        m_renameBg->setColor(hasText
            ? ccc3(70, 200, 70)
            : ccc3(125, 125, 125));
    }
}

void BackgroundPreviewPopup::enterRenameMode() {
    if (!m_buttonMenu || !m_renameInput || !m_renameMenu)
        return;

    auto savedPath = pathToSaved(m_file);
    auto name = getDisplayName(savedPath);
    if (name.empty() && m_file.filename())
        name = displayFileName(m_file.filename()->string());

    m_renameInput->setString(name);
    updateRenameButtons();

    m_buttonMenu->setVisible(false);
    m_renameInput->setVisible(true);
    m_renameMenu->setVisible(true);
    m_renameInput->focus();
}

void BackgroundPreviewPopup::exitRenameMode() {
    if (m_renameInput) {
        m_renameInput->defocus();
        m_renameInput->setVisible(false);
    }
    if (m_renameMenu)
        m_renameMenu->setVisible(false);
    if (m_buttonMenu)
        m_buttonMenu->setVisible(true);
}

void BackgroundPreviewPopup::onRename(cocos2d::CCObject*) {
    enterRenameMode();
}

void BackgroundPreviewPopup::onRenameConfirm(cocos2d::CCObject*) {
    if (!m_renameInput)
        return;

    auto name = truncateName(trimAscii(m_renameInput->getString()));
    if (name.empty()) {
        exitRenameMode();
        return;
    }

    // Only the display name is saved; the file on disk keeps its name.
    auto savedPath = pathToSaved(m_file);
    setDisplayName(savedPath, name);
    (void)Mod::get()->saveData();

    this->setTitle(name);
    if (m_owner) {
        m_owner->updateCurrentPreview();
        m_owner->rebuildList(true);
    }

    exitRenameMode();
}

// Defined further below; declared here so the expand action can re-resolve
// which saved background is actually in effect after writing the new keys.
static BackgroundScope const* effectiveScopeFor(
    std::vector<BackgroundScope> const& scopes,
    std::string const& layerKey,
    std::string& savedOut
);

// Tint used when a background is applied to the live screen. Daily/weekly/
// event and gauntlet levels keep their dark look; The Safe's top two
// (Daily/Weekly) tabs do the same on the level browser.
static cocos2d::ccColor3B backgroundTintFor(cocos2d::CCNode* layer) {
    if (!Mod::get()->getSettingValue<bool>("dark"))
        return ccc3(255, 255, 255);

    if (auto info = typeinfo_cast<LevelInfoLayer*>(layer)) {
        if (auto level = info->m_level) {
            if (level->m_gauntletLevel || level->m_gauntletLevel2
                || level->m_dailyID.value() > 0) {
                return ccc3(100, 100, 100);
            }
        }
    }

    if (auto browser = typeinfo_cast<LevelBrowserLayer*>(layer)) {
        auto type = browser->m_searchObject
            ? browser->m_searchObject->m_searchType
            : SearchType::Search;
        if (type == SearchType::DailySafe || type == SearchType::WeeklySafe)
            return ccc3(100, 100, 100);
    }

    return ccc3(255, 255, 255);
}

void BackgroundPreviewPopup::onExpand(cocos2d::CCObject*) {
    if (!m_owner)
        return;

    auto savedPath = pathToSaved(m_file);
    auto name = getDisplayName(savedPath);
    if (name.empty() && m_file.filename())
        name = displayFileName(m_file.filename()->string());

    // When the selected scope is above Layer (Daily, Level, Map, List, ...),
    // expanding only fills in this interface: the dedicated scope and the
    // interface's Layer entry. Other interfaces keep their own backgrounds.
    bool scoped = !m_owner->m_scopes.empty()
        && m_owner->m_scopeIndex < m_owner->m_scopes.size()
        && m_owner->m_scopes[m_owner->m_scopeIndex].tier > 0;

    if (scoped) {
        std::string layerScopeKey;
        for (auto const& scope : m_owner->m_scopes) {
            if (scope.tier == 0) {
                layerScopeKey = scope.key;
                break;
            }
        }

        createQuickPopup(
            "Expand Background",
            fmt::format(
                "Are you sure to expand this background to all of <cy>{}</c>?",
                m_owner->m_layerDisplay
            ),
            "Cancel", "Expand",
            420.f,
            [this, savedPath, layerScopeKey](FLAlertLayer*, bool confirmed) {
                if (!confirmed || !m_owner)
                    return;

                // Write the dedicated scope and the interface's Layer entry,
                // leaving every other interface untouched.
                setSavedBackground(m_owner->m_layerKey, savedPath);
                if (!layerScopeKey.empty()
                    && layerScopeKey != m_owner->m_layerKey) {
                    setSavedBackground(layerScopeKey, savedPath);
                }
                (void)Mod::get()->saveData();

                if (m_owner->m_bgSprite) {
                    // Re-resolve what is actually in effect after writing, so
                    // the screen matches the Current preview even if a
                    // higher-priority scope still overrides this one.
                    std::string effectiveSaved;
                    auto effective = effectiveScopeFor(
                        m_owner->m_scopes, m_owner->m_layerKey, effectiveSaved
                    );
                    auto path = savedToPath(effectiveSaved);
                    if (!path.empty() && path.exists()
                        && applyImageToSprite(m_owner->m_bgSprite, path)) {
                        if (effective) {
                            setBackgroundTag(
                                m_owner->m_bgSprite, effective->key
                            );
                        }
                        m_owner->m_bgSprite->setColor(
                            backgroundTintFor(m_owner->m_targetLayer)
                        );
                    }
                    else {
                        restoreDefaultBackground(m_owner->m_bgSprite);
                    }
                }

                m_owner->updateCurrentPreview();
                m_owner->updateScopeUi();
                m_owner->rebuildList(true);

                updateApplyIcon();
                Notification::create(
                    fmt::format("Expanded to all of {}", m_owner->m_layerDisplay),
                    NotificationIcon::Success
                )->show();
            }
        );
        return;
    }

    createQuickPopup(
        "Apply to All Interfaces",
        fmt::format(
            "Apply <cy>{}</c> to every interface? This overwrites the "
            "background picked for each screen.",
            name
        ),
        "Cancel", "Apply",
        420.f,
        [this, savedPath](FLAlertLayer*, bool confirmed) {
            if (!confirmed || !m_owner)
                return;

            if (m_owner->m_bgSprite) {
                if (!applyImageToSprite(m_owner->m_bgSprite, m_file)) {
                    Notification::create(
                        "Failed to load image", NotificationIcon::Error
                    )->show();
                    return;
                }
                setBackgroundTag(m_owner->m_bgSprite, m_owner->m_layerKey);
                m_owner->m_bgSprite->setColor(ccc3(255, 255, 255));
            }

            auto backgrounds = getSavedBackgrounds();
            backgrounds[m_owner->m_layerKey] = savedPath;
            for (auto& entry : backgrounds)
                entry.second = savedPath;
            Mod::get()->setSavedValue("backgrounds", backgrounds);
            (void)Mod::get()->saveData();

            m_owner->updateCurrentPreview();
            m_owner->rebuildList(true);

            updateApplyIcon();
            Notification::create(
                "Applied to all interfaces", NotificationIcon::Success
            )->show();
        }
    );
}

void BackgroundPreviewPopup::onCyclePlacement(cocos2d::CCObject*) {
    auto placement = getImagePlacementForFile(m_file);
    switch (placement) {
        case ImagePlacement::Stretch: placement = ImagePlacement::Fit; break;
        case ImagePlacement::Fit: placement = ImagePlacement::Fill; break;
        case ImagePlacement::Fill: placement = ImagePlacement::Stretch; break;
    }
    setImagePlacementForFile(m_file, placement);
    (void)Mod::get()->saveData();

    // Keep the preview window in sync with the placement mode.
    updateImageLayout();

    // Re-apply the current background so the new placement shows immediately.
    if (m_owner) {
        auto saved = getSavedBackground(m_owner->m_layerKey);
        if (!saved.empty() && m_owner->m_bgSprite) {
            auto path = savedToPath(saved);
            if (!path.empty() && path.exists())
                applyImageToSprite(m_owner->m_bgSprite, path);
        }
    }

    Notification::create(
        fmt::format("Image placement: {}", imagePlacementName(placement)),
        NotificationIcon::Info
    )->show();
}

void BackgroundPreviewPopup::onApply(cocos2d::CCObject*) {
    if (!m_owner)
        return;

    if (isApplied())
        m_owner->clearBackground();
    else
        m_owner->applyFile(m_file);

    updateApplyIcon();
}

void BackgroundPreviewPopup::onDelete(cocos2d::CCObject*) {
    auto savedPath = pathToSaved(m_file);
    auto name = getDisplayName(savedPath);
    if (name.empty() && m_file.filename())
        name = displayFileName(m_file.filename()->string());

    createQuickPopup(
        "Delete Image",
        fmt::format("Are you sure you want to delete <cy>{}</c>?", name),
        "Cancel", "Delete",
        380.f,
        [this](FLAlertLayer*, bool confirmed) {
            if (!confirmed)
                return;

            if (!deleteImageFile(m_file)) {
                Notification::create("Failed to delete file", NotificationIcon::Error)->show();
                return;
            }

            if (m_owner)
                m_owner->forgetFile(m_file);

            Notification::create("File deleted", NotificationIcon::Success)->show();
            this->onClose(nullptr);
        }
    );
}

void BackgroundPreviewPopup::onFavorite(cocos2d::CCObject*) {
    bool favorite = toggleFavorite(pathToSaved(m_file));

    if (m_favoriteIcon) {
        auto icon = favorite ? "gj_heartOn_001.png" : "gj_heartOff_001.png";
        if (auto frame = CCSpriteFrameCache::get()->spriteFrameByName(icon))
            m_favoriteIcon->setDisplayFrame(frame);
    }

    (void)Mod::get()->saveData();
    if (m_owner)
        m_owner->rebuildList();

    Notification::create(
        favorite ? "Added to favorites" : "Removed from favorites",
        NotificationIcon::Info
    )->show();
}

void BackgroundPreviewPopup::onRefresh(cocos2d::CCObject*) {
    if (m_owner) {
        m_owner->refreshFile(m_file);
    }
    else {
        CCTextureCache::sharedTextureCache()->removeTextureForKey(m_file.string().c_str());
    }

    loadImage();
    Notification::create("Image refreshed", NotificationIcon::Success)->show();
}

// ---------------------------------------------------------------------------
// finding the screen that is currently open
// ---------------------------------------------------------------------------

static std::string interfaceClassName(cocos2d::CCNode* layer) {
    if (!layer)
        return "";

    std::string name{geode::cocos::getObjectName(layer)};
    if (name.empty() || name.starts_with("CC"))
        return "";
    return name;
}

static cocos2d::CCNode* findCurrentLayer() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene || !scene->getChildren())
        return nullptr;

    auto children = scene->getChildren();
    for (int i = static_cast<int>(children->count()) - 1; i >= 0; --i) {
        auto node = static_cast<cocos2d::CCNode*>(children->objectAtIndex(i));
        if (!node)
            continue;
        if (typeinfo_cast<FLAlertLayer*>(node) || typeinfo_cast<CCScene*>(node))
            continue;
        if (typeinfo_cast<cocos2d::CCLayer*>(node))
            return node;
    }

    return nullptr;
}

static cocos2d::CCSprite* findCurrentBackground(cocos2d::CCNode* layer) {
    if (!layer)
        return nullptr;

    for (auto const id : {"background"_spr, "background", "bg", "content-background", "bg-texture"}) {
        if (auto node = layer->getChildByIDRecursive(id))
            if (auto sprite = typeinfo_cast<cocos2d::CCSprite*>(node))
                return sprite;
    }

    // Fallback: the largest visible sprite behind everything else.
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float minArea = winSize.width * winSize.height * 0.25f;
    cocos2d::CCSprite* best = nullptr;
    float bestArea = 0.f;

    std::function<void(cocos2d::CCNode*)> walk = [&](cocos2d::CCNode* node) {
        // Never treat Geode's animated background sprites as editable ones.
        if (node->getID() == "SwelvyBG")
            return;

        if (auto sprite = typeinfo_cast<cocos2d::CCSprite*>(node)) {
            if (sprite->isVisible() && sprite->getZOrder() <= 0) {
                auto size = sprite->getContentSize();
                float area = size.width * size.height;
                if (area > minArea && area > bestArea) {
                    best = sprite;
                    bestArea = area;
                }
            }
        }

        if (auto children = node->getChildren()) {
            for (auto child : CCArrayExt<cocos2d::CCNode*>(children))
                walk(child);
        }
    };

    walk(layer);
    return best;
}

// Interfaces that only get a static background when their extra-layer setting
// is enabled. Used to hide the "current background" thumbnail when the
// interface is still at its default (no background) state.
static char const* extraLayerSettingKey(cocos2d::CCNode* layer) {
    if (!layer)
        return nullptr;

    if (typeinfo_cast<MenuLayer*>(layer))
        return "menu-layer";
    if (typeinfo_cast<LoadingLayer*>(layer))
        return "loading-layer";
    if (typeinfo_cast<LevelSelectLayer*>(layer))
        return "level-select-layer";
    if (typeinfo_cast<GauntletLayer*>(layer))
        return "gauntlet-map";
    if (layer->getChildByIDRecursive("SwelvyBG"))
        return "geode";

    std::string name{geode::cocos::getObjectName(layer)};
    if (name == "ModsLayer")
        return "geode";
    if (name == "CustomCreatorLayer" || name == "DailyViewLayer"
        || name == "LevelSearchViewLayer" || name == "LeaderboardViewLayer"
        || name == "RewardGroupLayer" || name == "RewardViewLayer")
        return "better-info";
    if (name.starts_with("Globed"))
        return "globed";
    if (name == "GPFeedbackLayer" || name == "GPKofiLayer")
        return "garage-plus";
    if (name == "TextureWorkshopLayer")
        return "texture-workshop";
    if (name == "GDPSHubLayer")
        return "gdps-hub";

    return nullptr;
}

static bool shouldHideDefaultPreview(
    cocos2d::CCNode* layer,
    std::string const& saved
) {
    if (!saved.empty())
        return false;

    auto setting = extraLayerSettingKey(layer);
    if (!setting)
        return false;

    return !Mod::get()->getSettingValue<bool>(setting);
}

// Saved key used by the hook that creates this interface's extra background.
static std::string extraLayerSavedKey(cocos2d::CCNode* layer) {
    if (!layer)
        return "";

    if (typeinfo_cast<MenuLayer*>(layer))
        return "MenuLayer";
    if (typeinfo_cast<LoadingLayer*>(layer))
        return "LoadingLayer";
    if (typeinfo_cast<LevelSelectLayer*>(layer))
        return "LevelSelectLayer";
    if (typeinfo_cast<GauntletLayer*>(layer))
        return "GauntletLayer";

    // Keys used by the hooks in external.cpp.
    auto name = interfaceClassName(layer);
    if (name == "ModsLayer") return "geode.loader-ModsLayer";
    if (name == "CustomCreatorLayer") return "cvolton.bettterinfo-CustomCreatorLayer";
    if (name == "DailyViewLayer") return "cvolton.bettterinfo-DailyViewLayer";
    if (name == "LevelSearchViewLayer") return "cvolton.bettterinfo-LevelSearchViewLayer";
    if (name == "LeaderboardViewLayer") return "cvolton.bettterinfo-LeaderboardViewLayer";
    if (name == "RewardGroupLayer") return "cvolton.bettterinfo-RewardGroupLayer";
    if (name == "RewardViewLayer") return "cvolton.bettterinfo-RewardViewLayer";
    if (name == "GlobedMenuLayer") return "dankmeme.globed2-GlobedMenuLayer";
    if (name == "GlobedServersLayer") return "dankmeme.globed2-GlobedServersLayer";
    if (name == "GPFeedbackLayer") return "omgrod.garage_plus-GPFeedbackLayer";
    if (name == "GPKofiLayer") return "omgrod.garage_plus-GPKofiLayer";
    if (name == "TextureWorkshopLayer") return "uproxide.textures-TextureWorkshopLayer";
    if (name == "GDPSHubLayer") return "lblazen.gdps_hub-GDPSHubLayer";

    return "";
}

// Finds the node Geode used as the theme background on the mod browser.
static cocos2d::CCNode* findModsLayerThemeBackground(cocos2d::CCNode* modsLayer) {
    if (!modsLayer)
        return nullptr;

    auto theme = g_geodeThemeBackground;
    if (theme && theme->getParent() == modsLayer)
        return theme;

    if (auto swelvy = modsLayer->getChildByIDRecursive("SwelvyBG"))
        return swelvy;
    if (auto bg = modsLayer->getChildByID("bg"))
        return bg;

    // Sapphire theme: a full-screen sprite without an ID.
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto children = modsLayer->getChildren();
    if (!children)
        return nullptr;

    for (auto child : CCArrayExt<cocos2d::CCNode*>(children)) {
        auto sprite = typeinfo_cast<cocos2d::CCSprite*>(child);
        if (!sprite)
            continue;

        float width = sprite->getContentWidth() * sprite->getScaleX();
        float height = sprite->getContentHeight() * sprite->getScaleY();
        if (width >= winSize.width * 0.9f && height >= winSize.height * 0.9f)
            return sprite;
    }

    return nullptr;
}

// Adds our background directly under ModsLayer, with Geode's theme
// background kept untouched underneath it. Returns the sprite we added.
static cocos2d::CCSprite* attachModsLayerBackground(cocos2d::CCNode* modsLayer) {
    if (!modsLayer)
        return nullptr;
    if (!Mod::get()->getSettingValue<bool>("BG")
        || !Mod::get()->getSettingValue<bool>("geode")) {
        return nullptr;
    }

    if (auto existing = modsLayer->getChildByIDRecursive("background"_spr))
        if (auto sprite = typeinfo_cast<cocos2d::CCSprite*>(existing))
            return sprite;

    auto theme = findModsLayerThemeBackground(modsLayer);

    auto background = geode::createLayerBG();
    // Capture the default look before any saved image is applied, so turning
    // the custom image off restores the plain gradient.
    rememberDefaultBackground(background);
    tryReplace(background, "layers", "geode.loader-ModsLayer");
    setBackgroundTag(background, "geode.loader-ModsLayer");
    background->setColor(ccc3(255, 255, 255));
    background->setID("background"_spr);

    auto container = CCNode::create();
    container->setID("background-container"_spr);
    container->setContentSize(CCDirector::sharedDirector()->getWinSize());

    if (theme) {
        theme->retain();
        theme->removeFromParentAndCleanup(false);
        theme->setZOrder(0);
        container->addChild(theme);
        theme->release();
    }

    background->setZOrder(1);
    container->addChild(background);

    // Sit behind every ModsLayer UI element (all of which use z >= 0).
    container->setZOrder(-2);
    modsLayer->addChild(container);

    return background;
}

// Adds the same covering background on interfaces whose background comes from
// an Extra Layers option. Mirrors the hooks that create them on entry.
static cocos2d::CCSprite* createExtraLayerBackground(cocos2d::CCNode* layer) {
    if (!layer || !Mod::get()->getSettingValue<bool>("BG"))
        return nullptr;

    if (interfaceClassName(layer) == "ModsLayer")
        return attachModsLayerBackground(layer);

    if (auto existing = layer->getChildByIDRecursive("background"_spr))
        if (auto sprite = typeinfo_cast<cocos2d::CCSprite*>(existing))
            return sprite;

    auto background = geode::createLayerBG();
    rememberDefaultBackground(background);
    background->setColor(ccc3(255, 255, 255));
    background->setID("background"_spr);

    if (typeinfo_cast<MenuLayer*>(layer)) {
        // The animated menu background stays alive underneath ours; only its
        // input is disabled while the covering background is shown.
        coverMenuGameLayer(layer);
        tryReplace(background, "layers", "MenuLayer");
        background->setZOrder(-19);
    }
    else if (typeinfo_cast<LoadingLayer*>(layer)) {
        if (auto original = layer->getChildByID("bg-texture"))
            original->setVisible(false);
        tryReplace(background, "layers", "LoadingLayer");
        background->setZOrder(-19);
    }
    else if (typeinfo_cast<LevelSelectLayer*>(layer)) {
        if (auto original = layer->getChildByID("background"))
            original->setVisible(false);
        tryReplace(background, "layers", "LevelSelectLayer");
        background->setZOrder(-19);
    }
    else if (auto gauntlet = typeinfo_cast<GauntletLayer*>(layer)) {
        if (auto original = layer->getChildByID("background"))
            original->setVisible(false);

        int index = static_cast<int>(gauntlet->m_gauntletType) - 1;
        bool applied = false;
        if (index >= 0 && index < static_cast<int>(gauntletIndex.size()))
            applied = tryReplace(background, "gauntlet", gauntletIndex[index], gauntletIndex[index]);
        if (!applied)
            tryReplace(background, "layers", "GauntletLayer", "GauntletLayer");

        background->setZOrder(-19);
    }
    else {
        // Mod-provided interfaces: cover whatever original background the
        // corresponding hook would hide.
        for (auto id : {"background", "cvolton.betterinfo/background", "swelvy-node"}) {
            if (auto original = layer->getChildByID(id))
                original->setVisible(false);
        }

        auto key = extraLayerSavedKey(layer);
        if (!key.empty())
            tryReplace(background, "layers", key, key);
        background->setZOrder(-2);
    }

    background->setColor(ccc3(255, 255, 255));
    layer->addChild(background);
    return background;
}

// MenuGameLayer is a full-screen layer that draws the animated menu
// background and handles taps on it. While our background covers the menu it
// is kept alive (see coverMenuGameLayer) and only disabled for input.
// Restoring its z-order with setZOrder leaves it *after* any UI children that
// share the same z-order, so it would be drawn over the menu and swallow
// clicks. Put the touch and z-order back, then move it to the front of its
// z-order group (child index 0 when the menu UI uses z-order 0).
static void restoreMenuGameLayerOrder(cocos2d::CCNode* layer) {
    revealMenuGameLayer(layer);

    auto menu = typeinfo_cast<MenuLayer*>(layer);
    if (!menu || !menu->m_menuGameLayer)
        return;

    auto gameLayer = menu->m_menuGameLayer;
    if (gameLayer->getParent() != menu)
        return;

    auto children = menu->getChildren();
    if (!children)
        return;

    // Children with the same z-order are drawn in arrival order, and the
    // z-order restore above gives this layer the newest arrival, i.e. it is
    // drawn last. Send it back to the front of the equal-z group.
    gameLayer->setOrderOfArrival(0);

    // Flush any pending z-order sort from restoreOriginals first, so the
    // engine cannot reorder the children again after the fix below.
    menu->sortAllChildren();

    // Find the layer's current child index and how many children belong
    // strictly below its z-order (those must stay in front of it).
    unsigned int count = children->count();
    unsigned int current = count;
    unsigned int below = 0;
    int zOrder = gameLayer->getZOrder();
    for (unsigned int i = 0; i < count; ++i) {
        auto node = static_cast<cocos2d::CCNode*>(children->objectAtIndex(i));
        if (node == gameLayer)
            current = i;
        else if (node && node->getZOrder() < zOrder)
            ++below;
    }

    if (current == count)
        return;

    // Already at the front of its z-order group: nothing to reorder.
    if (current != below) {
        // insertObject/removeObjectAtIndex balance the array's reference; keep
        // an extra one so the node cannot be freed between the two calls.
        gameLayer->retain();
        children->removeObjectAtIndex(current);
        children->insertObject(gameLayer, below);
        gameLayer->release();
    }

    // GD 2.2 breaks equal z-order ties with the per-object child indexes, and
    // a raw array move does not update them. Rebuild both fresh from the new
    // order, otherwise the next z-order sort could put the game layer back on
    // top of the UI.
    children->recreateNewIndexes();
    menu->updateChildIndexes();
}

// Removes the background this mod added and reveals the original one.
static void removeExtraLayerBackground(cocos2d::CCNode* layer) {
    if (!layer)
        return;

    if (auto ourNode = layer->getChildByIDRecursive("background"_spr)) {
        auto parent = ourNode->getParent();
        if (parent && parent != layer) {
            // Unwrap Geode's theme background before dropping the container.
            std::vector<cocos2d::CCNode*> keep;
            if (auto children = parent->getChildren())
                for (auto child : CCArrayExt<cocos2d::CCNode*>(children))
                    if (child != ourNode)
                        keep.push_back(child);

            for (auto child : keep) {
                child->retain();
                child->removeFromParentAndCleanup(false);
                layer->addChild(child, -2);
                child->release();
            }
            parent->removeFromParentAndCleanup(true);
        }
        else {
            ourNode->removeFromParentAndCleanup(true);
        }
    }

    // Reveal the animated menu background and put it back behind the UI; the
    // z-order restore alone can leave it on top of same-z menu elements.
    restoreMenuGameLayerOrder(layer);

    for (auto id : {"bg-texture", "background", "cvolton.betterinfo/background", "swelvy-node"}) {
        if (auto original = layer->getChildByID(id))
            original->setVisible(true);
    }
}

// Returns the covering background this mod added to an Extra Layers interface.
static cocos2d::CCSprite* findExtraLayerBackground(cocos2d::CCNode* layer) {
    if (!layer)
        return nullptr;
    if (auto node = layer->getChildByIDRecursive("background"_spr))
        return typeinfo_cast<cocos2d::CCSprite*>(node);
    return nullptr;
}

// Nodes the extra-layer hooks hide behind the covering background.
static std::vector<cocos2d::CCNode*> extraLayerOriginalNodes(cocos2d::CCNode* layer) {
    std::vector<cocos2d::CCNode*> nodes;
    if (!layer)
        return nodes;

    auto add = [&](cocos2d::CCNode* node) {
        // Only nodes our covering background actually hid.
        if (!node || node->isVisible())
            return;
        if (std::find(nodes.begin(), nodes.end(), node) == nodes.end())
            nodes.push_back(node);
    };

    if (auto menu = typeinfo_cast<MenuLayer*>(layer))
        add(menu->m_menuGameLayer);

    for (auto id : {"bg-texture", "background", "cvolton.betterinfo/background", "swelvy-node"}) {
        if (auto node = layer->getChildByID(id))
            add(node);
    }

    return nodes;
}

// Temporarily shows the original background directly below our covering one,
// so opacity fades blend between the two instead of popping. Returns the
// original z-orders to restore afterwards.
static std::vector<std::pair<cocos2d::CCNode*, int>> placeOriginalsUnder(
    cocos2d::CCNode* layer,
    cocos2d::CCNode* ourBackground
) {
    std::vector<std::pair<cocos2d::CCNode*, int>> originals;
    if (!layer || !ourBackground)
        return originals;

    for (auto node : extraLayerOriginalNodes(layer)) {
        originals.emplace_back(node, node->getZOrder());
        if (node->getZOrder() >= ourBackground->getZOrder())
            node->setZOrder(ourBackground->getZOrder() - 1);
        node->setVisible(true);
    }
    return originals;
}

static void restoreOriginals(
    std::vector<std::pair<cocos2d::CCNode*, int>> const& originals,
    bool hide
) {
    for (auto [node, z] : originals) {
        if (!node || !node->getParent())
            continue;
        if (hide)
            node->setVisible(false);
        node->setZOrder(z);
    }
}

// Fades a freshly created covering background in from the interface's
// original background, so it never flashes the default material.
static void fadeInExtraLayerBackground(
    cocos2d::CCNode* layer,
    cocos2d::CCSprite* background
) {
    if (!background)
        return;

    auto originals = placeOriginalsUnder(layer, background);

    GLubyte targetOpacity = background->getOpacity();
    if (targetOpacity == 0)
        targetOpacity = 255;

    background->stopActionByTag(BACKGROUND_FADE_TAG);
    background->setUserObject(
        "fade-opacity"_spr, cocos2d::CCInteger::create(targetOpacity)
    );
    background->setOpacity(0);

    auto sequence = cocos2d::CCSequence::create(
        cocos2d::CCFadeTo::create(BACKGROUND_FADE_TIME, targetOpacity),
        geode::cocos::CallFuncExt::create([originals]() {
            // Cover the original again, matching the state the hooks leave it
            // in after the screen is recreated.
            restoreOriginals(originals, true);
        }),
        nullptr
    );
    sequence->setTag(BACKGROUND_FADE_TAG);
    background->runAction(sequence);
}

// Fades the covering background out over the original one before removing it.
static void fadeOutExtraLayerBackground(
    cocos2d::CCNode* layer,
    cocos2d::CCSprite* background
) {
    if (!layer || !background) {
        removeExtraLayerBackground(layer);
        return;
    }

    auto originals = placeOriginalsUnder(layer, background);

    background->stopActionByTag(BACKGROUND_FADE_TAG);
    auto sequence = cocos2d::CCSequence::create(
        cocos2d::CCFadeTo::create(BACKGROUND_FADE_TIME, 0),
        geode::cocos::CallFuncExt::create([layer, originals]() {
            // Restore the original z-orders first, then drop our background:
            // removing it also fixes up the menu game layer's child order,
            // which a later setZOrder would scramble again.
            restoreOriginals(originals, false);
            removeExtraLayerBackground(layer);
        }),
        nullptr
    );
    sequence->setTag(BACKGROUND_FADE_TAG);
    background->runAction(sequence);
}

static LoadingLayer* findLoadingLayerInScene() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene)
        return nullptr;
    return geode::cocos::findFirstChildRecursive<LoadingLayer>(
        scene, [](LoadingLayer*) { return true; }
    );
}

// Tier colors for the scope toggle and its description line, high to low.
static cocos2d::ccColor3B scopeTierColor(int tier) {
    switch (tier) {
        case 2: return ccc3(255,200,200);   // Map / Level / List
        case 1: return ccc3(200, 255, 200);   // Daily / Weekly / Event / Gauntlet
        default: return ccc3(200, 200, 255);  // Layer
    }
}

static std::string scopeDescriptionText(
    BackgroundScope const& scope,
    std::string const& interfaceName
) {
    if (scope.tier == 1)
        return fmt::format("Only Apply for {} Level Pages", scope.label);
    if (scope.tier == 2) {
        if (scope.label == "Map") {
            auto name = scope.key;
            if (name.starts_with("g-"))
                name = name.substr(2);
            return fmt::format("{} Gauntlet Map Specialize", name);
        }
        return fmt::format("{} Specialize", scope.label);
    }
    return fmt::format("Apply for anywhere of {}", interfaceName);
}

// Highest-priority scope with an existing saved background. The scopes vector
// is ordered from lowest to highest priority (Layer first), and `savedOut` is
// left empty when the default material applies.
static BackgroundScope const* effectiveScopeFor(
    std::vector<BackgroundScope> const& scopes,
    std::string const& layerKey,
    std::string& savedOut
) {
    if (scopes.empty()) {
        auto candidate = getSavedBackground(layerKey);
        if (!candidate.empty() && savedToPath(candidate).exists())
            savedOut = candidate;
        return nullptr;
    }

    for (size_t i = scopes.size(); i-- > 0;) {
        auto candidate = getSavedBackground(scopes[i].key);
        if (!candidate.empty() && savedToPath(candidate).exists()) {
            savedOut = candidate;
            return &scopes[i];
        }
    }
    return nullptr;
}

// Scopes the picker can edit on screens whose background logic has multiple
// saved keys (see the tryReplace calls in main.cpp). Ordered from lowest to
// highest priority: Layer -> category -> specific. The specific keys use the
// shared key helpers ("le-<id>", "li-<id>", "g-<map>") so they match the
// hooks' saved lookups exactly.
static std::vector<BackgroundScope> backgroundScopesForLayer(cocos2d::CCNode* layer) {
    std::vector<BackgroundScope> scopes;
    if (!layer)
        return scopes;

    if (auto info = typeinfo_cast<LevelInfoLayer*>(layer)) {
        scopes.push_back({"Layer", "LevelInfoLayer", 0});
        if (auto level = info->m_level) {
            auto dailyID = level->m_dailyID.value();
            if (dailyID > 2000000)
                scopes.push_back({"Event", "event", 1});
            else if (dailyID > 1000000)
                scopes.push_back({"Weekly", "weekly", 1});
            else if (dailyID > 0)
                scopes.push_back({"Daily", "daily", 1});

            if (level->m_gauntletLevel || level->m_gauntletLevel2)
                scopes.push_back({"Gauntlet", "gauntlet", 1});

            auto levelKey = levelBackgroundKey(level);
            if (!levelKey.empty())
                scopes.push_back({"Level", levelKey, 2});
        }
        return scopes;
    }

    if (auto gauntlet = typeinfo_cast<GauntletLayer*>(layer)) {
        scopes.push_back({"Layer", "GauntletLayer", 0});
        int index = static_cast<int>(gauntlet->m_gauntletType) - 1;
        if (index >= 0 && index < static_cast<int>(gauntletIndex.size()))
            scopes.push_back({"Map", gauntletBackgroundKey(gauntletIndex[index]), 2});
        return scopes;
    }

    if (auto list = typeinfo_cast<LevelListLayer*>(layer)) {
        scopes.push_back({"Layer", "LevelListLayer", 0});
        if (auto levelList = list->m_levelList) {
            auto listKey = listBackgroundKey(levelList);
            if (!listKey.empty())
                scopes.push_back({"List", listKey, 2});
        }
        return scopes;
    }

    if (auto edit = typeinfo_cast<EditLevelLayer*>(layer)) {
        scopes.push_back({"Layer", "EditLevelLayer", 0});
        if (auto level = edit->m_level) {
            auto levelKey = levelBackgroundKey(level);
            if (!levelKey.empty())
                scopes.push_back({"Level", levelKey, 2});
        }
        return scopes;
    }

    return scopes;
}

// LevelAreaLayer / LevelAreaInnerLayer have no static get() accessor, so find
// them among the running scene's children by type instead.
static bool inLevelAreaScreen() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene || !scene->getChildren())
        return false;

    for (auto child : CCArrayExt<cocos2d::CCNode*>(scene->getChildren())) {
        if (typeinfo_cast<LevelAreaLayer*>(child) || typeinfo_cast<LevelAreaInnerLayer*>(child))
            return true;
    }
    return false;
}

bool isLoadingScreen() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene || !scene->getChildren())
        return false;

    for (auto child : CCArrayExt<cocos2d::CCNode*>(scene->getChildren())) {
        if (typeinfo_cast<LoadingLayer*>(child))
            return true;
    }
    return false;
}

bool canOpenBackgroundPopup() {
    if (PlayLayer::get() || LevelEditorLayer::get())
        return false;

    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene || !scene->getChildren())
        return false;

    // On the platformer level areas the picker edits the LoadingLayer
    // background instead of the screen itself, so no background is required.
    if (inLevelAreaScreen())
        return true;

    auto layer = findCurrentLayer();
    if (!layer)
        return false;

    // Extra-layer interfaces can be configured even while their background
    // display option is off: applying offers to turn the option on.
    if (extraLayerSettingKey(layer) != nullptr)
        return true;

    return findCurrentBackground(layer) != nullptr;
}

// ---------------------------------------------------------------------------
// BackgroundPopup
// ---------------------------------------------------------------------------

// One-time migration from the old storage layout (raw level IDs, gauntlet map
// names and editor level/list names as keys, "~/" path prefixes) to the ID
// based keys ("le-", "li-", "g-") and plain relative paths.
//
// Runs from the scene listener (once the game managers exist) and again from
// BackgroundPopup::init as a fallback; the "storage-migrated-v2" flag makes it
// idempotent. While migrating:
//   - raw numeric keys become "le-<id>" (or "li-<id>" if a saved list uses it)
//   - gauntlet map names become "g-<lowercase name>"
//   - old editor level/list names are resolved through LocalLevelManager and
//     the optional Editor Level ID API; entries whose name no longer exists
//     are dropped (the user allowed clearing them)
//   - "~/" / "config/" prefixes are stripped from all stored paths
static void migrateBackgroundStorage() {
    if (Mod::get()->getSavedValue<bool>("storage-migrated-v2", false))
        return;

    auto backgrounds = getSavedBackgrounds();
    if (backgrounds.empty()) {
        Mod::get()->setSavedValue("storage-migrated-v2", true);
        return;
    }

    std::map<std::string, std::string> migrated;
    std::map<std::string, std::string> names; // old name keys -> path

    auto isKnownKey = [](std::string const& key) {
        if (std::find(INTERFACE_KEYS.begin(), INTERFACE_KEYS.end(), key)
            != INTERFACE_KEYS.end()) {
            return true;
        }
        if (key.starts_with("le-") || key.starts_with("li-")
            || key.starts_with("g-")) {
            return true;
        }
        // Unknown interface classes are still kept.
        if (key.ends_with("Layer"))
            return true;
        return key == "daily" || key == "weekly" || key == "event"
            || key == "gauntlet";
    };

    auto manager = GameLevelManager::sharedState();

    for (auto const& [key, value] : backgrounds) {
        auto canonical = canonicalSavedPath(value);

        if (isKnownKey(key)) {
            migrated[key] = canonical;
            continue;
        }

        bool numeric = !key.empty() && std::all_of(
            key.begin(), key.end(),
            [](unsigned char c) { return std::isdigit(c) != 0; }
        );
        if (numeric) {
            char* end = nullptr;
            long id = std::strtol(key.c_str(), &end, 10);
            bool isList = manager
                && manager->getSavedLevelList(static_cast<int>(id)) != nullptr;
            migrated[(isList ? "li-" : "le-") + key] = canonical;
            continue;
        }

        if (std::find(gauntletIndex.begin(), gauntletIndex.end(), key)
            != gauntletIndex.end()) {
            migrated[gauntletBackgroundKey(key)] = canonical;
            continue;
        }

        // Likely an old editor level / list name; resolve to an editor ID.
        names[key] = canonical;
    }

    if (!names.empty()) {
        auto local = LocalLevelManager::sharedState();
        // If the local level store has not been initialized yet, retry on the
        // next scene instead of dropping the old entries.
        if (!local || (!local->m_localLevels && !local->m_localLists))
            return;

        if (local->m_localLevels) {
            for (auto level : CCArrayExt<GJGameLevel*>(local->m_localLevels)) {
                auto it = names.find(level->m_levelName);
                if (it == names.end())
                    continue;
                auto key = levelBackgroundKey(level);
                if (!key.empty() && !migrated.contains(key))
                    migrated[key] = it->second;
            }
        }
        if (local->m_localLists) {
            for (auto list : CCArrayExt<GJLevelList*>(local->m_localLists)) {
                auto it = names.find(list->m_listName);
                if (it == names.end())
                    continue;
                auto key = listBackgroundKey(list);
                if (!key.empty() && !migrated.contains(key))
                    migrated[key] = it->second;
            }
        }
    }

    // Drop empty dedicated entries left behind by the old "clear" behaviour.
    std::erase_if(migrated, [](auto const& entry) {
        bool dedicated = entry.first.starts_with("le-")
            || entry.first.starts_with("li-")
            || entry.first.starts_with("g-");
        return dedicated && entry.second.empty();
    });

    Mod::get()->setSavedValue("backgrounds", migrated);

    auto storedNames = Mod::get()->getSavedValue<std::map<std::string, std::string>>(
        "display-names"
    );
    std::map<std::string, std::string> canonicalNames;
    for (auto const& [key, value] : storedNames)
        canonicalNames[canonicalSavedPath(key)] = value;
    Mod::get()->setSavedValue("display-names", canonicalNames);

    auto storedFavorites = Mod::get()->getSavedValue<std::vector<std::string>>(
        "favorites"
    );
    for (auto& entry : storedFavorites)
        entry = canonicalSavedPath(entry);
    Mod::get()->setSavedValue("favorites", storedFavorites);

    Mod::get()->setSavedValue("storage-migrated-v2", true);
    (void)Mod::get()->saveData();
}

BackgroundPopup* BackgroundPopup::create() {
    auto ret = new BackgroundPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool BackgroundPopup::init() {
    if (!Popup::init(420.f, 280.f))
        return false;

    // Existing saves may still use the old keys / path prefixes.
    migrateBackgroundStorage();

    m_loadingSettings = inLevelAreaScreen();

    if (m_loadingSettings) {
        // Opened from the platformer level areas: the picker edits the loading
        // screen background instead of anything on the current screen.
        m_layerDisplay = "LoadingLayer";
        m_layerKey = "LoadingLayer";
    }
    else {
        m_targetLayer = findCurrentLayer();

        // Safety net for the mod browser: if the scene listener has not run
        // yet, attach the background under ModsLayer before looking it up.
        if (m_targetLayer && interfaceClassName(m_targetLayer) == "ModsLayer")
            attachModsLayerBackground(m_targetLayer);

        m_bgSprite = findCurrentBackground(m_targetLayer);

        m_layerDisplay = interfaceClassName(m_targetLayer);
        if (m_layerDisplay.empty())
            m_layerDisplay = "current screen";

        // Interfaces whose background comes from an Extra Layers option must
        // only ever edit the covering background this mod created. While the
        // option is off (or that node is missing) applying offers to turn the
        // option on and create it instead of touching the game's own node.
        if (auto setting = extraLayerSettingKey(m_targetLayer)) {
            std::string ourID = "background"_spr;
            bool ours = m_bgSprite && m_bgSprite->getID() == ourID;
            if (!Mod::get()->getSettingValue<bool>(setting) || !ours)
                m_bgSprite = nullptr;
        }

        m_layerKey = backgroundTag(m_bgSprite);
        if (m_layerKey.empty())
            m_layerKey = extraLayerSavedKey(m_targetLayer);
        if (m_layerKey.empty())
            m_layerKey = interfaceClassName(m_targetLayer);
        if (m_layerKey.empty())
            m_layerKey = "unknown";

        m_scopes = backgroundScopesForLayer(m_targetLayer);
        if (!m_scopes.empty()) {
            auto tag = backgroundTag(m_bgSprite);
            m_scopeIndex = 0;
            for (size_t i = 0; i < m_scopes.size(); ++i) {
                if (m_scopes[i].key == tag) {
                    m_scopeIndex = i;
                    break;
                }
            }
            m_layerKey = m_scopes[m_scopeIndex].key;
        }
    }

    // The title is the interface this picker is editing, e.g. "MenuLayer".
    this->setTitle(m_layerDisplay);

    // Current background: preview, file name, scope line and path, compacted
    // towards the top so the toolbar row fits between them and the list.
    // Without a scope line the name and path sit closer together.
    float nameOffset = m_scopes.empty() ? 96.f : 100.f;
    float pathOffset = m_scopes.empty() ? 84.f : 80.f;

    m_currentNameLabel = Label::create("", "bigFont.fnt");
    m_currentNameLabel->setAnchorPoint(ccp(0.f, 0.5f));
    m_currentNameLabel->setLimitLabelWidth(230.f, 0.36f, 0.15f);
    m_mainLayer->addChildAtPosition(
        m_currentNameLabel, Anchor::Center, ccp(-125.f, nameOffset), ccp(0.f, 0.5f)
    );

    m_scopeDescription = Label::create("", "bigFont.fnt");
    m_scopeDescription->setAnchorPoint(ccp(0.f, 0.5f));
    m_scopeDescription->setLimitLabelWidth(230.f, 0.24f, 0.12f);
    m_mainLayer->addChildAtPosition(
        m_scopeDescription, Anchor::Center, ccp(-125.f, 90.f), ccp(0.f, 0.5f)
    );

    m_currentPathLabel = Label::create("", "bigFont.fnt");
    m_currentPathLabel->setAnchorPoint(ccp(0.f, 0.5f));
    m_currentPathLabel->setLimitLabelWidth(230.f, 0.28f, 0.15f);
    m_mainLayer->addChildAtPosition(
        m_currentPathLabel, Anchor::Center, ccp(-125.f, pathOffset), ccp(0.f, 0.5f)
    );

    updateCurrentPreview();

    // Name filter plus its toggle, favorites and rescan buttons on one row.
    // The node stays unscaled so the caret lines up with the clicked text
    // position; only its background and font are drawn at 80%. It is wider
    // than before because the folder/refresh buttons moved to the top-right
    // corner: it now spans x 20..340 and leaves the two toggles at 356/388.
    m_filterInput = TextInput::create(320.f, "Filter name or path", "bigFont.fnt");
    shrinkTextInputVisuals(m_filterInput);
    m_filterInput->setCallback([this](std::string const& text) {
        this->onFilterChanged(text);
    });
    // Allow every printable ASCII character in the filter.
    {
        std::string asciiAllowed;
        for (int c = 32; c <= 126; ++c)
            asciiAllowed.push_back(static_cast<char>(c));
        m_filterInput->setFilter(std::move(asciiAllowed));
    }
    m_mainLayer->addChildAtPosition(
        m_filterInput, Anchor::Center, ccp(-30.f, 56.f), ccp(0.5f, 0.5f)
    );

    auto toolbarMenu = CCMenu::create();
    toolbarMenu->setContentSize({420.f, 280.f});
    toolbarMenu->setAnchorPoint(ccp(0.f, 0.f));
    toolbarMenu->setPosition(ccp(0.f, 0.f));
    m_mainLayer->addChild(toolbarMenu);

    // Toolbar row: the two filter toggles are right-aligned with the scroll
    // layer (right edge x = 400), so they move to the old folder/refresh
    // slots. The filter input above ends at x = 340, leaving a small gap.
    if (auto button = createToggleButton(
            "GJ_filterIcon_001.png", m_filterEnabled, this,
            menu_selector(BackgroundPopup::onToggleFilter), &m_filterToggle)) {
        button->setPosition(ccp(356.f, 196.f));
        toolbarMenu->addChild(button);
    }

    if (auto button = createToggleButton(
            "gj_heartOn_001.png", m_favoritesOnly, this,
            menu_selector(BackgroundPopup::onToggleFavorites), &m_favoriteToggle)) {
        button->setPosition(ccp(388.f, 196.f));
        toolbarMenu->addChild(button);
    }

    // Folder and refresh live in the top-right corner of the popup, stacked
    // vertically: folder on top, refresh underneath. x = 400 keeps their
    // 24 px slots just clear of the scope toggle at 356; the visible circle
    // bases do not overlap.
    if (auto button = createGreenCircleButton(
            "folderIcon_001.png", 14.f, this,
            menu_selector(BackgroundPopup::onOpenConfigFolder))) {
        button->setPosition(ccp(400.f, 260.f));
        toolbarMenu->addChild(button);
    }

    if (auto button = createBareButton(
            "GJ_updateBtn_001.png", 24.f, 22.f, this,
            menu_selector(BackgroundPopup::onRescan))) {
        button->setPosition(ccp(400.f, 230.f));
        toolbarMenu->addChild(button);
    }

    // Scope toggle for screens with per-level / per-category backgrounds.
    // Sits directly above the favorites filter toggle, capped at toolbar
    // button height so it does not look too tall.
    if (!m_scopes.empty()) {
        constexpr float SCOPE_WIDTH = 68.f;
        float scopeHeight = fitToScreenAspect(56.f, 24.f).height;

        auto scopeNode = CCNode::create();
        scopeNode->setContentSize({SCOPE_WIDTH, scopeHeight});

        if (auto background = NineSlice::create("square02d_001.png")) {
            background->setContentSize({SCOPE_WIDTH * 2.f, scopeHeight * 2.f});
            background->setScale(0.5f);
            background->setAnchorPoint(ccp(0.5f, 0.5f));
            background->setPosition(ccp(SCOPE_WIDTH / 2.f, scopeHeight / 2.f));
            scopeNode->addChild(background);
            m_scopeBg = background;
        }

        m_scopeLabel = Label::create(m_scopes[m_scopeIndex].label, "bigFont.fnt");
        m_scopeLabel->setLimitLabelWidth(SCOPE_WIDTH - 8.f, 0.35f, 0.2f);
        m_scopeLabel->setPosition(ccp(SCOPE_WIDTH / 2.f, scopeHeight / 2.f));
        scopeNode->addChild(m_scopeLabel);

        auto item = CCMenuItemSpriteExtra::create(
            scopeNode, this, menu_selector(BackgroundPopup::onCycleScope)
        );
        // Vertically aligned with the current background thumbnail (offset y=90).
        item->setPosition(ccp(356.f, 230.f));
        toolbarMenu->addChild(item);
    }

    updateScopeUi();

    m_list = ScrollLayer::create({380.f, 160.f});
    m_list->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(4.f));
    m_list->setTouchEnabled(true);
    // ScrollLayer is a CCLayer and ignores its anchor point, so position its
    // bottom-left corner relative to the popup centre.
    m_mainLayer->addChildAtPosition(
        m_list, Anchor::Center, ccp(-190.f, -124.f), ccp(0.f, 0.f)
    );

    auto scrollbar = Scrollbar::create(m_list);
    m_mainLayer->addChildAtPosition(
        scrollbar, Anchor::Center, ccp(197.f, -44.f), ccp(0.5f, 0.5f)
    );

    m_spinner = LoadingSpinner::create(40.f);
    m_mainLayer->addChildAtPosition(
        m_spinner, Anchor::Center, ccp(0.f, -44.f), ccp(0.5f, 0.5f)
    );

    m_scanning = true;
    startScan();
    return true;
}

void BackgroundPopup::startScan() {
    auto dir = configDir();

    std::thread([self = Ref<BackgroundPopup>(this), dir]() mutable {
        std::vector<asp::fs::Path> files;

        if (auto result = utils::file::readDirectory(dir, true); result.isOk()) {
            files = result.unwrap();
            std::erase_if(files, [](asp::fs::Path const& path) {
                return !isImagePath(path) || !path.isFile() || !isAsciiConfigPath(path);
            });
            // Decode on this worker thread and drop anything the game cannot
            // actually load, so broken images never show up in the picker.
            std::erase_if(files, [](asp::fs::Path const& path) {
                return !canDecodeImage(path);
            });
            std::sort(files.begin(), files.end(), [](auto const& a, auto const& b) {
                return a.string() < b.string();
            });
        }

        queueInMainThread([self = std::move(self), files = std::move(files)]() mutable {
            self->onScanFinished(std::move(files));
        });
    }).detach();
}

void BackgroundPopup::onScanFinished(std::vector<asp::fs::Path> files) {
    m_scanning = false;

    if (m_spinner) {
        m_spinner->removeFromParent();
        m_spinner = nullptr;
    }

    m_files = std::move(files);
    rebuildList();
}

void BackgroundPopup::rebuildList(bool preserveScroll) {
    if (!m_list || !m_list->m_contentLayer)
        return;

    // Applying a background from a cell should not jump back to the top.
    float scrollFromTop = preserveScroll ? m_list->getScrollFromTop() : 0.f;

    auto content = m_list->m_contentLayer;
    content->removeAllChildren();
    if (m_emptyLabel) {
        m_emptyLabel->removeFromParent();
        m_emptyLabel = nullptr;
    }
    m_visibleFiles.clear();

    auto filter = lowerAscii(m_filterText);

    for (auto const& file : m_files) {
        auto savedPath = pathToSaved(file);
        if (m_favoritesOnly && !isFavorite(savedPath))
            continue;

        if (m_filterEnabled && !filter.empty()) {
            // Match what the list actually shows: the custom display name when
            // one is set, otherwise the file name. The relative path is also
            // searched so a folder name can be used as a filter.
            std::string name = getDisplayName(savedPath);
            if (name.empty()) {
                name = file.filename()
                    ? displayFileName(file.filename()->string())
                    : file.string();
            }

            bool nameMatches = lowerAscii(name).find(filter) != std::string::npos;
            bool pathMatches = lowerAscii(savedPath).find(filter) != std::string::npos;
            if (!nameMatches && !pathMatches)
                continue;
        }

        int index = static_cast<int>(m_visibleFiles.size());
        m_visibleFiles.push_back(file);
        content->addChild(createCell(file, index));
    }

    if (m_visibleFiles.empty()) {
        char const* message = "No images found in the config folder";
        if (m_favoritesOnly)
            message = "No favorite images";
        if (m_filterEnabled && !filter.empty())
            message = "No matching images";

        // Center the hint in the scroll layer viewport instead of placing it
        // in the (empty) scrollable content.
        m_emptyLabel = Label::create(message, "bigFont.fnt");
        m_emptyLabel->setScale(0.45f);
        m_emptyLabel->setColor(ccc3(180, 180, 180));
        m_emptyLabel->setAnchorPoint(ccp(0.5f, 0.5f));
        m_list->addChild(m_emptyLabel);
        m_emptyLabel->setPosition(ccp(
            m_list->getContentSize().width / 2.f,
            m_list->getContentSize().height / 2.f
        ));
    }

    content->updateLayout();
    if (preserveScroll)
        m_list->scrollToFromTop(scrollFromTop);
    else
        m_list->scrollToTop();
}

// Semi-transparent cell background using the game's own square02_001.png.
static cocos2d::CCNode* createCellBackground(float width, float height, int index) {
    if (auto scale9 = NineSlice::create("square02_001.png")) {
        scale9->setContentSize({width, height});
        scale9->setScale(1.f);
        scale9->setColor(ccc3(0, 0, 0));
        scale9->setOpacity(index % 2 ? 50 : 80);
        return scale9;
    }

    return CCLayerColor::create(
        ccColor4B{0, 0, 0, static_cast<GLubyte>(index % 2 ? 50 : 80)},
        width, height
    );
}

cocos2d::CCNode* BackgroundPopup::createCell(asp::fs::Path const& file, int index) {
    constexpr float CELL_WIDTH = 380.f;
    constexpr float CELL_HEIGHT = 30.f;

    auto cell = CCNode::create();
    cell->setContentSize({CELL_WIDTH, CELL_HEIGHT});
    cell->setAnchorPoint(ccp(0.5f, 0.5f));

    auto background = createCellBackground(CELL_WIDTH, CELL_HEIGHT, index);
    if (typeinfo_cast<CCLayerColor*>(background)) {
        // CCLayerColor is a CCLayer and ignores its anchor point, so place its
        // bottom-left corner at the cell origin.
        background->setAnchorPoint(ccp(0.f, 0.f));
        background->setPosition(ccp(0.f, 0.f));
        cell->addChild(background);
    }
    else {
        cell->addChildAtPosition(background, Anchor::Center, ccp(0.f, 0.f), ccp(0.5f, 0.5f));
    }

    constexpr float THUMB_WIDTH = 32.f;
    constexpr float THUMB_HEIGHT = 24.f;

    auto thumb = LazySprite::create({THUMB_WIDTH, THUMB_HEIGHT}, true);
    thumb->setAutoResize(false);

    Ref<LazySprite> thumbRef = thumb;
    auto thumbTarget = fitToScreenAspect(THUMB_WIDTH, THUMB_HEIGHT);
    // Install the callback before loading: cached textures finish loading
    // synchronously and the callback would not fire if set afterwards.
    thumb->setLoadCallback([thumbRef, thumbTarget](Result<> result) {
        if (result.isErr())
            return;

        auto size = thumbRef->getContentSize();
        if (size.width <= 0.f || size.height <= 0.f)
            return;

        thumbRef->setScaleX(thumbTarget.width / size.width);
        thumbRef->setScaleY(thumbTarget.height / size.height);
    });
    thumb->loadFromFile(file);

    cell->addChildAtPosition(thumb, Anchor::Center, ccp(-165.f, 0.f), ccp(0.5f, 0.5f));

    std::string fileName = file.filename() ? std::string(file.filename()->string()) : file.string();
    auto savedPath = pathToSaved(file);

    auto displayName = getDisplayName(savedPath);
    auto nameLabel = Label::create(
        displayName.empty() ? displayFileName(fileName) : displayName,
        "bigFont.fnt"
    );
    nameLabel->setAnchorPoint(ccp(0.f, 0.5f));
    nameLabel->setLimitLabelWidth(250.f, 0.32f, 0.12f);
    cell->addChildAtPosition(
        nameLabel, Anchor::Center, ccp(-140.f, 5.5f), ccp(0.f, 0.5f)
    );

    auto pathLabel = Label::create(displayPathWithoutRoot(savedPath), "goldFont.fnt");
    pathLabel->setAnchorPoint(ccp(0.f, 0.5f));
    pathLabel->setLimitLabelWidth(250.f, 0.32f, 0.1f);
    cell->addChildAtPosition(
        pathLabel, Anchor::Center, ccp(-140.f, -5.5f), ccp(0.f, 0.5f)
    );

    auto menu = CCMenu::create();
    menu->setContentSize({CELL_WIDTH, CELL_HEIGHT});
    menu->setAnchorPoint(ccp(0.f, 0.f));
    menu->setPosition(ccp(0.f, 0.f));
    cell->addChild(menu);

    bool favorite = isFavorite(savedPath);
    auto currentBackground = getSavedBackground(m_layerKey);
    // Reflect the saved configuration even while an Extra Layers display is
    // off: applying records the image before offering to enable the display.
    bool inUse = !currentBackground.empty()
        && canonicalSavedPath(currentBackground) == canonicalSavedPath(savedPath);

    auto addButton = [&](char const* icon, float size, float x, cocos2d::SEL_MenuHandler handler) {
        auto spr = CCSprite::createWithSpriteFrameName(icon);
        spr->setScale(size);
        auto button = CCMenuItemSpriteExtra::create(spr, this, handler);
        button->setTag(index);
        button->setPosition(ccp(x, 15.f));
        menu->addChild(button);
    };

    // Favorite and use swapped; the use button is slightly larger.
    addButton(favorite ? "gj_heartOn_001.png" : "gj_heartOff_001.png",
        0.65f, 305.f, menu_selector(BackgroundPopup::onFavorite));
    addButton("sideBtn_options_001.png", 0.7f, 335.f,
        menu_selector(BackgroundPopup::onPreview));
    addButton(inUse ? "GJ_selectSongOnBtn_001.png" : "GJ_selectSongBtn_001.png",
        0.6f, 365.f, menu_selector(BackgroundPopup::onApply));

    return cell;
}

void BackgroundPopup::updateScopeUi() {
    // The button reflects the scope being edited, so it changes when cycling.
    BackgroundScope selected = m_scopes.empty()
        ? BackgroundScope{"Layer", m_layerKey, 0}
        : m_scopes[m_scopeIndex];

    if (m_scopeLabel)
        m_scopeLabel->setString(selected.label.c_str());
    if (m_scopeBg)
        m_scopeBg->setColor(scopeTierColor(selected.tier));

    // The description shows which level is actually in effect. It is derived
    // from the effective background, so cycling the scope toggle does not
    // change it.
    std::string effectiveSaved;
    auto effective = effectiveScopeFor(m_scopes, m_layerKey, effectiveSaved);
    BackgroundScope shown = effective
        ? *effective
        : BackgroundScope{"Layer", m_layerKey, 0};

    if (m_scopeDescription) {
        // Only screens with several editable background scopes show the
        // description line; plain interfaces leave it hidden.
        m_scopeDescription->setVisible(!m_scopes.empty());
        if (!m_scopes.empty()) {
            m_scopeDescription->setString(
                scopeDescriptionText(shown, m_layerDisplay).c_str()
            );
            m_scopeDescription->setColor(scopeTierColor(shown.tier));
        }
    }
}

void BackgroundPopup::updateCurrentPreview() {
    if (m_currentThumb) {
        m_currentThumb->removeFromParent();
        m_currentThumb = nullptr;
    }

    // Effective background: the highest-priority scope with an existing saved
    // image, mirroring the tryReplace fallback order in main.cpp.
    std::string saved;
    effectiveScopeFor(m_scopes, m_layerKey, saved);

    // Interfaces controlled by an Extra Layers setting do not display their
    // saved image while the setting is off. The Current area then hides the
    // thumbnail and shows "Disabled" as the path instead of the default file.
    char const* extraSetting = m_loadingSettings
        ? "loading-layer"
        : extraLayerSettingKey(m_targetLayer);
    bool displayDisabled = extraSetting
        && !Mod::get()->getSettingValue<bool>(extraSetting);
    if (displayDisabled)
        saved.clear();

    cocos2d::CCSprite* thumb = nullptr;
    if (!shouldHideDefaultPreview(m_targetLayer, saved)) {
        if (!saved.empty()) {
            auto path = savedToPath(saved);
            if (!path.empty() && path.exists())
                thumb = CCSprite::create(path.string().c_str());
        }

        // Fall back to the live sprite (default backgrounds, broken saves, and
        // the loading-settings mode where there is no live sprite at all).
        if (!thumb && m_bgSprite) {
            if (auto frame = m_bgSprite->displayFrame())
                thumb = CCSprite::createWithSpriteFrame(frame);
        }
    }

    if (thumb) {
        auto size = thumb->getContentSize();
        if (size.width > 0.f && size.height > 0.f) {
            // Show the current background at the game's aspect ratio.
            auto target = fitToScreenAspect(56.f, 42.f);
            thumb->setScaleX(target.width / size.width);
            thumb->setScaleY(target.height / size.height);
        }

        m_currentThumb = thumb;
        m_mainLayer->addChildAtPosition(
            thumb, Anchor::Center, ccp(-160.f, 90.f), ccp(0.5f, 0.5f)
        );
    }

    // First line: the effective background name, or gray "Default".
    if (m_currentNameLabel) {
        std::string name;
        if (!saved.empty()) {
            name = getDisplayName(saved);
            if (name.empty()) {
                auto path = savedToPath(saved);
                if (auto fileName = path.filename())
                    name = displayFileName(fileName->string());
            }
        }
        else {
            name = "Default";
        }

        m_currentNameLabel->setString(name.c_str());
        m_currentNameLabel->setColor(saved.empty() ? ccc3(190, 190, 190) : ccWHITE);
        m_currentNameLabel->setLimitLabelWidth(230.f, 0.36f, 0.15f);
    }

    // Third line: the applied path (white/gold), the default material (green)
    // or "Disabled" in red when an Extra Layers display is turned off.
    if (m_currentPathLabel) {
        bool custom = !saved.empty();
        if (m_pathLabelGold != custom) {
            m_currentPathLabel->setFont(custom ? "goldFont.fnt" : "bigFont.fnt");
            m_pathLabelGold = custom;
        }

        if (displayDisabled) {
            m_currentPathLabel->setString("Disabled");
            m_currentPathLabel->setColor(ccc3(255, 0, 0));
            m_currentPathLabel->setLimitLabelWidth(230.f, 0.28f, 0.15f);
        }
        else if (custom) {
            m_currentPathLabel->setString(displayPathWithoutRoot(saved).c_str());
            m_currentPathLabel->setColor(ccWHITE);
            m_currentPathLabel->setLimitLabelWidth(230.f, 0.35f, 0.15f);
        }
        else {
            m_currentPathLabel->setString("GJ_gradientBG.png");
            m_currentPathLabel->setColor(ccc3(0, 255, 0));
            m_currentPathLabel->setLimitLabelWidth(230.f, 0.28f, 0.15f);
        }
    }
}

asp::fs::Path BackgroundPopup::fileForSender(cocos2d::CCObject* sender) {
    auto node = typeinfo_cast<cocos2d::CCNode*>(sender);
    if (!node)
        return {};

    int index = node->getTag();
    if (index < 0 || index >= static_cast<int>(m_visibleFiles.size()))
        return {};

    return m_visibleFiles[index];
}

void BackgroundPopup::onClose(cocos2d::CCObject* sender) {
    already_has_menu = false;
    Popup::onClose(sender);
}

void BackgroundPopup::onPreview(cocos2d::CCObject* sender) {
    auto file = fileForSender(sender);
    if (file.empty())
        return;

    if (auto popup = BackgroundPreviewPopup::create(file, this))
        popup->show();
}

void BackgroundPopup::applyFile(asp::fs::Path const& file) {
    char const* extraSetting = m_loadingSettings
        ? "loading-layer"
        : extraLayerSettingKey(m_targetLayer);
    auto savedPath = pathToSaved(file);

    // The interface's Extra Layers option is off. Record the choice first (so
    // the apply buttons already reflect it), then offer to turn the display
    // on so it takes effect immediately.
    if (extraSetting && !Mod::get()->getSettingValue<bool>(extraSetting)) {
        setSavedBackground(m_layerKey, savedPath);
        (void)Mod::get()->saveData();

        updateCurrentPreview();
        updateScopeUi();
        rebuildList(true);

        askEnableExtraLayer(file, extraSetting);
        return;
    }

    bool createdBackground = false;
    if (!m_bgSprite && !m_loadingSettings) {
        // The option is on but the node is missing (for example when the mod
        // hook did not run); create the same covering background here.
        if (extraSetting) {
            m_bgSprite = findExtraLayerBackground(m_targetLayer);
            if (!m_bgSprite) {
                m_bgSprite = createExtraLayerBackground(m_targetLayer);
                createdBackground = m_bgSprite != nullptr;
            }
            if (m_bgSprite)
                setBackgroundTag(m_bgSprite, m_layerKey);
        }

        if (!m_bgSprite) {
            Notification::create("No background found on this screen", NotificationIcon::Error)->show();
            return;
        }
    }

    // A higher-priority scope already supplies the visible background: only
    // record this choice, leaving the current wallpaper and screen untouched.
    if (!m_scopes.empty()) {
        std::string effectiveSaved;
        auto effective = effectiveScopeFor(m_scopes, m_layerKey, effectiveSaved);
        if (effective) {
            size_t effectiveIndex = static_cast<size_t>(
                effective - m_scopes.data()
            );
            if (effectiveIndex > m_scopeIndex) {
                setSavedBackground(m_layerKey, savedPath);
                (void)Mod::get()->saveData();

                // A background we just created must not sit there showing the
                // default material; show the higher-priority image instead.
                if (createdBackground && m_bgSprite) {
                    auto path = savedToPath(effectiveSaved);
                    if (!path.empty() && path.exists()
                        && applyImageToSprite(m_bgSprite, path, false)) {
                        setBackgroundTag(m_bgSprite, effective->key);
                        m_bgSprite->setColor(backgroundTintFor(m_targetLayer));
                        fadeInExtraLayerBackground(m_targetLayer, m_bgSprite);
                    }
                }

                updateCurrentPreview();
                updateScopeUi();
                rebuildList(true);
                Notification::create(
                    "Current wallpaper is from a higher priority",
                    NotificationIcon::Info
                )->show();
                return;
            }
        }
    }

    if (m_bgSprite) {
        if (!applyImageToSprite(m_bgSprite, file, !createdBackground)) {
            Notification::create("Failed to load image", NotificationIcon::Error)->show();
            return;
        }

        setBackgroundTag(m_bgSprite, m_layerKey);

        // Keep the dark tint for daily/weekly/event and gauntlet levels.
        m_bgSprite->setColor(backgroundTintFor(m_targetLayer));

        // A newly created covering background fades in its first image from
        // the interface's original background instead of flashing the default.
        if (createdBackground)
            fadeInExtraLayerBackground(m_targetLayer, m_bgSprite);
    }

    setSavedBackground(m_layerKey, savedPath);
    (void)Mod::get()->saveData();

    updateCurrentPreview();
    updateScopeUi();
    rebuildList(true);
    Notification::create("Background applied", NotificationIcon::Success)->show();
}

void BackgroundPopup::onApply(cocos2d::CCObject* sender) {
    auto file = fileForSender(sender);
    if (file.empty())
        return;

    // Pressing the button on the currently applied image un-applies it and
    // restores the default background.
    auto saved = getSavedBackground(m_layerKey);
    if (!saved.empty()
        && canonicalSavedPath(saved) == canonicalSavedPath(pathToSaved(file))) {
        clearBackground();
        return;
    }

    applyFile(file);
}

void BackgroundPopup::clearBackground() {
    // Same priority guard as applying: a higher-priority scope keeps the
    // current wallpaper, so only the saved entry changes.
    if (!m_scopes.empty()) {
        std::string effectiveSaved;
        auto effective = effectiveScopeFor(m_scopes, m_layerKey, effectiveSaved);
        if (effective) {
            size_t effectiveIndex = static_cast<size_t>(
                effective - m_scopes.data()
            );
            if (effectiveIndex > m_scopeIndex) {
                removeSavedBackground(m_layerKey);
                (void)Mod::get()->saveData();

                updateCurrentPreview();
                updateScopeUi();
                rebuildList(true);
                Notification::create(
                    "Current wallpaper is from a higher priority",
                    NotificationIcon::Info
                )->show();
                return;
            }
        }
    }

    removeSavedBackground(m_layerKey);
    (void)Mod::get()->saveData();

    std::string effectiveSaved;
    auto effective = effectiveScopeFor(m_scopes, m_layerKey, effectiveSaved);
    auto path = savedToPath(effectiveSaved);
    bool hasEffective = !path.empty() && path.exists();

    char const* extraSetting = m_loadingSettings
        ? "loading-layer"
        : extraLayerSettingKey(m_targetLayer);

    // The interface's background is supplied by an Extra Layers option and
    // nothing custom is left: ask whether to show the default material or to
    // turn the option off and remove the background this mod added.
    if (!hasEffective && extraSetting
        && Mod::get()->getSettingValue<bool>(extraSetting)) {
        askDisableExtraLayer(extraSetting);
        return;
    }

    // Re-apply whatever now has the highest priority (or the default material).
    if (m_bgSprite) {
        if (hasEffective && applyImageToSprite(m_bgSprite, path)) {
            if (effective)
                setBackgroundTag(m_bgSprite, effective->key);
            m_bgSprite->setColor(backgroundTintFor(m_targetLayer));
        }
        else {
            restoreDefaultBackground(m_bgSprite);
        }
    }

    updateCurrentPreview();
    updateScopeUi();
    rebuildList(true);
    Notification::create("Background reset to default", NotificationIcon::Success)->show();
}

void BackgroundPopup::askEnableExtraLayer(
    asp::fs::Path const& file,
    char const* setting
) {
    createQuickPopup(
        "Enable Background",
        fmt::format(
            "This image is now saved for <cy>{}</c>, but its background "
            "display is turned off.\nEnable it and show the image now?",
            m_layerDisplay
        ),
        "Later", "Enable",
        420.f,
        [this, file, setting](FLAlertLayer*, bool confirmed) {
            if (!confirmed)
                return;

            Mod::get()->setSettingValue(setting, true);

            if (m_loadingSettings) {
                // If a loading screen is present right now, give it the
                // covering background immediately; otherwise the next one
                // will use the new setting.
                if (auto loading = findLoadingLayerInScene()) {
                    auto background = findExtraLayerBackground(loading);
                    if (!background)
                        background = createExtraLayerBackground(loading);

                    if (background) {
                        auto path = savedToPath(getSavedBackground(m_layerKey));
                        if (!path.empty() && path.exists())
                            applyImageToSprite(background, path, false);
                        fadeInExtraLayerBackground(loading, background);
                    }
                }

                updateCurrentPreview();
                updateScopeUi();
                rebuildList(true);
                Notification::create(
                    "Background applied", NotificationIcon::Success
                )->show();
                return;
            }

            if (!m_bgSprite)
                m_bgSprite = createExtraLayerBackground(m_targetLayer);

            if (m_bgSprite) {
                // Respect a higher-priority scope, like the normal apply path.
                std::string effectiveSaved;
                auto effective = effectiveScopeFor(
                    m_scopes, m_layerKey, effectiveSaved
                );
                auto path = savedToPath(effectiveSaved);
                if (path.empty() || !path.exists())
                    path = file;

                bool ownImage = canonicalSavedPath(effectiveSaved)
                    == canonicalSavedPath(pathToSaved(file));

                if (applyImageToSprite(m_bgSprite, path, false)) {
                    setBackgroundTag(
                        m_bgSprite, effective ? effective->key : m_layerKey
                    );
                    m_bgSprite->setColor(backgroundTintFor(m_targetLayer));
                    fadeInExtraLayerBackground(m_targetLayer, m_bgSprite);
                }

                Notification::create(
                    ownImage
                        ? "Background applied"
                        : "Current wallpaper is from a higher priority",
                    ownImage ? NotificationIcon::Success : NotificationIcon::Info
                )->show();
            }

            updateCurrentPreview();
            updateScopeUi();
            rebuildList(true);
        }
    );
}

void BackgroundPopup::askDisableExtraLayer(char const* setting) {
    createQuickPopup(
        "Background Removed",
        fmt::format(
            "No custom background is left for <cy>{}</c>.\n"
            "Show the default background, or disable the background display "
            "for this interface?",
            m_layerDisplay
        ),
        "Default", "Disable",
        440.f,
        [this, setting](FLAlertLayer*, bool disable) {
            if (!disable) {
                // Keep the option enabled and fall back to the default image.
                if (m_bgSprite) {
                    restoreDefaultBackground(m_bgSprite);
                    m_bgSprite->setColor(ccc3(255, 255, 255));
                }

                updateCurrentPreview();
                updateScopeUi();
                rebuildList(true);
                Notification::create(
                    "Background reset to default", NotificationIcon::Success
                )->show();
                return;
            }

            Mod::get()->setSettingValue(setting, false);
            if (m_loadingSettings) {
                if (auto loading = findLoadingLayerInScene()) {
                    if (auto background = findExtraLayerBackground(loading))
                        fadeOutExtraLayerBackground(loading, background);
                    else
                        removeExtraLayerBackground(loading);
                }
            }
            else {
                auto background = m_bgSprite;
                m_bgSprite = nullptr;
                fadeOutExtraLayerBackground(m_targetLayer, background);
            }

            updateCurrentPreview();
            updateScopeUi();
            rebuildList(true);
            Notification::create(
                "Background display disabled", NotificationIcon::Success
            )->show();
        }
    );
}

void BackgroundPopup::forgetFile(asp::fs::Path const& file) {
    auto original = file.string();
    std::erase_if(m_files, [&](asp::fs::Path const& entry) {
        return entry.string() == original;
    });
    rebuildList();
}

void BackgroundPopup::onDelete(cocos2d::CCObject* sender) {
    auto file = fileForSender(sender);
    if (file.empty())
        return;

    if (!deleteImageFile(file)) {
        Notification::create("Failed to delete file", NotificationIcon::Error)->show();
        return;
    }

    forgetFile(file);
    Notification::create("File deleted", NotificationIcon::Success)->show();
}

void BackgroundPopup::refreshFile(asp::fs::Path const& file) {
    // Drop the cached texture so the file is decoded from disk again.
    CCTextureCache::sharedTextureCache()->removeTextureForKey(file.string().c_str());

    auto saved = savedToPath(getSavedBackground(m_layerKey));
    if (m_bgSprite && !saved.empty() && saved.string() == file.string()) {
        applyImageToSprite(m_bgSprite, file);
        updateCurrentPreview();
    }

    rebuildList();
}

void BackgroundPopup::onRefresh(cocos2d::CCObject* sender) {
    auto file = fileForSender(sender);
    if (file.empty())
        return;

    refreshFile(file);
    Notification::create("Image refreshed", NotificationIcon::Success)->show();
}

void BackgroundPopup::onFavorite(cocos2d::CCObject* sender) {
    auto file = fileForSender(sender);
    if (file.empty())
        return;

    bool favorite = toggleFavorite(pathToSaved(file));
    (void)Mod::get()->saveData();

    rebuildList();
    Notification::create(
        favorite ? "Added to favorites" : "Removed from favorites",
        NotificationIcon::Info
    )->show();
}

void BackgroundPopup::onToggleFavorites(cocos2d::CCObject*) {
    m_favoritesOnly = !m_favoritesOnly;

    setToggleState(m_favoriteToggle, m_favoritesOnly);

    rebuildList();
}

void BackgroundPopup::onCycleScope(cocos2d::CCObject*) {
    if (m_scopes.empty())
        return;

    m_scopeIndex = (m_scopeIndex + 1) % m_scopes.size();
    m_layerKey = m_scopes[m_scopeIndex].key;

    updateScopeUi();
    updateCurrentPreview();
    rebuildList(true);
}

void BackgroundPopup::onToggleFilter(cocos2d::CCObject*) {
    // With an empty input there is nothing to filter by, so the toggle should
    // never light up.
    if (!m_filterEnabled && m_filterText.empty())
        return;

    m_filterEnabled = !m_filterEnabled;

    setToggleState(m_filterToggle, m_filterEnabled);

    rebuildList();
}

void BackgroundPopup::onFilterChanged(std::string const& text) {
    bool wasEmpty = m_filterText.empty();
    m_filterText = text;
    bool isEmpty = m_filterText.empty();

    // Typing the first character switches the filter on; clearing the input
    // switches it back off so the list shows every image again.
    if (wasEmpty != isEmpty) {
        m_filterEnabled = !isEmpty;
        setToggleState(m_filterToggle, m_filterEnabled);
        rebuildList();
        return;
    }

    if (m_filterEnabled)
        rebuildList();
}

void BackgroundPopup::onRescan(cocos2d::CCObject*) {
    rescan();
}

// Toolbar folder button: opens the mod's config folder in the OS file manager
// so users can drop wallpaper images in without hunting for the path.
void BackgroundPopup::onOpenConfigFolder(cocos2d::CCObject*) {
    if (!file::openFolder(configDir())) {
        Notification::create(
            "Failed to open the config folder", NotificationIcon::Error
        )->show();
    }
}

void BackgroundPopup::rescan() {
    if (m_scanning || !m_list)
        return;

    m_list->m_contentLayer->removeAllChildren();
    if (m_emptyLabel) {
        m_emptyLabel->removeFromParent();
        m_emptyLabel = nullptr;
    }
    m_visibleFiles.clear();

    if (m_spinner)
        m_spinner->removeFromParent();

    m_spinner = LoadingSpinner::create(40.f);
    m_mainLayer->addChildAtPosition(
        m_spinner, Anchor::Center, ccp(0.f, -44.f), ccp(0.5f, 0.5f)
    );

    m_scanning = true;
    startScan();
}
