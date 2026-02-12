#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "draw_util.h"

#include "zb3.h"

int CHudZB3::Init(void)
{
	return 0;
}

void CHudZB3::Reset(void)
{
}

int CHudZB3::Draw(float flTime)
{
	return 1;
}

int CHudZB3::MsgFunc_ZB3Msg(const char *pszName, int iSize, void *pbuf)
{
	return 1;
}
