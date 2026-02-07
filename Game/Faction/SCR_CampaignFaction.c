//------------------------------------------------------------------------------------------------
class SCR_CampaignFaction : SCR_MilitaryFaction
{
	[Attribute("", UIWidgets.ResourceNamePicker, "AI group prefab", "et")]
	private ResourceName m_AIGroupPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "AI group vehicle prefab", "et")]
	private ResourceName m_AIVehiclePrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Defenders group prefab", "et")]
	private ResourceName m_DefendersGroupPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Default transport vehicle", "et")]
	private ResourceName m_DefaultTransportPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	private ResourceName m_MobileHQPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "For radio operators", "et")]
	private ResourceName m_RadioPrefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "HQ composition in small bases", "et")]
	private ResourceName m_BaseBuildingHQ;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "HQ composition in building. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingHQNoSlot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition (empty)", "et")]
	private ResourceName m_BaseBuildingStash0;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition in building. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingStash0NoSlot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition (30% capacity)", "et")]
	private ResourceName m_BaseBuildingStash1;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition (30% capacity) in building. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingStash1NoSlot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition (60% capacity)", "et")]
	private ResourceName m_BaseBuildingStash2;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition (60% capacity) in building. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingStash2NoSlot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition (full)", "et")]
	private ResourceName m_BaseBuildingStash3;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Supply stash composition (full) in building. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingStash3NoSlot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Fuel depot composition (empty)", "et")]
	private ResourceName m_BaseBuildingFuel0;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Fuel depot composition (empty) in building. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingFuel0NoSlot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Fuel depot composition (30% capacity)", "et")]
	private ResourceName m_BaseBuildingFuel1;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Fuel depot composition (60% capacity)", "et")]
	private ResourceName m_BaseBuildingFuel2;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Fuel depot composition (full)", "et")]
	private ResourceName m_BaseBuildingFuel3;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Armory composition", "et")]
	private ResourceName m_BaseBuildingArmory;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Armory composition in building. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingArmoryNoSlot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Repair depot composition", "et")]
	private ResourceName m_BaseBuildingRepair;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Repair depot composition in building. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingRepairNoSlot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Military hospital composition", "et")]
	private ResourceName m_BaseBuildingHospital;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Barracks composition", "et")]
	private ResourceName m_BaseBuildingBarracks;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Barracks composition. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingBarracksNoSlot;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Radio antenna composition", "et")]
	private ResourceName m_BaseBuildingRadioAntenna;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Radio antenna composition. NO SLOT variant.", "et")]
	private ResourceName m_BaseBuildingRadioAntennaNoSlot;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Faction flag material, used on flag poles.", params: "emat")]
	private ResourceName m_FactionFlagMaterial;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Faction flag sign material, used on flag signs.", params: "emat")]
	private ResourceName m_FactionSignMaterial;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Faction flag sign material MLOD, used on flag signs.", params: "emat")]
	private ResourceName m_FactionSignMaterialMLOD;
	
	[Attribute()]
	ref array<ref SCR_CampaignSlotComposition> m_aCampaignSlotCompositions;

	[Attribute("1", UIWidgets.CheckBox, "Allow AI units")]
	protected bool m_bSpawnAIs;
	
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_CampaignRemnantsGroup> m_aRemnantGroups;
	
	protected ref array<ref ResourceName> m_aAvailableSlotResources = new array<ref ResourceName>();
	protected SCR_CampaignBase m_MainBase = null;
	protected ref array<int> m_aAvailableCallsigns = new array<int>();
	
	//------------------------------------------------------------------------------------------------
	void SendHQMessage(SCR_ERadioMsg msgType, int baseCallsign = SCR_CampaignBase.INVALID_BASE_INDEX, int calledID = SCR_CampaignBase.INVALID_PLAYER_INDEX, bool public = true, int param = SCR_CampaignRadioMsg.INVALID_RADIO_MSG_PARAM)
	{
		if (msgType == SCR_ERadioMsg.NONE)
			return;
		
		SCR_CampaignBase HQ = GetMainBase();
		
		if (!HQ)
			return;
		
		BaseRadioComponent radio = BaseRadioComponent.Cast(HQ.FindComponent(BaseRadioComponent));
		
		if (!radio)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));
		
		if (!callsignManager)
			return;
		
		IEntity called = GetGame().GetPlayerManager().GetPlayerControlledEntity(calledID);
		int companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex;
		
		if (called && !callsignManager.GetEntityCallsignIndexes(called, companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex, characterCallsignIndex))
	    	return;
		
		SCR_CampaignRadioMsg msg = new SCR_CampaignRadioMsg;
		msg.SetRadioMsg(msgType);
		msg.SetBaseCallsign(baseCallsign);
		msg.SetCalledCallsign(companyCallsignIndex, platoonCallsignIndex, squadCallsignIndex);
		msg.SetIsPublic(public);
		msg.SetParam(param);
		msg.SetPlayerID(calledID);
		
		radio.Transmit(msg);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetDeployedMobileAssembly()
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return null;
		
		SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(campaign.GetDeployedMobileAssemblyID(GetFactionKey())));
		
		if (!comp)
			return null;
		
		return comp.GetOwner();
	}
		
	//------------------------------------------------------------------------------------------------
	void SetAvailableSlotResources()
	{		
		for (int i = 0, count = m_aCampaignSlotCompositions.Count(); i < count; i++)
		{
			ResourceName res = m_aCampaignSlotCompositions[i].GetResourceName();		
			m_aAvailableSlotResources.Insert(res);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Get the list of all compositions that can be built on certain type of the slot
	array<ref SCR_CampaignSlotComposition> GetSlotResource(SCR_ESlotTypesEnum slotType)
	{	
		ref array<ref SCR_CampaignSlotComposition> availableComposition = new ref array<ref SCR_CampaignSlotComposition>();
		
		// Go through all slot compositions and check if the type of slot match.
		for (int i = 0, count = m_aCampaignSlotCompositions.Count(); i < count; i++)
		{
			// If so, add the composition into the available compositions array
			if (m_aCampaignSlotCompositions[i].GetSlotType() == slotType)
				availableComposition.Insert(m_aCampaignSlotCompositions[i]);
		}
		
		return availableComposition;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref ResourceName> GetAvailableSlotResources()
	{
		return m_aAvailableSlotResources;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_CampaignSlotComposition> GetCampaignSlotsComposition()
	{	
		return m_aCampaignSlotCompositions;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMainBase(SCR_CampaignBase mainBase)
	{
		m_MainBase = mainBase;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetAIsAllowed()
	{
		return m_bSpawnAIs;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRadioPrefab()
	{
		return m_RadioPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetAIGroupPrefab()
	{
		return m_AIGroupPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetAIGroupVehiclePrefab()
	{
		return m_AIVehiclePrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefendersGroupPrefab()
	{
		return m_DefendersGroupPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultTransportPrefab()
	{
		return m_DefaultTransportPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetMobileHQPrefab()
	{
		return m_MobileHQPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetBuildingPrefab(ECampaignCompositionType type, CampaignBaseType baseType = CampaignBaseType.SMALL)
	{
		switch (type)
		{
			case ECampaignCompositionType.HQ: {return m_BaseBuildingHQ;};
			case ECampaignCompositionType.SUPPLIES_EMPTY: {return m_BaseBuildingStash0;};
			case ECampaignCompositionType.SUPPLIES_LOW: {return m_BaseBuildingStash1;};
			case ECampaignCompositionType.SUPPLIES_HIGH: {return m_BaseBuildingStash2;};
			case ECampaignCompositionType.SUPPLIES_FULL: {return m_BaseBuildingStash3;};
			case ECampaignCompositionType.FUEL: {return m_BaseBuildingFuel0;};
			case ECampaignCompositionType.FUEL_EMPTY: {return m_BaseBuildingFuel0;};
			case ECampaignCompositionType.FUEL_LOW: {return m_BaseBuildingFuel1;};
			case ECampaignCompositionType.FUEL_HIGH: {return m_BaseBuildingFuel2;};
			case ECampaignCompositionType.FUEL_FULL: {return m_BaseBuildingFuel3;};
			case ECampaignCompositionType.REPAIR: {return m_BaseBuildingRepair;};
			case ECampaignCompositionType.ARMORY: {return m_BaseBuildingArmory;};
			case ECampaignCompositionType.HOSPITAL: {return m_BaseBuildingHospital;};
			case ECampaignCompositionType.BARRACKS: {return m_BaseBuildingBarracks;};
			case ECampaignCompositionType.RADIO_ANTENNA: {return m_BaseBuildingRadioAntenna;};
		}
		
		return ResourceName.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetBuildingPrefabNoSlot(ECampaignCompositionType type, CampaignBaseType baseType = CampaignBaseType.SMALL)
	{
		switch (type)
		{
			case ECampaignCompositionType.HQ: {return m_BaseBuildingHQNoSlot;};
			case ECampaignCompositionType.SUPPLIES_EMPTY: {return m_BaseBuildingStash0NoSlot;};
			case ECampaignCompositionType.SUPPLIES_LOW: {return m_BaseBuildingStash1NoSlot;};
			case ECampaignCompositionType.SUPPLIES_HIGH: {return m_BaseBuildingStash2NoSlot;};
			case ECampaignCompositionType.SUPPLIES_FULL: {return m_BaseBuildingStash3NoSlot;};
			case ECampaignCompositionType.FUEL: {return m_BaseBuildingFuel0NoSlot;};
			case ECampaignCompositionType.FUEL_LOW: {return m_BaseBuildingFuel0NoSlot;};
			case ECampaignCompositionType.FUEL_HIGH: {return m_BaseBuildingFuel0NoSlot;};			
			case ECampaignCompositionType.FUEL_FULL: {return m_BaseBuildingFuel0NoSlot;};
			case ECampaignCompositionType.FUEL: {return m_BaseBuildingFuel0NoSlot;};
			case ECampaignCompositionType.FUEL_EMPTY: {return m_BaseBuildingRepairNoSlot;};
			case ECampaignCompositionType.REPAIR: {return m_BaseBuildingRepairNoSlot;};
			case ECampaignCompositionType.ARMORY: {return m_BaseBuildingArmoryNoSlot;};
			case ECampaignCompositionType.BARRACKS: {return m_BaseBuildingBarracksNoSlot;};
			case ECampaignCompositionType.RADIO_ANTENNA: {return m_BaseBuildingRadioAntennaNoSlot;};
		}
		
		return ResourceName.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFactionNameUpperCase()
	{
		SCR_FactionUIInfo UI = SCR_FactionUIInfo.Cast(GetUIInfo());
		
		if (UI)
			return UI.GetFactionNameUpperCase();
		else
			return "";
	}
	
	//------------------------------------------------------------------------------------------------
	//Called everywhere, used to generate initial data for this faction
	override void InitializeFaction()
	{
		if (SCR_GameModeCampaignMP.NotPlaying())
			return;
		
		// initialize all available slots resources for faction
		SetAvailableSlotResources();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBase GetMainBase()
	{
		if (m_MainBase)
			return m_MainBase;
		
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		
		array<SCR_CampaignBase> bases = new array<SCR_CampaignBase>();
		baseManager.GetFilteredBases(CampaignBaseType.MAIN, bases);
		
		for (int i = 0; i < bases.Count(); i++)
		{
			if (bases[i].GetOwningFaction() == this)
			{
				m_MainBase = bases[i];
				break;
			}
		}
		
		return m_MainBase;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRemnantsByProbability(float prob, notnull out array<ResourceName> groups)
	{
		float lowestProb = float.MAX;
		
		foreach (SCR_CampaignRemnantsGroup group: m_aRemnantGroups)
		{
			float grpProb = group.m_fProbability;
			
			if (grpProb > prob)
			{	
				if (grpProb <= lowestProb)
				{
					if (grpProb < lowestProb)
					{
						groups.Clear();
						lowestProb = grpProb;
					}
					
					groups.Insert(group.m_Prefab);
				}
			}
		}
		
		return groups.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRemnantsByType(SCR_CampaignRemnantsGroupType type)
	{
		foreach (SCR_CampaignRemnantsGroup group: m_aRemnantGroups)
		{
			if (group.m_eType == type)
				return group.m_Prefab;
		}
		
		return ResourceName.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetFactionFlagResource()
	{
		return m_FactionFlagMaterial;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetFactionSignResource()
	{
		return m_FactionSignMaterial;
	}
	//------------------------------------------------------------------------------------------------
	ResourceName GetFactionSignMLOD()
	{
		return m_FactionSignMaterialMLOD;
	}
};

[BaseContainerProps()]
class SCR_CampaignSlotComposition
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Composition prefab.", "et")]
	protected ResourceName m_Resource;
	
	[Attribute("0 0 0", UIWidgets.EditBox, "...", category: "Offset of building controller from center of the composition.")]
	protected vector m_vBuildingControllerOffset;
	
	[Attribute("Composition", desc: "Display name to be shown in UI.")]
	protected string m_sDisplayName;
	
	[Attribute("", desc: "Name of used marker.")]
	protected string m_sMarkerImage;
	
	[Attribute("", desc: "Name of used HUD icon.")]
	protected string m_sIconImage;
	
	[Attribute("10", UIWidgets.EditBox, "The cost of the composition", "", )]
	protected int m_iPrice;
	
	[Attribute("25", UIWidgets.EditBox, "The percentage of the original composition cost, which is returned back to the player when he disassemble the composition", "", )]
	protected int m_iRefundPercentage;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESlotTypesEnum))]
	protected SCR_ESlotTypesEnum m_SlotType;
		
	[Attribute("1", UIWidgets.CheckBox, "Can user rotate this composition?")]
	protected bool m_bCanBeRotated;
	
	[Attribute("0", UIWidgets.CheckBox, "Is this a service composition i.e. Barracks, Hospital...")]
	protected bool m_bIsServiceComposition;
	
	[Attribute()]
	ref array<ref SCR_BuildingPhase> m_aBuildingPhase;
		
	//------------------------------------------------------------------------------------------------
	ResourceName GetResourceName()
	{
		return m_Resource;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetBuildingControllerOffset()
	{
		return m_vBuildingControllerOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPrice()
	{
		return m_iPrice;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRefundPercentage()
	{
		return m_iRefundPercentage / 100;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ESlotTypesEnum GetSlotType()
	{
		return m_SlotType;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanBeRotated()
	{
		return m_bCanBeRotated;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCompositionName()
	{
		return m_sDisplayName;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsServiceComposition()
	{
		return m_bIsServiceComposition;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_BuildingPhase> GetBuildingPhaseResources()
	{
		return m_aBuildingPhase;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetMarkerImage()
	{
		return m_sMarkerImage;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetIconImage()
	{
		return m_sIconImage;
	}
};

[BaseContainerProps()]
class SCR_BuildingPhase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Composition prefab.", "et")]
	protected ResourceName m_BuildingPhaseResource;
	
	[Attribute("10", UIWidgets.EditBox, "How long this building stage should takse. Composition building time = sum of all building stages.", "", )]
	protected int m_iBuildingTime;
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetPhaseResourceName()
	{
		return m_BuildingPhaseResource;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBuildingTime()
	{
		return m_iBuildingTime;
	}
};

enum SCR_ESlotTypesEnum
{
	FlatSmall,
	FlatMedium,
	FlatLarge,
	CheckpointSmall,
	CheckpointMedium,
	CheckpointLarge,
	Services
};