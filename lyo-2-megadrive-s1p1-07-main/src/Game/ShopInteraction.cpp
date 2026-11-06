#include "pch.h"
#include "ShopInteraction.h"
#include "ShopManager.h"
#include "BonusCatalog.h"

void ShopInteraction::UpdateShop()
{
	XMFLOAT2 mouse;
	const bool hasPosition = GetMouseTablePosition(mouse);

	bool isOffer = false;
	int hoveredSlot = -1;
	const int hoveredId = hasPosition ? FindHoveredItem(isOffer, hoveredSlot) : -1;
	m_owner.m_hoveredBonus = hoveredId;

	const bool down = cpuInput.vi.IsKey(VK_LBUTTON);

	if (down && m_owner.m_leftMouseWasDown == false && hasPosition && m_owner.m_shopItemsReady && m_owner.m_bonusFusion.IsActive() == false)
	{
		if (hoveredId >= 0 && isOffer)
		{
			m_owner.m_shop.PurchaseOffer(hoveredSlot);
			m_owner.m_leftMouseWasDown = true;
			return;
		}
		else if (hoveredId >= 0)
		{
			if (hoveredSlot < ShopManager::ACTIVE_SLOT_COUNT)
			{
				m_owner.m_draggedActiveSlot = hoveredSlot;
			}
			else
			{
				m_owner.m_draggedPassiveSlot = hoveredSlot - ShopManager::ACTIVE_SLOT_COUNT;
			}

			m_owner.m_leftMouseWasDown = true;

			return;
		}
	}

	if (down && hasPosition)
	{
		if (m_owner.m_draggedActiveSlot >= 0 && m_owner.m_draggedActiveSlot < ShopManager::ACTIVE_SLOT_COUNT && m_owner.m_activeBonuses[m_owner.m_draggedActiveSlot].entity)
		{
			m_owner.m_activeBonuses[m_owner.m_draggedActiveSlot].entity->transform.SetPosition(mouse.x, ShopManager::ITEM_Y, mouse.y);
		}

		if (m_owner.m_draggedPassiveSlot >= 0 && m_owner.m_draggedPassiveSlot < ShopManager::PASSIVE_SLOT_COUNT && m_owner.m_passiveBonuses[m_owner.m_draggedPassiveSlot].entity)
		{
			m_owner.m_passiveBonuses[m_owner.m_draggedPassiveSlot].entity->transform.SetPosition(mouse.x, ShopManager::ITEM_Y, mouse.y);
		}
	}

	if (down == false && m_owner.m_leftMouseWasDown)
	{
		if (hasPosition == false || IsOutsideTable(mouse))
		{
			if (m_owner.m_draggedActiveSlot >= 0 && m_owner.m_draggedActiveSlot < ShopManager::ACTIVE_SLOT_COUNT)
			{
				const cpu_entity* entity = m_owner.m_activeBonuses[m_owner.m_draggedActiveSlot].entity;
				const XMFLOAT2 releasePosition = hasPosition ? mouse : XMFLOAT2{ entity->transform.pos.x, entity->transform.pos.z };
				m_owner.m_inventory.DiscardInventoryItem(m_owner.m_draggedActiveSlot, false, releasePosition);
			}
			if (m_owner.m_draggedPassiveSlot >= 0 && m_owner.m_draggedPassiveSlot < ShopManager::PASSIVE_SLOT_COUNT)
			{
				const cpu_entity* entity = m_owner.m_passiveBonuses[m_owner.m_draggedPassiveSlot].entity;
				const XMFLOAT2 releasePosition = hasPosition ? mouse : XMFLOAT2{ entity->transform.pos.x, entity->transform.pos.z };
				m_owner.m_inventory.DiscardInventoryItem(m_owner.m_draggedPassiveSlot, true, releasePosition);
			}
		}
		else if (m_owner.m_draggedActiveSlot >= 0 && m_owner.m_draggedActiveSlot < ShopManager::ACTIVE_SLOT_COUNT)
		{
			const int draggedSlot = m_owner.m_draggedActiveSlot;
			ShopItem& dragged = m_owner.m_activeBonuses[draggedSlot];
			int mergeSlot = -1;
			const FusionRecipe* recipe = nullptr;

			for (int i = 0; i < ShopManager::ACTIVE_SLOT_COUNT; ++i)
			{
				if (i == draggedSlot)
					continue;

				const ShopItem& target = m_owner.m_activeBonuses[i];

				if (dragged.entity == nullptr || target.entity == nullptr)
					continue;

				recipe = BonusCatalog::FindRecipe((BonusId)dragged.id, (BonusId)target.id);

				if (recipe)
				{
					const float dx = dragged.entity->transform.pos.x - target.entity->transform.pos.x;
					const float dz = dragged.entity->transform.pos.z - target.entity->transform.pos.z;

					if (fabsf(dx) <= ShopManager::HIT_RADIUS * 2.f && fabsf(dz) <= ShopManager::HIT_RADIUS * 2.f)
					{
						mergeSlot = i;
						break;
					}
				}
			}

			if (mergeSlot >= 0 && recipe != nullptr)
			{
				ShopItem& target = m_owner.m_activeBonuses[mergeSlot];
				m_owner.m_bonusFusion.Begin(dragged, target, mergeSlot, recipe->result);
				dragged = ShopItem{};
				target = ShopItem{};
			}
			else if (dragged.entity)
			{
				dragged.target = { ShopManager::INVENTORY_X, ShopManager::ITEM_Y, ShopManager::ACTIVE_SLOT_Z[draggedSlot] };
				dragged.moving = true;
			}
		}
		else if (m_owner.m_draggedPassiveSlot >= 0 && m_owner.m_draggedPassiveSlot < ShopManager::PASSIVE_SLOT_COUNT)
		{
			ShopItem& item = m_owner.m_passiveBonuses[m_owner.m_draggedPassiveSlot];

			if (item.entity)
			{
				item.target = { ShopManager::INVENTORY_X, ShopManager::ITEM_Y, ShopManager::PASSIVE_SLOT_Z[m_owner.m_draggedPassiveSlot] };
				item.moving = true;
			}
		}

		m_owner.m_draggedActiveSlot = m_owner.m_draggedPassiveSlot = -1;

		bool offer = false;
		int slot = -1;
		m_owner.m_hoveredBonus = hasPosition ? FindHoveredItem(offer, slot) : -1;
	}

	m_owner.m_leftMouseWasDown = down;
}

