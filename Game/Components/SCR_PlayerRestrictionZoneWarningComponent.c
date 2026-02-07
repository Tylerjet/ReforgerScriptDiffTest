[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_PlayerRestrictionZoneWarningComponentClass: ScriptComponentClass
{
};
class SCR_PlayerRestrictionZoneWarningComponent: ScriptComponent
{
	protected SCR_RestrictionZoneWarningHUDComponent m_WarningHUD;
	protected bool m_bShowingWarning;
	
	
	void ShowWarningServer(bool showWarning)
	{
		if (m_bShowingWarning == showWarning || !Replication.IsServer())
			return;
		
		m_bShowingWarning = showWarning;
		Rpc(ShowWarningOwner, showWarning);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ShowWarningOwner(bool showWarning)
	{		
		if (m_WarningHUD)
			m_WarningHUD.Show(showWarning);
	}
	
	override void EOnInit(IEntity owner)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(owner.FindComponent(SCR_HUDManagerComponent));
		if (hudManager)
		{
			array<BaseInfoDisplay> infoDisplays = new array<BaseInfoDisplay>;
			int count = hudManager.GetInfoDisplays(infoDisplays);
		
			for(int i = 0; i < count; i++)
       		{
	            if (infoDisplays[i].Type() == SCR_RestrictionZoneWarningHUDComponent)
				{
					m_WarningHUD = SCR_RestrictionZoneWarningHUDComponent.Cast(infoDisplays[i]);
					if (m_WarningHUD)
						break;
				}
       	 	}
		}
	}
	override void OnPostInit(IEntity owner)
	{
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT);
	}
};
