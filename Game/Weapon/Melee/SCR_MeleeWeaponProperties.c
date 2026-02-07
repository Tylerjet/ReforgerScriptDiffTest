[ComponentEditorProps(category: "GameScripted/Weapon", description:"Keeps settings for melee weapon")]
class SCR_MeleeWeaponPropertiesClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MeleeWeaponProperties : ScriptComponent
{
	[Attribute("10", "Size of damage dealt by the weapon", category: "Global")]
	private float m_fDamage;	
	
	[Attribute("1", "Range of the weapon [m]", category: "Global")]
	private float m_fRange;
	
	[Attribute("0.3", "Accuracy of melee attacks, smaller values are more accurate", category: "Global")]
	protected float m_fAccuracy;
	
	protected SCR_BayonetComponent m_Bayonet;
	
	//------------------------------------------------------------------------------------------------
	//! Returns instance of attached bayonet
	SCR_BayonetComponent GetBayonet()
	{
		return m_Bayonet;
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Value of damage dealt to the target
	float GetWeaponDamage()
	{
		if (m_Bayonet)
			return m_Bayonet.GetDamage();
		else
			return m_fDamage;
	}

	//------------------------------------------------------------------------------------------------	
	//! Range in meters that is used as max raycast length
	float GetWeaponRange()
	{
		if (m_Bayonet)
			return m_Bayonet.GetRange();
		else
			return m_fRange;
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Size of the raysphere used to trace the target
	float GetWeaponMeleeAccuracy()
	{
		if (m_Bayonet)
			return m_Bayonet.GetAccuracy();
		else
			return m_fAccuracy;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAttachmentAdded(IEntity item, int slotID)
	{
		SCR_BayonetComponent bayonetComponent = SCR_BayonetComponent.Cast(item.FindComponent(SCR_BayonetComponent));
		if (bayonetComponent)
			m_Bayonet = bayonetComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAttachmentRemoved(IEntity item, int slotID)
	{
		SCR_BayonetComponent bayonetComponent = SCR_BayonetComponent.Cast(item.FindComponent(SCR_BayonetComponent));
		if (bayonetComponent)
			m_Bayonet = null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_WeaponAttachmentsStorageComponent attachmentStorage = SCR_WeaponAttachmentsStorageComponent.Cast(GetOwner().FindComponent(SCR_WeaponAttachmentsStorageComponent));
		if (!attachmentStorage)
			return;
		
		attachmentStorage.m_OnItemAddedToSlotInvoker.Insert(OnAttachmentAdded);
		attachmentStorage.m_OnItemRemovedFromSlotInvoker.Insert(OnAttachmentRemoved);	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
};