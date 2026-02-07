class SCR_AIDecoTestUsingDisposableWeapon : DecoratorTestScripted
{
	BaseWeaponManagerComponent m_weaponManager;
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!m_weaponManager)
		{
			m_weaponManager = BaseWeaponManagerComponent.Cast(controlled.FindComponent(BaseWeaponManagerComponent));
			if (!m_weaponManager)
				return false;
		}
		
		BaseWeaponComponent weapon = m_weaponManager.GetCurrentWeapon();
		if (!weapon)
			return false;
		
		BaseMuzzleComponent muzzle = weapon.GetCurrentMuzzle();
		if (!muzzle)
			return false;
		
		return muzzle.IsDisposable();
	}
};