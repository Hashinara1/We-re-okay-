#include "pch.h"
#include "Button3D.h"

bool Button3D::Create(cstr label, XMFLOAT3 center, float width, float height, float textSize, XMFLOAT3 plaqueColor, XMFLOAT3 textColor, XMFLOAT3 edgeColor, int fontPx)
{
	Destroy();

	if (label == nullptr || width <= 0.f || height <= 0.f || textSize <= 0.f)
		return false;

	if (m_label.IsReady() == false && m_label.Init(fontPx, "Dracutaz") == false)
		return false;

	m_center = center;
	m_w = width;
	m_h = height;
	m_baseColor = plaqueColor;
	m_hoverColor = { plaqueColor.x + 0.16f > 1.f ? 1.f : plaqueColor.x + 0.16f, plaqueColor.y + 0.16f > 1.f ? 1.f : plaqueColor.y + 0.16f, plaqueColor.z + 0.16f > 1.f ? 1.f : plaqueColor.z + 0.16f };
	m_material.color = m_baseColor;
	m_material.lighting = CPU_LIGHTING_UNLIT;

	const float halfThick = 0.125f * height;
	const float textGap = 0.0125f * height;
	const float edgeB = 0.025f * (width < height ? width : height);
	m_mesh.CreateBox(width * 0.5f, height * 0.5f, halfThick, CPU_WHITE);
	m_entity = cpuEngine.CreateEntity();
	m_entity->pMesh = &m_mesh;
	m_entity->pMaterial = &m_material;
	m_entity->transform.SetPosition(center.x, center.y, center.z);

	m_label.SetBaseOrientation({ 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, m_dir);
	m_label.SetAnchor(0.5f, 0.5f);
	m_label.Build(label, { center.x, center.y, center.z - halfThick - textGap }, textSize, textColor);

	if (edgeColor.x >= 0.f && edgeColor.y >= 0.f && edgeColor.z >= 0.f)
	{
		m_edgeMaterial.color = edgeColor;
		m_edgeMaterial.lighting = CPU_LIGHTING_UNLIT;
		const float hw = width * 0.5f;
		const float hh = height * 0.5f;
		const float hd = halfThick;
		const float b = edgeB;
		const float hx[12] = { hw, hw, b, b, hw, hw, b, b, b, b, b, b };
		const float hy[12] = { b, b, hh, hh, b, b, hh, hh, b, b, b, b };
		const float hz[12] = { b, b, b, b, b, b, b, b, hd, hd, hd, hd };
		const float lx[12] = { 0, 0, -hw, hw, 0, 0, -hw, hw, -hw, hw, -hw, hw };
		const float ly[12] = { hh, -hh, 0, 0, hh, -hh, 0, 0, hh, hh, -hh, -hh };
		const float lz[12] = { -hd, -hd, -hd, -hd, hd, hd, hd, hd, 0, 0, 0, 0 };

		for (int i = 0; i < 12; ++i)
		{
			m_edgeMesh[i].CreateBox(hx[i], hy[i], hz[i], CPU_WHITE);
			m_edgeEnt[i] = cpuEngine.CreateEntity();
			m_edgeEnt[i]->pMesh = &m_edgeMesh[i];
			m_edgeEnt[i]->pMaterial = &m_edgeMaterial;
			m_edgeEnt[i]->transform.SetPosition(center.x + lx[i], center.y + ly[i], center.z + lz[i]);
		}
	}

	CapturePose(m_entity);

	for (cpu_entity* entity : m_label.GetEntities())
		CapturePose(entity);

	for (cpu_entity* entity : m_edgeEnt)
		CapturePose(entity);

	m_created = true;
	m_visible = true;
	m_hovered = false;
	m_scale = 1.f;
	m_hoverShakeTimer = 0.f;
	m_clickTimer = 0.f;
	ApplyAnimation(0.f);
	return true;
}

void Button3D::CapturePose(cpu_entity* entity)
{
	if (entity == nullptr)
		return;
	PartPose pose;
	pose.entity = entity;
	pose.position = entity->transform.pos;
	pose.scale = entity->transform.sca;
	pose.right = entity->transform.right;
	pose.up = entity->transform.up;
	pose.dir = entity->transform.dir;
	m_parts.push_back(pose);
}

void Button3D::Update(float dt)
{
	if (m_created == false)
		return;

	dt = cpu::Clamp(dt, 0.f, 0.05f);
	bool hovered = false;
	if (m_visible)
	{
		cpu_ray ray;
		cpuEngine.GetCursorRay(ray);
		cpu_hit hit;
		hovered = Contains(cpuEngine.HitEntity(hit, ray));

		if (hovered && cpuInput.vi.IsKeyPressed(VK_LBUTTON))
		{
			PlayClick();
			if (m_OnLeftClick)
				m_OnLeftClick();
		}
		if (hovered && cpuInput.vi.IsKeyPressed(VK_RBUTTON))
		{
			PlayClick();
			if (m_OnRightClick)
				m_OnRightClick();
		}
		if (hovered && cpuInput.vi.IsKeyPressed(VK_MBUTTON))
		{
			PlayClick();
			if (m_OnMiddleClick)
				m_OnMiddleClick();
		}
	}
	SetHovered(hovered);

	const float targetScale = m_hovered ? HOVER_SCALE : 1.f;
	const float scaleStep = 10.f * dt;
	if (m_scale < targetScale)
		m_scale = m_scale + scaleStep < targetScale ? m_scale + scaleStep : targetScale;
	else if (m_scale > targetScale)
		m_scale = m_scale - scaleStep > targetScale ? m_scale - scaleStep : targetScale;

	m_hoverShakeTimer = m_hoverShakeTimer > dt ? m_hoverShakeTimer - dt : 0.f;
	m_clickTimer = m_clickTimer > dt ? m_clickTimer - dt : 0.f;
	ApplyAnimation(dt);
}

void Button3D::SetHovered(bool hovered)
{
	if (hovered == m_hovered)
		return;
	m_hovered = hovered;
	m_material.color = hovered ? m_hoverColor : m_baseColor;
	if (hovered)
		m_hoverShakeTimer = HOVER_SHAKE_DURATION;
}

void Button3D::PlayClick()
{
	m_clickTimer = CLICK_DURATION;
}

void Button3D::ApplyAnimation(float)
{
	const float hoverProgress = m_hoverShakeTimer / HOVER_SHAKE_DURATION;
	const float clickProgress = m_clickTimer / CLICK_DURATION;
	const float hoverRoll = hoverProgress > 0.f ? sinf((1.f - hoverProgress) * XM_2PI * 3.f) * 0.07f * hoverProgress : 0.f;
	const float clickRoll = clickProgress > 0.f ? sinf((1.f - clickProgress) * XM_2PI * 3.f) * 0.11f * clickProgress : 0.f;
	const float roll = hoverRoll + clickRoll;
	const float push = clickProgress > 0.f ? sinf((1.f - clickProgress) * XM_PI) * 0.10f : 0.f;
	const XMMATRIX rotation = XMMatrixRotationAxis(XMLoadFloat3(&m_dir), roll);

	for (const PartPose& pose : m_parts)
	{
		if (pose.entity == nullptr)
			continue;
		XMVECTOR offset = XMLoadFloat3(&pose.position) - XMLoadFloat3(&m_center);
		offset = XMVector3TransformNormal(offset * m_scale, rotation);
		XMFLOAT3 position;
		XMStoreFloat3(&position, XMLoadFloat3(&m_center) + offset + XMLoadFloat3(&m_dir) * push);
		pose.entity->transform.SetPosition(position.x, position.y, position.z);
		pose.entity->transform.SetScaling({ pose.scale.x * m_scale, pose.scale.y * m_scale, pose.scale.z * m_scale });

		XMStoreFloat3(&pose.entity->transform.right, XMVector3TransformNormal(XMLoadFloat3(&pose.right), rotation));
		XMStoreFloat3(&pose.entity->transform.up, XMVector3TransformNormal(XMLoadFloat3(&pose.up), rotation));
		XMStoreFloat3(&pose.entity->transform.dir, XMVector3TransformNormal(XMLoadFloat3(&pose.dir), rotation));
		pose.entity->transform.SetRotationFromAxes();
	}
}

bool Button3D::Contains(cpu_entity* entity) const
{
	if (entity == nullptr)
		return false;
	for (const PartPose& part : m_parts)
		if (part.entity == entity)
			return true;
	return false;
}

void Button3D::SetVisible(bool visible)
{
	m_visible = visible;

	for (const PartPose& part : m_parts)
	{
		if (part.entity != nullptr)
		{
			part.entity->visible = visible;
			part.entity->active = visible;
		}
	}

	if (visible == false)
		SetHovered(false);
}

void Button3D::Destroy()
{
	if (m_entity != nullptr)
		cpuEngine.Release(m_entity);

	m_entity = nullptr;

	for (cpu_entity*& entity : m_edgeEnt)
	{
		if (entity != nullptr)
			cpuEngine.Release(entity);
		entity = nullptr;
	}

	m_label.Release();
	m_parts.clear();

	m_created = false;
	m_visible = false;
	m_hovered = false;

	m_scale = 1.f;
	m_hoverShakeTimer = 0.f;
	m_clickTimer = 0.f;
}
