#include "pch.h"
#include "ShopItemPresentation.h"
#include "ShopManager.h"

#include "GameCamera.h"

void ShopItemPresentation::SetShopItemsVisible(bool visible)
{
	for (cpu_entity* entity : m_owner.m_activeSlotEntities)
	{
		if (entity)
			entity->visible = visible;
	}

	for (cpu_entity* entity : m_owner.m_passiveSlotEntities)
	{
		if (entity)
			entity->visible = visible;
	}

	for (ShopItem& item : m_owner.m_activeBonuses)
	{
		if (item.entity)
			item.entity->visible = visible;
	}

	for (ShopItem& item : m_owner.m_passiveBonuses)
	{
		if (item.entity)
			item.entity->visible = visible;
	}
}

void ShopItemPresentation::SetGameItemsVisible(bool visible)
{
	for (cpu_entity* entity : m_owner.m_activeSlotEntities)
	{
		if (entity)
			entity->visible = visible;
	}

	for (cpu_entity* entity : m_owner.m_passiveSlotEntities)
	{
		if (entity)
			entity->visible = visible;
	}

	for (ShopItem& item : m_owner.m_activeBonuses)
	{
		if (item.entity)
			item.entity->visible = visible;
	}

	for (ShopItem& item : m_owner.m_passiveBonuses)
	{
		if (item.entity)
			item.entity->visible = visible;
	}
}

void ShopItemPresentation::UpdateItems(float dt)
{
	for (ShopItem& item : m_owner.m_offers)
	{
		if (item.entity == nullptr || item.shakeTimer <= 0.f)
			continue;

		item.shakeTimer -= dt;

		if (item.shakeTimer <= 0.f)
		{
			item.shakeTimer = 0.f;
			item.entity->transform.pos.x = item.target.x;
			continue;
		}

		const float strength = item.shakeTimer / ShopManager::SHAKE_DURATION;
		item.entity->transform.pos.x = item.target.x + sinf(item.shakeTimer * 70.f) * ShopManager::SHAKE_AMPLITUDE * strength;
	}

	for (ShopItem& item : m_owner.m_offers)
	{
		if (item.entity == nullptr || (item.falling == false && item.rising == false))
		{
			continue;
		}

		if (item.delay > 0.f)
		{
			item.delay -= dt;

			if (item.delay > 0.f)
			{
				continue;
			}

			item.delay = 0.f;
		}

		if (item.falling)
		{
			item.entity->transform.pos.y -= ShopManager::DROP_SPEED * dt;

			if (item.entity->transform.pos.y <= ShopManager::ITEM_Y)
			{
				item.entity->transform.pos.y = ShopManager::ITEM_Y;
				item.falling = false;

				m_owner.GetGameCamera().Shake(0.1f);
			}
		}
		else if (item.rising)
		{
			item.entity->transform.pos.y += ShopManager::DROP_SPEED * dt;

			if (item.entity->transform.pos.y >= ShopManager::DROP_Y)
			{
				item.entity->visible = false;
				item.rising = false;
			}
		}
	}

	for (int group = 0; group < 2; ++group)
	{
		ShopItem* items = group == 0 ? m_owner.m_activeBonuses : m_owner.m_passiveBonuses;
		const int count = group == 0 ? ShopManager::ACTIVE_SLOT_COUNT : ShopManager::PASSIVE_SLOT_COUNT;

		for (int i = 0; i < count; ++i)
		{
			ShopItem& item = items[i];

			if (item.entity == nullptr || item.moving == false)
			{
				continue;
			}

			XMFLOAT3& p = item.entity->transform.pos;
			const XMFLOAT3 d = { item.target.x - p.x, item.target.y - p.y, item.target.z - p.z };
			const float length = sqrtf(d.x*d.x + d.y*d.y + d.z*d.z);

			if (length <= ShopManager::MOVE_SPEED * dt)
			{
				p = item.target;
				item.moving = false;
			}
			else if (length > CPU_EPSILON)
			{
				const float step = ShopManager::MOVE_SPEED * dt / length;
				p.x += d.x * step;
				p.y += d.y * step;
				p.z += d.z * step;
			}
		}
	}
}

void ShopItemPresentation::UpdateDiscardedItems(float dt)
{
	m_owner.m_coins.UpdateCoinPickupEmitters(dt);

	for (size_t i = 0; i < m_owner.m_discardedBonuses.size(); )
	{
		ShopItem& item = m_owner.m_discardedBonuses[i];

		if (item.entity == nullptr)
		{
			m_owner.m_discardedBonuses.erase(m_owner.m_discardedBonuses.begin() + i);
			continue;
		}

		item.entity->transform.pos.y -= ShopManager::DROP_SPEED * dt;

		if (item.entity->transform.pos.y <= -1.5f)
		{
			m_owner.GetGameCamera().Shake(0.2f);
			DestroyItem(item);
			m_owner.m_discardedBonuses.erase(m_owner.m_discardedBonuses.begin() + i);
			continue;
		}

		++i;
	}
}

void ShopItemPresentation::DestroyItem(ShopItem& item)
{
	item.entity = cpuEngine.Release(item.entity);
	item.id = -1;
	item.falling = item.rising = item.moving = false;
}

void ShopItemPresentation::Render(bool showShopTooltip)
{
	if (showShopTooltip == false || m_owner.m_hoveredBonus < 0)
		return;

	const BonusInfo& info = BonusCatalog::Get((BonusId)m_owner.m_hoveredBonus);

	const int x = cpuDevice.GetWidth() * 5 / 6;
	const int y = cpuDevice.GetHeight() / 2;
	const int lineSpacing = m_owner.m_tooltipFont.height + 2;

	cpuDevice.DrawText(&m_owner.m_tooltipFont, info.name, x, y - lineSpacing, CPU_TEXT_CENTER);

	const char* bonusType = info.active ? "Actif" : "Passif";
	const std::string price = CPU_STR(info.price) + " pieces - " + bonusType;

	cpuDevice.DrawText(&m_owner.m_tooltipFont, price.c_str(), x, y, CPU_TEXT_CENTER);
	cpuDevice.DrawText(&m_owner.m_tooltipFont, info.description, x, y + lineSpacing, CPU_TEXT_CENTER);
}
