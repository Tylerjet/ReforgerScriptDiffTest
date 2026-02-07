//------------------------------------------------------------------------------------------------
class SCR_PlayerLoadingDetectionComponentClass: ScriptGameComponentClass
{}

//------------------------------------------------------------------------------------------------
//! Used to give server feedback when player is in loading screen, attached to SCR_PlayerController
class SCR_PlayerLoadingDetectionComponent : ScriptComponent
{
	[RplProp(onRplName: "OnLoadingStateChanged")]
	protected bool m_bIsLoading;
	
	protected ref ScriptInvoker m_OnLoadingFinished = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnLoadingFinished()
	{
		return m_OnLoadingFinished;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true, if player has loading screen
	bool IsLoading()
	{
		return m_bIsLoading;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoadingStateChanged()
	{
		if (!m_bIsLoading)
			m_OnLoadingFinished.Invoke(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetIsLoading()
	{
		bool isLoading = ArmaReforgerLoadingAnim.IsOpen();
		Rpc(RpcAsk_SetIsLoading, isLoading);
		RpcAsk_SetIsLoading(isLoading);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetIsLoading(bool isLoading)
	{
		if (m_bIsLoading == isLoading)
			return;
		
		m_bIsLoading = isLoading;
		OnLoadingStateChanged();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------		
	override void EOnInit(IEntity owner)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
		
		ArmaReforgerLoadingAnim.s_OnEnterLoadingScreen.Insert(SetIsLoading);
		ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Insert(SetIsLoading);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_PlayerLoadingDetectionComponent()
	{
		ArmaReforgerLoadingAnim.s_OnEnterLoadingScreen.Remove(SetIsLoading);
		ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Remove(SetIsLoading);
	}
}