#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
using namespace geode::prelude;

static bool s_enabled = true;
static float s_cullDistance = 500.f;
static float s_cullInterval = 0.35f;

$on_mod(Loaded) {
	auto* mod = Mod::get();
	s_enabled = mod->getSettingValue<bool>("enabled");
	s_cullDistance = std::clamp(static_cast<float>(mod->getSettingValue<double>("cull-distance")), 150.f, 2000.f);
	s_cullInterval = std::clamp(static_cast<float>(mod->getSettingValue<double>("cull-interval")), 0.05f, 1.5f);

	listenForSettingChanges<bool>("enabled", [](bool v) {
		s_enabled = v;
	});
	listenForSettingChanges<double>("cull-distance", [](double v) {
		s_cullDistance = std::clamp(static_cast<float>(v), 150.f, 2000.f);
	});
	listenForSettingChanges<double>("cull-interval", [](double v) {
		s_cullInterval = std::clamp(static_cast<float>(v), 0.05f, 1.5f);
	});
}

// orbs, pads, portals, coins, triggers — anything that affects gameplay
static bool isProtectedObject(GameObject* obj) {
	if (typeinfo_cast<EffectGameObject*>(obj)) return true;
	if (typeinfo_cast<StartPosObject*>(obj)) return true;

	switch (obj->m_objectType) {
	case GameObjectType::YellowJumpPad:
	case GameObjectType::PinkJumpPad:
	case GameObjectType::RedJumpPad:
	case GameObjectType::GravityPad:
	case GameObjectType::SpiderPad:
	case GameObjectType::YellowJumpRing:
	case GameObjectType::PinkJumpRing:
	case GameObjectType::RedJumpRing:
	case GameObjectType::GreenRing:
	case GameObjectType::GravityRing:
	case GameObjectType::DashRing:
	case GameObjectType::GravityDashRing:
	case GameObjectType::DropRing:
	case GameObjectType::CustomRing:
	case GameObjectType::SpiderOrb:
	case GameObjectType::TeleportOrb:
	case GameObjectType::ShipPortal:
	case GameObjectType::CubePortal:
	case GameObjectType::BallPortal:
	case GameObjectType::UfoPortal:
	case GameObjectType::WavePortal:
	case GameObjectType::RobotPortal:
	case GameObjectType::SpiderPortal:
	case GameObjectType::SwingPortal:
	case GameObjectType::InverseGravityPortal:
	case GameObjectType::NormalGravityPortal:
	case GameObjectType::DualPortal:
	case GameObjectType::SoloPortal:
	case GameObjectType::MiniSizePortal:
	case GameObjectType::RegularSizePortal:
	case GameObjectType::NormalMirrorPortal:
	case GameObjectType::InverseMirrorPortal:
	case GameObjectType::TeleportPortal:
	case GameObjectType::GravityTogglePortal:
	case GameObjectType::SecretCoin:
	case GameObjectType::UserCoin:
	case GameObjectType::Collectible:
	case GameObjectType::Modifier:
	case GameObjectType::AnimatedHazard:
	case GameObjectType::CollisionObject:
		return true;
	default:
		return false;
	}
}

class $modify(PlayLayer) {
	struct Fields {
		std::vector<Ref<GameObject>> m_hiddenBehindPlayer;
		float m_checkTimer = 0.f;
	};

	void update(float dt) {
		PlayLayer::update(dt);

		if (!s_enabled || m_hasCompletedLevel)
			return;
		if (!m_player1 || !m_objects)
			return;
		if (m_player1->m_isDead)
			return;

		m_fields->m_checkTimer += dt;
		if (m_fields->m_checkTimer < s_cullInterval)
			return;
		m_fields->m_checkTimer = 0.f;

		auto playerX = m_player1->getPositionX();
		auto cutoff = playerX - s_cullDistance; // only hide stuff far enough behind

		for (auto* obj : CCArrayExt<GameObject*>(m_objects)) {
			if (!obj) continue;

			if (!obj->isVisible() || obj->getPositionX() > cutoff)
				continue;
			if (isProtectedObject(obj))
				continue;

			obj->setVisible(false);
			m_fields->m_hiddenBehindPlayer.push_back(obj);
		}
	}

	void restoreHiddenObjects() {
		for (auto& obj : m_fields->m_hiddenBehindPlayer) {
			if (obj) obj->setVisible(true);
		}
		m_fields->m_hiddenBehindPlayer.clear();
		m_fields->m_checkTimer = 0.f;
	}

	void resetLevel() {
		restoreHiddenObjects();
		PlayLayer::resetLevel();
	}

	void onQuit() {
		restoreHiddenObjects();
		PlayLayer::onQuit();
	}
};
