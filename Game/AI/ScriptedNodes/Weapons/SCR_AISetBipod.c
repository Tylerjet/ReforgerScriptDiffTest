class SCR_AISetBipod : AITaskScripted
{
	[Attribute("true", UIWidgets.CheckBox, "When true, bipod will be unfolded, otherwise it will be folded")]
	protected bool m_bUnfoldBipod;
	
	BaseWeaponManagerComponent m_WeaponMgr;
	
	//------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity myEntity = owner.GetControlledEntity();
		m_WeaponMgr = BaseWeaponManagerComponent.Cast(myEntity.FindComponent(BaseWeaponManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_WeaponMgr)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent weaponComp = m_WeaponMgr.GetCurrentWeapon();
		if (!weaponComp)
			return ENodeResult.FAIL;
		
		IEntity weaponEntity = weaponComp.GetOwner();
		WeaponAnimationComponent weaponAnimComp = WeaponAnimationComponent.Cast(weaponEntity.FindComponent(WeaponAnimationComponent));
		if (!weaponAnimComp)
			return ENodeResult.FAIL;
		
		weaponAnimComp.SetBipod(m_bUnfoldBipod);
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------
	override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------------------------
	override string GetOnHoverDescription() { return "Unfolds or folds a bipon on current weapon";}
}