#pragma once

class CHudZB3 : public CHudBase
{
public:
	int Init(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

	CHudMsgFunc(ZB3Msg);

private:
	int m_iMoraleType;
	int m_iMoraleLevel;
	int m_iZombieLevel;
	int m_iRagePercent;
	float m_flLastUpdateTime;
};
