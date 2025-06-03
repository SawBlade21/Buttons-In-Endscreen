#include <Geode/Geode.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/FLAlertLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LikeItemLayer.hpp>
#include <Geode/modify/RateStarsLayer.hpp>
#include <Geode/modify/RateDemonLayer.hpp>
#include <Geode/modify/InfoLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CommentCell.hpp>

using namespace geode::prelude;

bool isRated = false;
bool useButton = false;

class $modify(FLAlertLayer) {
	void destructor() {
		if (!PlayLayer::get()) {
			FLAlertLayer::~FLAlertLayer();
			return;
		}

		bool popup = (this->getID() == "popupLayer"_spr);
		CCNode* layer = PlayLayer::get()->getChildByID("fakeInfoLayer"_spr);

		if (layer && popup) {
			layer->removeFromParentAndCleanup(false);
		} 
		else if (popup && typeinfo_cast<InfoLayer*>(this)) {
			EndLevelLayer* endscreen = static_cast<EndLevelLayer*>(PlayLayer::get()->getChildByID("EndLevelLayer"));
			Loader::get()->queueInMainThread([endscreen] {
				if (CCTouchHandler* handler = CCTouchDispatcher::get()->findHandler(endscreen)) 
					CCTouchDispatcher::get()->setPriority(-501, handler->getDelegate());
			});
			Loader::get()->queueInMainThread([endscreen] {
				if (CCTouchHandler* handler = CCTouchDispatcher::get()->findHandler(endscreen)) 
					CCTouchDispatcher::get()->setPriority(-502, handler->getDelegate());
			});
			//endscreen->setTouchEnabled(true);
		}

		FLAlertLayer::~FLAlertLayer();
	}
};

class $modify(LevelInfoLayer) {
	void levelDownloadFailed(int w) {
		if (!PlayLayer::get())
			LevelInfoLayer::levelDownloadFailed(w);
	}

	void levelUpdateFailed(int w) {
		if (!PlayLayer::get())
			LevelInfoLayer::levelUpdateFailed(w);
	}
};

class likeBtnDelegate : public LikeItemDelegate {
	public:
	CCMenuItemSpriteExtra* button;
	LevelInfoLayer* layer;
	virtual void likedItem(LikeItemType, int, bool) override {
		auto sprite = CCSprite::createWithSpriteFrameName("GJ_like2Btn2_001.png");
		sprite->setScale(-1);
		button->setSprite(sprite);
		button->setScale(-1);
		layer->removeFromParentAndCleanup(true);
	}
};

class rateBtnDelegate : public RateLevelDelegate {
	public:
	CCMenuItemSpriteExtra* button;
	LevelInfoLayer* layer;
	void onRate(CCObject* sender) {
		auto sprite = CCSprite::createWithSpriteFrameName("GJ_starBtn2_001.png");
		if (isRated) 
			sprite = CCSprite::createWithSpriteFrameName("GJ_rateDiffBtn2_001.png");
		sprite->setScale(-1);
		button->setSprite(sprite);
		button->setScale(-1);
		layer->removeFromParentAndCleanup(true);
	}
};

LevelInfoLayer* createInfoLayer() {
	auto infoLayer = LevelInfoLayer::create(PlayLayer::get()->m_level, false);
	infoLayer->setID("fakeInfoLayer"_spr);
	infoLayer->setKeyboardEnabled(false);
	infoLayer->setTouchEnabled(false);
	infoLayer->setVisible(false);
	PlayLayer::get()->addChild(infoLayer);
	return infoLayer;
}

class Buttons {
	public:
	void likeButton(CCObject* obj) {
		if (!useButton) return;
		LevelInfoLayer* infoLayer = createInfoLayer();
		infoLayer->onLike(nullptr);
		auto delegate = new likeBtnDelegate();
		delegate->button = static_cast<CCMenuItemSpriteExtra*>(obj);
		delegate->layer = infoLayer;
		auto likeItemLayer = static_cast<LikeItemLayer*>(CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject());
		likeItemLayer->m_likeDelegate = delegate;
		likeItemLayer->setID("popupLayer"_spr);
	}

