//------------------------------------------------------------------------------------------------
class SCR_ReconnectSynchronizationComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Takes care of synchronization between server side ReconnectComponent and client side RespawnComponent
//! Attached to SCR_PlayerController prefab
class SCR_ReconnectSynchronizationComponent : ScriptComponent
{
	protected const string DIALOG_RECON_RESTORE = "reconnect_restored";
	protected const string DIALOG_RECON_DISCARD = "reconnect_discarded";
	
	protected bool m_bIsReconnecting;
	
	//------------------------------------------------------------------------------------------------
	//! Get if player is considered reconnecting
	bool GetIsReconnecting()
	{
		return m_bIsReconnecting;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update the current reconnection status of a player
	void UpdateReconnectStatus()
	{
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		
		Rpc(RPC_AskRecconectStatusUpdate, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ConfigurableDialogUi event
	protected void OnDialogConfirm(SCR_ConfigurableDialogUi dialog)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;
		
		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(pm.GetPlayerRespawnComponent(SCR_PlayerController.GetLocalPlayerId()));
		if (rc)
			rc.RequestQuickRespawn();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Ask RPC for server
	//! \param playerID is ID of a subject
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    protected void RPC_AskRecconectStatusUpdate(int playerID)
	{	
		SCR_ReconnectComponent reconComp = SCR_ReconnectComponent.GetInstance();
		if (!reconComp || !reconComp.IsReconnectEnabled())
			return;
				
		Rpc(RPC_DoUpdateReconnectStatus, reconComp.IsInReconnectList(playerID));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Do RPC from server 
	//! \param state is subject SCR_EReconnectState
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
    protected void RPC_DoUpdateReconnectStatus(int state)
    {		
		m_bIsReconnecting = false;
		
		if (state == SCR_EReconnectState.NOT_RECONNECT)
			return;
		
		m_bIsReconnecting = true;
		if (state == SCR_EReconnectState.ENTITY_AVAILABLE)
		{
			SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog(DIALOG_RECON_RESTORE);
			dialog.m_OnConfirm.Insert(OnDialogConfirm);
		}
		else if (state == SCR_EReconnectState.ENTITY_DISCARDED)
			SCR_CommonDialogs.CreateDialog(DIALOG_RECON_DISCARD);
    }
	
};
