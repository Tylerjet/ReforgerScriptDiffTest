[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_PressureTriggerComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PressureTriggerComponent : ScriptComponent
{
	protected const float MIN_DELAY = 125; //time between explosion tries in ms
	protected float m_fLastTryTime = 0;
	
	[Attribute("0", "Is this mine live by default?")]
	protected bool m_bLive;
	
	[RplProp(onRplName:"OnActivatedChanged")]
	protected bool m_bActivated = false;
	
	[Attribute("10", desc: "Min. weight that can set off this trigger when applied to it. [kg]")]
	protected float m_fMinWeight;
	
	[Attribute("", desc: "Name of the fuze mesh that should unhide when mine is activated")]
	protected string m_sFuzeMeshName;
	
	//------------------------------------------------------------------------------------------------
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (GetGame().GetWorld().GetWorldTime() - m_fLastTryTime < MIN_DELAY)
			return;
		
		Physics otherPhysics = other.GetPhysics();
		if (!otherPhysics)
			return;
		
	 	float otherMass = otherPhysics.GetMass();
		VehicleWheeledSimulation vehicleSimulation = VehicleWheeledSimulation.Cast(other.FindComponent(VehicleWheeledSimulation));
		if (vehicleSimulation)
			otherMass /= vehicleSimulation.WheelCount(); // assume it's a vehicle and assume min. weight it lays on the trigger is weight / wheels count
		
		m_fLastTryTime = GetGame().GetWorld().GetWorldTime();
		
		if (otherMass < m_fMinWeight)
			return; // Too light, won't set the trigger off
		
		BaseTriggerComponent baseTriggerComponent = BaseTriggerComponent.Cast(GetOwner().FindComponent(BaseTriggerComponent));
		if (!baseTriggerComponent)
			return;
		
		GetGame().GetCallqueue().CallLater(RPC_DoTrigger); // Delay it to next frame, cannot delete entity in EOnContact
		Rpc(RPC_DoTrigger);
	}

	//------------------------------------------------------------------------------------------------
	bool IsActivated()
	{
		return m_bActivated;
	}
	
	//------------------------------------------------------------------------------------------------
	// Only call this on the server
	void Activate()
	{
		IEntity owner = GetOwner();
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rplComponent || rplComponent.IsProxy())
			return;
		
		BaseTriggerComponent baseTriggerComponent = BaseTriggerComponent.Cast(owner.FindComponent(BaseTriggerComponent));
		if (!baseTriggerComponent)
			return;
		
		GarbageManager garbageManager = GetGame().GetGarbageManager();
		if (garbageManager)
			garbageManager.Withdraw(owner); //withdraw from garbage manager to avoid unwanted deletion
		
		baseTriggerComponent.SetLive(owner);
		m_bActivated = true;
		Replication.BumpMe();
		
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.CONTACT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowFuse()
	{
		IEntity owner = GetOwner();
		int meshIndex = GameAnimationUtils.FindMeshIndex(owner, m_sFuzeMeshName);
		if (meshIndex == -1)
			return;
		
		GameAnimationUtils.ShowMesh(owner, meshIndex, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActivatedChanged()
	{
		if (m_bActivated)
			ShowFuse();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoTrigger()
	{
		BaseTriggerComponent baseTriggerComponent = BaseTriggerComponent.Cast(GetOwner().FindComponent(BaseTriggerComponent));
		if (!baseTriggerComponent)
			return;
		
		baseTriggerComponent.OnUserTrigger(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void EOnInit(IEntity owner)
	{
		if (m_bLive)
			Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnPostInit(IEntity owner)
	{
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT);
	}
};
