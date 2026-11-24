#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/loader/SettingV3.hpp>

#include "ui/NeoBackendLayer.hpp"
#include "ui/RestartButtons.hpp"
#include "BetterIncludes.hpp"
#include "updater/NeoUpdater.hpp"
#include "ui/NeoCustomMenu.hpp"
#include "ui/NeoHub/NeoHubLayer.hpp"
#ifndef GEODE_IS_IOS

using namespace geode::prelude;

#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>

class $modify(PctPlayLayer, PlayLayer) {
	struct Fields {
		CCLabelTTF* ttfLabel = nullptr;
		std::string ttfPath;
	};

	void setupHasCompleted() {
		PlayLayer::setupHasCompleted();
		this->restylePercentage();
	}

	void updateProgressbar() {
		PlayLayer::updateProgressbar();
		this->restylePercentage();
	}

	void restylePercentage() {
		auto label = m_percentageLabel;
		if (!label) return;
		auto mod = Mod::get();

		float size = mod->getSettingValue<double>("size");
		auto color = mod->getSettingValue<ccColor3B>("color");
		auto font = mod->getSettingValue<std::string>("font");
		auto pos = mod->getSettingValue<std::string>("position");
		auto opacity = static_cast<GLubyte>(mod->getSettingValue<int64_t>("opacity") * 255 / 100);

		// custom ttf, needs a separate label
		std::string ttfPath;
		if (font == "Custom") {
			auto path = mod->getSettingValue<asp::fs::Path>("custom-font"); // THATS THE ENTIRE THING TO PORT BTW
			if (!path.string().empty() && path.exists()) ttfPath = path.string();
			else font = "bigFont";
		}

		CCNode* target = label;
		CCLabelTTF* ttf = nullptr;
		auto f = m_fields.self();

		if (!ttfPath.empty()) {
			if (f->ttfLabel && f->ttfPath != ttfPath) {
				f->ttfLabel->removeFromParent();
				f->ttfLabel = nullptr;
			}
			if (!f->ttfLabel) {
				f->ttfLabel = CCLabelTTF::create(label->getString(), ttfPath.c_str(), 60.f);
				f->ttfLabel->setID("ttf-percentage-label"_spr);
				f->ttfPath = ttfPath;
				label->getParent()->addChild(f->ttfLabel, label->getZOrder());
			}
			if (strcmp(f->ttfLabel->getString(), label->getString()) != 0)
				f->ttfLabel->setString(label->getString());

			f->ttfLabel->setVisible(label->isVisible());
			label->setOpacity(0); // original stays in the tree, just invisible
			target = f->ttfLabel;
			ttf = f->ttfLabel;
		} else {
			if (f->ttfLabel) {
				f->ttfLabel->removeFromParent();
				f->ttfLabel = nullptr;
			}
			auto fnt = font + ".fnt";
			if (strcmp(label->getFntFile(), fnt.c_str()) != 0) label->setFntFile(fnt.c_str());
		}

		target->setScale(size);
		if (ttf) {
			ttf->setColor(color);
			ttf->setOpacity(opacity);
		} else {
			label->setColor(color);
			label->setOpacity(opacity);
		}

		auto win = CCDirector::get()->getWinSize();
		float margin = static_cast<float>(mod->getSettingValue<double>("margin"));
		float ax = 0.5f, ay = 0.5f, x = win.width / 2, y = win.height / 2;

		if (pos.starts_with("Top")) { ay = 1.f; y = win.height - margin; }
		else if (pos.starts_with("Bottom")) { ay = 0.f; y = margin; }

		if (pos.ends_with("Left")) { ax = 0.f; x = margin; }
		else if (pos.ends_with("Right")) { ax = 1.f; x = win.width - margin; }

		target->setAnchorPoint({ax, ay});
		target->setPosition({x, y});
	}
};
// Neo Buttons
//class $modify(NeoCustomMenuButton, MenuLayer) {

//    bool init() {
  //      if (!MenuLayer::init())
      //      return false;

   //     auto winSize = CCDirector::get()->getWinSize();

        // Button menu
//        auto menu = CCMenu::create();

      //  if (!menu)
     //       return true;

     //   menu->setID("neo-custom-menu-button-menu");
       // menu->setPosition(CCPointZero);

     //   this->addChild(menu, 1000);

        // Button sprite
//        auto sprite = ButtonSprite::create(
        //    "Credits",
      //      "goldFont.fnt",
       //     "GJ_button_03.png",
     //       0.7f
 //       );

      //  if (!sprite)
     //       return true;

     //   auto button = CCMenuItemSpriteExtra::create(
 //           sprite,
  //          this,
    //        menu_selector(NeoCustomMenuButton::onOpenNeoMenu)
    //    );

        //if (!button)
         //   return true;

//        button->setPosition({
//            winSize.width - 42.f,
//            winSize.height - 45.f
//        });

//        menu->addChild(button);

//        return true;
//    }

//    void onOpenNeoMenu(CCObject*) {
//        neo::ui::openCustomMenu();
//    }
//};

// --------------------------------------------------
// MENU LAYER
// --------------------------------------------------
class $modify(AltMenuLayer, MenuLayer) {

    // ----------------------------------------------
    // SCENE
    // ----------------------------------------------

    static CCScene* scene(bool p0) {
        if (!neo::ui::RestartButtons::consumeReloadFlag()) {
            return MenuLayer::scene(p0);
        }

        return MenuLayer::scene(false);
    }