bool ShopInteraction::GetMouseTablePosition(XMFLOAT2& position) const
{
	cpu_ray ray;
	cpuEngine.GetCursorRay(ray);

	if (fabsf(ray.dir.y) < CPU_EPSILON)
		return false;

	const float t = (ShopManager::ITEM_Y - ray.pos.y) / ray.dir.y;

	if (t < 0.f)
		return false;

	position = { ray.pos.x + ray.dir.x * t, ray.pos.z + ray.dir.z * t };

	if (std::isfinite(position.x) == false || std::isfinite(position.y) == false)
		return false;

	return true;
}

int ShopInteraction::FindHoveredItem(bool& isOffer, int& slot) const
{
	isOffer = false;
	slot = -1;

	cpu_ray ray;
	cpuEngine.GetCursorRay(ray);

	cpu_hit hit;
	cpu_entity* hitEntity = cpuEngine.HitEntity(hit, ray);

	if (hitEntity == nullptr)
		return -1;

	for (int i = 0; i < ShopManager::OFFER_COUNT; ++i)
	{
		const ShopItem& item = m_owner.m_offers[i];

		if (item.entity != hitEntity)
			continue;

		if (item.id < 0 || item.entity->visible == false || item.falling || item.rising)
			return -1;

		isOffer = true;
		slot = i;
		return item.id;
	}

	for (int group = 0; group < 2; ++group)
	{
		const ShopItem* items = group == 0 ? m_owner.m_activeBonuses : m_owner.m_passiveBonuses;
		const int count = group == 0 ? ShopManager::ACTIVE_SLOT_COUNT : ShopManager::PASSIVE_SLOT_COUNT;

		for (int i = 0; i < count; ++i)
		{
			const ShopItem& item = items[i];

			if (item.entity != hitEntity)
				continue;

			if (item.id < 0 || item.entity->visible == false || item.moving)
				return -1;

			slot = group == 0 ? i : ShopManager::ACTIVE_SLOT_COUNT + i;
			return item.id;
		}
	}

	return -1;
}

bool ShopInteraction::IsOutsideTable(const XMFLOAT2& position) const
{
	return fabsf(position.x) > 2.5f || fabsf(position.y) > 5.6f;
}