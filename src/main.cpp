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

using namespace geode::prelude;

bool isRated = false;
bool useButton = false;

class $modify(FLAlertLayer) {
	void destructor() {
		if (!PlayLayer::get()) {
			FLAlertLayer::~FLAlertLayer();
			return;
		}
		bool popup = (this->getID() == "CommentsLayer"_spr || this->getID() == "LikeItemLayer"_spr || this->getID() == "RateLayer"_spr || this->getID() == "LevelInfo"_spr);
		CCNode* layer = PlayLayer::get()->getChildByID("fakeInfoLayer"_spr);
		if (layer && popup) {
			layer->removeFromParentAndCleanup(true);
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

LevelInfoLayer* createInfoLayer(bool addChild) {
	auto infoLayer = LevelInfoLayer::create(PlayLayer::get()->m_level, false);
	infoLayer->setID("fakeInfoLayer"_spr);
	infoLayer->setKeyboardEnabled(false);
	infoLayer->setVisible(false);
	if (addChild) PlayLayer::get()->addChild(infoLayer);
	return infoLayer;
}

class Buttons {
	public:
	void likeButton(CCObject* obj) {
		if (!useButton) return;
		LevelInfoLayer* infoLayer = createInfoLayer(true);
		infoLayer->onLike(nullptr);
		auto delegate = new likeBtnDelegate();
		delegate->button = static_cast<CCMenuItemSpriteExtra*>(obj);
		delegate->layer = infoLayer;
		CCArray* children = CCDirector::sharedDirector()->getRunningScene()->getChildren();
		auto likeItemLayer = dynamic_cast<LikeItemLayer*>(children->lastObject());
		likeItemLayer->m_likeDelegate = delegate;
		likeItemLayer->setID("LikeItemLayer"_spr);
	}

	void rateButton(CCObject* obj) {
		if (!useButton) return;
		LevelInfoLayer* infoLayer = createInfoLayer(true);
		if (isRated) {
			infoLayer->onRateDemon(nullptr);
			CCArray* children = CCDirector::sharedDirector()->getRunningScene()->getChildren();
			if (auto ratePopup = dynamic_cast<RateDemonLayer*>(children->lastObject()))
				ratePopup->setID("RateLayer"_spr);
			else if (auto ratePopup = dynamic_cast<FLAlertLayer*>(children->lastObject())) {
				ratePopup->keyBackClicked();
				dynamic_cast<RateDemonLayer*>(children->lastObject())->setID("RateLayer"_spr);
			}
		}
		else {
			infoLayer->onRateStars(nullptr);
			CCArray* children = CCDirector::sharedDirector()->getRunningScene()->getChildren();
			if (auto ratePopup = dynamic_cast<RateStarsLayer*>(children->lastObject()))
				ratePopup->setID("RateLayer"_spr);
			else if (auto ratePopup = dynamic_cast<FLAlertLayer*>(children->lastObject())) {
				ratePopup->keyBackClicked();
				dynamic_cast<RateStarsLayer*>(children->lastObject())->setID("RateLayer"_spr);
			}
		}
	}

	void lbButton(CCObject* obj) {
		if (!useButton) return;
		auto level = PlayLayer::get()->m_level;
		auto lbType = static_cast<LevelLeaderboardType>(GameManager::get()->getIntGameVariable("0098"));
		auto lbMode = static_cast<LevelLeaderboardMode>(GameManager::get()->getIntGameVariable("0164"));
		auto lbMenu = LevelLeaderboard::create(level, lbType, lbMode);
		lbMenu->show();
		lbMenu->setID("leaderboardLayer"_spr);
	}

	void infoButton(CCObject* obj) {
		if (!useButton) return;
		LevelInfoLayer* infoLayer = createInfoLayer(true);
		infoLayer->onLevelInfo(nullptr);
		CCArray* children = CCDirector::sharedDirector()->getRunningScene()->getChildren();
		static_cast<FLAlertLayer*>(children->lastObject())->setID("LevelInfo"_spr);
	}

	void commentsButton(CCObject* obj) {
		if (!useButton) return;
		LevelInfoLayer* infoLayer = createInfoLayer(false);
		infoLayer->onInfo(nullptr);
		auto commentsLayer = static_cast<InfoLayer*>(CCDirector::get()->getRunningScene()->getChildByID("InfoLayer"));
		commentsLayer->setID("CommentsLayer"_spr);
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
		std::string stringLevelID;
		if (likeButton) 
			stringLevelID = "like_1_" + std::to_string(levelID);
		else 
			stringLevelID = std::to_string(levelID);
		CCObject* obj;
     	CCARRAY_FOREACH(keys, obj) {
        	CCString* str = dynamic_cast<CCString*>(obj);	
        	if (str) {			
				size_t position;		
				position = std::string(str->getCString()).find(stringLevelID);
				if (position != std::string::npos)
					return true;
			}				
     	}
		return false;
	}

	void customSetup() {
		EndLevelLayer::customSetup();
		useButton = true;
		auto levelID = PlayLayer::get()->m_level->m_levelID.value();

		auto menu = CCMenu::create();
		menu->setID("customMenu"_spr);
		this->getChildByID("main-layer")->addChild(menu);

		if (Mod::get()->getSettingValue<bool>("show-info-button")) {
			auto infoSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
			infoSprite->setScale(0.9);
			auto infoButton = CCMenuItemSpriteExtra::create(infoSprite, this, menu_selector(Buttons::infoButton));
			infoButton->setID("infoButton"_spr);
			if (this->getChildByID("main-layer")->getChildByID("button-menu")->getChildByID("absolllute.megahack/cheat-indicator-info"))
				infoButton->setPosition({-164, 77});
			else
				infoButton->setPosition({-181, 127});
			menu->addChild(infoButton);
		}

		if (levelID < 23 || (levelID > 5000 && levelID < 5005) || levelID == 3001 || PlayLayer::get()->m_level->m_isEditable) 
			return;
		bool grayButton = false;
		bool grayStarButton = false;

		if (!PlayLayer::get()->m_levelSettings->m_platformerMode && Mod::get()->getSettingValue<bool>("show-leaderboard-button")) {
			auto lbSprite = CCSprite::createWithSpriteFrameName("GJ_levelLeaderboardBtn_001.png");
			auto lbButton = CCMenuItemSpriteExtra::create(lbSprite, this, menu_selector(Buttons::lbButton));
			lbButton->setID("leaderboardButton"_spr);
			lbButton->setPosition({178, 115});
			menu->addChild(lbButton);
		}

		if (Mod::get()->getSettingValue<bool>("show-comments-button")) {
			auto commentsSprite = CCSprite::createWithSpriteFrameName("GJ_infoBtn_001.png");
			auto commentsButton = CCMenuItemSpriteExtra::create(commentsSprite, this, menu_selector(Buttons::commentsButton));
			commentsButton->setID("commentsButton"_spr);
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

	void onReplay(CCObject* sender) {
		EndLevelLayer::onReplay(sender);
		useButton = false;
	}

	void onRestartCheckpoint(CCObject* sender) {
		EndLevelLayer::onRestartCheckpoint(sender);
		useButton = false;
	}
};

void makePopup() {
	FLAlertLayer::create(
		"Warning",
		"You must <cr>exit</c> the level to open this page.",
		"Ok"
	)->show();
}

class $modify (InfoLayer) {
	void onOriginal(CCObject* sender) {
		if (!PlayLayer::get()) InfoLayer::onOriginal(sender);
		else makePopup();
	}
};

class $modify (ProfilePage) {
	static void onModify(auto& self) {
		if (Loader::get()->isModLoaded("cvolton.betterinfo"))
			self.setHookPriority("ProfilePage::loadPageFromUserInfo", -1);
	}

	void loadPageFromUserInfo(GJUserScore* a2){
		ProfilePage::loadPageFromUserInfo(a2);
		if (PlayLayer::get()) {
			if (auto lbButton = this->m_mainLayer->getChildByID("main-menu")->getChildByID("cvolton.betterinfo/leaderboard-button"))
				lbButton->setVisible(false);
		}
	}
};

class $modify (CCDirector) {
	bool pushScene(CCScene* scene) {
		bool endLevelLayer = false;
		if (PlayLayer::get())
			endLevelLayer = PlayLayer::get()->getChildByID("EndLevelLayer") != nullptr;

		if (!endLevelLayer) {
			return CCDirector::pushScene(scene);
		}
		else {
			makePopup();
			return false;
		}
		return true;
	}
};