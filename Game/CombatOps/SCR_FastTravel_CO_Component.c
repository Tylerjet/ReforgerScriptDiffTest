
class SCR_FastTravel_CO_Component : ScriptedUserAction
{
	ChimeraCharacter m_player; 
	SCR_SpawnPoint m_NewPos;
	//SCR_MapEditorComponent m_Map;
	SCR_GadgetManagerComponent gadgetComponent;
	//IEntity mapGadget;
	SCR_MapGadgetComponent mapGadgetComponent;
	ref array<SCR_SpawnPoint> a_SpawnPoints = {};
	Faction m_Faction;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		SCR_MapEntity.GetOnSelection().Insert(fnMapClick);
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_player = ChimeraCharacter.Cast(pUserEntity);
		if (!m_player)
			return; 
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(m_player);
		if (!gadgetManager)
		return;
		
		IEntity mapGadget = gadgetManager.GetGadgetByType(EGadgetType.MAP);
		if (!mapGadget)
		return;
		
		gadgetManager.SetGadgetMode(mapGadget, EGadgetMode.IN_HAND);
		//gadgetComponent = SCR_GadgetManagerComponent.Cast(m_player.FindComponent(SCR_GadgetManagerComponent));
		//mapGadget = gadgetComponent.GetGadgetByType(EGadgetType.MAP);
		//mapGadget = gadgetComponent.GetOwner();
		//gadgetComponent.SetGadgetMode(mapGadget, EGadgetMode.IN_HAND);
		//mapGadgetComponent = SCR_MapGadgetComponent.Cast(mapGadget.FindComponent(SCR_MapGadgetComponent));
		//mapGadgetComponent.SetMapMode(true);
		
		/*SCR_MapEditorComponent mapEditorComponent = SCR_MapEditorComponent.Cast(SCR_MapEditorComponent.GetInstance(SCR_MapEditorComponent));
		if (mapEditorComponent)
		{
			mapEditorComponent.ToggleMap();
		}*/
		SCR_FastTravelComponent.ToggleMapDestinationSelection(true);
		/*if (a_SpawnPoints.Count() < 2) 
		{
			SCR_PopUpNotification.GetInstance().PopupMsg("NO HIDEOUT DISCOVERED FOR  FAST TRAVEL",5,"",1);
		} else {
			SCR_PopUpNotification.GetInstance().PopupMsg("RANDOM HIDEOUT WAS SELECTED (CONCEPT)",5,"",1);
		}*/
		

		m_NewPos = a_SpawnPoints.GetRandomElement();
		vector positionOfSpawn = m_NewPos.GetOrigin();
		//m_player.SetOrigin(positionOfSpawn);
		
		/*int spawndistance = vector.Distance(m_player.GetOrigin(),positionOfSpawn);
		if (spawndistance < 200)
		{
			a_SpawnPoints.RemoveItem(m_NewPos);
			m_NewPos = a_SpawnPoints.GetRandomElement();
			positionOfSpawn = m_NewPos.GetOrigin();
		}
		m_player.SetOrigin(positionOfSpawn);*/
	}
	
	void fnMapClick(vector neco)
	{
		// close map
		//mapGadgetComponent.SetMapMode(false);
		//SCR_GadgetManagerComponent.GetGadgetManager(m_player).RemoveHeldGadget();
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		a_SpawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_Faction.GetFactionKey());
		return (a_SpawnPoints.Count() > 1);
	}
	
	/*override bool GetActionNameScript(out string outName)
	{
		outName = "HIDEOUTS";
		a_SpawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(m_Faction.GetFactionKey());
			return (a_SpawnPoints.Count() < 1);
	}*/
	
	override bool CanBeShownScript(IEntity user)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager && m_Faction)
			return false;
		m_Faction = Faction.Cast(factionManager.GetLocalPlayerFaction());
			return true;
	}
}
