enum EServiceStatus
{
	RUNNING = 0,
	WARNING,
	ERROR,
};

class SCR_ServicesStatusDialogUI : DialogUI
{
	protected TextWidget m_wLastUpdate;
	protected string m_sLastUpdate;
	protected Widget m_wLegend;

	protected SCR_ServicesStatusDialogComponent m_ServicesStatusDialogComponent;

	protected static const int REFRESH_TIMEOUT = 5000;
	protected static const int LAST_UPDATED_REFRESH_RATE = 5000;
	protected static const int PING_REFRESH_RATE = 100;
	protected static const int PING_EXPIRE_DATE = 60000;
	protected static const int BACKEND_READY_REFRESH_RATE = 1000;

	protected static const string OK_STATUS = "ok";
	protected static const string ERROR_STATUS = "error";

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
			m_Cancel.m_OnActivated.Insert(OnRefresh);
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
		OnRefresh();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuHide()
	{
		if (m_Cancel && m_Cancel.m_OnActivated)
			m_Cancel.m_OnActivated.Remove(OnRefresh);

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
				m_ServicesStatusDialogComponent.SetStatusImageAndColor(iconWidget, legendStatuses[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! refresh service statuses
	//! refresh UI through ServicesStatusDialogComponent
	protected void OnRefresh()
	{
		if (!m_ServicesStatusDialogComponent)
		{
			m_Cancel.SetVisible(false, false);
			return;
		}

		EServiceStatus generalStatus = EServiceStatus.RUNNING; // General Status reporting! o7
		string motd;

		if (CanRefresh())
		{
			SCR_ServicesStatusHelper.RefreshPing();
			m_ServicesStatusDialogComponent.SetPing(0); // sets the " ... " value to indicate value refresh
			RefreshPingAndItsUI();

			SCR_ServicesStatusHelper.RefreshStatuses();
		}
		else
		{
			m_ServicesStatusDialogComponent.SetPing(SCR_ServicesStatusHelper.GetPingValue());
		}

		// Main Status & MOTD
		ServiceStatusItem mainStatus = SCR_ServicesStatusHelper.GetMainStatus();
		if (!mainStatus || mainStatus.Status().IsEmpty())
		{
			generalStatus = EServiceStatus.ERROR;
			m_ServicesStatusDialogComponent.SetAllServicesState(generalStatus);
		}
		else
		{
			motd = mainStatus.Message();

			EServiceStatus status;
			array<EServiceStatus> statuses = {};
			foreach (SCR_ServicesStatusDialogComponent_Service serviceInfo : m_ServicesStatusDialogComponent.GetAllServices())
			{
				status = GetEnumStatus(serviceInfo.m_sServiceId);
				m_ServicesStatusDialogComponent.SetServiceState(serviceInfo.m_sId, status);
				statuses.Insert(status);
			}

			if (statuses.Contains(EServiceStatus.ERROR))
				generalStatus = EServiceStatus.ERROR;
			else if (statuses.Contains(EServiceStatus.WARNING))
				generalStatus = EServiceStatus.WARNING;
		}

		m_ServicesStatusDialogComponent.SetMOTD(motd);

		if (m_wImgTopLine)
			m_ServicesStatusDialogComponent.SetStatusImageColor(m_wImgTopLine, generalStatus);

		if (m_wImgTitleIcon)
			m_ServicesStatusDialogComponent.SetStatusImageAndColor(m_wImgTitleIcon, generalStatus);

		if (m_Cancel)
			m_Cancel.SetEnabled(false);

		RefreshLastUpdated();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshPingAndItsUI()
	{
		if (SCR_ServicesStatusHelper.IsPingReady())
		{
			m_ServicesStatusDialogComponent.SetPing(SCR_ServicesStatusHelper.GetPingValue());
			return;
		}

		if (!SCR_ServicesStatusHelper.IsPinging())
			SCR_ServicesStatusHelper.RefreshPing();

		GetGame().GetCallqueue().CallLater(RefreshPingAndItsUI, PING_REFRESH_RATE, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshLastUpdated()
	{
		if (CanRefresh() && m_Cancel && !m_Cancel.IsEnabled())
		{
			m_Cancel.SetEnabled(true);
		}

		GetGame().GetCallqueue().Remove(RefreshLastUpdated);
		if (m_wLastUpdate)
		{
			m_wLastUpdate.SetTextFormat(m_sLastUpdate, SCR_ServicesStatusHelper.GetPingAge() / 60000);
			GetGame().GetCallqueue().CallLater(RefreshLastUpdated, LAST_UPDATED_REFRESH_RATE);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param secondsSinceUpdate optional
	//! \return true if enough time has passed
	protected static bool CanRefresh()
	{
		if (!SCR_ServicesStatusHelper.IsBackendReady())
			return false;

		if (!SCR_ServicesStatusHelper.DidPingHappen())
			return true;

		if (SCR_ServicesStatusHelper.IsPinging())
			return false;

		return SCR_ServicesStatusHelper.GetPingAge() >= REFRESH_TIMEOUT;
	}

	//------------------------------------------------------------------------------------------------
	protected EServiceStatus GetEnumStatus(string wantedItemName)
	{
		ServiceStatusItem serviceItem = SCR_ServicesStatusHelper.GetStatusByName(wantedItemName);
		if (!serviceItem)
			return EServiceStatus.WARNING;

		return GetStatusFromString(serviceItem.Status());
	}

	//------------------------------------------------------------------------------------------------
	protected EServiceStatus GetStatusFromString(string status)
	{
		status.ToLower();

		if (status == OK_STATUS)
			return EServiceStatus.RUNNING;

		if (status == ERROR_STATUS)
			return EServiceStatus.ERROR;

		return EServiceStatus.WARNING;
	}

	//------------------------------------------------------------------------------------------------
	// the waiting-for-display logic should happen in MainMenuUI, the refresh logic in here
	static void OpenIfServicesAreNotOK()
	{
		// returns false even if enabled but without internet connection for now
		if (!SCR_ServicesStatusHelper.IsBackendEnabled())
		{
			// GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServicesStatusDialog);
			return;
		}

		if (SCR_ServicesStatusHelper.IsPinging())
		{
			GetGame().GetCallqueue().CallLater(OpenIfServicesAreNotOK, PING_REFRESH_RATE); // waiting for the ping to be done
			return;
		}

		if (!SCR_ServicesStatusHelper.DidPingHappen() || SCR_ServicesStatusHelper.GetPingAge() > PING_EXPIRE_DATE)
		{
			if (!SCR_ServicesStatusHelper.IsBackendReady())
			{
				GetGame().GetCallqueue().CallLater(OpenIfServicesAreNotOK, BACKEND_READY_REFRESH_RATE); // check later if backend is ready
				return;
			}

			SCR_ServicesStatusHelper.RefreshPing();
			GetGame().GetCallqueue().CallLater(OpenIfServicesAreNotOK, PING_REFRESH_RATE);
			return;
		}

		SCR_ServicesStatusHelper.RefreshStatuses();
		bool isOK = true;
		ServiceStatusItem mainStatus = SCR_ServicesStatusHelper.GetMainStatus();

		if (!mainStatus || mainStatus.Status() != OK_STATUS)
		{
			isOK = false;
		}
		else
		{
			array<ServiceStatusItem> statusItems = {};
			SCR_ServicesStatusHelper.GetStatuses(statusItems);

			foreach (ServiceStatusItem statusItem : statusItems)
			{
				if (statusItem.Status() != OK_STATUS)
				{
					isOK = false;
					break;
				}
			}
		}

		if (!isOK)
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServicesStatusDialog);
	}
};
