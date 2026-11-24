#include <Geode/modify/CCLayer.hpp>
using namespace geode::prelude;

class $modify(CCLayer) {
    void onEnter() override {
        CCLayer::onEnter();

        std::string typeName = typeid(*this).name();

        auto isType = [&](const char* name) {
            return typeName.find(name) != std::string::npos;
        };

        if (
            (isType("GauntletLayer") &&
                !Mod::get()->getSettingValue<bool>("gauntletlayer")) ||

            isType("GauntletSelectLayer") ||
            isType("SecretRewardsLayer") ||
            isType("LevelSelectLayer") ||
            isType("SogLayer") ||

            isType("LevelAreaInnerLayer") ||

            (isType("GJGarageLayer") &&
                !Mod::get()->getSettingValue<bool>("gjgaragelayer")) ||

            (isType("CreatorLayer") &&
                !Mod::get()->getSettingValue<bool>("creatorlayer")) ||

            (isType("LoadingLayer") &&
                !Mod::get()->getSettingValue<bool>("loadinglayer")) ||

            (isType("GlobedServersLayer") &&
                !Mod::get()->getSettingValue<bool>("globedlayers")) ||

            (isType("GlobedMenuLayer") &&
                !Mod::get()->getSettingValue<bool>("globedlayers")) ||

            ((isType("SecretLayer4") ||
              isType("SecretLayer2") ||
              isType("SecretLayer")) &&
                !Mod::get()->getSettingValue<bool>("secretlayers")) ||

            (isType("LevelAreaLayer") &&
                !Mod::get()->getSettingValue<bool>("levelarealayer")) ||

            (isType("LevelInfoLayer") &&
                !Mod::get()->getSettingValue<bool>("levelinfolayer"))
        ) {
            return;
        }

        ccColor3B randomColor = {255, 255, 255};

        if (Mod::get()->getSettingValue<bool>("loadinglayer")) {
            if (auto sprite = typeinfo_cast<CCSprite*>(
                this->getChildByIDRecursive("bg-texture")
            )) {
                sprite->setColor(randomColor);
            }
        }

        if (auto sprite = typeinfo_cast<CCSprite*>(
            this->getChildByIDRecursive("background")
        )) {
            sprite->setColor(randomColor);
        }

        if (Mod::get()->getSettingValue<bool>("cvoltonbetterinfobackground")) {
            if (auto sprite = typeinfo_cast<CCSprite*>(
                this->getChildByIDRecursive("cvolton.betterinfo/background")
            )) {
                sprite->setColor(randomColor);
            }
        }

        if (auto sprite = typeinfo_cast<CCSprite*>(
            this->getChildByIDRecursive("bg")
        )) {
            sprite->setColor(randomColor);
        }

        auto children = this->getChildren();

        if (children) {
            for (CCNode* node : this->getChildrenExt()) {
                if (auto scaleSprite = typeinfo_cast<CCScale9Sprite*>(node)) {
                    const ccColor3B& color = scaleSprite->getColor();
                    GLubyte opacity = scaleSprite->getOpacity();

                    if (opacity == 255 &&
                        (
                            (color.r == 0 && color.g == 39 && color.b == 98) ||
                            (color.r == 0 && color.g == 56 && color.b == 141) ||
                            (color.r == 0 && color.g == 36 && color.b == 96) ||
                            (color.r == 0 && color.g == 36 && color.b == 91)
                        )
                    ) {
                        scaleSprite->setColor({0, 0, 0});
                        scaleSprite->setOpacity(130);
                    }
                    else if (opacity == 255 &&
                        (
                            (color.r == 0 && color.g == 46 && color.b == 117) ||
                            (color.r == 0 && color.g == 31 && color.b == 79)
                        )
                    ) {
                        scaleSprite->setColor({0, 0, 0});
                        scaleSprite->setOpacity(140);
                    }
                }
            }
        }
    }
};
