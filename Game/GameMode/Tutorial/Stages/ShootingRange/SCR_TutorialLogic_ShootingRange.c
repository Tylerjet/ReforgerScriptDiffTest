[BaseContainerProps()]
class SCR_TutorialLogic_ShootingRange : SCR_BaseTutorialCourseLogic
{
	protected ResourceName m_sM16MagResource = "{D8F2CA92583B23D3}Prefabs/Weapons/Magazines/Magazine_556x45_STANAG_30rnd_M855_M856_Last_5Tracer.et";
	protected ResourceName m_sM249MagResource = "{4FCBBDF274FD2157}Prefabs/Weapons/Magazines/Box_556x45_M249_200rnd_Ball.et";
	
	//------------------------------------------------------------------------------------------------
	void OnInputChanged(bool gamepad)
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		if (gamepad)
		{
			if (tutorial.GetStageIndexByName("GAMEPAD") == -1)
				tutorial.InsertStage("GAMEPAD", tutorial.GetActiveStageIndex());
			
			tutorial.SetStage("GAMEPAD");
			return;
		}
		
		if (tutorial.GetStageIndexByName("KMB") == -1)
			tutorial.InsertStage("KMB", tutorial.GetActiveStageIndex());
		
		tutorial.SetStage("KMB");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		tutorial.SetupTargets("TARGETS_BACK", null, ETargetState.TARGET_DOWN, false);
		tutorial.SetupTargets("TARGETS_ECHELON_LEFT", null, ETargetState.TARGET_DOWN, false);
		tutorial.SetupTargets("TARGETS_ECHELON_RIGHT", null, ETargetState.TARGET_DOWN, false);
		tutorial.SetupTargets("TARGETS_FRONT", null, ETargetState.TARGET_DOWN, false);
		tutorial.SetupTargets("TARGETS_MIDDLE", null, ETargetState.TARGET_DOWN, false);
		tutorial.SetupTargets("TARGETS_WRECK_CAR", null, ETargetState.TARGET_DOWN, false);
		tutorial.SetupTargets("TARGETS_WRECK_TRUCK", null, ETargetState.TARGET_DOWN, false);
		
		RestockAmmoBox();
		GetGame().GetCallqueue().CallLater(RestockAmmoBox, 10000, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void RestockAmmoBox()
	{
		IEntity ammobox = GetGame().GetWorld().FindEntityByName("Ammobox");
		if (!ammobox)
			return;
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(ammobox.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageManComp)
			return;
		
		bool hasM249Ammo, hasM16Ammo;
		
		array <IEntity> items = {};
		storageManComp.GetAllRootItems(items);
		
		if (items && !items.IsEmpty())
		{
			foreach (IEntity item : items)
			{
				if (!item)
					continue;
				
				if (!hasM16Ammo && item.GetPrefabData().GetPrefabName() == m_sM16MagResource)
					hasM16Ammo = true;
				
				if (!hasM249Ammo && item.GetPrefabData().GetPrefabName() == m_sM249MagResource)
					hasM249Ammo = true;
			}
		}
		
		if (!hasM16Ammo)
			storageManComp.InsertItem(GetGame().SpawnEntityPrefab(Resource.Load(m_sM16MagResource)));
		
		if (!hasM249Ammo)
			storageManComp.InsertItem(GetGame().SpawnEntityPrefab(Resource.Load(m_sM249MagResource)));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputChanged);
		GetGame().GetCallqueue().Remove(RestockAmmoBox);
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		tutorial.EnableArsenal("Ammobox", false);
	}
}