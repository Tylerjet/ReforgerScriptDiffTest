[EntityEditorProps(category: "GameScripted/Gadgets", description: "Consumable gadget")]
class SCR_ConsumableItemComponentClass : SCR_GadgetComponentClass
{
};

//------------------------------------------------------------------------------------------------
// Consumable gadget component
class SCR_ConsumableItemComponent : SCR_GadgetComponent
{
	[Attribute("", UIWidgets.Object, category: "Consumable" )]
	protected ref SCR_ConsumableEffectBase m_ConsumableEffect;

	[Attribute("0", UIWidgets.CheckBox, "Switch model on use, f.e. from packaged version", category: "Consumable")]
	protected bool m_bAlternativeModelOnAction;	
	
	[Attribute("0", UIWidgets.CheckBox, "Show the model of the item on the character after using", category: "Consumable")]
	protected bool m_bVisibleEquipped;

	protected SCR_CharacterControllerComponent m_CharController;
	
	//Target character is the target set when it's not m_CharacterOwner
	protected IEntity m_TargetCharacter;

	//------------------------------------------------------------------------------------------------
	//! Get consumable type
	//! \return consumable type
	EConsumableType GetConsumableType()
	{
		if (m_ConsumableEffect)
			return m_ConsumableEffect.m_eConsumableType;

		return EConsumableType.None;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get consumable effect
	//! \return consumable effect
	SCR_ConsumableEffectBase GetConsumableEffect()
	{
		return m_ConsumableEffect;
	}

	//------------------------------------------------------------------------------------------------
	//! Apply consumable effect
	//! \param target is the target entity
	void ApplyItemEffect(IEntity target, SCR_ConsumableEffectAnimationParameters animParams, IEntity item, bool deleteItem = true)
	{
		if (!m_ConsumableEffect)
			return;

		m_ConsumableEffect.ApplyEffect(target, target, item, animParams);

		ModeClear(EGadgetMode.IN_HAND);
		
		if (deleteItem)
			RplComponent.DeleteRplEntity(GetOwner(), false);
	}

	//------------------------------------------------------------------------------------------------
	//! OnItemUseComplete event from SCR_CharacterControllerComponent
	protected void OnApplyToCharacter(IEntity item, bool successful, SCR_ConsumableEffectAnimationParameters animParams)
	{
		if (!successful)
			return;
		
		bool deleteItem = true;
		if (GetConsumableEffect())
		{
			deleteItem = GetConsumableEffect().GetDeleteOnUse();
		}
		
		IEntity target = GetTargetCharacter();
		if (!target)
			target = m_CharacterOwner;
		if (!target)
			return;

		ApplyItemEffect(target, animParams, item, deleteItem);
	}

	//------------------------------------------------------------------------------------------------
	//! OnItemUseBegan event from SCR_CharacterControllerComponent
	protected void OnUseBegan()
	{
		if (m_bAlternativeModelOnAction)
			SetAlternativeModel(true);
	}

	//-----------------------------------------------------------------------------
	//! Switch item model
	//! \param useAlternative determines whether alternative or base model is used
	void SetAlternativeModel(bool useAlternative)
	{
		InventoryItemComponent inventoryItemComp = InventoryItemComponent.Cast( GetOwner().FindComponent(InventoryItemComponent));
		if (!inventoryItemComp)
			return;

		ItemAttributeCollection attributeCollection = inventoryItemComp.GetAttributes();
		if (!attributeCollection)
			return;

		SCR_AlternativeModel additionalModels = SCR_AlternativeModel.Cast( attributeCollection.FindAttribute(SCR_AlternativeModel));
		if (!additionalModels)
			return;

		ResourceName consumableModel;
		if (useAlternative)
			consumableModel = additionalModels.GetAlternativeModel();
		else
			consumableModel = SCR_Global.GetPrefabAttributeResource(GetOwner(), "MeshObject", "Object");

		Resource resource = Resource.Load(consumableModel);
		if (!resource)
			return;

		BaseResourceObject baseResource = resource.GetResource();
		if (baseResource)
		{
			VObject asset = baseResource.ToVObject();
			if (asset)
				GetOwner().SetObject(asset, "");
		}
	}

	//-----------------------------------------------------------------------------
	//! Set visibility of item
	//! \Sets item visible on the outside of the physical character
	override bool IsVisibleEquipped()
	{
		return m_bVisibleEquipped;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);

		if (mode == EGadgetMode.IN_HAND)
		{
			m_CharController = SCR_CharacterControllerComponent.Cast(m_CharacterOwner.FindComponent(SCR_CharacterControllerComponent));
			m_CharController.m_OnItemUseEndedInvoker.Insert(OnApplyToCharacter);
			m_CharController.m_OnItemUseBeganInvoker.Insert(OnUseBegan);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{
		if (mode == EGadgetMode.IN_HAND)
		{
			if (m_CharController)
			{
				m_CharController.m_OnItemUseEndedInvoker.Remove(OnApplyToCharacter);
				m_CharController.m_OnItemUseBeganInvoker.Remove(OnUseBegan);
				m_CharController = null;
			}
		}

		super.ModeClear(mode);
	}

	//------------------------------------------------------------------------------------------------
	override void ActivateAction()
	{
		if (!m_ConsumableEffect || !m_ConsumableEffect.CanApplyEffect(m_CharacterOwner, GetOwner()))
			return;
		
		m_ConsumableEffect.ActivateEffect(m_CharacterOwner, m_CharacterOwner, GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.CONSUMABLE;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeToggled()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTargetCharacter(IEntity target)
	{
		m_TargetCharacter = target;
	}	
	
	//------------------------------------------------------------------------------------------------
	IEntity GetTargetCharacter()
	{
		return m_TargetCharacter;
	}
};
