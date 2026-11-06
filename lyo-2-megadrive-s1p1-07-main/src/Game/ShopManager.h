#pragma once

#include "BonusCatalog.h"
#include "Shop.h"
#include "BonusInventory.h"
#include "ShopInteraction.h"
#include "ShopItemPresentation.h"
#include "BonusEffects.h"
#include "CoinSystem.h"
#include "LaserGuide.h"
#include "BonusFusion.h"
#include "GameCamera.h"

class PlayerStriker;
class AIStriker;
class Puck;
class Eyes;
class GameManager;
class DebrisManager;

struct ShopItem
{
	int id = -1;

	cpu_entity* entity = nullptr;

	float delay = 0.f;

	XMFLOAT3 target = { 0.f, 0.f, 0.f };

	bool falling = false;
	bool rising = false;
	bool moving = false;
	float shakeTimer = 0.f;
};

struct CoinPickupEmitter
{
	cpu_particle_emitter* emitter = nullptr;
	float lifetime = 0.f;
};

struct GoldCoin
{
	cpu_entity* entity = nullptr;
	cpu_entity* shadow = nullptr;
	float rotation = 0.f;
};

class ShopManager
{
private:
	friend class Shop;
	friend class BonusInventory;
	friend class ShopInteraction;
	friend class ShopItemPresentation;
	friend class BonusEffects;
	friend class CoinSystem;
	friend class LaserGuide;
	friend class BonusFusion;

public:
	ShopManager();
	~ShopManager() = default;

	void Create(GameManager* gameManager);
	void Destroy();

	void BeginShop();

	void UpdateShop(float dt);
	void UpdateItems(float dt);
	void UpdateDiscardedItems(float dt);

	void StartOfferDrop();
	void StartOfferRise();

	bool AreOffersLanded() const;
	bool AreOffersGone() const;

	bool IsInventorySettled() const;

	void SetShopItemsVisible(bool visible);
	void SetGameItemsVisible(bool visible);
	void ClearRoundCoins();
	void ClearInventory();

	void UpdateGame(float dt, bool acceptInput);
	void UpdateLaser(float dt, bool allowCoinSpawn);
	void UpdateFusion(float dt);
	void HideLaser();
	void Render(bool showShopTooltip);

	bool IsGuardianActive() const;
	bool AreCagesActive() const;
	bool IsBigStrikerActive() const;
	bool IsSuddenDeathActive() const;
	bool ConsumeSuddenDeathHit();
	bool IsPuckSlowdownActive() const;
	bool IsExtraLifeOwned() const;
	void ConsumeExtraLife();
	float GetTableFriction() const;
	void SetAIStatusVisual(bool frozen, bool confused);
	void SetAIBaseDifficulty(float difficulty) { m_aiBaseDifficulty = difficulty; }

	GameCamera& GetGameCamera();
	void AddMoney(int p_value);

	void SetDebris(DebrisManager* debris) { m_debris = debris; }

private:
	static constexpr int BONUS_COUNT = (int)BonusId::Count;
	static constexpr int OFFER_COUNT = 4;
	static constexpr float BASE_FRICTION = 0.25f;
	static constexpr int ACTIVE_SLOT_COUNT = 2;
	static constexpr int PASSIVE_SLOT_COUNT = 1;

	cpu_font m_tooltipFont;

	cpu_mesh m_bonusMeshes[BONUS_COUNT];
	cpu_mesh m_meshSlot;
	cpu_mesh m_meshGuardianWall;
	cpu_mesh m_meshCageWall;
	cpu_mesh m_meshCoin;
	cpu_mesh m_meshCoinShadow;

	cpu_particle_emitter* m_suddenDeathAura = nullptr;
	cpu_particle_emitter* m_suddenDeathAura2 = nullptr;

	float m_suddenDeathAuraPhase = 0.f;
	void UpdateSuddenDeathAura(float dt);

	BonusFusion m_bonusFusion;

	std::vector<CoinPickupEmitter> m_coinPickupEmitters;

	cpu_mesh m_aiStrikerMeshes[3];

