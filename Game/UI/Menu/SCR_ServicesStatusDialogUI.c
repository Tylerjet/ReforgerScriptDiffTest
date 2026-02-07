enum EServiceStatus
{
	RUNNING = 0,
	WARNING,
	ERROR,
};

class SCR_ServicesStatusDialogUI : DialogUI
{
	// STATIC to prevent close/open and allow for another update
	protected static int s_iLastUpdateTick;
	protected static const ref map<string, ServiceStatusItem> SERVICE_STATUSES = new map<string, ServiceStatusItem>();

	protected TextWidget m_wLastUpdate;
	protected string m_sLastUpdate;
	protected Widget m_wLegend;

	protected SCR_ServicesStatusDialogComponent m_ServicesStatusDialogComponent;

	protected static const int S_REFRESH_TIMEOUT = 5000;

	protected static const string S_SERVICE_MAIN = "MAIN_SERVICE";
	// all these values MUST be lowercase
	protected static const string S_SERVICE_PROFILE = "game-identity";
	protected static const string S_SERVICE_MULTIPLAYER = "game-api";
	protected static const string S_SERVICE_WORKSHOP = "reforger-workshop-api";

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		ArmaReforgerScripted game = GetGame();
		TextWidget buildInfo = TextWidget.Cast(w.FindAnyWidget("BuildInfo"));
		if (buildInfo)
			buildInfo.SetTextFormat(buildInfo.GetText(), game.GetBuildVersion(), game.GetBuildTime());

		m_wLegend = w.FindAnyWidget("Legend");

		m_wLastUpdate = TextWidget.Cast(w.FindAnyWidget("LastUpdate"));
		if (m_wLastUpdate)
			m_sLastUpdate = m_wLastUpdate.GetText();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuShow()
	{
		super.OnMenuShow();

		// recycle ESC button as R(eload) button
		if (m_Cancel)
		{
			m_Cancel.m_OnActivated.Remove(OnCancel);
			m_Cancel.m_OnActivated.Insert(RefreshServicesStatus);
		}

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, OnConfirm);
#ifdef WORKBENCH
			inputManager.AddActionListener("MenuBackWB", EActionTrigger.DOWN, OnConfirm);
#endif
		}

		m_ServicesStatusDialogComponent = SCR_ServicesStatusDialogComponent.Cast(GetRootWidget().FindHandler(SCR_ServicesStatusDialogComponent));
		if (!m_ServicesStatusDialogComponent)
			Print("No SCR_ServicesStatusDialogComponent component found | " + __FILE__ + ": " + __LINE__, LogLevel.WARNING);

		SetLegend();
		RefreshServicesStatus();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuHide()
	{
		if (m_Cancel && m_Cancel.m_OnActivated)
			m_Cancel.m_OnActivated.Remove(RefreshServicesStatus);

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnConfirm);
#ifdef WORKBENCH
			inputManager.RemoveActionListener("MenuBackWB", EActionTrigger.DOWN, OnConfirm);
