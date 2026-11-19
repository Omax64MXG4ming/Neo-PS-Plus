#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PctPlayLayer, PlayLayer) {

    struct Fields {
        CCLabelTTF* ttfLabel = nullptr;
        std::string ttfPath;
    };

    // --------------------------------------------------
    // Lifecycle
    // --------------------------------------------------

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        restylePercentage();
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        restylePercentage();
    }

    // --------------------------------------------------
    // Remove custom label
    // --------------------------------------------------

    void removeCustomLabel() {
        auto fields = m_fields.self();

        if (!fields->ttfLabel)
            return;

        fields->ttfLabel->removeFromParentAndCleanup(true);
        fields->ttfLabel = nullptr;
        fields->ttfPath.clear();

        if (m_percentageLabel)
            m_percentageLabel->setOpacity(255);
    }

    // --------------------------------------------------
    // Percentage styling
    // --------------------------------------------------

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

        auto fields = m_fields.self();

        // --------------------------------------------------
        // Custom font
        // --------------------------------------------------

        std::string ttfPath;

        if (font == "Custom") {
            auto path = mod->getSettingValue<std::filesystem::path>(
                "custom-font"
            );

            std::error_code ec;

            if (!path.empty() && std::filesystem::exists(path, ec)) {
                ttfPath = path.string();
            } else {
                font = "bigFont";
            }
        }

        CCNode* target = label;
        CCLabelTTF* ttf = nullptr;

        if (!ttfPath.empty()) {

            // Recreate if font changed.
            if (fields->ttfLabel && fields->ttfPath != ttfPath) {
                removeCustomLabel();
            }

            // Create custom label.
            if (!fields->ttfLabel) {
                auto parent = label->getParent();

                if (!parent)
                    return;

                fields->ttfLabel = CCLabelTTF::create(
                    label->getString(),
                    ttfPath.c_str(),
                    60.f
                );

                if (!fields->ttfLabel)
                    return;

                fields->ttfLabel->setID(
                    "ttf-percentage-label"_spr
                );

                fields->ttfPath = ttfPath;

                parent->addChild(
                    fields->ttfLabel,
                    label->getZOrder()
                );
            }

            ttf = fields->ttfLabel;

            // Synchronize text.
            if (strcmp(ttf->getString(), label->getString()) != 0) {
                ttf->setString(label->getString());
            }

            ttf->setVisible(label->isVisible());

            // Hide original label.
            label->setOpacity(0);

            target = ttf;
        }
        else {
            // Restore original label.
            removeCustomLabel();

            auto fnt = font + ".fnt";

            if (strcmp(label->getFntFile(), fnt.c_str()) != 0) {
                label->setFntFile(fnt.c_str());
            }
        }

        // --------------------------------------------------
        // Appearance
        // --------------------------------------------------

        target->setScale(size);
        target->setColor(color);
        target->setOpacity(opacity);

        // --------------------------------------------------
        // Position
        // --------------------------------------------------

        auto win = CCDirector::get()->getWinSize();

        float ax = 0.5f;
        float ay = 0.5f;

        float x = win.width / 2.f;
        float y = win.height / 2.f;

        // Vertical alignment.
        if (pos.starts_with("Top")) {
            ay = 1.f;
            y = win.height - margin;
        }
        else if (pos.starts_with("Bottom")) {
            ay = 0.f;
            y = margin;
        }

        // Horizontal alignment.
        if (pos.ends_with("Left")) {
            ax = 0.f;
            x = margin;
        }
        else if (pos.ends_with("Right")) {
            ax = 1.f;
            x = win.width - margin;
        }

        target->setAnchorPoint({ax, ay});
        target->setPosition({x, y});
    }

    // --------------------------------------------------
    // Cleanup
    // --------------------------------------------------

    void onExit() {
        removeCustomLabel();
        PlayLayer::onExit();
    }
};
