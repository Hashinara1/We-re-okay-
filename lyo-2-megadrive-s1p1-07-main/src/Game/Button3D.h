#pragma once

#include <functional>
#include <vector>
#include "Text3D.h"

// Bouton 3D fixe : plaque, texte et animations de survol/clic.
class Button3D
{
public:

	bool Create(cstr label, XMFLOAT3 center, float width, float height, float textSize, XMFLOAT3 plaqueColor, XMFLOAT3 textColor = CPU_WHITE, XMFLOAT3 edgeColor = { -1.0f, -1.0f, -1.0f }, int fontPx = 64);
	void Update(float p_dt);

	void SetVisible(bool p_visible);
	bool Contains(cpu_entity* p_entity) const;
	void Destroy();

	XMFLOAT3 GetCenter() const { return m_center; }
	float GetWidth() const { return m_w; }
	float GetHeight() const { return m_h; }

	void SetOnRightClick(std::function<void()> p_function) { m_OnRightClick = p_function; }
	void SetOnLeftClick(std::function<void()> p_function) { m_OnLeftClick = p_function; }
	void SetOnMiddleClick(std::function<void()> p_function) { m_OnMiddleClick = p_function; }

private:
	struct PartPose
	{
		cpu_entity* entity = nullptr;
		XMFLOAT3 position = { 0.f, 0.f, 0.f };
		XMFLOAT3 scale = { 1.f, 1.f, 1.f };
		XMFLOAT3 right = { 1.f, 0.f, 0.f };
		XMFLOAT3 up = { 0.f, 1.f, 0.f };
		XMFLOAT3 dir = { 0.f, 0.f, 1.f };
	};

	std::function<void()> m_OnRightClick;
	std::function<void()> m_OnLeftClick;
	std::function<void()> m_OnMiddleClick;

	void SetHovered(bool hovered);
	void PlayClick();
	void CapturePose(cpu_entity* entity);
	void ApplyAnimation(float dt);

	cpu_mesh m_mesh;
	cpu_material m_material;
	XMFLOAT3 m_baseColor = { 1.f, 1.f, 1.f };
	XMFLOAT3 m_hoverColor = { 1.f, 1.f, 1.f };
	cpu_entity* m_entity = nullptr;
	cpu_mesh m_edgeMesh[12];
	cpu_material m_edgeMaterial;
	cpu_entity* m_edgeEnt[12] = {};
	Text3D m_label;
	std::vector<PartPose> m_parts;
	XMFLOAT3 m_center = { 0.f, 0.f, 0.f };
	XMFLOAT3 m_dir = { 0.f, 0.f, 1.f };
	float m_w = 0.f;
	float m_h = 0.f;
	float m_scale = 1.f;
	float m_hoverShakeTimer = 0.f;
	float m_clickTimer = 0.f;
	bool m_hovered = false;
	bool m_visible = false;
	bool m_created = false;

	static constexpr float HOVER_SCALE = 1.08f;
	static constexpr float HOVER_SHAKE_DURATION = 0.28f;
	static constexpr float CLICK_DURATION = 0.24f;
};
