class SCR_InventoryAmmoTypeIndicator : ScriptedWidgetComponent
{
	[Attribute("AmmoTypeContainer")]
	protected string m_sContainer;
	protected Widget m_wContainer;
	
	//------------------------------------------------------------------------------------------------	
	override void HandlerAttached(Widget w)
	{
		m_wContainer = w.FindAnyWidget(m_sContainer);
	}
	
	//------------------------------------------------------------------------------------------------	
	bool SetAmmoType(InventoryItemComponent item)
	{
		Widget child = m_wContainer.GetChildren();
		while (child)
		{
			child.SetVisible(false);
			child = child.GetSibling();
		}
	
		MagazineComponent magComp = MagazineComponent.Cast(item.GetOwner().FindComponent(MagazineComponent));
		if (magComp)
		{
			MagazineUIInfo ammoInfo = MagazineUIInfo.Cast(magComp.GetUIInfo());
			if (!ammoInfo)
				return false;
	
			SetAmmoTypeIcon(ammoInfo.GetAmmoTypeFlags());	
			return true;
		}

		WeaponComponent weaponComp = WeaponComponent.Cast(item.GetOwner().FindComponent(WeaponComponent));
		if (weaponComp)
		{
			GrenadeUIInfo ammoInfo = GrenadeUIInfo.Cast(weaponComp.GetUIInfo());
			if (!ammoInfo)
				return false;
	
			SetAmmoTypeIcon(ammoInfo.GetAmmoTypeFlags());	
			return true;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------	
	protected void SetAmmoTypeIcon(EAmmoType flags)
	{
		// let's just assume all of these exist
		if (flags & EAmmoType.FMJ)
			m_wContainer.FindAnyWidget("ammotype-fmj").SetVisible(true);
		if (flags & EAmmoType.TRACER)
			m_wContainer.FindAnyWidget("ammotype-tracer").SetVisible(true);
		if (flags & EAmmoType.AP)
			m_wContainer.FindAnyWidget("ammotype-AP").SetVisible(true);
		if (flags & EAmmoType.HE)
			m_wContainer.FindAnyWidget("ammotype-HE").SetVisible(true);
		if (flags & EAmmoType.HEAT)
			m_wContainer.FindAnyWidget("ammotype-HEAT").SetVisible(true);
		if (flags & EAmmoType.FRAG)
			m_wContainer.FindAnyWidget("ammotype-frag").SetVisible(true);
		if (flags & EAmmoType.SMOKE)
			m_wContainer.FindAnyWidget("ammotype-smoke").SetVisible(true);
		if (flags & EAmmoType.INCENDIARY)
			m_wContainer.FindAnyWidget("ammotype-incendiary").SetVisible(true);
		if (flags & EAmmoType.SNIPER)
			m_wContainer.FindAnyWidget("ammotype-sniper").SetVisible(true);		
		if (flags & EAmmoType.ILLUMINATION)
			m_wContainer.FindAnyWidget("ammotype-illumination").SetVisible(true);
	}
};