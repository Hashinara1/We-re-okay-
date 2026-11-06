#include "pch.h"
#include "Table.h"

Table::Table()
{
	m_pTable = nullptr;
	m_pMiddleLine = nullptr;
	m_pLeftBorder = nullptr;
	m_pRightBorder = nullptr;
}

Table::~Table()
{
}

void Table::Create(const XMFLOAT3& pos)
{
	m_position = pos;

	// Table
	m_meshTable.CreateBox(2.f, 0.05f, 5.5f, cpu::ToColor(48, 23, 4));
	m_pTable = cpuEngine.CreateEntity();
	m_pTable->pMesh = &m_meshTable;
	m_pTable->transform.SetPosition(pos.x, pos.y, pos.z);

	// Borders
	m_meshBorder.CreateBox(BORDER_HALF_WIDTH, 0.25f, 5.6f, cpu::ToColor(48, 23, 4));

	m_pLeftBorder = cpuEngine.CreateEntity();
	m_pLeftBorder->pMesh = &m_meshBorder;
	m_pLeftBorder->transform.SetPosition(pos.x - 2.25f, pos.y, pos.z);

	m_pRightBorder = cpuEngine.CreateEntity();
	m_pRightBorder->pMesh = &m_meshBorder;
	m_pRightBorder->transform.SetPosition(pos.x + 2.25f, pos.y, pos.z);

	// Middle line
	m_meshMiddleLine.CreateBox(2.f, 0.05f, 0.1f, cpu::ToColor(158, 11, 11));
	m_pMiddleLine = cpuEngine.CreateEntity();
	m_pMiddleLine->pMesh = &m_meshMiddleLine;
	m_pMiddleLine->transform.SetPosition(pos.x, pos.y + 0.003f, pos.z);
}

void Table::SetClosingAmount(float amount)
{
	amount = cpu::Clamp(amount, 0.f, 1.f);

	float scaleX = 1.f + amount * (CLOSED_BORDER_HALF_WIDTH / BORDER_HALF_WIDTH - 1.f);
	float halfWidth = BORDER_HALF_WIDTH * scaleX;
	float centerOffset = BORDER_OUTER_X - halfWidth;

	if (m_pLeftBorder)
	{
		m_pLeftBorder->transform.SetScaling({ scaleX, 1.f, 1.f });
		m_pLeftBorder->transform.SetPosition(m_position.x - centerOffset, m_position.y, m_position.z);
	}
	if (m_pRightBorder)
	{
		m_pRightBorder->transform.SetScaling({ scaleX, 1.f, 1.f });
		m_pRightBorder->transform.SetPosition(m_position.x + centerOffset, m_position.y, m_position.z);
	}
}

void Table::Destroy()
{
	m_pTable = cpuEngine.Release(m_pTable);
	m_pMiddleLine = cpuEngine.Release(m_pMiddleLine);
	m_pLeftBorder = cpuEngine.Release(m_pLeftBorder);
	m_pRightBorder = cpuEngine.Release(m_pRightBorder);
}