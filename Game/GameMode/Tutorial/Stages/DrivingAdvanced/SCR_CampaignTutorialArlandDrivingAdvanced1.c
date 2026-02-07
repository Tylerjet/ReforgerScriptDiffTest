[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandDrivingAdvanced1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandDrivingAdvanced1 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 10;
		m_bConditionPassCheck = true;
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_TutorialComponent.ResetStage_VehiclesHeavy();
		
		Vehicle repairTruck = m_TutorialComponent.GetRepairTruck();
		

		GetGame().GetCallqueue().CallLater(PlaySoundSystem, 2000, false, "Start", false);
		
		if (!repairTruck)
			return;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(repairTruck.FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return;
		
		EntitySlotInfo slotInfo = slotManager.GetSlotByName("RepairBox");
		if (!slotInfo)
			return;
		
		IEntity cargo = slotInfo.GetAttachedEntity();
		if (!cargo)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(cargo);
		
		if (!resourceComponent)
			return;
		
		SCR_ResourceGenerator resourceGenerator = resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, EResourceType.SUPPLIES);

		if (!resourceGenerator)
			return;

		resourceGenerator.RequestGeneration(350);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
};