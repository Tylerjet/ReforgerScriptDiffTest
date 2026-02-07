
//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingServerTriggerClass: SCR_CampaignBuildingTriggerClass
{
};

class SCR_CampaignBuildingServerTrigger : SCR_CampaignBuildingTrigger
{
	protected SCR_SiteSlotEntity m_Slot;
	protected SCR_CampaignFaction m_Faction;
	protected int m_iIndex;
	protected vector m_vPosition;
	protected SCR_CampaignBase m_Base;
	protected SCR_CampaignNetworkComponent m_NetworkComponent;
	protected IEntity m_BlockingEntity;
	
	protected ref array<IEntity> m_aInside = {}; 
	
	[Attribute("{06E1B6EBD480C6E0}Prefabs/AI/Waypoints/AIWaypoint_ForcedMove.et", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sAILeaveWP;
	
	[Attribute("{87086D9285798BF1}Prefabs/MP/Campaign/BuildingAreaBlocker.et", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sBuildingAreaBlocker;
		
	[Attribute("0 0 30", UIWidgets.Auto, "AI WP offset", category: "Root dirt settings")]
	private vector m_vWPoffset;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{		
		QueryEntitiesInside();
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_NetworkComponent || !m_Slot || !m_Faction || !m_aInside.IsEmpty())
			return;
		
		m_NetworkComponent.BuildComposition(m_Slot, m_Faction,	m_iIndex, m_vPosition);
		
		if (m_Base)
			m_Base.HandleMapInfo();

		if (m_BlockingEntity)
			SCR_EntityHelper.DeleteEntityAndChildren(m_BlockingEntity);
		SCR_EntityHelper.DeleteEntityAndChildren(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity ent)
	{		
		if (CanBlockPreview(ent))	
		{
			m_aInside.Insert(ent);
			ClearAreaRequest(ent);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeactivate(IEntity ent)
	{
 		super.OnDeactivate(ent);
		
		m_aInside.RemoveItem(ent);	
		
		// Remove invoker
		SCR_CharacterControllerComponent charControllerComp = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
		if (charControllerComp)
			charControllerComp.m_OnPlayerDeathWithParam.Remove(OnDeath);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnQueryFinished(bool bIsEmpty)
	{
		super.OnQueryFinished(bIsEmpty);

		GetEntitiesInside(m_aInside);

		for (int i = m_aInside.Count() - 1; i >= 0; i--)
		{
			if (CanBlockPreview(m_aInside[i]))
				continue;
			
			m_aInside.Remove(i);
		}

		SetEventMask(EntityEvent.FRAME);
		m_BlockingEntity = SpawnBlockingEntity();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBlockPreview(notnull IEntity element)
	{	
		ChimeraCharacter char = ChimeraCharacter.Cast(element);
		if (char)
		{
			// It's a character but it's dead.
			SCR_CharacterControllerComponent charControllerComp = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
			if (charControllerComp && charControllerComp.IsDead())
				return false;
			
			// If the character is alive, add scripted invoker to remove dead soldier from the list of entities inside of the trigger
			if (charControllerComp)
				charControllerComp.m_OnPlayerDeathWithParam.Insert(OnDeath);	
			
			if (FindAiAgent(element))
				return true;
		}

		// If it's something else like a gear, weapon etc, remove it from array of blocking elements.
		return super.CanBlockPreview(element);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDeath(SCR_CharacterControllerComponent charControllerComp)
	{
		if (!charControllerComp)
			return;
		
		IEntity ent = charControllerComp.GetCharacter();
		if (!ent)
			return;
		
		m_aInside.RemoveItem(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	private void ClearAreaRequest(IEntity ent)
	{
		AIControlComponent AIControlComp = AIControlComponent.Cast(ent.FindComponent(AIControlComponent));
		if (!AIControlComp)
			return;
			
		AIAgent agent = AIControlComp.GetControlAIAgent();
		if (!agent)
			return;
		
		AIWaypoint wp = CreateWaypoint(m_sAILeaveWP);
		if (!wp)
			return;
		
		wp.SetOrigin(GetOrigin() + m_vWPoffset);

		AIGroup group = agent.GetParentGroup();
		if (group)
			group.AddWaypointAt(wp, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	AIWaypoint CreateWaypoint(ResourceName path)
	{
		Resource resource = Resource.Load(path);
		if (!resource || !resource.IsValid())
			return null;
		
		AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resource));
		if (!wp)
			return null;
		
		return wp;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity SpawnBlockingEntity()
	{
		Resource res = Resource.Load(m_sBuildingAreaBlocker);
		if (!res.IsValid())
			return null;
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;				
		params.Transform[3] = this.GetOrigin();
		IEntity blocker = IEntity.Cast(GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params));
		return blocker;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSlot(SCR_SiteSlotEntity slot)
	{
		m_Slot = slot;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOwningFaction(SCR_CampaignFaction faction)
	{
		m_Faction = faction;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCompositionIndex(int index)
	{
		m_iIndex = index;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSlotAngle(vector angle)
	{
		m_vPosition = angle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBase(SCR_CampaignBase base)
	{
		m_Base = base;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetNetworkComponent(notnull SCR_CampaignNetworkComponent networkComponent)
	{
		m_NetworkComponent = networkComponent;
	}
};