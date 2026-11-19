#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <filesystem>
#include <string>
#include <cstring>
#include <system_error>

using namespace geode::prelude;

class $modify(PctPlayLayer, PlayLayer) {

    struct Fields {
        CCLabelBMFont* bmLabel = nullptr;
        CCLabelTTF* ttfLabel = nullptr;
        std::string activeFont;
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        this->restylePercentage();
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        this->restylePercentage();
    }

    void removeCustomLabel() {
        auto f = m_fields.self();
        

        if (f->bmLabel) {
            f->bmLabel->removeFromParentAndCleanup(true);
            f->bmLabel = nullptr;
        }

        if (f->ttfLabel) {
            f->ttfLabel->removeFromParentAndCleanup(true);
            f->ttfLabel = nullptr;
        }

        f->activeFont.clear();
    }

    void restylePercentage() {
    log::info("[PercentageCustomizer] 1. Entered");

    auto label = m_percentageLabel;

    log::info(
        "[PercentageCustomizer] 2. Label: {}",
        static_cast<void*>(label)
    );

    if (!label) {
        log::warn("[PercentageCustomizer] Label is null");
        return;
    }

    log::info("[PercentageCustomizer] 3. Before getParent");

    auto parent = label->getParent();

    log::info(
        "[PercentageCustomizer] 4. Parent: {}",
        static_cast<void*>(parent)
    );

    if (!parent) {
        log::warn("[PercentageCustomizer] Parent is null");
        return;
    }

    log::info("[PercentageCustomizer] 5. Before Mod::get");

    auto mod = Mod::get();

    log::info(
        "[PercentageCustomizer] 6. Mod: {}",
        static_cast<void*>(mod)
    );

    if (!mod) {
        log::warn("[PercentageCustomizer] Mod is null");
        return;
    }

    log::info("[PercentageCustomizer] 7. Before Fields");

    auto f = m_fields.self();

    log::info("[PercentageCustomizer] 8. Fields obtained");

    // No realizar ninguna modificación todavía.
}

        float size = static_cast<float>(
            mod->getSettingValue<double>("size")
        );

        auto color = mod->getSettingValue<ccColor3B>("color");

        auto font = mod->getSettingValue<std::string>("font");
        auto pos = mod->getSettingValue<std::string>("position");

        auto opacity = static_cast<GLubyte>(
            mod->getSettingValue<int64_t>("opacity") * 255 / 100
        );

        // Determine selected font
        bool useTTF = false;
        std::string fontPath;

        if (font == "Custom") {
            auto path = mod->getSettingValue<std::string>("custom-font");

            std::error_code ec;

            if (!path.empty() && std::filesystem::exists(path, ec) && !ec) {
                useTTF = true;
                fontPath = path;
            } else {
                font = "bigFont";
            }
        }

        if (!useTTF) {
            fontPath = font + ".fnt";
        }

        std::string fontKey = (useTTF ? "ttf:" : "bmf:") + fontPath;

        // Recreate label only when the selected font changes
        if (f->activeFont != fontKey) {
            CCNode* newLabel = nullptr;

            if (useTTF) {
                f->ttfLabel = CCLabelTTF::create(
                    label->getString(),
                    fontPath.c_str(),
                    60.f
                );

                newLabel = f->ttfLabel;
            } else {
                f->bmLabel = CCLabelBMFont::create(
                    label->getString(),
                    fontPath.c_str()
                );

                newLabel = f->bmLabel;
            }

            if (!newLabel) {
                removeCustomLabel();
                label->setVisible(true);
                return;
            }

            // Remove previous custom label
            if (useTTF) {
                if (f->bmLabel) {
                    f->bmLabel->removeFromParentAndCleanup(true);
                    f->bmLabel = nullptr;
                }
            } else {
                if (f->ttfLabel) {
                    f->ttfLabel->removeFromParentAndCleanup(true);
                    f->ttfLabel = nullptr;
                }
            }

            if (useTTF) {
                f->ttfLabel->setID("ttf-percentage-label"_spr);
            } else {
                f->bmLabel->setID("bmf-percentage-label"_spr);
            }

            parent->addChild(newLabel, label->getZOrder());

            f->activeFont = fontKey;
        }

        // Synchronize text and visibility
        CCNode* target = nullptr;

        if (useTTF) {
            target = f->ttfLabel;

            if (!target) return;

            auto ttf = f->ttfLabel;

            if (strcmp(ttf->getString(), label->getString()) != 0) {
                ttf->setString(label->getString());
            }
        } else {
            target = f->bmLabel;

            if (!target) return;

            auto bm = f->bmLabel;

            if (strcmp(bm->getString(), label->getString()) != 0) {
                bm->setString(label->getString());
            }
        }

        // Hide original without modifying its BMFont
        bool originalVisible = label->isVisible();

label->setVisible(false);
target->setVisible(originalVisible);

        // Apply appearance
        target->setScale(size);

        if (useTTF) {
            f->ttfLabel->setColor(color);
            f->ttfLabel->setOpacity(opacity);
        } else {
            f->bmLabel->setColor(color);
            f->bmLabel->setOpacity(opacity);
        }

        // Position
        auto win = CCDirector::get()->getWinSize();

        float margin = static_cast<float>(
            mod->getSettingValue<double>("margin")
        );

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

        target->setAnchorPoint({ax, ay});
        target->setPosition({x, y});
    }
};
