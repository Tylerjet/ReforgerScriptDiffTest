//! This class may become obsolete on BackendAPI update
class SCR_ServicesStatusHelper
{
	protected static int s_iLastPingUpdate = -1;
	protected static int s_iLastStatusUpdate = -1;
	protected static BackendApi BACKEND_API;
	protected static ServiceStatusItem s_MainStatus;
	protected static const ref array<ServiceStatusItem> SERVICE_STATUSES = {};

	//------------------------------------------------------------------------------------------------
	static bool IsBackendEnabled()
	{
		GetBackendAPI();

		return BACKEND_API && BACKEND_API.IsRunning();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsBackendReady()
	{
		GetBackendAPI();

		return BACKEND_API && BACKEND_API.IsActive();
	}

	//------------------------------------------------------------------------------------------------
	static protected BackendApi GetBackendAPI()
	{
		if (!BACKEND_API)
			BACKEND_API = GetGame().GetBackendApi();

		return BACKEND_API;
	}

	//------------------------------------------------------------------------------------------------
	// PING
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	static bool DidPingHappen()
	{
		if (!IsBackendReady())
			return false;

		return BACKEND_API.GetCommTestStatus() > 0;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsPinging()
	{
		if (!IsBackendReady())
			return false;

		return BACKEND_API && BACKEND_API.GetCommTestStatus() == 1;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsPingReady()
	{
		if (!IsBackendReady())
			return false;

		return BACKEND_API && BACKEND_API.GetCommTestStatus() == 2;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsPingFailed()
	{
		if (!IsBackendReady())
			return false;

		return BACKEND_API && BACKEND_API.GetCommTestStatus() == 3;
	}

	//------------------------------------------------------------------------------------------------
	//! Refresh ping value - won't do anything if is already pinging and waiting for the result
	static void RefreshPing()
	{
		if (!IsBackendReady() || IsPinging())
			return;

		BACKEND_API.RefreshCommStatus();
		s_iLastPingUpdate = System.GetTickCount();
	}

	//------------------------------------------------------------------------------------------------
	//! Return ping value
	//! \return ping in milliseconds
	static int GetPingValue()
	{
		if (!IsBackendReady())
			return -1;

		return Math.Round(BACKEND_API.GetCommResponseTime());
	}

	//------------------------------------------------------------------------------------------------
	//! Return the last ping's age in milliseconds
	static int GetPingAge()
	{
		if (!IsBackendReady())
			return -1;

		return System.GetTickCount() - s_iLastPingUpdate;
	}

	//------------------------------------------------------------------------------------------------
	// STATUSES
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	static void RefreshStatuses()
	{
		if (!IsBackendReady())
			return;

		s_MainStatus = BACKEND_API.GetMainStatus();

		SERVICE_STATUSES.Clear();
		ServiceStatusItem item;
		for (int i, cnt = BACKEND_API.GetStatusCount(); i < cnt; i++)
		{
			item = BACKEND_API.GetStatusItem(i);
			if (item)
				SERVICE_STATUSES.Insert(item);
		}

		s_iLastStatusUpdate = System.GetTickCount();

		if (GetGame().IsDev())
			DEBUG();
	}

	//------------------------------------------------------------------------------------------------
	static ServiceStatusItem GetMainStatus()
	{
		return s_MainStatus;
	}

	//------------------------------------------------------------------------------------------------
	//! finds a service by name
	//! \param statusName name of the wanted status, case INsensitive
	static ServiceStatusItem GetStatusByName(string statusName)
	{
		ServiceStatusItem result;

		statusName.ToLower();
		string serviceNameLC;

		foreach (ServiceStatusItem statusItem : SERVICE_STATUSES)
		{
			serviceNameLC = statusItem.Name();
			serviceNameLC.ToLower();

			if (serviceNameLC == statusName)
				return statusItem;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	static void GetStatuses(notnull array<ServiceStatusItem> statuses)
	{
		statuses.Copy(SERVICE_STATUSES);
	}

	//------------------------------------------------------------------------------------------------
	static bool AreServicesReady()
	{
		return s_MainStatus && !SERVICE_STATUSES.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Return statuses age IN SECONDS
	static int GetStatusesAge()
	{
		if (!IsBackendReady())
			return -1;

		return Math.Round((float)(System.GetTickCount() - s_iLastStatusUpdate) / 1000);
	}

	//------------------------------------------------------------------------------------------------
	static void DEBUG()
	{
		Print("--------------------------------------------------");
		Print("SERVICE STATUS DEBUG");
		Print("--------------------------------------------------");
		if (!BACKEND_API)
		{
			Print("no backend API found.");
			return;
		}

		Print("is main service found: " + (BACKEND_API.GetMainStatus() != null));
		Print("number of services found: " + BACKEND_API.GetStatusCount());

		if (s_MainStatus)
			PrintFormat("Service %1: %2 (Name/Status/Message: %3/%4/%5)", -1, s_MainStatus, s_MainStatus.Name(), s_MainStatus.Status(), s_MainStatus.Message());
		else
			Print("Main Status not found");

		foreach (int index, ServiceStatusItem statusItem : SERVICE_STATUSES)
		{
			if (statusItem)
				PrintFormat("Service %1: %2 (Name/Status/Message: %3/%4/%5)", index, statusItem, statusItem.Name(), statusItem.Status(), statusItem.Message());
		}

		Print("--------------------------------------------------");
	}
};
