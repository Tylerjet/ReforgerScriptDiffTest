//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingClientTriggerClass: SCR_CampaignBuildingTriggerClass
{
};

class SCR_CampaignBuildingClientTrigger : SCR_CampaignBuildingTrigger
{
	protected SCR_CampaignBuildingControllerComponent m_BuildingController;
	protected ref array<IEntity> m_aInside = {}; 
	protected bool m_bWaitingToBeBuilt = false;
	IEntity m_BuildingBlocker;
	
	//------------------------------------------------------------------------------------------------
	void SetBuildingController(notnull SCR_CampaignBuildingControllerComponent controller)
	{
		m_BuildingController = controller;
		m_BuildingController.SetCompositionColor();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{		
		QueryEntitiesInside();		
		GetEntitiesInside(m_aInside);
		
		for (int i = m_aInside.Count() - 1; i >= 0; i--)
		{			
			if (IsEntityBlocker(m_aInside[i]))
			{
				SetToBeBuilt(true);
				m_BuildingBlocker = m_aInside[i];
			}
				
			if (CanBlockPreview(m_aInside[i]))
				continue;
			
			m_aInside.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{		
		if (!m_BuildingController || !m_BuildingController.GetUsedData() || m_BuildingController.GetUsedSlot().IsOccupied())
			return;
		
		if (!m_BuildingBlocker)
			SetToBeBuilt(false);
						
		if (m_BuildingController.CanBeRotated() && !m_BuildingBlocker)
			m_BuildingController.RotatePreview();
				
		m_BuildingController.SetCompositionColor();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsBlocked()
	{
		if (!m_BuildingController)	
			return true;
		
		return !m_aInside.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity ent)
	{			
		if (!m_BuildingController)
			return;
		
		if (ent && ent == SCR_PlayerController.GetLocalControlledEntity())
		{
			m_BuildingController.ActivateController();
			m_BuildingController.ActivateActionListeners();
			m_aInside.Insert(ent);
			super.OnActivate(ent);
			return;
		}
		
		if ((IsEntityBlocker(ent)))
		{
			SetToBeBuilt(true);
			m_BuildingBlocker = ent;
			super.OnActivate(ent);
		}
			
		if (CanBlockPreview(ent))	
			m_aInside.Insert(ent);
		
		m_BuildingController.SetCompositionColor();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeactivate(IEntity ent)
	{		
		if (!m_BuildingController)
			return;
		
		if (IsEntityBlocker(ent))
		{
			SetToBeBuilt(false);
			m_BuildingBlocker = ent;
		}
			
		if (ent && ent == SCR_PlayerController.GetLocalControlledEntity())
		{
			m_BuildingController.DeactivateController();
			m_BuildingController.DeactivateActionListeners();			
		}
		else
		{
			m_aInside.RemoveItem(ent);
		}
		
		m_BuildingController.SetCompositionColor();
		
		// Remove invoker
		SCR_CharacterControllerComponent charControllerComp = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
		if (charControllerComp)
			charControllerComp.m_OnPlayerDeathWithParam.Remove(OnDeath);
		
		if (m_aInside.IsEmpty())
			ClearEventMask(EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBlockPreview(notnull IEntity element)
	{	
		// It's a character but it's dead.
		ChimeraCharacter char = ChimeraCharacter.Cast(element);
		if (char)
		{
			SCR_CharacterControllerComponent charControllerComp = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
			if (charControllerComp && charControllerComp.IsDead())
				return false;
			
			// If the character is alive, add scripted invoker to remove dead soldier from the list of entities inside of the trigger
			if (charControllerComp)
			{
				charControllerComp.m_OnPlayerDeathWithParam.Insert(OnDeath);
				return true;
			}
		}
		
		if (IsEntityBlocker(element))
			return true;

		// Check for vehicles and other elements
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
		
		if (m_BuildingController)
			m_BuildingController.SetCompositionColor();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEntityBlocker(notnull IEntity ent)
	{
		return SCR_BuildingAreaBlocker.Cast(ent) != null;
	}
	//------------------------------------------------------------------------------------------------
	void SetToBeBuilt(bool val)
	{
		m_bWaitingToBeBuilt = val;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsToBeBuilt()
	{
		return m_bWaitingToBeBuilt;
	}
};
