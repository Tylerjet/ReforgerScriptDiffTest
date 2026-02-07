class SCR_OnlineServiceClient
{
	//------------------------------------------------------------------------------------------------
	protected static ref ScriptInvoker OnEvent_ServerListObtained = new ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	protected static ref SCR_OnlineServiceClient onlineServiceClient;
	protected static ref SCR_OnlineServiceClientCallbacks m_ServerListCallback;
	
	//------------------------------------------------------------------------------------------------
	protected static bool s_bIsObtainingServerList;
	
	//------------------------------------------------------------------------------------------------
	void OnServerList(SBServerInfoList server_info_list, int server_count,  int page_count,  int page_index, OnlineError error)
	{
		s_bIsObtainingServerList = false;
		
		ServerListDebugPrint(server_info_list, server_count, page_count, page_index, error);
		
		OnEvent_ServerListObtained.Invoke(server_info_list, server_count, page_count, page_index, error);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ServerListDebugPrint(SBServerInfoList serverInfoList, int serverCount,  int pageCount,  int pageIndex, OnlineError error)
	{
		/*
		Print("ServerListDebugPrint");
		Print("Error: " + error.ToString());
		Print("Server Count: " + serverCount.ToString());
		Print("Page Count: " + pageCount.ToString());
		Print("Page Index: " + pageIndex.ToString());
		*/
		
		if (error != OnlineError.NONE)
		{
			return;
		}
		
		/*
		for (int i = 0; i < serverInfoList.Count(); i++)
		{
			Print("Server: " + i.ToString());
			Print("Identifier: " + serverInfoList[i].Identifier);
			Print("Flags: " + serverInfoList[i].Flags.ToString());
			Print("Name: " + serverInfoList[i].Name);
			Print("Description: " + serverInfoList[i].Description);
			Print("Host Port: " + serverInfoList[i].HostPort.ToString());
			Print("Host Ip: " + serverInfoList[i].HostIp);
			Print("Game Type: " + serverInfoList[i].GameType.ToString());
			Print("Game Version: " + serverInfoList[i].GameVersion);
			Print("Game Mode: " + serverInfoList[i].GameMode.ToString());
			Print("Client Version: " + serverInfoList[i].ClientVersion);
			Print("Region: " + serverInfoList[i].Region.ToString());
			Print("Platform: " + serverInfoList[i].Platform.ToString());
			Print("Max Players: " + serverInfoList[i].MaxPlayers.ToString());
			Print("Free Slots: " + serverInfoList[i].FreeSlots.ToString());
			Print("Num Players: " + serverInfoList[i].NumPlayers.ToString());
			Print("Json Metadata: " + serverInfoList[i].JsonMetadata);
		}
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	static bool GetServers(SBGetServersParams getServerParams)
	{
		if (s_bIsObtainingServerList)
		{
			return false;
		}
		
		SetupOnlineServiceClient();
		
		bool result = ServerBrowser.GetServers(getServerParams, m_ServerListCallback);
//		s_bIsObtainingServerList = result; // i have no idea how this event could worked before! anyway - there is problem with blocking request - will be reworked later
		s_bIsObtainingServerList = false;
		
		if (!result)
		{
			//Error("Online Services - GetServers: Request was not successfull");
			Print("Online Services - GetServers: Request was not successfull");
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetCallbackInvoker()
	{
		return OnEvent_ServerListObtained;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected static void SetupOnlineServiceClient()
	{
		if (onlineServiceClient == null)
		{
			onlineServiceClient 	= new SCR_OnlineServiceClient;
			m_ServerListCallback 	= new SCR_OnlineServiceClientCallbacks(onlineServiceClient);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_OnlineServiceClient()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_OnlineServiceClient()
	{
		
	}
};

class SCR_OnlineServiceClientCallbacks : SBServerListCallback
{
	protected SCR_OnlineServiceClient m_Delegate;
	
	//------------------------------------------------------------------------------------------------
	override void OnServerList(SBServerInfoList server_info_list, int server_count, int page_count, int page_index, OnlineError error)
	{	
		m_Delegate.OnServerList(server_info_list, server_count, page_count, page_index, error);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_OnlineServiceClientCallbacks(SCR_OnlineServiceClient delegate)
	{
		m_Delegate = delegate;
	}
};