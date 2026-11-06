#pragma once

class Table
{
public:
	Table();
	~Table();

	void Create(const XMFLOAT3& pos);
	void Destroy();
	void SetClosingAmount(float amount);

private:

	XMFLOAT3 m_position = { 0.f, 0.f, 0.f };

	//-- GLOBAL --

	//Table
	cpu_mesh m_meshTable;
	cpu_entity* m_pTable;

	//Borders
	cpu_mesh m_meshBorder;
	cpu_entity* m_pLeftBorder;
	cpu_entity* m_pRightBorder;

	//-- MARKING --
	
	//Middle Line
	cpu_mesh m_meshMiddleLine;
	cpu_entity* m_pMiddleLine;

	//CONST
	static constexpr float BORDER_HALF_WIDTH = 0.25f;
	static constexpr float BORDER_START_X = 2.25f;
	static constexpr float BORDER_OUTER_X = 2.5f;
	static constexpr float CLOSED_BORDER_HALF_WIDTH = BORDER_OUTER_X * 0.5f;
};

