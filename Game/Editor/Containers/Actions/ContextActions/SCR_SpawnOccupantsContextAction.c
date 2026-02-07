//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SpawnOccupantsContextAction : SCR_SelectedEntitiesContextAction
{	
	[Attribute(desc: "Compartments Types To fill with characters within vehicle.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECompartmentType))]
	protected ref array<ECompartmentType> m_aCompartmentsTypes;
	
	//~ Store vehicles to spawn occupants in
	protected ref array<SCR_EditableVehicleComponent> m_aVehiclesToOccupyQueue = {};
	protected bool m_bHasSendNotEnoughBudgetNotification= false;
	protected bool m_bIsSpawningOccupants = false;
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{		
		//~ Disabled until the system is reworked. HasEnoughBudgetForDefaultOccupants should be called here or at least in the budget component and the player Id needs to be send over to get the contentbrowser!
		return false;
		
		SCR_EditableVehicleUIInfo uiInfo = SCR_EditableVehicleUIInfo.Cast(selectedEntity.GetInfo());
		if (uiInfo && !uiInfo.CanFillWithGivenTypes(m_aCompartmentsTypes))
			return false;
		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));

		return compartmentManager && compartmentManager.HasDefaultOccupantsDataForTypes(m_aCompartmentsTypes);
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		SCR_EditableVehicleComponent vehicle = SCR_EditableVehicleComponent.Cast(selectedEntity);
		return vehicle && vehicle.CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, false, false);
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{		
		SCR_EditableVehicleComponent vehicle = SCR_EditableVehicleComponent.Cast(selectedEntity);
		
		//~ Spawn occupants in vehicle and check when spawning was completed
		if (!m_bIsSpawningOccupants)
		{		
			//~ Check if spawn still valid
			if (vehicle.CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, false, false))
			{
				//~ Check if enough budget
				if (vehicle.CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, false, true, false, false))
				{
					m_bIsSpawningOccupants = true;
					
					SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
					compartmentManager.GetOnDoneSpawningDefaultOccupants().Insert(DoneSpawningOccupantsInEntity);
					vehicle.OccupyVehicleWithDefaultCharacters(m_aCompartmentsTypes);
				}
				else 
				{
					if (!m_bHasSendNotEnoughBudgetNotification)
					{
						//~ Todo: Send notification to client if max budget is reached
						m_bHasSendNotEnoughBudgetNotification = true;
						//SCR_NotificationsComponent.SendToPlayer(m_ActionsManager.GetManager().GetPlayerID(), ENotification.EDITOR_PLACING_BUDGET_MAX_FOR_VEHICLE_OCCUPANTS);
						//SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_BUDGET_MAX_FOR_VEHICLE_OCCUPANTS);
					}
				}
			}
			
			//~ Executes CheckDone after one frame to check if all selected vehicles are done spawning all characters.
			if (!m_bIsSpawningOccupants)
				GetGame().GetCallqueue().CallLater(DelayedCheckDone);
		}
		//~ Already occupying another vehicle so add it to the queue
		else 
		{
			m_aVehiclesToOccupyQueue.Insert(vehicle);
		}		
	}
	
	//~ When done spawning characters in entity
	protected void DoneSpawningOccupantsInEntity(SCR_BaseCompartmentManagerComponent compartmentManager, array<IEntity> spawnedCharacters, bool wasCanceled)
	{
		if (compartmentManager)
			compartmentManager.GetOnDoneSpawningDefaultOccupants().Remove(DoneSpawningOccupantsInEntity);
		
		if (m_aVehiclesToOccupyQueue.IsEmpty())
		{
			m_bIsSpawningOccupants = false;
			m_bHasSendNotEnoughBudgetNotification = false;
			return;
		}
		
		SCR_EditableVehicleComponent vehicleToOccupy;
		
		for(int i = 0; i < m_aVehiclesToOccupyQueue.Count(); i++)
        {
			//~ Invalid compartment (deleted) or can no longer fill the vehicle so remove it from the list
           	if (!m_aVehiclesToOccupyQueue[i] || !m_aVehiclesToOccupyQueue[i].CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, false, false))
			{
				m_aVehiclesToOccupyQueue.RemoveOrdered(i);
				i--;
				continue;
			}
			
			//~ Check if spawn is still valid
			if (!m_aVehiclesToOccupyQueue[i].CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, false, false, true, true))
			{
				m_aVehiclesToOccupyQueue.RemoveOrdered(i);
				i--;
				
				continue;
			}
			
			//~ No longer has enough budget to fill vehicle
			if (!m_aVehiclesToOccupyQueue[i].CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, false, true, false, false))
			{
				m_aVehiclesToOccupyQueue.RemoveOrdered(i);
				i--;
				
				//~ Send not enough budget notification once
				if (!m_bHasSendNotEnoughBudgetNotification)
				{
					m_bHasSendNotEnoughBudgetNotification = true;
					
					
					
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_BUDGET_MAX_FOR_VEHICLE_OCCUPANTS);
				}
				
				continue;
			}
			
			//~ Found a new vehicle to fill
			vehicleToOccupy = m_aVehiclesToOccupyQueue[i];
			m_aVehiclesToOccupyQueue.RemoveOrdered(i);
			break;
        }
		
		//~ No longer found a valid vehicle to fill
		if (!vehicleToOccupy)
		{
			m_bIsSpawningOccupants = false;
			m_bHasSendNotEnoughBudgetNotification = false;
			m_aVehiclesToOccupyQueue.Clear();
			return;
		}
		
		//~ Fill next vehicle in queue
		compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicleToOccupy.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		compartmentManager.GetOnDoneSpawningDefaultOccupants().Insert(DoneSpawningOccupantsInEntity);
		vehicleToOccupy.OccupyVehicleWithDefaultCharacters(m_aCompartmentsTypes);
	}
	
	protected void DelayedCheckDone()
	{
		if (!m_aVehiclesToOccupyQueue.IsEmpty())
			return;

		m_bIsSpawningOccupants = false;
		m_bHasSendNotEnoughBudgetNotification = false;
	}
};