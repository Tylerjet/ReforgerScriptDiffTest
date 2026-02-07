/*!
Action to spawn entities using the ActionsManagerComponent
*/
class SCR_CatalogSpawnerUserAction : ScriptedUserAction
{
	protected SCR_CatalogEntitySpawnerComponent m_EntitySpawner; 
	protected SCR_EEntityRequestStatus m_iRequestStatus;
	protected SCR_EntitySpawnerSlotComponent m_PreviewSlot;
	protected SCR_EntityCatalogEntry m_EntityData
	protected SCR_EntityCatalogSpawnerData m_EntitySpawnerData;
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnerData(SCR_EntityCatalogEntry entityData)
	{
		if (!entityData)
		{
			m_EntityData = null;
			m_EntitySpawnerData = null;
			return;
		}
		
		SCR_EntityCatalogSpawnerData spawnerData = SCR_EntityCatalogSpawnerData.Cast(entityData.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!spawnerData)
			return;
		
		m_EntityData = entityData;
		m_EntitySpawnerData = spawnerData;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	} 
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		IEntity parent = pOwnerEntity;
		SCR_CatalogEntitySpawnerComponent comp;
		while (parent)
		{
			comp = SCR_CatalogEntitySpawnerComponent.Cast(parent.FindComponent(SCR_CatalogEntitySpawnerComponent));
			if (comp)
			{
				m_EntitySpawner = comp;
				return;
			}
			
			parent = parent.GetParent();
		}
	}
	//------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		if (!m_EntitySpawner)
			return;
		
		if (IsAnySlotAvailable())
			m_EntitySpawner.CreatePreviewEntity(m_EntityData, m_PreviewSlot);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		m_EntitySpawner.DeletePreviewEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (!m_EntitySpawner)
			return;
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		SCR_SpawnerRequestComponent playerReqComponent = SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
		if (!playerReqComponent)
			return;
			
		SCR_EntitySpawnerSlotComponent usedPreviewSlot = m_PreviewSlot;
		playerReqComponent.RequestCatalogEntitySpawn(GetActionIndex(), m_EntitySpawner, pUserEntity, usedPreviewSlot);
		
		m_EntitySpawner.DeletePreviewEntity();
		m_EntitySpawner.ClearKnownSlots();
		m_EntitySpawner.AddKnownOccupiedSlot(usedPreviewSlot);
		
		if (IsAnySlotAvailable())
			m_EntitySpawner.CreatePreviewEntity(m_EntityData, m_PreviewSlot);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_EntitySpawner || !m_EntitySpawnerData)
			return false;
		
		ActionNameParams[0] = m_EntitySpawnerData.GetOverwriteName();
		if (ActionNameParams[0] == string.Empty)
			ActionNameParams[0] = m_EntityData.GetEntityName();
		
		if (m_EntitySpawner.IsSuppliesConsumptionEnabled())
		{
			ActionNameParams[1] = m_EntitySpawnerData.GetSupplyCost().ToString();
			ActionNameParams[2] = m_EntitySpawner.GetSpawnerSupplies().ToString();
		}

		outName = "#AR-EntitySpawner_Request";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPreviewSlot(SCR_EntitySpawnerSlotComponent previewSlot)
	{
		if (m_PreviewSlot == previewSlot)
			return;
		
		m_PreviewSlot = previewSlot;
		if (m_PreviewSlot)
			m_EntitySpawner.CreatePreviewEntity(m_EntityData, m_PreviewSlot);
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsAnySlotAvailable()
	{	
		if (!m_EntityData)
			return false;
		
		SCR_EntityCatalogSpawnerData data = SCR_EntityCatalogSpawnerData.Cast(m_EntityData.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		
		SetPreviewSlot(m_EntitySpawner.GetFreeSlot(data));
		
		return m_PreviewSlot != null;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_EntitySpawner)
			return false;
		
		switch (m_iRequestStatus)
		{
			case SCR_EEntityRequestStatus.REQUESTER_NOT_GROUPLEADER:
			{
				SetCannotPerformReason("#AR-EntitySpawner_RequestDenied_NotGroupLeader-UC");
				break;
			}
			
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
					rankName = faction.GetRankName(m_EntitySpawnerData.GetMinimumRequiredRank());
					
				SetCannotPerformReason(rankName);
				break;
			}
			
			case SCR_EEntityRequestStatus.COOLDOWN:
			{
				SetCannotPerformReason("#AR-Campaign_Action_Cooldown-UC");
				break;
			}
			
			case SCR_EEntityRequestStatus.CAN_SPAWN:
			{	
				if (IsAnySlotAvailable())
					return true;
					
				SetCannotPerformReason("#AR-Campaign_Action_BuildBlocked-UC");		
				break;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_EntitySpawner || !m_EntityData || !m_EntitySpawnerData)
			return false;
		
		SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(user);
		if (!chimeraCharacter || chimeraCharacter.GetFaction() != m_EntitySpawner.GetOwningFaction())
			return false;
		
		m_iRequestStatus = m_EntitySpawner.GetRequestState(m_EntityData, user);
		if (m_iRequestStatus != SCR_EEntityRequestStatus.NOT_AVAILABLE)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetActionIndex()
	{
		return (GetActionID() * 0.5) - 1;
	}
}