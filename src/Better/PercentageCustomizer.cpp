#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PctPlayLayer, PlayLayer) {

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        log::info("[PercentageCustomizer] setupHasCompleted");

        this->restylePercentage();
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        this->restylePercentage();
    }

    void restylePercentage() {
        log::info("[PercentageCustomizer] 1. Entered");

        // Obtener el porcentaje original.
        auto label = m_percentageLabel;

        log::info(
            "[PercentageCustomizer] 2. Label: {}",
            static_cast<void*>(label)
        );

        if (!label) {
            log::warn("[PercentageCustomizer] Percentage label is null");
            return;
        }

        // Obtener el padre.
        auto parent = label->getParent();

        log::info(
            "[PercentageCustomizer] 3. Parent: {}",
            static_cast<void*>(parent)
        );

        if (!parent) {
            log::warn("[PercentageCustomizer] Label parent is null");
            return;
        }

        // Obtener la instancia del mod.
        auto mod = Mod::get();

        if (!mod) {
            log::error("[PercentageCustomizer] Mod instance is null");
            return;
        }

        log::info("[PercentageCustomizer] 4. Mod obtained");

        // Leer configuraciones.
        float size = static_cast<float>(
            mod->getSettingValue<double>("size")
        );

        auto color = mod->getSettingValue<ccColor3B>("color");

        auto opacity = static_cast<GLubyte>(
            mod->getSettingValue<int64_t>("opacity") * 255 / 100
        );

        auto pos = mod->getSettingValue<std::string>("position");

        log::info("[PercentageCustomizer] 5. Settings obtained");

        // Aplicar apariencia.
        label->setScale(size);
        label->setColor(color);
        label->setOpacity(opacity);

        log::info("[PercentageCustomizer] 6. Appearance applied");

        // Calcular posición.
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

        label->setAnchorPoint({ax, ay});
        label->setPosition({x, y});

        log::info("[PercentageCustomizer] 7. Position applied");
    }
};
