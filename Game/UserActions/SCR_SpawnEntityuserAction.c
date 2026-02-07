/*!
Action to spawn entities using the ActionsManagerComponent
*/
class SCR_SpawnEntityUserAction : ScriptedUserAction
{
	protected SCR_EntitySpawnerComponent m_EntitySpawner; 
	protected int m_iCanRequestResult = SCR_EEntityRequestStatus.CAN_SPAWN;
	
	static const int INIT_DELAY = 1000;
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit(IEntity parent)
	{
		SCR_EntitySpawnerComponent comp;
		while (parent)
		{
			comp = SCR_EntitySpawnerComponent.Cast(parent.FindComponent(SCR_EntitySpawnerComponent));
			if (comp)
			{
				m_EntitySpawner = comp;
				return;
			}
			
			parent = parent.GetParent();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		GetGame().GetCallqueue().CallLater(DelayedInit, 1000, false, pOwnerEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (!m_EntitySpawner)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		SCR_PlayerEntitySpawnerRequestComponent playerReqComponent = SCR_PlayerEntitySpawnerRequestComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId).FindComponent(SCR_PlayerEntitySpawnerRequestComponent));
		if (!playerReqComponent)
			return;
			
		playerReqComponent.RequestEntitySpawn(GetActionIndex(), m_EntitySpawner, pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_EntitySpawner)
			return false;
		
		SCR_EntityInfo entityInfo = m_EntitySpawner.GetEntryAtIndex(GetActionIndex());
		if (!entityInfo)
			return false;
		
		ActionNameParams[0] = entityInfo.GetDisplayNameUC();
		ActionNameParams[1] = entityInfo.GetCost().ToString();
		ActionNameParams[2] = m_EntitySpawner.GetSpawnerSupplies().ToString();

		outName = "#AR-EntitySpawner_Request";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_EntitySpawner)
			return false;
		
		SCR_EntityInfo entityInfo = m_EntitySpawner.GetEntryAtIndex(GetActionIndex());
		
		switch (m_iCanRequestResult)
		{
			case SCR_EEntityRequestStatus.NOT_ENOUGH_SUPPLIES:
			{
				SetCannotPerformReason("");
				break;
			}
				
			case SCR_EEntityRequestStatus.RANK_LOW:
			{
				string rankName;
				FactionAffiliationComponent factionAffiliationComp = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
				if (!factionAffiliationComp)
					break;
				
				SCR_Faction faction = SCR_Faction.Cast(factionAffiliationComp.GetAffiliatedFaction());
				if (faction)
					rankName = faction.GetRankName(entityInfo.GetMinimumRankID());
					
				SetCannotPerformReason(rankName);
				break;
			}
			
			case SCR_EEntityRequestStatus.COOLDOWN:
			{
				SetCannotPerformReason("#AR-Campaign_Action_Cooldown-UC");
				break;
			}
			
			case SCR_EEntityRequestStatus.CAN_SPAWN:
			case SCR_EEntityRequestStatus.CAN_SPAWN_TRIGGER:
			{
				return true;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_EntitySpawner)
			return false;
		
		SCR_EntityInfo entityInfo = m_EntitySpawner.GetEntryAtIndex(GetActionIndex());
		if (!entityInfo)
			return false;
		
		//nothing will be shown, if entity is not allowed at this spawner
		if (!m_EntitySpawner.GetIsEntityAllowed(entityInfo))
			return false;
			
		m_iCanRequestResult = m_EntitySpawner.CanSpawn(GetActionIndex(), user);
			
		if (m_iCanRequestResult != SCR_EEntityRequestStatus.NOT_AVAILABLE)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetActionIndex()
	{
		return (GetActionID() / 2) - 1;
	}
	
}