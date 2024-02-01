#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <game/client/animstate.h>
#include <game/client/render.h>
#include <game/generated/client_data.h>
#include <game/generated/protocol.h>

#include "verify.h"

static bool m_pVerified;
static int m_pTries;
static int lastCheck = 0;

void CVerify::OnInit()
{
	m_pVerified = false;
	m_pTries = 0;
	m_pHttp = Kernel()->RequestInterface<IHttp>();
}

void CVerify::OnRender()
{
	if(!g_Config.m_ClAutoVerify)
		return;
	if(m_pVerified)
		return;
	if(m_pTries > 10)
		return;

	if(lastCheck != 0 && time_get() - lastCheck < time_freq() * 15)
		return;

	lastCheck = time_get();
	if(m_pJob == nullptr || m_pJob->Status() == IJob::STATE_DONE)
	{
		Engine()->AddJob(std::make_shared<CJob>(this));
		m_pTries++;
	}
}

void CVerify::CJob::Run()
{
	const char *pUrl = "https://ger10.ddnet.org/";
	CTimeout Timeout{10000, 0, 8000, 10};
	std::shared_ptr<CHttpRequest> pHead = HttpHead(pUrl);
	pHead->Timeout(Timeout);
	pHead->LogProgress(HTTPLOG::FAILURE);
	{
		CLockScope ls(m_Lock);
		m_pHead = pHead;
	}

	m_pParent->m_pHttp->Run(pHead);
	pHead->Wait();
	if(pHead->State() == EHttpState::ABORTED)
	{
		return;
	}
	if(pHead->State() != EHttpState::DONE)
	{
		return;
	}

	auto StartTime = time_get_nanoseconds();
	std::shared_ptr<CHttpRequest> pGet = HttpGet(pUrl);
	pGet->Timeout(Timeout);
	pGet->LogProgress(HTTPLOG::FAILURE);
	{
		CLockScope ls(m_Lock);
		m_pGet = pGet;
	}
	m_pParent->m_pHttp->Run(pGet);
	pGet->Wait();

	auto Time = std::chrono::duration_cast<std::chrono::milliseconds>(time_get_nanoseconds() - StartTime);
	if(pHead->State() == EHttpState::ABORTED)
	{
		dbg_msg("verify", "https://ger10.ddnet.org/ aborted");
		return;
	}
	if(pGet->State() != EHttpState::DONE)
	{
		dbg_msg("verify", "https://ger10.ddnet.org/ failed");
		return;
	}
	m_pTries = 0;
	m_pVerified = true;
	dbg_msg("verify", "https://ger10.ddnet.org/ verified");
	return;
}

void CVerify::CJob::Abort()
{
	CLockScope ls(m_Lock);
	if(m_pHead != nullptr)
	{
		m_pHead->Abort();
	}

	if(m_pGet != nullptr)
	{
		m_pGet->Abort();
	}
}