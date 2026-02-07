[ComponentEditorProps(category: "GameScripted/AI", description: "Component for AI checking state of group", color: "0 0 255 255")]
class SCR_AIGroupInfoComponentClass: SCR_AIInfoBaseComponentClass
{
};

enum EGroupControlMode
{
	NONE = 0,
	IDLE, 					///< Group has no waypoints and does not engage an enemy
	AUTONOMOUS, 			///< Group behaves autonomously, e.g. engaging an enemy
	FOLLOWING_WAYPOINT,		///< Group is following a waypoint
	LAST,
};

//------------------------------------------------------------------------------------------------
class SCR_AIGroupInfoComponent : SCR_AIInfoBaseComponent
{
	private EGroupControlMode m_eGroupControlMode;
	
	protected ref ScriptInvoker Event_OnControlModeChanged = new ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);	
	}	
	
	//------------------------------------------------------------------------------------------------
	void SetGroupControlMode(EGroupControlMode currControlMode) ///< This is informative property, does not set the behavior of group to the respective state!
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetGroupControlMode: %1", typename.EnumToString(EGroupControlMode, currControlMode)), msgType:EAIDebugMsgType.INFO);
		#endif
		
		if (currControlMode == m_eGroupControlMode)
			return;
		
		if (!Replication.IsServer())
			return;
		
		
		RplSetGroupControlMode(currControlMode);
		Rpc(RplSetGroupControlMode, currControlMode);	
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RplSetGroupControlMode(EGroupControlMode currControlMode)
	{
		m_eGroupControlMode = currControlMode;
		Event_OnControlModeChanged.Invoke(currControlMode);
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			aiWorld.GetOnControlModeChanged().Invoke(GetOwner(), currControlMode);
	}
	
	//------------------------------------------------------------------------------------------------
	EGroupControlMode GetGroupControlMode()
	{
		return m_eGroupControlMode; 
	}	
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnControlModeChanged()
	{
		return Event_OnControlModeChanged;
	}	
	
	//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
    {	
        writer.WriteIntRange(m_eGroupControlMode, 0, EGroupControlMode.LAST-1);
		
        return true;
    }
     
    override bool RplLoad(ScriptBitReader reader)
    {
		EGroupControlMode groupControlMode;
		
        reader.ReadIntRange(groupControlMode, 0, EGroupControlMode.LAST-1);
		
		RplSetGroupControlMode(groupControlMode);
		
        return true;
    }
};