	void rateButton(CCObject* obj) {
		if (!useButton) return;
		LevelInfoLayer* infoLayer = createInfoLayer();
		if (isRated) {
			infoLayer->onRateDemon(nullptr);
			CCArray* children = CCDirector::sharedDirector()->getRunningScene()->getChildren();
			if (auto ratePopup = typeinfo_cast<RateDemonLayer*>(children->lastObject()))
				ratePopup->setID("popupLayer"_spr);
			else if (auto ratePopup = typeinfo_cast<FLAlertLayer*>(children->lastObject())) {
				ratePopup->keyBackClicked();
				static_cast<RateDemonLayer*>(children->lastObject())->setID("popupLayer"_spr);
			}
		}
		else {
			infoLayer->onRateStars(nullptr);
			CCArray* children = CCDirector::sharedDirector()->getRunningScene()->getChildren();
			if (auto ratePopup = typeinfo_cast<RateStarsLayer*>(children->lastObject()))
				ratePopup->setID("popupLayer"_spr);
			else if (auto ratePopup = typeinfo_cast<FLAlertLayer*>(children->lastObject())) {
				ratePopup->keyBackClicked();
				static_cast<RateStarsLayer*>(children->lastObject())->setID("popupLayer"_spr);
			}
		}
	}

	/*void lbButton(CCObject* obj) {
		if (!useButton) return;
		auto level = PlayLayer::get()->m_level;
		auto lbType = static_cast<LevelLeaderboardType>(GameManager::get()->getIntGameVariable("0098"));
		auto lbMode = static_cast<LevelLeaderboardMode>(GameManager::get()->getIntGameVariable("0164"));
		auto lbMenu = LevelLeaderboard::create(level, lbType, lbMode);
		lbMenu->show();
		lbMenu->setID("popupLayer"_spr);
	}*/

	void infoButton(CCObject* obj) {
		if (!useButton) return;
		auto level = PlayLayer::get()->m_level;
		if (level->m_isEditable) {
			auto editLayer = EditLevelLayer::create(level);
			editLayer->setID("fakeInfoLayer"_spr);
			editLayer->setKeyboardEnabled(false);
			editLayer->setVisible(false);
			PlayLayer::get()->addChild(editLayer);
			editLayer->onLevelInfo(nullptr);
		} else {
			LevelInfoLayer* infoLayer = createInfoLayer();
			infoLayer->onLevelInfo(nullptr);
		}
		CCArray* children = CCDirector::sharedDirector()->getRunningScene()->getChildren();
		static_cast<FLAlertLayer*>(children->lastObject())->setID("popupLayer"_spr);
	}

	void commentsButton(CCObject* obj) {
		if (!useButton) return;
		auto commentsLayer = InfoLayer::create(PlayLayer::get()->m_level, nullptr, nullptr);
		commentsLayer->show();
		commentsLayer->setID("popupLayer"_spr);
		// LevelInfoLayer* infoLayer = createInfoLayer();
		// infoLayer->onInfo(nullptr);
		// CCArray* children = CCDirector::sharedDirector()->getRunningScene()->getChildren();
		// static_cast<FLAlertLayer*>(children->lastObject())->setID("popupLayer"_spr);
	}
};

class $modify (RateDemonLayer) {
	void onRate(CCObject* sender) {
		RateDemonLayer::onRate(sender);
		if (!PlayLayer::get()) 
			return;
		CCNode* layer = PlayLayer::get()->getChildByID("EndLevelLayer")->getChildByID("main-layer")->getChildByID("customMenu"_spr);
		CCMenuItemSpriteExtra* button = static_cast<CCMenuItemSpriteExtra*>(layer->getChildByID("rateButton"_spr));
		auto sprite = CCSprite::createWithSpriteFrameName("GJ_rateDiffBtn2_001.png");
		sprite->setScale(-1);
		button->setSprite(sprite);
		button->setScale(-1);
	}
};

class $modify (RateStarsLayer) {
	void onRate(CCObject* sender) {
		RateStarsLayer::onRate(sender);
		if (!PlayLayer::get()) 
			return;
		CCNode* layer = PlayLayer::get()->getChildByID("EndLevelLayer")->getChildByID("main-layer")->getChildByID("customMenu"_spr);
		CCMenuItemSpriteExtra* button = static_cast<CCMenuItemSpriteExtra*>(layer->getChildByID("rateButton"_spr));
		auto sprite = CCSprite::createWithSpriteFrameName("GJ_starBtn2_001.png");
		sprite->setScale(-1);
		button->setSprite(sprite);
		button->setScale(-1);
	}
};

