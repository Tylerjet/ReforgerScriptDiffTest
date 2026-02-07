//------------------------------------------------------------------------------------------------
class SCR_PlayerLoadingDetectionComponentClass: ScriptGameComponentClass
{}

//------------------------------------------------------------------------------------------------
//! Used to give server feedback when player is in loading screen, attached to SCR_PlayerController
class SCR_PlayerLoadingDetectionComponent : ScriptComponent
{
	[RplProp(onRplName: "OnLoadingStateChanged")]
	protected bool m_bIsLoading;
	
	protected int m_iLoadingScreensActive;
	protected ref ScriptInvoker m_OnLoadingFinished = new ScriptInvoker();
	
	protected static const int LOADING_STOPPED_CHECK_DELAY = 500;
	
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
	protected void AddLoadingScreen()
	{
		if (m_iLoadingScreensActive < 1)
			SetIsLoading();
		
		m_iLoadingScreensActive++;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveLoadingScreen()
	{
		m_iLoadingScreensActive--;
		
		//Delayed call, as another loading screen might appear. Delay between them is usually lot shorter.
		if (m_iLoadingScreensActive < 1)
			GetGame().GetCallqueue().CallLater(SetLoadingCheckDelayed, LOADING_STOPPED_CHECK_DELAY, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetLoadingCheckDelayed()
	{
		if (m_iLoadingScreensActive < 1)
			SetIsLoading();
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
		
		ArmaReforgerLoadingAnim.s_OnEnterLoadingScreen.Insert(AddLoadingScreen);
		ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Insert(RemoveLoadingScreen);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		m_iLoadingScreensActive = 0;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_PlayerLoadingDetectionComponent()
	{
		ArmaReforgerLoadingAnim.s_OnEnterLoadingScreen.Remove(SetIsLoading);
		ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Remove(SetIsLoading);
	}
}