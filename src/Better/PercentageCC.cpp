#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PctPlayLayer, PlayLayer) {

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        restylePercentage();
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        restylePercentage();
    }

    void restylePercentage() {
        auto label = m_percentageLabel;

        if (!label)
            return;

        auto mod = Mod::get();

        // Settings
        float size = static_cast<float>(
            mod->getSettingValue<double>("size")
        );

        auto color = mod->getSettingValue<ccColor3B>("color");

        auto font = mod->getSettingValue<std::string>("font");

        auto pos = mod->getSettingValue<std::string>("position");

        int64_t opacityValue =
            mod->getSettingValue<int64_t>("opacity");

        opacityValue = std::clamp<int64_t>(opacityValue, 0, 100);

        auto opacity = static_cast<GLubyte>(
            opacityValue * 255 / 100
        );

        float margin = static_cast<float>(
            mod->getSettingValue<double>("margin")
        );

        // Built-in Geometry Dash fonts only
        auto fnt = font + ".fnt";

        if (strcmp(label->getFntFile(), fnt.c_str()) != 0) {
            label->setFntFile(fnt.c_str());
        }

        // Appearance
        label->setScale(size);
        label->setColor(color);
        label->setOpacity(opacity);

        // Position
        auto win = CCDirector::get()->getWinSize();

        float ax = 0.5f;
        float ay = 0.5f;

        float x = win.width / 2.f;
        float y = win.height / 2.f;

        if (pos.starts_with("Top")) {
            ay = 1.f;
            y = win.height - margin;
        }
        else if (pos.starts_with("Bottom")) {
            ay = 0.f;
            y = margin;
        }

        if (pos.ends_with("Left")) {
            ax = 0.f;
            x = margin;
        }
        else if (pos.ends_with("Right")) {
            ax = 1.f;
            x = win.width - margin;
        }

        label->setAnchorPoint({ax, ay});
        label->setPosition({x, y});
    }
};