class $modify (EndLevelLayer) {
	bool findID(CCArray* keys, int levelID, bool likeButton) {
		std::string stringLevelID = (likeButton) ? "like_1_" + std::to_string(levelID) : std::to_string(levelID);
		CCObject* obj;
     	CCARRAY_FOREACH(keys, obj) {
        	CCString* str = static_cast<CCString*>(obj);	
        	if (str) {			
				size_t position = std::string(str->getCString()).find(stringLevelID);
				if (position != std::string::npos)
					return true;
			}				
     	}
		return false;
	}

	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("EndLevelLayer::customSetup", Priority::Last)) {
			geode::log::warn("Failed to set hook priority.");
		}
	}

	void customSetup() {
		EndLevelLayer::customSetup();
		useButton = true;
		auto levelID = PlayLayer::get()->m_level->m_levelID.value();

		auto menu = CCMenu::create();
		menu->setID("customMenu"_spr);
		//menu->setTouchPriority(-550);
		this->getChildByID("main-layer")->addChild(menu);

		Loader::get()->queueInMainThread([this] {
			if (CCTouchHandler* handler = CCTouchDispatcher::get()->findHandler(this)) 
				CCTouchDispatcher::get()->setPriority(-502, handler->getDelegate());
		});

		if (Mod::get()->getSettingValue<bool>("show-info-button")) {
			auto infoSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
			infoSprite->setScale(0.9);
			auto infoButton = CCMenuItemSpriteExtra::create(infoSprite, this, menu_selector(Buttons::infoButton));
			infoButton->setID("infoButton"_spr);
			if (this->getChildByID("main-layer")->getChildByID("button-menu")->getChildByID("absolllute.megahack/cheat-indicator-info") || this->getChildByID("main-layer")->getChildByID("thesillydoggo.qolmod/info-menu"))
				infoButton->setPosition({-164, 77});
			else
				infoButton->setPosition({-181, 127});
			menu->addChild(infoButton);
		}

		if (levelID < 23 || (levelID > 5000 && levelID < 5005) || levelID == 3001 || PlayLayer::get()->m_level->m_isEditable) 
			return;
		bool grayButton = false;
		bool grayStarButton = false;

		/*if (!PlayLayer::get()->m_levelSettings->m_platformerMode && Mod::get()->getSettingValue<bool>("show-leaderboard-button")) {
			auto lbSprite = CCSprite::createWithSpriteFrameName("GJ_levelLeaderboardBtn_001.png");
			auto lbButton = CCMenuItemSpriteExtra::create(lbSprite, this, menu_selector(Buttons::lbButton));
			lbButton->setID("leaderboardButton"_spr);
			lbButton->setPosition({178, 115});
			menu->addChild(lbButton);
		}*/

		if (Mod::get()->getSettingValue<bool>("show-comments-button")) {
			auto commentsSprite = CCSprite::createWithSpriteFrameName("GJ_infoBtn_001.png");
			auto commentsButton = CCMenuItemSpriteExtra::create(commentsSprite, this, menu_selector(Buttons::commentsButton));
			commentsButton->setID("commentsButton"_spr);
			if (this->getChildByID("main-layer")->getChildByID("button-menu")->getChildByID("sorkopiko.playlists/next"))
				commentsButton->setPosition({180, -77});
			else
				commentsButton->setPosition({180, -125});
			menu->addChild(commentsButton);
		}	

		bool showLikeButton = Mod::get()->getSettingValue<bool>("show-like-button");
		bool showRateButton = Mod::get()->getSettingValue<bool>("show-rate-button");
		if (showRateButton) {
			std::string levelStars = (std::to_string(PlayLayer::get()->m_level->m_stars));
			if (levelStars == "0") {
				isRated = false;
				auto ratedLevels = GameLevelManager::get()->m_ratedLevels->allKeys();
				grayStarButton = findID(ratedLevels, levelID, false);
			}
			else if (levelStars == "10") {
				isRated = true;
				auto ratedDemons = GameLevelManager::get()->m_ratedDemons->allKeys();
				grayStarButton = findID(ratedDemons, levelID, false);
			}
			else {
			showRateButton = false;
			isRated = false;
			}

			if (showRateButton) {
				std::string rateButtonFrameName = "GJ_starBtn_001.png";
				if (isRated) 
					rateButtonFrameName = "GJ_rateDiffBtn_001.png";
				int rateButtonScale = 1;
				if (grayStarButton) {
					if (isRated) 
						rateButtonFrameName = "GJ_rateDiffBtn2_001.png";
					else 
						rateButtonFrameName = "GJ_starBtn2_001.png";
					
					rateButtonScale = -1;
				}

				auto rateSprite = CCSprite::createWithSpriteFrameName(rateButtonFrameName.c_str());
				rateSprite->setScale(rateButtonScale);
				auto rateButton = CCMenuItemSpriteExtra::create(rateSprite, this, menu_selector(Buttons::rateButton));
				if (showLikeButton)
					rateButton->setPosition({31.3, -125.2});
				else
					rateButton->setPosition({0, -125.2});
				rateButton->setScale(rateButtonScale);
				rateButton->setID("rateButton"_spr);
				menu->addChild(rateButton);
			}
		}

		if (showLikeButton) {
			auto likedLevels = GameLevelManager::get()->m_likedLevels->allKeys();
			grayButton = findID(likedLevels, levelID, true);
			std::string buttonFrameName = "GJ_like2Btn_001.png";	
			int buttonScale = 1;
			if (grayButton) {
				buttonFrameName = "GJ_like2Btn2_001.png";
				buttonScale = -1;
			}

			auto sprite = CCSprite::createWithSpriteFrameName(buttonFrameName.c_str());
			sprite->setScale(buttonScale);
			auto likeButton = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(Buttons::likeButton));
			likeButton->setID("likeButton"_spr);
			if (showRateButton)
				likeButton->setPosition({-31.3, -125.2});
			else
				likeButton->setPosition({0, -125.2});
			likeButton->setScale(buttonScale);
			menu->addChild(likeButton);
		}
	}

	void removeLayer() {
		auto scene = CCDirector::sharedDirector()->getRunningScene();
		if (auto popupLayer = static_cast<FLAlertLayer*>(scene->getChildByID("popupLayer"_spr)))
			popupLayer->keyBackClicked();
		if (auto infoLayer = static_cast<LevelInfoLayer*>(scene->getChildByID("fakeInfoLayer"_spr)))
			infoLayer->removeFromParentAndCleanup(true);
	}

	void onReplay(CCObject* sender) {
		useButton = false;
		removeLayer();
		EndLevelLayer::onReplay(sender);
	}

	void onRestartCheckpoint(CCObject* sender) {
		useButton = false;
		removeLayer();
		EndLevelLayer::onRestartCheckpoint(sender);
	}
};

void makePopup() {
	FLAlertLayer::create(
		"Warning",
		"You must <cr>exit</c> the level to open this page.",
		"Ok"
	)->show();
}

class $modify (ProfilePage) {
	void onMyLevels(CCObject* sender) {
		if (!PlayLayer::get()) ProfilePage::onMyLevels(sender);
		else makePopup();
	}

	void onMyLists(CCObject* sender) {
			if (!PlayLayer::get()) ProfilePage::onMyLists(sender);
			else makePopup();
	}
};

class $modify (CommentCell) {
	void onGoToLevel(CCObject* sender) {
		if (!PlayLayer::get()) CommentCell::onGoToLevel(sender);
		else makePopup();
	}
};

class $modify (InfoLayer) {
	void onOriginal(CCObject* sender) {
		if (!PlayLayer::get()) InfoLayer::onOriginal(sender);
		else makePopup();
	}
};

class $modify (CCDirector) {
	bool pushScene(CCScene* scene) {
		if (!PlayLayer::get())
			return CCDirector::pushScene(scene);
			
		else if (PlayLayer::get()->getChildByID("EndLevelLayer")) {
			makePopup();
			return false;
		}
		return true;
	}
};