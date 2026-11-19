#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PctPlayLayer, PlayLayer) {

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        log::info("[PercentageCustomizer] Base setup completed");

        auto label = m_percentageLabel;

        log::info(
            "[PercentageCustomizer] Label pointer: {}",
            static_cast<void*>(label)
        );

        if (!label) {
            log::warn("[PercentageCustomizer] Label is null");
            return;
        }

        log::info("[PercentageCustomizer] Label exists");
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
    }
};
