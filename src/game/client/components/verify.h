#ifndef GAME_CLIENT_COMPONENTS_VERIFY_H
#define GAME_CLIENT_COMPONENTS_VERIFY_H
#include <game/client/component.h>
#include <engine/shared/http.h>
#include <engine/shared/jobs.h>

#include <base/lock.h>
#include <base/system.h>

#include <chrono>

class CVerify : public CComponent
{
	class CJob : public IJob
	{
		CVerify *m_pParent;
		CLock m_Lock;
		std::shared_ptr<CHttpRequest> m_pHead;
		std::shared_ptr<CHttpRequest> m_pGet;
		void Run() override REQUIRES(!m_Lock);

	public:
		CJob(CVerify *pParent) :
			m_pParent(pParent) {}
		void Abort() REQUIRES(!m_Lock);
	};

	IHttp *m_pHttp;
	std::shared_ptr<CJob> m_pJob;
	const char *m_pUrl = "https://ger10.ddnet.org/";

public:
	virtual int Sizeof() const override { return sizeof(*this); }
	virtual void OnInit() override;
	virtual void OnRender() override;
};

#endif