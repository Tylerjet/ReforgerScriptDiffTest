[EntityEditorProps(category: "GameScripted/Campaign", description: "Simple component for changing material of flags and signs.", color: "0 0 255 255")]
class SCR_FlagComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_FlagComponent: SCR_MilitaryBaseLogicComponent
{
	[Attribute("1", UIWidgets.CheckBox, "Is Owner entity Flag Pole?", "")]
	protected bool m_bIsFlagPole;
	
	[Attribute("{9BE1341F99D33483}Assets/Props/Fabric/Flags/Data/Flag_1_2_EVERON.emat", UIWidgets.ResourcePickerThumbnail, "Default material to be used.", params: "emat")]
	private ResourceName m_DefaultMaterial;
	
	protected IEntity m_FlagEntity;
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether component is set to Flag pole or something else (sign)
	bool IsFlagPole()
	{
		return m_bIsFlagPole;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultMaterial()
	{
		return m_DefaultMaterial;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes Material used on Flag or Sign
	//! Parameters: flagResource = material to be used, resourceMlod = MLOD , Flags doesn't need this, only signs
	void ChangeMaterial(ResourceName flagResource, ResourceName resourceMLOD = "")
	{
		if (!m_FlagEntity)
			SetAttachedFlagEntity();
		
		//Custom set material. Finds original material "slot" with MLOD and assigns appropriate new one, if possible.
		if (m_FlagEntity && flagResource)
		{
			VObject mesh = m_FlagEntity.GetVObject();
			if (!mesh)
				return;
			
			string remap;
			string materials[256];
			int numMats = mesh.GetMaterials(materials);
				
			for (int i = 0; i < numMats; i++)
			{
				if (materials[i].Contains("MLOD"))
					remap += string.Format("$remap '%1' '%2';", materials[i], resourceMLOD);
				else
					remap += string.Format("$remap '%1' '%2';", materials[i], flagResource);
			}
			m_FlagEntity.SetObject(mesh, remap);
			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set flag entity variable from SlotManager (For FlagPoles)
	protected void SetAttachedFlagEntity()
	{
		if (!m_bIsFlagPole)
			m_FlagEntity = GetOwner();
		else
		{
			SlotManagerComponent slotManager = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
			if (!slotManager)
				return;
		
			array <EntitySlotInfo> slots = new array <EntitySlotInfo>;
		
			slotManager.GetSlotInfos(slots);
		
			foreach (EntitySlotInfo slot : slots)
			{
				IEntity flag = slot.GetAttachedEntity();
				if (flag)
				{
					m_FlagEntity = flag;
					break;
				}
			}
		}
		
		ChangeMaterial(m_DefaultMaterial);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		if (!GetGame().InPlayMode())
			return;
		
		SetAttachedFlagEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	// PostInit
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
};