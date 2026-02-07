[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical12Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical12: SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));

		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FirstAid_CheckChest");
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_PlayerInventory)
			return false;
		
		IEntity ent = m_PlayerInventory.GetCurrentItem();
		if (!ent)
			return false;
		
		SCR_ConsumableItemComponent consumItem = SCR_ConsumableItemComponent.Cast(ent.FindComponent(SCR_ConsumableItemComponent));
		if (!consumItem)
			return false;
		
		SCR_CharacterDamageManagerComponent damManager = m_TutorialComponent.GetVictimDamageManager();
		if (!damManager)
			return false;
		
		return consumItem.GetConsumableType() == SCR_EConsumableType.BANDAGE || !damManager.GetGroupDamageOverTime(ECharacterHitZoneGroup.UPPERTORSO, EDamageType.BLEEDING);
	}
};