#pragma once

#include "pch.h"

class NailDisplay
{
public:
	void Reset(int count)
	{
		m_count = count;
		m_hammered = 0;
	}

	void SetCount(int count)
	{
		m_count = count;
		if (m_hammered > m_count)
			m_hammered = m_count;
	}

	int GetCount() const { return m_count; }
	int GetHammered() const { return m_hammered; }

	float GetZ(int index) const
	{
		return (m_count - index) * 0.7f;
	}

	int HammerNext()
	{
		if (m_hammered >= m_count)
			return -1;
		return m_hammered++;
	}

private:
	int m_count = 2;
	int m_hammered = 0;
};