#include "head.hpp"

using namespace geode::prelude;

class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool arr) {
        if (!down) return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);

        // Check for your preferred keybind (F5 in this case)
        if (key == enumKeyCodes::KEY_F5) {
            TransparentBackgroundUI::showSettingsPopup();
            return true;
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
    }
};

void TransparentBackgroundUI::setupKeybind() {
    // Keybind is handled in the modify above
    log::info("Transparent Background UI keybind registered (F5)");
}

void TransparentBackgroundUI::showSettingsPopup() {
    createSettingsPopup();
}

void TransparentBackgroundUI::createSettingsPopup() {
    std::string bgStatus = Mod::get()->getSettingValue<bool>("BG") ? "Enabled" : "Disabled";

    auto popup = FLAlertLayer::create(
        "Transparent Background Settings",
        fmt::format(
            "<cg>Quick Settings</c>\n\n"
            "Press <cy>F5</c> to open this popup anytime\n\n"
            "Main background replacement: <cb>{}</c>\n\n"
            "<co>Use the full mod settings menu for detailed configuration</c>",
            bgStatus
        ),
        "Close"
    );

    // Create a simple toggle button
    auto toggleBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName(Mod::get()->getSettingValue<bool>("BG") ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png"),
        popup,
        [](CCObject* sender) {
            bool currentValue = Mod::get()->getSettingValue<bool>("BG");
            Mod::get()->setSettingValue("BG", !currentValue);

            // Show a brief confirmation
            FLAlertLayer::create(
                "Setting Changed",
                fmt::format("Background replacement is now {}.", !currentValue ? "enabled" : "disabled"),
                "OK"
            )->show();
        }
    );

    auto menu = CCMenu::create();
    menu->addChild(toggleBtn);
    menu->setPosition({popup->m_mainLayer->getContentSize().width / 2, 60});

    popup->m_mainLayer->addChild(menu);

    popup->show();
}