#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetLegend()
	{
		if (!m_wLegend || !m_ServicesStatusDialogComponent)
			return;

		array<EServiceStatus> legendStatuses = { EServiceStatus.RUNNING, EServiceStatus.WARNING, EServiceStatus.ERROR };
		array<string> legendImages = { "RUNNING", "WARNING", "ERROR" };
		for (int i; i < 3; i++)
		{
			Widget legendWidget = m_wLegend.FindAnyWidget(legendImages[i]);
			if (!legendWidget)
				continue;

			ImageWidget iconWidget = ImageWidget.Cast(legendWidget.FindAnyWidget("Icon"));
			if (iconWidget)
				m_ServicesStatusDialogComponent.SetStatusImage(iconWidget, legendStatuses[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshServicesStatus()
	{
		if (!m_ServicesStatusDialogComponent)
			return;

		RefreshServiceStatusItems();
		array<EServiceStatus> statuses = {};
		statuses.Insert(GetEnumStatus(S_SERVICE_PROFILE));
		statuses.Insert(GetEnumStatus(S_SERVICE_MULTIPLAYER));
		statuses.Insert(GetEnumStatus(S_SERVICE_WORKSHOP));

		int i;
		m_ServicesStatusDialogComponent.SetServiceState("Profile", statuses[i++]);
		m_ServicesStatusDialogComponent.SetServiceState("Multiplayer", statuses[i++]);
		m_ServicesStatusDialogComponent.SetServiceState("Workshop", statuses[i++]);

		EServiceStatus generalStatus = EServiceStatus.RUNNING; // General Status reporting! o7
		if (statuses.Contains(EServiceStatus.ERROR))
			generalStatus = EServiceStatus.ERROR;
		else if (statuses.Contains(EServiceStatus.WARNING))
			generalStatus = EServiceStatus.WARNING;

		// Main Status & MOTD
		ServiceStatusItem mainStatus = SERVICE_STATUSES.Get(S_SERVICE_MAIN);
		if (mainStatus)
		{
			if (mainStatus.Status().IsEmpty())
				generalStatus = EServiceStatus.WARNING;

			m_ServicesStatusDialogComponent.SetMOTD(mainStatus.Message());
		}
		else
		{
			m_ServicesStatusDialogComponent.SetMOTD();
		}

		if (m_wImgTopLine)
			m_ServicesStatusDialogComponent.SetStatusImageColor(m_wImgTopLine, generalStatus);
		if (m_wImgTitleIcon)
			m_ServicesStatusDialogComponent.SetStatusImage(m_wImgTitleIcon, generalStatus);

		if (m_Cancel)
			m_Cancel.SetEnabled(false);

		RefreshLastUpdated();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshLastUpdated()
	{
		int secondsSinceUpdate;
		if (CanRefresh(secondsSinceUpdate) && m_Cancel && !m_Cancel.IsEnabled())
		{
			m_Cancel.SetEnabled(true);
		}

		GetGame().GetCallqueue().Remove(RefreshLastUpdated);
		if (m_wLastUpdate)
		{
			m_wLastUpdate.SetTextFormat(m_sLastUpdate, (secondsSinceUpdate / 60));
			GetGame().GetCallqueue().CallLater(RefreshLastUpdated, 1000);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param secondsSinceUpdate optional
	//! \return true if enough time has passed
	protected bool CanRefresh(out int secondsSinceUpdate = 0)
	{
		secondsSinceUpdate = Math.Round((float)(System.GetTickCount() - s_iLastUpdateTick) / 1000);
		return secondsSinceUpdate * 1000 >= S_REFRESH_TIMEOUT;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshServiceStatusItems()
	{
		if (!CanRefresh())
			return;

		BackendApi backendAPI = GetGame().GetBackendApi();
		if (!backendAPI)
			return;

		SERVICE_STATUSES.Clear();
		SERVICE_STATUSES.Insert(S_SERVICE_MAIN, backendAPI.GetMainStatus());

		string nameLC;
		ServiceStatusItem item;
		for (int i, cnt = backendAPI.GetStatusCount(); i < cnt; i++)
		{
			item = backendAPI.GetStatusItem(i);
			if (!item)
				continue;

			nameLC = item.Name();
			nameLC.ToLower();
			SERVICE_STATUSES.Set(nameLC, item);
		}

		s_iLastUpdateTick = System.GetTickCount();
	}

	//------------------------------------------------------------------------------------------------
	protected EServiceStatus GetEnumStatus(string wantedItemName)
	{
		wantedItemName.ToLower();
		ServiceStatusItem serviceItem = SERVICE_STATUSES.Get(wantedItemName);
		if (!serviceItem)
			return EServiceStatus.WARNING;

		return GetStatusFromString(serviceItem.Status());
	}

	//------------------------------------------------------------------------------------------------
	protected EServiceStatus GetStatusFromString(string status)
	{
		status.ToLower();

		if (status == "ok")
			return EServiceStatus.RUNNING;

		if (status == "error")
			return EServiceStatus.ERROR;

		return EServiceStatus.WARNING;
	}
};
