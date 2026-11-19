#include <Geode/Geode.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>
#ifndef GEODE_IS_IOS
using namespace geode::prelude;

// why u guys move this on top ever? - user95401

class $modify(GauntletSelectLayerFix, GauntletSelectLayer) {
    $override void setupGauntlets() {
        GauntletSelectLayer::setupGauntlets();

        findFirstChildRecursive<ExtendedLayer>(
            this,
            [](ExtendedLayer* gauntlet_pages) {
                if (gauntlet_pages->getChildrenCount() > 1)
                    return false;

                gauntlet_pages->setAnchorPoint(CCPointMake(1.f, 0.f));
                gauntlet_pages->ignoreAnchorPointForPosition(false);

                return true;
            }
        );
    }
};
