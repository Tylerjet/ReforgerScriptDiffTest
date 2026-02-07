[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "")]
class SCR_BaseTriggerComponentClass : BaseTriggerComponentClass
{
}

class SCR_BaseTriggerComponent : BaseTriggerComponent
{
	[Attribute("0", "Is this mine live by default?")]
	protected bool m_bLive;
	
	[RplProp(onRplName:"OnActivatedChanged")]
	protected bool m_bActivated = false;
	
	[Attribute("", desc: "Name of the fuze mesh that should unhide when mine is activated")]
	protected string m_sFuzeMeshName;
	
	//protected Instigator m_User;

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsActivated()
	{
		return m_bActivated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] user
	void SetUser(notnull IEntity user)
	{
		GetInstigator().SetInstigator(user);
		//m_User = user;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	// Only call this on the server
	void ActivateTrigger()
	{
		GenericEntity owner = GenericEntity.Cast(GetOwner());
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rplComponent || rplComponent.IsProxy())
			return;

		auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(owner);
		if (garbageSystem)
			garbageSystem.Withdraw(owner);

		SetLive();
		m_bActivated = true;
		ShowFuse();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ShowFuse()
	{
		IEntity owner = GetOwner();
		int meshIndex = GameAnimationUtils.FindMeshIndex(owner, m_sFuzeMeshName);
		if (meshIndex == -1)
			return;
		
		GameAnimationUtils.ShowMesh(owner, meshIndex, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Method called on the clients, the item should be outside inventory already
	void OnActivatedChanged()
	{
		if (m_bActivated)
			ShowFuse();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoTrigger()
	{
		BaseTriggerComponent baseTriggerComponent = BaseTriggerComponent.Cast(GetOwner().FindComponent(BaseTriggerComponent));
		if (!baseTriggerComponent)
			return;
		
		baseTriggerComponent.OnUserTriggerOverrideInstigator(GetOwner(), GetInstigator());
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void EOnInit(IEntity owner)
	{
		if (m_bLive)
			ActivateTrigger(); // Using call later to avoid accessing uninitialized components
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_BaseTriggerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetEventMask(ent, EntityEvent.INIT);
	}
}