	cpu_entity* m_activeSlotEntities[ACTIVE_SLOT_COUNT] = {};
	cpu_entity* m_passiveSlotEntities[PASSIVE_SLOT_COUNT] = {};
	cpu_entity* m_effectEntities[3] = {};

	std::vector<GoldCoin> m_goldCoins;

	float m_goldSpawnTimer = 0.f;

	ShopItem m_offers[OFFER_COUNT];
	ShopItem m_activeBonuses[ACTIVE_SLOT_COUNT];
	ShopItem m_passiveBonuses[PASSIVE_SLOT_COUNT];

	std::vector<ShopItem> m_discardedBonuses;

	float m_bonusTimers[BONUS_COUNT] = {};
	int m_hoveredBonus = -1;
	int m_draggedActiveSlot = -1;
	int m_draggedPassiveSlot = -1;
	bool m_ownedExtraLife = false;
	bool m_ownedCages = false;
	bool m_leftMouseWasDown = false;
	bool m_shopItemsReady = false;
	bool m_suddenDeathHit = false;

	GameManager* m_pGameManager = nullptr;
	DebrisManager* m_debris = nullptr;

	PlayerStriker* m_pPlayer = nullptr;
	AIStriker* m_pAI = nullptr;
	Puck* m_pPuck = nullptr;
	Eyes* m_pEyes = nullptr;

	float m_aiBaseDifficulty = 0.5f;

	ui32 m_seed = 0;

	static constexpr float ITEM_HALF_SIZE = 0.19f;
	static constexpr float HIT_RADIUS = 0.19f;
	static constexpr float ITEM_HALF_HEIGHT = 0.3f;
	static constexpr float SLOT_HALF_WIDTH = 0.23f;
	static constexpr float OFFER_X[4] = { -1.15f, -0.4f, 0.4f, 1.15f };
	static constexpr float OFFER_Z = 2.25f;
	static constexpr float INVENTORY_X = -2.25f;
	static constexpr float ACTIVE_SLOT_Z[2] = { -4.65f, -4.f };
	static constexpr float PASSIVE_SLOT_Z[PASSIVE_SLOT_COUNT] = { -3.f };
	static constexpr float ITEM_Y = 0.51f;
	static constexpr float SLOT_Y = 0.26f;
	static constexpr float DROP_Y = 4.f;
	static constexpr float DROP_SPEED = 8.5f;
	static constexpr float COIN_SPAWN_MIN = 3.f;
	static constexpr float COIN_SPAWN_MAX = 8.f;
	static constexpr float COIN_SPAWN_HALF_WIDTH = 1.65f;
	static constexpr float COIN_SPAWN_HALF_LENGTH = 3.f;
	static constexpr float COIN_FALL_SPEED = 1.f;
	static constexpr float COIN_RADIUS = 0.12f;
	static constexpr float COIN_HALF_HEIGHT = 0.035f;
	static constexpr float PASSIVE_FRICTION = 1.35f;
	static constexpr float TABLE_HALF_WIDTH = 2.f;
	static constexpr float TABLE_HALF_LENGTH = 5.5f;
	static constexpr float MOVE_SPEED = 6.f;
	static constexpr float SHAKE_DURATION = 0.3f;
	static constexpr float SHAKE_AMPLITUDE = 0.06f;
	static constexpr float FUSION_DURATION = 1.1f;
	static constexpr float FUSION_RETURN_DURATION = 0.35f;
	static constexpr float SUDDEN_DEATH_DURATION = 0.5f;
	static constexpr float SUDDEN_DEATH_AURA_ORBIT_RADIUS = 0.32f;
	static constexpr float SUDDEN_DEATH_AURA_ORBIT_SPEED = 2.3f;
	static constexpr float SUDDEN_DEATH_AURA_ORBIT_Y_AMPLITUDE = 0.35f;

	Shop m_shop;
	BonusInventory m_inventory;
	ShopInteraction m_interaction;
	ShopItemPresentation m_presentation;
	BonusEffects m_effects;
	CoinSystem m_coins;
	LaserGuide m_laser;
};
