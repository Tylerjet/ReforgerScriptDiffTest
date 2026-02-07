//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingStartUserAction : ScriptedUserAction
{		
	protected SCR_CampaignBuildingProviderComponent m_ProviderComponent;
	protected SCR_CampaignSuppliesComponent m_SupplyComponent;
	protected RplComponent m_RplComponent;
	static const float MAX_SEARCH_DISTANCE = 20;
	
	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void InitializeSuppliesComponent()
	{
		// Check if the supplies component is at the owner (supply truck)
		m_ProviderComponent = SCR_CampaignBuildingProviderComponent.Cast(GetOwner().FindComponent(SCR_CampaignBuildingProviderComponent));
		if (m_ProviderComponent)
			return;
		
		// if not, look around for a base as the sign has no way how to reach root of the Editable entity
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		vector origin = GetOwner().GetOrigin();
		world.QueryEntitiesBySphere(origin, MAX_SEARCH_DISTANCE, ProcessTracedEntity, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		m_ProviderComponent.RequestBuildingMode(playerID, true);
	}
		
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{		
		if (GetUserRank(user) >= m_ProviderComponent.GetAccessRank())
			return true;
		
		FactionAffiliationComponent factionAffiliationComp = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComp)
			return false;
		
		//HOTFIX on stable because Revision: 79642 is not merged
		//string rankName;
		//SCR_Faction faction = SCR_Faction.Cast(factionAffiliationComp.GetAffiliatedFaction());
		//if (faction)
		//	rankName = faction.GetRankName(m_ProviderComponent.GetAccessRank());
			
		SetCannotPerformReason("Too low rank");
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterRank GetUserRank(notnull IEntity user)
	{		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return ECharacterRank.INVALID;
		
		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.GetInstance();
		if (!factionManager)
			return ECharacterRank.INVALID;
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return ECharacterRank.INVALID;
		
		return SCR_CharacterRankComponent.GetCharacterRank(playerController.GetControlledEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_ProviderComponent)
			InitializeSuppliesComponent();
				
		if (m_ProviderComponent.IsPlayerFactionSame(user))
			return true;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{	
		if (!m_SupplyComponent)
			m_SupplyComponent = m_ProviderComponent.GetSuppliesComponent();
		
		if (!m_SupplyComponent)
			return false;
		
		ActionNameParams[0] = string.ToString(m_SupplyComponent.GetSupplies());
		outName = ("#AR-Campaign_Action_ShowBuildPreview-UC");
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ProcessTracedEntity(IEntity ent)
	{
		SCR_CampaignBase base = SCR_CampaignBase.Cast(ent);
		if (!base)
			return true;
		
		m_ProviderComponent = SCR_CampaignBuildingProviderComponent.Cast(ent.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (m_ProviderComponent)
			return false;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
};
