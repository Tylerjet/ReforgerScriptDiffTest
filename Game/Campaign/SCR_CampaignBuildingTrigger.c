//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingTriggerClass: ScriptedGameTriggerEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingTrigger : ScriptedGameTriggerEntity
{
	protected SCR_CampaignBuildingControllerComponent m_BuildingController;
	protected ref array<IEntity> m_aInside = {};
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		IEntity parent = owner.GetParent();
		if (!parent)
			return;
		
		m_BuildingController = SCR_CampaignBuildingControllerComponent.Cast(parent.FindComponent(SCR_CampaignBuildingControllerComponent));
		if (!m_BuildingController)
			return;
		
		QueryEntitiesInside();
		GetEntitiesInside(m_aInside);
		
		if (!m_aInside.IsEmpty())
			{
				for (int i = m_aInside.Count() - 1; i >= 0; i--)
				{
					if (CanBlockPreview(m_aInside[i]))
						continue;
					
					m_aInside.Remove(i);
				}
			}
		
		m_BuildingController.SetCompositionColor();
	}
		
	//------------------------------------------------------------------------------------------------
	bool CanBlockPreview(IEntity element)
	{
		// It's a character but it's dead.
		SCR_CharacterControllerComponent charControllerComp = SCR_CharacterControllerComponent.Cast(element.FindComponent(SCR_CharacterControllerComponent));
		if (charControllerComp && charControllerComp.IsDead())
			return false;
		
		// If the character is alive, add scripted invoker to remove dead soldier from the list of entities inside of the trigger
		if (charControllerComp)
		{
			charControllerComp.m_OnPlayerDeathWithParam.Insert(OnDeath);
			return true;
		}
	   		
		 Vehicle veh = Vehicle.Cast(element);
		if (veh)	
			return true;
		
		// If it's something else like a gear, weapon etc, remove it from array of blocking elements.
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEntity(int index)
	{
		m_aInside.Remove(index);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_BuildingController || !m_BuildingController.GetUsedData() || m_BuildingController.GetUsedSlot().IsOccupied())
			return;
						
		if (m_BuildingController.CanBeRotated())
			m_BuildingController.RotatePreview();
		
		for (int i = m_aInside.Count() - 1; i >= 0; i--)
		{
			if (m_aInside[i] == null)
				RemoveEntity(i);
		}
		
		m_BuildingController.SetCompositionColor();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity ent)
	{
		if (!m_BuildingController)
			return;
		
		if (IsEntityPlayer(ent))
		{
			m_BuildingController.ActivateController();
			m_BuildingController.ActivateActionListeners();
			// Start ticking in init
			SetEventMask(EntityEvent.FRAME);
			return;
		}
		
		if (CanBlockPreview(ent))
		{
			m_aInside.Insert(ent);
			m_BuildingController.SetCompositionColor();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeactivate(IEntity ent)
	{
		if (!m_BuildingController)
			return;

		// Remove invoker
		SCR_CharacterControllerComponent charControllerComp = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
		if (charControllerComp)
	    	charControllerComp.m_OnPlayerDeathWithParam.Remove(OnDeath);
			
		if (IsEntityPlayer(ent))
		{
			m_BuildingController.DeactivateController();
			m_BuildingController.DeactivateActionListeners();			
		}
		else
		{
			m_aInside.RemoveItem(ent);
			m_BuildingController.SetCompositionColor();
		}
					
		// Stop ticking init
		if (m_aInside.IsEmpty())
		{
			ClearEventMask(EntityEvent.FRAME);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsBlocked()
	{
		return (!m_aInside.IsEmpty() || !m_BuildingController);
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignBuildingTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	private bool IsEntityPlayer(IEntity ent)
	{
		IEntity playerEnt = SCR_PlayerController.GetLocalControlledEntity();
		if (!playerEnt || playerEnt != ent)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnDeath(SCR_CharacterControllerComponent charControllerComp, IEntity instigator)
	{
		if (!charControllerComp)
			return;
		
		IEntity ent = charControllerComp.GetCharacter();
		if (!ent)
			return;
		
		m_aInside.RemoveItem(ent);
		m_BuildingController.SetCompositionColor();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBuildingTrigger()
	{
		m_aInside = null;
	}
};
