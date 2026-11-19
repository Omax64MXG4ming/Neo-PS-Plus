#include <Geode/modify/CCLayer.hpp>
#ifndef GEODE_IS_IOS
using namespace geode::prelude;
class $modify(CCLayer) {
    void onEnter() override {
        CCLayer::onEnter();

        std::string typeName = typeid(*this).name();

        if (
            (exact_cast<GauntletLayer*>(this) && !Mod::get()->getSettingValue<bool>("gauntletlayer")) ||
            exact_cast<GauntletSelectLayer*>(this) ||
            exact_cast<SecretRewardsLayer*>(this) ||
            exact_cast<LevelSelectLayer*>(this) ||
            (typeName.find("SogLayer") != std::string::npos) ||
         /* (typeName.find("MeltdownSelectLayer") != std::string::npos) ||
            (typeName.find("SubZeroSelectLayer") != std::string::npos) || */
            exact_cast<LevelAreaInnerLayer*>(this) ||
            (exact_cast<GJGarageLayer*>(this) && !Mod::get()->getSettingValue<bool>("gjgaragelayer")) ||
            (exact_cast<CreatorLayer*>(this) && !Mod::get()->getSettingValue<bool>("creatorlayer")) ||
            (exact_cast<LoadingLayer*>(this) && !Mod::get()->getSettingValue<bool>("loadinglayer")) ||
            (typeName.find("GlobedServersLayer") != std::string::npos && !Mod::get()->getSettingValue<bool>("globedlayers")) ||
            (typeName.find("GlobedMenuLayer") != std::string::npos && !Mod::get()->getSettingValue<bool>("globedlayers")) ||
            (exact_cast<SecretLayer4*>(this) && !Mod::get()->getSettingValue<bool>("secretlayers")) ||
            (exact_cast<SecretLayer2*>(this) && !Mod::get()->getSettingValue<bool>("secretlayers")) ||
            (exact_cast<SecretLayer*>(this) && !Mod::get()->getSettingValue<bool>("secretlayers")) ||
            (exact_cast<LevelAreaLayer*>(this) && !Mod::get()->getSettingValue<bool>("levelarealayer")) ||
            (exact_cast<LevelInfoLayer*>(this) && !Mod::get()->getSettingValue<bool>("levelinfolayer"))
        )
            return;

        ccColor3B randomColor = {255, 255, 255};

        if (Mod::get()->getSettingValue<bool>("loadinglayer")) {
            if (auto sprite = typeinfo_cast<CCSprite*>(this->getChildByIDRecursive("bg-texture"))) {
                sprite->setColor(randomColor);
            }
        }

        if (auto sprite = typeinfo_cast<CCSprite*>(this->getChildByIDRecursive("background"))) {
            sprite->setColor(randomColor);
        }

        if (Mod::get()->getSettingValue<bool>("cvoltonbetterinfobackground")) {
            if (auto sprite = typeinfo_cast<CCSprite*>(this->getChildByIDRecursive("cvolton.betterinfo/background"))) {
                sprite->setColor(randomColor);
            }
        }

        if (auto sprite = typeinfo_cast<CCSprite*>(this->getChildByIDRecursive("bg"))) {
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
