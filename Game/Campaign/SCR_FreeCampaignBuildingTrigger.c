class SCR_FreeCampaignBuildingTriggerClass : ScriptedGameTriggerEntityClass
{
};

//ToDo: Get a rid of "Free" world - now used because of name duplicity.
class SCR_FreeCampaignBuildingTrigger : ScriptedGameTriggerEntity
{
	protected vector m_vProviderOrigin;
	protected vector m_vNewProviderOrigin;
	protected IEntity m_Player;
	protected bool m_bProviderIsMoving;
	protected const float MOVING_CHECK_DISTANCE = 10;
	protected const int MOVING_CHECK_PERIOD = 1000;
	
	protected ref ScriptInvoker m_OnEntityEnter;
	protected ref ScriptInvoker m_OnEntityLeave;
	protected static ref ScriptInvoker s_OnProviderCreated = new ScriptInvoker();
		
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_CampaignBase base = SCR_CampaignBase.Cast(GetParent());
		if (!base)
			return;
			
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(base.FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return;
		
		factionComponent.GetOnFactionUpdate().Insert(OnBaseOwnerChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckproviderMove()
	{		
		m_vNewProviderOrigin = GetOrigin();
		if (m_vProviderOrigin == vector.Zero)
		{
			m_vProviderOrigin = m_vNewProviderOrigin;
			return;
		}
					
		if (vector.DistanceSqXZ(m_vNewProviderOrigin, m_vProviderOrigin) > MOVING_CHECK_DISTANCE)
		{
 			if (!m_bProviderIsMoving && !IsProviderBase())
			{
				RemoveEditMode(m_Player);
				m_bProviderIsMoving = true;
			}
		}	
		else
		{
			if (m_bProviderIsMoving)
				CreateEditMode(m_Player);
				
			m_bProviderIsMoving = false;
		}
			
		m_vProviderOrigin = m_vNewProviderOrigin;
	}
	
	//------------------------------------------------------------------------------------------------
	private bool IsProviderBase()
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return false;
		
		SCR_EditorModeEntity modEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modEntity)
			return false;
		
		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(modEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return false;
		
		return SCR_CampaignBase.Cast(buildingComponent.GetProviderEntity());
	}
		
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnProviderCreated()
	{
		return s_OnProviderCreated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Editor mode
	protected SCR_EditorManagerEntity GetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;
		
		return core.GetEditorManager();
	}
	
	//------------------------------------------------------------------------------------------------
	//! GetNetworkManager
	SCR_CampaignBuildingNetworkComponent GetNetworkManager()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;
		
		return SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove a provider locally
	protected void RemoveProviderEntity(SCR_EditorModeEntity modEntity)
	{
		if (modEntity.GetModeType() != EEditorMode.BUILDING)
			return;
		
		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(modEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return;
		
		buildingComponent.RemoveProviderEntityEditorComponent(GetParent());
	}
		
	//------------------------------------------------------------------------------------------------
	//! Entity enters the trigger
	override void OnActivate(IEntity ent)
	{		
		GetOnEntityEnterTrigger().Invoke(ent);	
		
		// the provider entity is not players faction
		if (!IsPlayerFactionSame(ent))
			return;
		
		if (SCR_PlayerController.GetLocalControlledEntity() != ent)
			return;

		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;
		
		m_Player = ent;
		GetGame().GetCallqueue().CallLater(CheckproviderMove, MOVING_CHECK_PERIOD, true);
		CreateEditMode(ent);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Entity leaves the trigger
	override void OnDeactivate(IEntity ent)
	{
		GetOnEntityLeaveTrigger().Invoke(ent);
		
		if (SCR_PlayerController.GetLocalControlledEntity() != ent)
			return;

		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;
		
		SCR_EditorModeEntity modEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modEntity)
			return;
		
		SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(modEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingComponent)
			return;

		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;
		
		m_Player = null;
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		
		// Periodical check to remove an access to a building mode when the provider entity is on the move.
		GetGame().GetCallqueue().CallLater(RemoveProvidersDelayed, 150, false, playerID, buildingComponent, networkComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	//!This removes the editor mode with delay. It's because the delay in triggers evaluation loop. The new provider could be added after the last one was remvoed.
	protected void RemoveProvidersDelayed(int playerID, notnull SCR_CampaignBuildingEditorComponent buildingComponent, notnull SCR_CampaignBuildingNetworkComponent networkComponent)
	{
		if (!buildingComponent && !networkComponent)
			return;
		
		buildingComponent.RemoveProviderEntityEditorComponent(GetParent());
 		networkComponent.RemoveProviderEditorMode(playerID, GetParent());
		GetGame().GetCallqueue().Remove(CheckproviderMove);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Send a request to server to create a editor mode.
	protected void CreateEditMode(IEntity ent)
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		 
		// Mode already exist, don't create new one, only add a new available provider
		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (modeEntity)
		{
			SCR_CampaignBuildingEditorComponent buildingComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
			if (!buildingComponent)
				return;
		
			buildingComponent.AddProviderEntityEditorComponent(GetParent());
			networkComponent.AddProviderEditorMode(playerID, GetParent());
			return;
		}	
		
		networkComponent.CreateEditorMode(playerID, this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove editor mode server request
	protected void RemoveEditMode(IEntity ent)
	{		
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		networkComponent.RemoveEditorMode(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Does player faction match the provider faction
	bool IsPlayerFactionSame(IEntity player)
	{
		Faction playerFaction = GetPlayerFaction(player);
		if (!playerFaction)
			return false;
		
		IEntity provider = SCR_EntityHelper.GetMainParent(this);
		if (!provider)
			return false;

		Faction owningFaction = GetProviderFaction(provider);
		if (!owningFaction)
			return false;
		
		return playerFaction == owningFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! GetPlayer faction
	protected Faction GetPlayerFaction(IEntity player)
	{		
		if (!player)
			return null;
		
		if (!ChimeraCharacter.Cast(player))
			return null;
		
		auto foundComponent = ChimeraCharacter.Cast(player).FindComponent(FactionAffiliationComponent);
		Faction faction;

		if (foundComponent)
		{
			auto castedComponent = FactionAffiliationComponent.Cast(foundComponent);
			faction = castedComponent.GetAffiliatedFaction();
		};

		return faction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns faction of provider
	protected Faction GetProviderFaction(IEntity ent)
	{
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (!factionComp)
			return null;
		
		Faction faction = factionComp.GetAffiliatedFaction();
		if (!faction)
			faction = factionComp.GetDefaultAffiliatedFaction();
		
		return faction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Method triggered when owning faction of provider has changed.
	protected void OnBaseOwnerChanged()
	{
		array<IEntity> outEntities = {};
		GetEntitiesInside(outEntities);
		
		if (outEntities.IsEmpty())
			return;
		
		IEntity playerEnt = SCR_PlayerController.GetLocalControlledEntity();
		foreach (IEntity ent: outEntities)
		{
			if (!ent || playerEnt != ent)
				continue;	
			
			if (IsPlayerFactionSame(ent))
				CreateEditMode(ent);
			else
				RemoveEditMode(ent);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnEntityEnterTrigger()
	{
		if (!m_OnEntityEnter)
			m_OnEntityEnter =  new ScriptInvoker();
		
		return m_OnEntityEnter;
	}
	
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnEntityLeaveTrigger()
	{
		if (!m_OnEntityLeave)
			m_OnEntityLeave =  new ScriptInvoker();
		
		return m_OnEntityLeave;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		s_OnProviderCreated.Invoke();
		return true;
	}
}