    // ----------------------------------------------
    // INIT
    // ----------------------------------------------

    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::get()->getWinSize();

        // ------------------------------------------
        // BUTTON CONTAINER
        // ------------------------------------------

auto menu = typeinfo_cast<CCMenu*>(
    this->getChildByID("close-menu")
);

if (!menu) {
    menu = CCMenu::create();

    menu->setID("close-menu");
    menu->setContentSize({33.25f, 200.f});
    menu->ignoreAnchorPointForPosition(false);

    this->addChild(menu);
}

        menu->setAnchorPoint({0.f, 1.f});

        menu->setPosition(
            winSize.width / 300.f,
            winSize.height - winSize.width / 300.f
        );

        auto layout = ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setGap(4.f)
            ->setCrossAxisOverflow(false);

        menu->setLayout(layout);

        // ------------------------------------------
        // EXIT BUTTON
        // ------------------------------------------

        if (Mod::get()->getSettingValue<bool>("exit")) {

            if (!menu->getChildByID("close-button")) {
                auto sprite = CCSprite::createWithSpriteFrameName(
                    "GJ_closeBtn_001.png"
                );

                if (sprite) {
                    sprite->setScale(0.7f);

                    auto button = CCMenuItemSpriteExtra::create(
                        sprite,
                        this,
                        menu_selector(MenuLayer::onQuit)
                    );

                    if (button) {
                        button->setID("close-button");
                        menu->addChild(button);
                    }
                }
            }

        } else {
            menu->removeChildByID("close-button");
        }

        // ------------------------------------------
        // RESTART / RELOAD BUTTONS
        // ------------------------------------------

        neo::ui::RestartButtons::setup(
            menu,
            this,
            menu_selector(AltMenuLayer::onRestart),
            menu_selector(AltMenuLayer::onReload)
        );

        // ------------------------------------------
        // SIZE
        // ------------------------------------------

        const auto scale =
            Mod::get()->getSettingValue<double>("size");

        menu->setScale(static_cast<float>(scale));
        menu->updateLayout();

        // ------------------------------------------
        // NEO INSPECTOR BUTTON
        // ------------------------------------------

        if (!this->getChildByID("neo-inspector-menu")) {

            auto inspectorSprite = ButtonSprite::create(
                "Interceptor",
                "goldFont.fnt",
                "GJ_button_01.png",
                0.65f
            );

            if (inspectorSprite) {

                auto inspectorButton = CCMenuItemSpriteExtra::create(
                    inspectorSprite,
                    this,
                    menu_selector(AltMenuLayer::onNeoInspector)
                );

                if (inspectorButton) {

                    auto inspectorMenu = CCMenu::create(
                        inspectorButton,
                        nullptr
                    );

                    if (inspectorMenu) {
                        inspectorMenu->setID("neo-inspector-menu");

                        inspectorMenu->setPosition(
                            ccp(
                                winSize.width - 65.0f,
                                winSize.height - 35.0f
                            )
                        );

                        this->addChild(inspectorMenu, 100);
                    }
                }
            }
        }

        // ------------------------------------------
        // NEO HUB BUTTON
        // ------------------------------------------

     //  if (!this->getChildByID("neo-hub-menu")) {

      //      auto hubSprite = ButtonSprite::create(
      //          "Neo------++++HUB",
         //       "goldFont.fnt",
          //      "GJ_button_01.png",
        //        0.65f
      //      );

        //  if (hubSprite) {

            //    auto hubButton = CCMenuItemSpriteExtra::create(
                 //   hubSprite,
                 //   this,
         //           menu_selector(AltMenuLayer::onOpenNeoHub)
      //          );

            //    if (hubButton) {

                 //   auto hubMenu = CCMenu::create();

                //    if (hubMenu) {
                 //       hubMenu->setID("neo-hub-menu");
                    //    hubMenu->setPosition(CCPointZero);

                     //   hubButton->setPosition(
                         //   ccp(
                           //     winSize.width - 65.f,
                        //        winSize.height - 80.f
                   //         )
                     //   );

                      //  hubMenu->addChild(hubButton);

                     //   this->addChild(hubMenu, 100);
                    //}
              //  }
        //    }
      //  }
        
        return true;
    }

    // ----------------------------------------------
    // BUTTON CALLBACKS
    // ----------------------------------------------

    void onRestart(CCObject*) {
        neo::ui::RestartButtons::restart();
    }

    void onReload(CCObject*) {
        neo::ui::RestartButtons::reload();
    }

   // void onOpenNeoHub(CCObject*) {
    //    log::info("[NeoHub] Opening Neo Hub");

     //   neo::ui::openNeoHub(this);
//    }

    void onNeoInspector(CCObject*) {
        log::info("[NeoInspector] Opening Inspector");
        NeoBackendLayer::open(this);
    }
};

// --------------------------------------------------
// GAME LOADED
// --------------------------------------------------
$on_game(Loaded) {
    // ----------------------------------------------
    // NEO PS PLUS UPDATER
    // ----------------------------------------------
neo::updater::NeoUpdater::checkForUpdates();
    // ----------------------------------------------
    // RESTART / RELOAD KEYBINDS
    // ----------------------------------------------

    neo::ui::RestartButtons::registerKeybinds();
}

#endif
