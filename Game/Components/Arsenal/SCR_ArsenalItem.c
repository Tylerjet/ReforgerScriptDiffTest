[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Arsenal Data", "DISABLED - Arsenal Data", 1)]
class SCR_ArsenalItem : SCR_BaseEntityCatalogData
{	
	[Attribute("2", desc: "Type of the arsenal item. An arsenal will only spawn items of types that it allows to be spawned. The item will not show up if it is not allowed. Eg: FieldDressing = HEAL", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eItemType;
	
	[Attribute("2", desc: "Item mode of arsenal, set this to what the behaviour is for the item. EG: FieldDressing = CONSUMABLE as it is used up or M16 with attachments = WEAPON_VARIENTS as it is not a default M16. Check other items in the faction config to see how it works.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eItemMode;
	
	[Attribute("10", desc: "Supply cost of item when using the resupply action and/or taking them from the arsenal inventory. Note in overwrite arsenal config this value is ignored and still taken from catalog", params: "0 inf 1")]
	protected int m_iSupplyCost;
	
	[Attribute(desc: "Display data for SCR_ArsenalDisplayComponent. If Arsenal item has display data of the correct type for the entity with SCR_ArsenalDisplayComponent than it can be displayed on said entity")]
	protected ref array<ref SCR_ArsenalItemDisplayData> m_aArsenalDisplayData;
	
	[Attribute(desc: "Depending on the settings of the arsenal component arsenal items can have an alternative supply cost. So it will take the cost of the alternative rather than the default cost. \n\nIf an Arsenal is not cost type default and the arsenal item does not have that cost type defined than it will still use the default cost.\n\nIf multiple entries have the same value than the last in the array will be used")]
	protected ref array<ref SCR_ArsenalAlternativeCostData> m_aArsenalAlternativeCostData;
	
	protected ref map<SCR_EArsenalSupplyCostType, int> m_mArsenalAlternativeCostData;
	
	protected SCR_EntityCatalogEntry m_EntryParent;
	protected ref Resource m_ItemResource;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemType GetItemType()
	{
		return m_eItemType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemMode GetItemMode()
	{
		return m_eItemMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetItemResourceName()
	{
		if (!m_EntryParent)
			return string.Empty;	
		
		return m_EntryParent.GetPrefab();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Resource GetItemResource()
	{
		return m_ItemResource;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get display data of given type for displaying items on arsenal display
	//! \param[in] displayType type of display data to find
	//! \return Display data, can be null if not found
	SCR_ArsenalItemDisplayData GetDisplayDataOfType(EArsenalItemDisplayType displayType)
	{
		foreach (SCR_ArsenalItemDisplayData displayData : m_aArsenalDisplayData)
		{
			if (displayData.GetDisplayType() == displayType)
				return displayData;
		}
		
		return null;
	}
	
	//--------------------------------- Direct Getter general or any faction ---------------------------------\\

	//------------------------------------------------------------------------------------------------
	//! Get supply cost of arsenal item
	//! \param[in] supplyCostType What the supply cost is that should be obtained from the arsenal data. If the specific supply cost is not found then Default will be used instead
	//! \return Supplycost
	int GetSupplyCost(SCR_EArsenalSupplyCostType supplyCostType)
	{
		if (supplyCostType != SCR_EArsenalSupplyCostType.DEFAULT && m_mArsenalAlternativeCostData != null)
		{
			int returnValue; 
			if (m_mArsenalAlternativeCostData.Find(supplyCostType, returnValue))
				return returnValue;
		}
		
		return m_iSupplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitData(notnull SCR_EntityCatalogEntry entry)
	{
		m_EntryParent = entry;
		
		m_ItemResource = Resource.Load(m_EntryParent.GetPrefab());
		
		//~ Save alternative costs in map and delete the array
		if (m_aArsenalAlternativeCostData.IsEmpty())
			return;
		
		m_mArsenalAlternativeCostData = new map<SCR_EArsenalSupplyCostType, int>();
		
		foreach (SCR_ArsenalAlternativeCostData data : m_aArsenalAlternativeCostData)
		{
			//~ Ignore default as that is m_iSupplyCost defined in the arsenal item
			if (data.m_eAlternativeCostType == SCR_EArsenalSupplyCostType.DEFAULT)
				continue;
			
			m_mArsenalAlternativeCostData.Insert(data.m_eAlternativeCostType, data.m_iSupplyCost);
		}
		
		//~ Delete array
		m_aArsenalAlternativeCostData = null;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), BaseContainerCustomEnumWithValue(SCR_EArsenalSupplyCostType, "m_eAlternativeCostType", "m_iSupplyCost", "1", "%1 - Supply cost: %2")]
class SCR_ArsenalAlternativeCostData
{
	[Attribute(SCR_EArsenalSupplyCostType.GADGET_ARSENAL.ToString(), desc: "Cost type if system searches for the cost. Do not use DEFAULT as this will be ignored", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalSupplyCostType))]
	SCR_EArsenalSupplyCostType m_eAlternativeCostType;
	
	[Attribute("1", desc: "Alternative supply cost", params: "0 inf")]
	int m_iSupplyCost;
}
