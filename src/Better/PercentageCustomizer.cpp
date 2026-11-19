#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PctPlayLayer, PlayLayer) {

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        log::info("[PercentageCustomizer] Setup completed");

        this->restylePercentage();
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        this->restylePercentage();
    }

    void restylePercentage() {
        log::info("[PercentageCustomizer] Step 1");

        auto label = m_percentageLabel;

        if (!label) {
            log::warn("[PercentageCustomizer] Label is null");
            return;
        }

        log::info("[PercentageCustomizer] Step 2");

        auto parent = label->getParent();

        if (!parent) {
            log::warn("[PercentageCustomizer] Parent is null");
            return;
        }

        log::info("[PercentageCustomizer] Step 3");

        auto mod = Mod::get();

        if (!mod) {
            log::error("[PercentageCustomizer] Mod is null");
            return;
        }

        log::info("[PercentageCustomizer] Step 4");

        float size = static_cast<float>(
            mod->getSettingValue<double>("size")
        );

        log::info("[PercentageCustomizer] Step 5");

        label->setScale(size);

        log::info("[PercentageCustomizer] Step 6");

        auto color = mod->getSettingValue<ccColor3B>("color");

        label->setColor(color);

        log::info("[PercentageCustomizer] Step 7");

        auto opacity = static_cast<GLubyte>(
            mod->getSettingValue<int64_t>("opacity") * 255 / 100
        );

        label->setOpacity(opacity);

        log::info("[PercentageCustomizer] Step 8");

        auto pos = mod->getSettingValue<std::string>("position");

        auto win = CCDirector::get()->getWinSize();

        float margin = static_cast<float>(
            mod->getSettingValue<double>("margin")
        );

        log::info("[PercentageCustomizer] Step 9");

        float ax = 0.5f;
        float ay = 0.5f;

        float x = win.width / 2;
        float y = win.height / 2;

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

        log::info("[PercentageCustomizer] Step 10 - Completed");
    }
};
