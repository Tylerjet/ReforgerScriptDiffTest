[EntityEditorProps(category: "GameScripted/Components", description: "")]
class SCR_PlaceableItemComponentClass : ScriptComponentClass
{
	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Can this entity be attached to dynamic objects.")]
	protected bool m_bCanAttachToDynamicObject;

	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Can this entity be attached even when it is not upright.")]
	protected bool m_bCanAttachAngled;

	[Attribute(uiwidget: UIWidgets.Flags, desc: "Set of flags that will be used to ignore objects based on their physics layer.\nWARNING: To prevent players from attaching objects to weapons use Ignored Components list\nas checking weapon layer will make it impossible to attach the object to the armed vehicles!", enums: ParamEnumArray.FromEnum(EPhysicsLayerDefs))]
	protected EPhysicsLayerDefs m_eIgnoredPhysicsLayers;

	[Attribute(uiwidget: UIWidgets.Object, desc: "List of components that when present on target entity or its parent will make it impossible to attach placeable item to it")]
	protected ref array<string> m_aIgnoredComponents;

	[Attribute(SCR_ECharacterDistanceMeasurementMethod.FROM_EYES.ToString(), UIWidgets.ComboBox, "How the distance between player and placment position should be calculated", enums: ParamEnumArray.FromEnum(SCR_ECharacterDistanceMeasurementMethod))]
	protected SCR_ECharacterDistanceMeasurementMethod m_eMeasurementMethod;

	protected ref array<typename> m_aIgnoredComponentTypes = {};
	protected bool m_bValidated;

	//------------------------------------------------------------------------------------------------
	//! Returns true if it should be possible to attach this object to dynamic object like f.e. vehicle
	bool CanAttachToDynamicObject()
	{
		return m_bCanAttachToDynamicObject;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if this object is meant to be attached no matter what will be it is rotation when it will be attached
	bool CanBeAttachedWhileAngled()
	{
		return m_bCanAttachAngled;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns physics layer mask that can be used to determin which layers should be ignored
	EPhysicsLayerDefs GetIgnoredPhysicsLayers()
	{
		return m_eIgnoredPhysicsLayers;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns preferred Measurement method
	SCR_ECharacterDistanceMeasurementMethod GetDistanceMeasurementMethod()
	{
		return m_eMeasurementMethod;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns physics layer mask that can be used to determin which layers should be ignored
	int GetIgnoredComponents(notnull out array<typename> outIgnoredComponents)
	{
		outIgnoredComponents.Clear();
		foreach (typename comp : m_aIgnoredComponentTypes)
		{
			outIgnoredComponents.Insert(comp);
		}

		return outIgnoredComponents.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes unwanted types from ignored components list
	void ValidateIgnoredComponents()
	{
		if (m_bValidated || !m_aIgnoredComponents)
			return;

		m_bValidated = true;
		typename comp;
		for (int i = m_aIgnoredComponents.Count() - 1; i >= 0; i--)
		{
			comp = m_aIgnoredComponents[i].ToType();

			if (!comp.IsInherited(GenericComponent))
			{
#ifdef WORKBENCH
				Print("WARNING! " + m_aIgnoredComponents[i] + " is not inherited from GenericComponent and thus will be removed from the list of ignored components!", LogLevel.ERROR);
#endif
				m_aIgnoredComponents.Remove(i);
			}
			else
			{
				m_aIgnoredComponentTypes.Insert(comp);
			}
		}
	}
}

class SCR_PlaceableItemComponent : ScriptComponent
{
	[Attribute(params: "xob")]
	protected ResourceName m_sPreviewObject;
	
	[Attribute("0.5084", desc: "Max placement distance in meters.")]
	protected float m_fMaxPlacementDistance;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EPlacementType))]
	protected SCR_EPlacementType m_ePlacementType;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EPlacementType GetPlacementType()
	{
		return m_ePlacementType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxPlacementDistance()
	{
		return m_fMaxPlacementDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	VObject GetPreviewVobject()
	{
		if (m_sPreviewObject.IsEmpty())
			return GetOwner().GetVObject();
		
		Resource resource = Resource.Load(m_sPreviewObject);
		if (!resource.IsValid())
			return GetOwner().GetVObject();
		
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return GetOwner().GetVObject();
		
		return resourceObject.ToVObject();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if it should be possible to attach this object to dynamic object like f.e. vehicle
	bool CanAttachToDynamicObject()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.CanAttachToDynamicObject();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if this object is meant to be attached no matter what will be it is rotation when it will be attached
	bool CanBeAttachedWhileAngled()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.CanBeAttachedWhileAngled();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns physics layer mask that can be used to determin which layers should be ignored
	EPhysicsLayerDefs GetIgnoredPhysicsLayers()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 0;

		return data.GetIgnoredPhysicsLayers();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes unwanted types from ignored components list
	int GetIgnoredComponents(notnull out array<typename> outIgnoredComponents)
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return 0;

		return data.GetIgnoredComponents(outIgnoredComponents);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		data.ValidateIgnoredComponents();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns preferred Measurement method
	SCR_ECharacterDistanceMeasurementMethod GetDistanceMeasurementMethod()
	{
		SCR_PlaceableItemComponentClass data = SCR_PlaceableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return SCR_ECharacterDistanceMeasurementMethod.FROM_EYES;

		return data.GetDistanceMeasurementMethod();
	}

	static float GetDistanceFromCharacter(notnull ChimeraCharacter char, vector destination, SCR_ECharacterDistanceMeasurementMethod method = SCR_ECharacterDistanceMeasurementMethod.FROM_EYES)
	{
		vector charPos;
		switch (method)
		{
			case (SCR_ECharacterDistanceMeasurementMethod.FROM_ORIGIN):
			{
				charPos = char.GetOrigin();
				break;
			}

			case (SCR_ECharacterDistanceMeasurementMethod.FROM_CENTER_OF_MASS):
			{
				Physics phys = char.GetPhysics();
				if (!phys)
					return 0;

				charPos = phys.GetCenterOfMass();
				break;
			}

			default:	//SCR_ECharacterDistanceMeasurementMethod.FROM_EYES
			{
				charPos = char.EyePosition();
				break;
			}
		}

		return vector.Distance(charPos, destination);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RplSave(ScriptBitWriter writer)
	{
		RplId parentId = RplId.Invalid();
		int nodeId = -1;
		SCR_PlaceableInventoryItemComponent placeableIIC = SCR_PlaceableInventoryItemComponent.Cast(GetOwner().FindComponent(SCR_PlaceableInventoryItemComponent));
		if (placeableIIC)
		{
			parentId = placeableIIC.GetParentRplId();
			nodeId = placeableIIC.GetParentNodeId();
		}
		writer.WriteRplId(parentId);
		if (parentId.IsValid())
			writer.WriteInt(nodeId);

		return super.RplSave(writer);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RplLoad(ScriptBitReader reader)
	{
		RplId parentId = RplId.Invalid();
		reader.ReadRplId(parentId);
		if (parentId.IsValid())
		{
			int nodeId = -1;
			reader.ReadInt(nodeId);

			SCR_PlaceableInventoryItemComponent placeableIIC = SCR_PlaceableInventoryItemComponent.Cast(GetOwner().FindComponent(SCR_PlaceableInventoryItemComponent));
			if (placeableIIC)
				placeableIIC.SetNewParent(parentId, nodeId);
		}

		return super.RplLoad(reader);
	}
}
