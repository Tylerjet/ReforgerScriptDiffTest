//#define WEAPON_INFO_DEBUG
//#define WEAPON_INFO_DEBUG_STATES

//#define WEAPON_INFO_DEBUG_WATERFALL_EVENTS
//#define WEAPON_INFO_BLOCK_WATERFALL_EVENTS

// Will enable lots of logging on every event
// Can define it through -scrDefine WEAPON_INFO_DEBUG

enum EAmmoType
{
	/*UNDEFINED = 0,*/
	FMJ = 1,
	TRACER = 2,
	AP = 4,
	HE = 8,
	HEAT = 16,
	FRAG = 32,
	SMOKE = 64,
	INCENDIARY = 128,
	SNIPER = 256
};

enum EWeaponFeature
{
	NONE = 0,
	ZEROING = 1,
	FIREMODE = 2,
	AMMOCOUNT = 4,
	MAGAZINE = 8,
	MUZZLE = 16,
	WEAPON = 32,
	ADS	= 64,
	MAGAZINE_COUNT = 128,
	INSPECTION = 256,
	ZOOM = 512,
	MISC = 32768
};

class SCR_WeaponInfo : SCR_InfoDisplayExtended
{
	#ifdef WEAPON_INFO_DEBUG
	static void _print(string str)
	{
		int hour, minute, second;
		System.GetHourMinuteSecondUTC(hour, minute, second);
		string timeStr = string.Format("%1.%2.%3", hour, minute, second);
		Print(string.Format("%1 [Weapon Info UI] %2", timeStr, str), LogLevel.DEBUG);
	}
	#endif
	
	static const float SCALE_DEFAULT = 1;
	static const float SCALE_FIREMODE = 0.9;
	static const float SCALE_MAGAZINE = 0.8;
	
	const float UPDATE_INTERVAL = 0.1;	
	const float FADED_OPACITY = 0.3;
	
	const ref Color COLOR_WHITE = Color.FromSRGBA(255, 255, 255, 255);
	const ref Color COLOR_ORANGE = Color.FromSRGBA(226, 167, 80, 255);
	const ref Color COLOR_SAGE = Color.FromSRGBA(225, 232, 222, 255);
	
	protected const float FADEOUT_PANEL_DELAY = 6; // Time until whole panel fades out
	protected const float FADEOUT_OPTICS_DELAY = 6; // Time until zeroing fades out
	protected const float FADEOUT_AMMO_TYPE_DELAY = 6; // Time until ammo type fades out
	
	protected BaseWeaponManagerComponent m_WeaponManager = null;
	protected SCR_CharacterControllerComponent m_CharacterController = null;
	protected SCR_InventoryStorageManagerComponent m_InventoryManager = null;
	protected EventHandlerManagerComponent m_EventHandlerManager = null;
	protected CompartmentAccessComponent m_CompartmentAccess = null;
	protected MenuManager m_MenuManager = null;

	protected bool m_bMenuOpen = false;
	
	// Members for previous states of weapon
	protected ref SCR_WeaponState m_WeaponState;
	
	protected EWeaponFeature m_eWeaponStateEvent = EWeaponFeature.NONE;

	// Resources
	protected const ResourceName DEFAULT_WEAPON_INFO_LAYOUT = "{8170DE93810F928A}UI/layouts/HUD/WeaponInfo/WeaponInfo.layout";
	protected const ResourceName DEFAULT_MAGAZINE_INDICATOR = "{7C114BA7C59E198D}Configs/WeaponInfo/MagazineIndicators/box.conf";
		
	// Widgets
	ref SCR_WeaponInfoWidgets widgets;
	
	protected ref SCR_FadeInOutAnimator m_WeaponInfoPanelAnimator;
	protected ref SCR_FadeInOutAnimator m_ZeroingAnimator;
	//protected ref SCR_FadeInOutAnimator m_AmmoTypeAnimator;
	
	protected bool m_bFadeInOutActive = false;
	
	// Other
	ref SCR_MagazinePredicate m_pMagazineSearchPredicate = new SCR_MagazinePredicate(); // Predicate for searching for magazines
	ref SCR_PrefabDataPredicate m_pPrefabDataPredicate = new SCR_PrefabDataPredicate();
	
	//------------------------------------------------------------------------------------------------
	void OnWeaponChanged(BaseWeaponComponent weapon, BaseWeaponComponent prevWeapon)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnWeaponChanged");
		_print(string.Format("    weapon:     %1", weapon));
		#endif

		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.WEAPON;

		// In case of grenades, the provided weapon is a weapon slot, with the actual grenade inside this slot
		WeaponSlotComponent weaponSlot = WeaponSlotComponent.Cast(weapon);

		if (weaponSlot)
		{
			IEntity e = weaponSlot.GetWeaponEntity();
			
			if (e)
				weapon = BaseWeaponComponent.Cast(e.FindComponent(BaseWeaponComponent));
			else
				weapon = null;
		}

		// Create the layout if needed
		if (!weapon)
		{
			if (m_wRoot)
				m_wRoot.SetVisible(false);
			
			return;
		}
		else
		{
			if (!m_wRoot)
				CreateLayout(DEFAULT_WEAPON_INFO_LAYOUT);
			
			if (m_wRoot)
				m_wRoot.SetVisible(true);
		}		
						
		// Initialize weapon state from our current weapon
		if (!m_WeaponState)
			m_WeaponState = new SCR_WeaponState();

		if (m_WeaponState.m_Weapon != weapon)
		{
			// Remove any possible existing "zoom-changed" invokers
			if (m_WeaponState.m_SightsZoomFOVInfo)
				m_WeaponState.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Remove(OnZoomChanged);
			
			// Wipe the weapon state data
			m_WeaponState.Init();
		}
		
		// Store current weapon		
		m_WeaponState.m_Weapon = weapon;		
		
		// Get weapon & grenade UI info
		m_WeaponState.m_WeaponUi = WeaponUIInfo.Cast(weapon.GetUIInfo());
		m_WeaponState.m_GrenadeUi = GrenadeUIInfo.Cast(m_WeaponState.m_WeaponUi);

		// If weapon is a grenade, get its type
		if (m_WeaponState.m_GrenadeUi)
		{
			m_WeaponState.m_eAmmoTypeFlags = m_WeaponState.m_GrenadeUi.GetAmmoTypeFlags();
			m_WeaponState.m_bHasSpecialAmmo = !(m_WeaponState.m_eAmmoTypeFlags == 0 || m_WeaponState.m_eAmmoTypeFlags & EAmmoType.FMJ);
			
			m_WeaponState.m_MagazineConfig = m_WeaponState.m_GrenadeUi.m_MagIndicator;
		}		
		
		// Check if weapon is a grenade
		IEntity e = weapon.GetOwner();
		
		if (e && e.FindComponent(GrenadeMoveComponent))
			m_WeaponState.m_bIsGrenade = true;
		else
			m_WeaponState.m_bIsGrenade = false;

		// Check if weapon has scope sights component
		m_WeaponState.m_Sights = GetSights();
		
		if (m_WeaponState.m_Sights)
			m_WeaponState.m_SightsZoomFOVInfo = SCR_SightsZoomFOVInfo.Cast(m_WeaponState.m_Sights.GetFOVInfo());
		
		if (m_WeaponState.m_SightsZoomFOVInfo)
			m_WeaponState.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Insert(OnZoomChanged);
				
		// Auto-trigger child events
		#ifndef WEAPON_INFO_BLOCK_WATERFALL_EVENTS
		OnMuzzleChanged_init(weapon);
		OnADSChanged_init(weapon);
		#endif
	}	
	
	void OnMuzzleChanged_init(BaseWeaponComponent weapon)
	{
		BaseMuzzleComponent muzzle;
		
		if (weapon)
			muzzle = weapon.GetCurrentMuzzle();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		Print("OnMuzzleChanged_init");
		Print(string.Format("    weapon:     %1", weapon));
		Print(string.Format("    muzzle:     %1", muzzle));
		#endif
		
		OnMuzzleChanged(weapon, muzzle, null);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMuzzleChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseMuzzleComponent prevMuzzle)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnMuzzleChanged");
		_print(string.Format("    weapon:     %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		#endif
		
		m_eWeaponStateEvent |= EWeaponFeature.MUZZLE;
		
		m_WeaponState.m_Muzzle = muzzle;
		
		if (!muzzle)
		{
			// GRENADE
			
			m_WeaponState.m_MuzzleUi = null;
			m_WeaponState.m_bBarrelChambered = false;
			m_WeaponState.m_bBarrelCanBeChambered = false;
		}
		else
		{
			// NON-GRENADE
			
			m_WeaponState.m_MuzzleUi = MuzzleUIInfo.Cast(muzzle.GetUIInfo());
			m_WeaponState.m_bBarrelChambered = muzzle.IsBarrelChambered(muzzle.GetCurrentBarrelIndex());
			m_WeaponState.m_bBarrelCanBeChambered = muzzle.IsChamberingPossible();
		}
		
		// Auto-trigger child events
		#ifndef WEAPON_INFO_BLOCK_WATERFALL_EVENTS
		OnFiremodeChanged_init(weapon, muzzle);
		OnZeroingChanged_init(weapon);
		OnMagazineChanged_init(weapon, muzzle);		
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMagazineChanged_init(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle)
	{
		BaseMagazineComponent magazine;
		
		if (muzzle)
			magazine = muzzle.GetMagazine();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnMagazineChanged_init");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		_print(string.Format("    magazine:     %1", magazine));
		#endif
		
		OnMagazineChanged(weapon, magazine, null);
	}
	void OnMagazineChanged(BaseWeaponComponent weapon, BaseMagazineComponent magazine, BaseMagazineComponent prevMagazine)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnMagazineChanged");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    magazine:     %1", magazine));
		#endif
		
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.MAGAZINE;
		
		m_WeaponState.m_Magazine = magazine;
		
		if (magazine)
			m_WeaponState.m_MagazineUi = MagazineUIInfo.Cast(magazine.GetUIInfo());	
		else
			m_WeaponState.m_MagazineUi = null;

		if (m_WeaponState.m_MagazineUi)
		{
			m_WeaponState.m_eAmmoTypeFlags = m_WeaponState.m_MagazineUi.GetAmmoTypeFlags();
			m_WeaponState.m_bHasSpecialAmmo = !(m_WeaponState.m_eAmmoTypeFlags == 0 || m_WeaponState.m_eAmmoTypeFlags & EAmmoType.FMJ);
		}		

		// Initialize the magazine icon based on magazineUi > muzzleUi > weaponUi
		m_WeaponState.m_MagazineConfig = GetMagazineConfig(m_WeaponState);		
		
		#ifndef WEAPON_INFO_BLOCK_WATERFALL_EVENTS		
		OnMagazineCountChanged_init(weapon, magazine);
		OnAmmoCountChanged_init(weapon, magazine);
		#endif
		
		// Update magazine texture setup
		UpdateMagazineIndicator_Textures(m_WeaponState);
		UpdateAmmoTypeIndicator(m_WeaponState);		
	}

	//------------------------------------------------------------------------------------------------
	void OnMagazineCountChanged_init(BaseWeaponComponent weapon, BaseMagazineComponent magazine)
	{
		BaseMuzzleComponent muzzle;
		
		if (weapon)
			muzzle = weapon.GetCurrentMuzzle();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnMagazineCountChanged_init");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		_print(string.Format("    magazine:     %1", magazine));
		#endif
		
		// Get the character/vehicle inventory manager
		InventoryStorageManagerComponent invManager = SCR_WeaponInfo.GetInventoryManager();
		
		if (!weapon || !invManager)
		{
			OnMagazineCountChanged(weapon, 0, false);
			return;
		}
		
		int magazineCount = 0;
		
		if (muzzle)
		{
			// WEAPON WITH MUZZLE -> NON-GRENADE
			
			// Find compatible magazines for this magazine well
			BaseMagazineWell magWell = muzzle.GetMagazineWell();
			
			if (magWell)
			{
				m_pMagazineSearchPredicate.magWellType = magWell.Type();
				array<IEntity> magEntities = new array<IEntity>;
				invManager.FindItems(magEntities, m_pMagazineSearchPredicate);
				magazineCount = magEntities.Count();
			}
			else
			{
				// No magazine well, weapon is probably not reloadable
				magazineCount = 0;
			}
		}
		else
		{
			// WEAPON WITHOUT MUZZLE -> GRENADE
			
			// Find compatible weapons (same grenades for instance)
			m_pPrefabDataPredicate.prefabData = weapon.GetOwner().GetPrefabData();
			array<IEntity> sameWeapons = new array<IEntity>;
			invManager.FindItems(sameWeapons, m_pPrefabDataPredicate);
			
			// Remaining 'mags' count text
			magazineCount = sameWeapons.Count();
		}		
		
		bool isGrenade = m_WeaponState.m_bIsGrenade;
		
		OnMagazineCountChanged(weapon, magazineCount, isGrenade);
	}
	void OnMagazineCountChanged(BaseWeaponComponent weapon, int magazineCount, bool isGrenade)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnMagazineCountChanged");
		_print(string.Format("    weapon:        %1", weapon));
		_print(string.Format("    magazineCount: %1", magazineCount));
		_print(string.Format("    isGrenade:     %1", isGrenade));
		#endif
		
		BaseMagazineComponent magazine = weapon.GetCurrentMagazine();
		m_WeaponState.m_Magazine = magazine;
		
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.MAGAZINE_COUNT;
		
		m_WeaponState.m_iMagCount = magazineCount;
		m_WeaponState.m_bIsGrenade = isGrenade;

		// Update magazine textures for rocket and grenade launchers
		// Magazine for rocket and grenade launchers gets deleted after firing
		if (!magazine)
		{
			UpdateMagazineIndicator_Textures(m_WeaponState);		
			UpdateAmmoTypeIndicator(m_WeaponState);
		}
				
		UpdateMagazineIndicator_Count(m_WeaponState);
	}
		
	//------------------------------------------------------------------------------------------------
	void OnAmmoCountChanged_init(BaseWeaponComponent weapon, BaseMagazineComponent magazine)
	{
		BaseMuzzleComponent muzzle;
		
		if (weapon)
			muzzle = weapon.GetCurrentMuzzle();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnAmmoCountChanged_init");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		_print(string.Format("    magazine:     %1", magazine));
		#endif
		
		int ammoCount = 0;
		
		if (magazine)
			ammoCount = magazine.GetAmmoCount();
		
		bool isBarrelChambered = false;
		
		if (muzzle)
			isBarrelChambered = muzzle.IsCurrentBarrelChambered();
		
		OnAmmoCountChanged(weapon, muzzle, magazine, ammoCount, isBarrelChambered);
	}
	void OnAmmoCountChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseMagazineComponent magazine, int ammoCount, bool isBarrelChambered)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnAmmoCountChanged");
		_print(string.Format("    weapon:            %1", weapon));
		_print(string.Format("    muzzle:            %1", muzzle));
		_print(string.Format("    magazine:          %1", magazine));
		_print(string.Format("    ammoCount:         %1", ammoCount));
		_print(string.Format("    isBarrelChambered: %1", isBarrelChambered));
		#endif

		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.AMMOCOUNT;	
		
		m_WeaponState.m_bBarrelChambered = isBarrelChambered;
		
		m_WeaponState.m_iMagAmmoCount = ammoCount;
		m_WeaponState.m_iMagMaxAmmoCount = 1;
		
		if (magazine)
			m_WeaponState.m_iMagMaxAmmoCount = magazine.GetMaxAmmoCount();				
		
		if (m_WeaponState.m_iMagMaxAmmoCount == 0)
			m_WeaponState.m_iMagMaxAmmoCount = 1;
		
		m_WeaponState.m_fMagAmmoPerc = m_WeaponState.m_iMagAmmoCount / m_WeaponState.m_iMagMaxAmmoCount;
		
		if (magazine)
			UpdateMagazineIndicator_Progress(m_WeaponState);
		
		UpdateBulletInChamberIndicator(m_WeaponState);
	}	
	
	//------------------------------------------------------------------------------------------------
	void OnFiremodeChanged_init(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle)
	{
		BaseFireMode firemode;
		
		if (muzzle)
			firemode = muzzle.GetCurrentFireMode();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnFiremodeChanged_init");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		_print(string.Format("    firemode:     %1", firemode));
		#endif		
		
		OnFiremodeChanged(weapon, muzzle, firemode);
	}
	
	void OnFiremodeChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseFireMode firemode)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnFiremodeChanged");
		_print(string.Format("    weapon:   %1", weapon));
		_print(string.Format("    muzzle:   %1", muzzle));
		_print(string.Format("    firemode: %1", firemode));
		#endif	
	
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.FIREMODE;
		
		if (firemode)
			m_WeaponState.m_FireModeType = firemode.GetFiremodeType();
		else
			m_WeaponState.m_FireModeType = EWeaponFiremodeType.Semiauto;
		
		UpdateFireModeIndicator(m_WeaponState);
	}	
	
	//------------------------------------------------------------------------------------------------
	void OnZeroingChanged_init(BaseWeaponComponent weapon)
	{
		int zeroing = 0;
		
		if (weapon)
			zeroing = weapon.GetCurrentSightsZeroing();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnZeroingChanged_init");
		_print(string.Format("    weapon:   %1", weapon));
		_print(string.Format("    zeroing:   %1", zeroing));
		#endif			
		
		OnZeroingChanged(weapon, zeroing);
	}
	void OnZeroingChanged(BaseWeaponComponent weapon, int zeroing)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnZeroingChanged");
		_print(string.Format("    weapon:  %1", weapon));
		_print(string.Format("    zeroing: %1", zeroing));
		#endif
		
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.ZEROING;
		
		m_WeaponState.m_iZeroing = zeroing;
		
		UpdateZeroingIndicator(m_WeaponState);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnADSChanged_init(BaseWeaponComponent weapon)
	{
		bool inADS = false;
		
		if (weapon)
			inADS = weapon.IsSightADSActive();

		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnADSChanged_init");
		_print(string.Format("    weapon:  %1", weapon));
		_print(string.Format("    inADS: %1", inADS));
		#endif		
				
		OnADSChanged(weapon, inADS);
	}
	void OnADSChanged(BaseWeaponComponent weapon, bool inADS)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnADSChanged");
		_print(string.Format("    weapon: %1", weapon));
		_print(string.Format("    inADS:  %1", inADS));
		#endif
		
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.ADS;	
		
		m_WeaponState.m_bInADS = inADS;
		
		#ifndef WEAPON_INFO_BLOCK_WATERFALL_EVENTS		
		OnZoomChanged_init(weapon);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void OnZoomChanged_init(BaseWeaponComponent weapon)
	{
		float zoom = 0;

		SCR_2DSightsComponent sights = GetSights();
		
		// Sights has changed
		if (sights != m_WeaponState.m_Sights)
		{
			// Remove any possible existing "zoom-changed" invokers
			if (m_WeaponState.m_SightsZoomFOVInfo)
				m_WeaponState.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Remove(OnZoomChanged);
			
			m_WeaponState.m_Sights = sights;
			m_WeaponState.m_SightsZoomFOVInfo = null;
			
			if (m_WeaponState.m_Sights)
				m_WeaponState.m_SightsZoomFOVInfo = SCR_SightsZoomFOVInfo.Cast(m_WeaponState.m_Sights.GetFOVInfo());
			
			if (m_WeaponState.m_SightsZoomFOVInfo)
				m_WeaponState.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Insert(OnZoomChanged);			
		}
		
		if (m_WeaponState.m_SightsZoomFOVInfo)
			zoom = m_WeaponState.m_SightsZoomFOVInfo.GetCurrentZoom();

		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnZoomChanged_init");
		_print(string.Format("    weapon:  %1", weapon));
		_print(string.Format("    zoom: %1", zoom));
		#endif		
				
		OnZoomChanged(zoom, -1);
	}	
	void OnZoomChanged(float zoom, float fov)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnZoomChanged");
		_print(string.Format("    zoom: %1", zoom));
		_print(string.Format("    fov: %1", fov));
		#endif	
	
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.ZOOM;			
		
		m_WeaponState.m_fZoom = zoom;
			
		UpdateZoomIndicator(m_WeaponState);
	}	
	
	//------------------------------------------------------------------------------------------------
	void OnInspectionModeChanged(bool state)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnInspectionModeChanged");
		_print(string.Format("    state: %1", state));
		#endif	
	
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.INSPECTION;			
		
		m_WeaponState.m_bInInspectionMode = state;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{		
		if (!m_wRoot)
			return;

		bool menuOpen = m_MenuManager.IsAnyMenuOpen();
		
		if (menuOpen != m_bMenuOpen)
		{
			m_wRoot.SetVisible(!menuOpen);
			m_bMenuOpen = menuOpen;
		}
		
		if (m_eWeaponStateEvent != EWeaponFeature.NONE)
		{
			FadeElements();
						
			m_eWeaponStateEvent = EWeaponFeature.NONE;
			
			m_bFadeInOutActive = true;
		};
		
		if (!m_bFadeInOutActive)
			return;
		
		if (m_ZeroingAnimator)
			m_ZeroingAnimator.Update(timeSlice);
		if (m_WeaponInfoPanelAnimator)
			m_WeaponInfoPanelAnimator.Update(timeSlice);
		
		/*
		if (m_AmmoTypeAnimator)
			m_AmmoTypeAnimator.Update(timeSlice);
		*/
		
		//Print(string.Format("[ANIM STATES] m_WeaponInfoPanelAnimator: %1 | m_ZeroingAnimator: %2 | m_AmmoTypeAnimator: %3", m_WeaponInfoPanelAnimator.GetState(), m_ZeroingAnimator.GetState(), m_AmmoTypeAnimator.GetState()));
		
		m_bFadeInOutActive = m_WeaponInfoPanelAnimator.IsRunning() || m_ZeroingAnimator.IsRunning()/* || m_AmmoTypeAnimator.IsRunning()*/;
	}
	
	
	//------------------------------------------------------------------------------------------------
	void FadeElements()
	{
		// Weapon state debug
		#ifdef WEAPON_INFO_DEBUG_STATES			
		string weaponStateDebug = "Weapon event flags: " + m_eWeaponStateEvent.ToString();

		if (m_eWeaponStateEvent & EWeaponFeature.WEAPON)
			weaponStateDebug += "|WEAPON";			
		
		if (m_eWeaponStateEvent & EWeaponFeature.MUZZLE)
			weaponStateDebug += "|MUZZLE";			
					
		if (m_eWeaponStateEvent & EWeaponFeature.MAGAZINE)
			weaponStateDebug += "|MAGAZINE";
		
		if (m_eWeaponStateEvent & EWeaponFeature.AMMOCOUNT)
			weaponStateDebug += "|AMMOCOUNT";
		
		if (m_eWeaponStateEvent & EWeaponFeature.FIREMODE)
			weaponStateDebug += "|FIREMODE";
		
		if (m_eWeaponStateEvent & EWeaponFeature.ZEROING)
			weaponStateDebug += "|ZEROING";						

		if (m_eWeaponStateEvent & EWeaponFeature.ADS)
			weaponStateDebug += "|ADS";	

		if (m_eWeaponStateEvent & EWeaponFeature.MAGAZINE_COUNT)
			weaponStateDebug += "|MAGAZINE_COUNT";		

		if (m_eWeaponStateEvent & EWeaponFeature.INSPECTION)
			weaponStateDebug += "|INSPECTION";			
						
		if (m_eWeaponStateEvent & EWeaponFeature.ZOOM)
			weaponStateDebug += "|ZOOM";								

		if (m_eWeaponStateEvent & EWeaponFeature.MISC)
			weaponStateDebug += "|MISC";				

		Print(weaponStateDebug);
		#endif
		
		// Get some shared states / conditions for fading
		//bool inspectingWeapon = m_CharacterController && m_CharacterController.GetIsInspectionMode();
		bool isGrenade = m_WeaponState && m_WeaponState.m_bIsGrenade;
		bool inADS = m_WeaponState && m_WeaponState.m_bInADS;
		bool inInspection = m_WeaponState && m_WeaponState.m_bInInspectionMode;
					
		// Main weapon UI (magazine, magazine count, firemode) fadein
		bool panelVisibleEvent = m_eWeaponStateEvent != 0;
		bool panelVisibleOverride = isGrenade || inADS || inInspection;
		if (panelVisibleEvent || panelVisibleOverride)
			m_WeaponInfoPanelAnimator.FadeIn(!panelVisibleOverride);
		
		// Zeroing indicator fadein
		bool zeroingWidgetHasText = !widgets.m_ZeroingText.GetText().IsEmpty();
		bool zeroingVisibleEvent = m_eWeaponStateEvent & (EWeaponFeature.ADS | EWeaponFeature.ZEROING | EWeaponFeature.MUZZLE | EWeaponFeature.WEAPON | EWeaponFeature.INSPECTION);
		bool zeroingVisibleOverride = inADS || inInspection;
		if (zeroingWidgetHasText && (zeroingVisibleEvent || zeroingVisibleOverride))
			m_ZeroingAnimator.FadeIn(!zeroingVisibleOverride);			
		
		// Ammo type fadein
		/*
		bool ammoTypeVisibleEvent = m_eWeaponStateEvent & (EWeaponFeature.ADS | EWeaponFeature.MAGAZINE | EWeaponFeature.MUZZLE | EWeaponFeature.WEAPON | EWeaponFeature.INSPECTION);
		bool ammoTypeVisibleOverride = isGrenade || inADS || inInspection;
		if (m_WeaponState.m_bHasSpecialAmmo && (ammoTypeVisibleEvent || ammoTypeVisibleOverride))
			m_AmmoTypeAnimator.FadeIn(!ammoTypeVisibleOverride);	
		*/
	}

	//------------------------------------------------------------------------------------------------
	SCR_2DSightsComponent GetSights()
	{
		SightsComponent sights = m_WeaponManager.GetCurrentSights();
		
		if (!sights)
			return null;
		
		return SCR_2DSightsComponent.Cast(sights);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_MagazineIndicatorConfiguration GetMagazineConfig(SCR_WeaponState state)
	{
		if (!state)
			return null;
		
		SCR_MagazineIndicatorConfiguration config;
		
		if (m_WeaponState.m_Muzzle)
		{		
			// WEAPON WITH MUZZLE
			if (!m_WeaponState.m_MuzzleUi)
			{
				// No muzzle UI ?! Just hide it
				config = null;
			}
			else
			{
								
				if (m_WeaponState.m_Magazine)
				{
					// MAGAZINE IS IN THE GUN
					config = m_WeaponState.m_MagazineUi.m_MagIndicator;

					if (!config)
						config = m_WeaponState.m_MuzzleUi.m_MagIndicator;
				}
				else
				{
					// NO MAGAZINE
					// Take mag configuration from muzzle
					
					config = m_WeaponState.m_MuzzleUi.m_MagIndicator;
				}
			}
		}
		else
		{
			// NO MUZZLE, probably weapon must provide icon
			if (m_WeaponState.m_WeaponUi)
			{
				config = m_WeaponState.m_WeaponUi.m_MagIndicator;
			}
			else
			{
				config = null;
			}
		}	
	
		return config;
	}
		
	//------------------------------------------------------------------------------------------------
	protected void UpdateMagazineIndicator_Textures(SCR_WeaponState state)
	{
		if (!state)
			return;
		
		if (!state.m_MagazineConfig)
			state.m_MagazineConfig = GetMagazineConfig(state);
		
		SCR_MagazineIndicatorConfiguration config = state.m_MagazineConfig;
		
		if (!config)
		{
			widgets.m_MagazineIndicator.SetVisible(false);
		}
		else
		{
			widgets.m_MagazineIndicator.SetVisible(true);
			
			if (config.m_bProgressBar)
			{
				SetWidgetImage(widgets.m_MagazineOutline, config.m_sImagesetIcons, config.m_sOutline, SCALE_MAGAZINE);
				SetWidgetImage(widgets.m_MagazineBackground, config.m_sImagesetIcons, config.m_sBackground, SCALE_MAGAZINE);
				SetWidgetImage(widgets.m_MagazineGlow, config.m_sImagesetGlows, config.m_sBackground, SCALE_MAGAZINE);			
			}
			else
			{
				SetWidgetImage(widgets.m_MagazineOutline, config.m_sImagesetIcons, config.m_sOutline, SCALE_MAGAZINE);
				SetWidgetImage(widgets.m_MagazineGlow, config.m_sImagesetGlows, config.m_sOutline, SCALE_MAGAZINE);			
			}
			
			if (state.m_Magazine)
			{
				widgets.m_MagazineOutline.SetOpacity(1);
				
				if (config.m_bProgressBar)
				{
					widgets.m_MagazineProgress.SetVisible(true);
					widgets.m_MagazineBackground.SetVisible(true);
					
					SetWidgetImage(widgets.m_MagazineProgress, config.m_sImagesetIcons, config.m_sProgress, SCALE_MAGAZINE);
					widgets.m_MagazineProgress.LoadMaskTexture(config.m_sProgressAlphaMask);
					widgets.m_MagazineProgress.SetMaskMode(ImageMaskMode.REGULAR);
				}
				else
				{
					widgets.m_MagazineProgress.SetVisible(false);
					widgets.m_MagazineBackground.SetVisible(false);
				}
			}
			else
			{
				if (state.m_bIsGrenade)
					widgets.m_MagazineOutline.SetOpacity(1);
				else
					widgets.m_MagazineOutline.SetOpacity(FADED_OPACITY);
				
				widgets.m_MagazineProgress.SetVisible(false);
				widgets.m_MagazineBackground.SetVisible(false);
			}
			
			//AnimateWidget_ColorFlash(widgets.m_MagazineIndicator);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateMagazineIndicator_Progress(SCR_WeaponState state)
	{
		if (!state)
			return;
		
		//AnimateWidget_ColorFlash(widgets.m_MagazineIndicator, EWeaponFeature.AMMOCOUNT);
		
		widgets.m_MagazineProgress.SetMaskProgress(state.m_fMagAmmoPerc);
	}			

	//------------------------------------------------------------------------------------------------
	protected void UpdateMagazineIndicator_Count(SCR_WeaponState state)
	{
		if (!state)
			return;

		int count = state.m_iMagCount;
		
		// Add +1 if weapon is a grenade, so it shows the total # of grenades in possession
		if (state.m_bIsGrenade)
			count++;

		string countText = count.ToString();
		
		/*
		if (!state.m_bIsGrenade)
			countText = "+" + countText;
		*/
		
		if (count > 0)
			widgets.m_MagCountText.SetOpacity(1);
		else
			widgets.m_MagCountText.SetOpacity(FADED_OPACITY);
		
		if (widgets.m_MagCountText.GetText() == countText) 
			return;

		widgets.m_MagCountText.SetText(countText);		
		
		AnimateWidget_ColorFlash(widgets.m_MagCountText, EWeaponFeature.MAGAZINE_COUNT);	
		AnimateWidget_TextPopUp(widgets.m_MagCountText, 36, 54, EWeaponFeature.MAGAZINE_COUNT);
	}		
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateBulletInChamberIndicator(SCR_WeaponState state)
	{
		if (!state)
			return;

		if (state.m_bBarrelChambered)
			widgets.m_FiremodeIcon.SetOpacity(1);
		else
			widgets.m_FiremodeIcon.SetOpacity(FADED_OPACITY);
	}		
	
	//------------------------------------------------------------------------------------------------
	void UpdateFireModeIndicator(SCR_WeaponState state)
	{
		if (!state)
			return;

		widgets.m_FiremodeIcon.SetVisible(false);
		widgets.m_FiremodeGlow.SetVisible(false);

		if (state.m_Muzzle && state.m_MuzzleUi && state.m_MuzzleUi.ShowFiremodeIcon())
		{
			auto fm = state.m_FireModeType;
			
			SetWidgetImage(widgets.m_FiremodeIcon, state.m_MuzzleUi.GetFiremodeIconImageset(), state.m_MuzzleUi.GetFiremodeIconName(fm), SCALE_FIREMODE);
			SetWidgetImage(widgets.m_FiremodeGlow, state.m_MuzzleUi.GetFiremodeGlowImageset(), state.m_MuzzleUi.GetFiremodeIconName(fm), SCALE_FIREMODE);
		
			widgets.m_FiremodeIcon.SetVisible(true);
			widgets.m_FiremodeGlow.SetVisible(true);
			
			AnimateWidget_ColorFlash(widgets.m_FiremodeIcon, EWeaponFeature.FIREMODE);
		}		
	}

	//------------------------------------------------------------------------------------------------
	void UpdateZeroingIndicator(SCR_WeaponState state)
	{
		if (!state)
			return;

		int zeroing = state.m_iZeroing;

		if (zeroing <= 0)
		{
			widgets.m_Zeroing.SetVisible(false);
			widgets.m_ZeroingText.SetText(string.Empty);
		}
		else
		{
			widgets.m_Zeroing.SetVisible(true);
			string sZeroing = string.Format("%1 m", zeroing);
			widgets.m_ZeroingText.SetText(sZeroing);
			
			AnimateWidget_ColorFlash(widgets.m_Zeroing, EWeaponFeature.ZEROING);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateZoomIndicator(SCR_WeaponState state)
	{
		if (!state)
			return;

		float zoom = state.m_fZoom;

		if (zoom <= 0 || !state.m_bInADS)
		{
			widgets.m_Optics.SetVisible(false);
			widgets.m_OpticsText.SetText(string.Empty);
		}
		else
		{
			string sZoom = zoom.ToString(-1,1);
			sZoom = string.Format("%1 ×", sZoom);

			widgets.m_Optics.SetVisible(true);
			widgets.m_OpticsText.SetText(sZoom);
			
			AnimateWidget_ColorFlash(widgets.m_Optics, EWeaponFeature.ZOOM);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	void UpdateAmmoTypeIndicator(SCR_WeaponState state)
	{
		if (!state)
			return;
		
		if (!state.m_bHasSpecialAmmo || (!state.m_MagazineUi && !state.m_GrenadeUi))
		{
			widgets.m_AmmoType.SetVisible(false);
			return;
		}

		string sAmmoTypeText;
		bool bShowAmmoTypeText;
		
		if (state.m_GrenadeUi)
		{
			sAmmoTypeText = state.m_GrenadeUi.GetAmmoType();
			bShowAmmoTypeText = state.m_GrenadeUi.ShowAmmoTypeText();
		}
		else
		{
			sAmmoTypeText = state.m_MagazineUi.GetAmmoType();
			bShowAmmoTypeText = state.m_MagazineUi.ShowAmmoTypeText();
		}
		
		if (!bShowAmmoTypeText)
			sAmmoTypeText = "";
		
		widgets.m_AmmoTypeText.SetText(sAmmoTypeText);

		widgets.m_AmmoType.SetVisible(true);
				
		widgets.m_AmmoType_FMJ.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.FMJ);
		widgets.m_AmmoType_AP.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.AP);
		widgets.m_AmmoType_Frag.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.FRAG);
		widgets.m_AmmoType_Smoke.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.SMOKE);
		widgets.m_AmmoType_HE.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.HE);
		widgets.m_AmmoType_HEAT.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.HEAT);
		widgets.m_AmmoType_Incendiary.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.INCENDIARY);
		widgets.m_AmmoType_Tracer.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.TRACER);
		widgets.m_AmmoType_Sniper.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.SNIPER);
		
		//AnimateWidget_ColorFlash(widgets.m_AmmoType);
	}
	
	//------------------------------------------------------------------------------------------------
	void AnimateWidget_ColorFlash(Widget w, EWeaponFeature requiredFlag = -1, float speed = WidgetAnimator.FADE_RATE_SLOW)
	{
		if (!w)
			return;
		
		if (requiredFlag != -1 && m_eWeaponStateEvent != requiredFlag)
			return;
		
		w.SetColor(COLOR_ORANGE);
		WidgetAnimator.PlayAnimation(w, WidgetAnimationType.Color, COLOR_WHITE, speed);
	}	

	//------------------------------------------------------------------------------------------------
	void AnimateWidget_TextPopUp(Widget w, float size, float sizeBoosted, EWeaponFeature requiredFlag = -1, float speed = WidgetAnimator.FADE_RATE_SLOW)
	{
		if (!w)
			return;
		
		if (requiredFlag != -1 && m_eWeaponStateEvent != requiredFlag)
			return;
		
		float width = FrameSlot.GetSizeX(w);
		
		FrameSlot.SetSize(w, width, sizeBoosted);
		WidgetAnimator.PlayAnimation(w, WidgetAnimationType.FrameSize, speed, width, size);
	}		
		
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner);
		if (!character)
			return false;

		// Detect and store weapon manager
		m_WeaponManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
		if (!m_WeaponManager)
			return false;

		// Store character contoller
		m_CharacterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		if (!m_CharacterController)
			return false;

		
		m_CompartmentAccess = CompartmentAccessComponent.Cast(owner.FindComponent(CompartmentAccessComponent));
		if (!m_CompartmentAccess)
			return false;
		
		m_MenuManager = GetGame().GetMenuManager();
		if (!m_MenuManager)
			return false;
		
		m_EventHandlerManager = EventHandlerManagerComponent.Cast(character.FindComponent(EventHandlerManagerComponent));
		if (m_EventHandlerManager)
		{
			m_EventHandlerManager.RegisterScriptHandler("OnWeaponChanged", this, OnWeaponChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnMuzzleChanged", this, OnMuzzleChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnMagazineChanged", this, OnMagazineChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnMagazineCountChanged", this, OnMagazineCountChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnAmmoCountChanged", this, OnAmmoCountChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnFiremodeChanged", this, OnFiremodeChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnZeroingChanged", this, OnZeroingChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnADSChanged", this, OnADSChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnInspectionModeChanged", this, OnInspectionModeChanged, true);
		}
		
		OnWeaponChanged(m_WeaponManager.GetCurrentWeapon(), null);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		if (m_EventHandlerManager)
		{
			m_EventHandlerManager.RemoveScriptHandler("OnWeaponChanged", this, OnWeaponChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnMuzzleChanged", this, OnMuzzleChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnMagazineChanged", this, OnMagazineChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnMagazineCountChanged", this, OnMagazineCountChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnAmmoCountChanged", this, OnAmmoCountChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnFiremodeChanged", this, OnFiremodeChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnZeroingChanged", this, OnZeroingChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnADSChanged", this, OnADSChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnInspectionModeChanged", this, OnInspectionModeChanged);
		}
		m_EventHandlerManager = null;
		
		DestroyLayout();
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateLayout(string layout)
	{		
		// Destroy existing layout
		DestroyLayout();
		
		// Create weapon info layout
		SCR_HUDManagerComponent manager = SCR_HUDManagerComponent.GetHUDManager();
		if (manager)
			m_wRoot = manager.CreateLayout(layout, m_eLayer);

		if (!m_wRoot)
			return;
		
		widgets = new SCR_WeaponInfoWidgets();
		widgets.Init(m_wRoot);
		
		widgets.m_MagazineIndicator.SetVisible(true);
		
		if (!m_WeaponInfoPanelAnimator)
			m_WeaponInfoPanelAnimator = new SCR_FadeInOutAnimator(widgets.m_WeaponInfoPanel, WidgetAnimator.FADE_RATE_FAST, WidgetAnimator.FADE_RATE_SLOW, FADEOUT_PANEL_DELAY, true);

		if (!m_ZeroingAnimator)
			m_ZeroingAnimator = new SCR_FadeInOutAnimator(widgets.m_Zeroing, WidgetAnimator.FADE_RATE_FAST, WidgetAnimator.FADE_RATE_SLOW, FADEOUT_OPTICS_DELAY, true);
		
		/*
		if (!m_AmmoTypeAnimator)
			m_AmmoTypeAnimator = new SCR_FadeInOutAnimator(widgets.m_AmmoType, WidgetAnimator.FADE_RATE_FAST, WidgetAnimator.FADE_RATE_SLOW, FADEOUT_AMMO_TYPE_DELAY, true);
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	void DestroyLayout()
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		
		m_WeaponInfoPanelAnimator = null;
		m_ZeroingAnimator = null;
		//m_AmmoTypeAnimator = null;
		
		m_wRoot = null;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Returns player's inventory manager, or vehicle's inv manager if player is in vehicle
	protected static InventoryStorageManagerComponent GetInventoryManager()
	{		
		PlayerController pc = GetGame().GetPlayerController();
			
		if (!pc)
			return null;
		
		IEntity playerEntity = pc.GetControlledEntity();
		
		if (!playerEntity)
			return null;
		
		InventoryStorageManagerComponent invManager = InventoryStorageManagerComponent.Cast(playerEntity.FindComponent(InventoryStorageManagerComponent));
		
		CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(playerEntity.FindComponent(CompartmentAccessComponent));
		
		if (!compAccess)
			return invManager;
		
		BaseCompartmentSlot compSlot = compAccess.GetCompartment();
		
		if (!compSlot)
			return invManager;

		IEntity vehEntity = compSlot.GetVehicle();
		InventoryStorageManagerComponent vehInvMgr = InventoryStorageManagerComponent.Cast(
			vehEntity.FindComponent(InventoryStorageManagerComponent));
		
		if (!vehInvMgr)
		{
			vehEntity = compSlot.GetOwner();
			vehInvMgr = InventoryStorageManagerComponent.Cast(vehEntity.FindComponent(InventoryStorageManagerComponent));		
		}
		
		if (!vehInvMgr)
			return null;
		
		return vehInvMgr;
	}
	
	//------------------------------------------------------------------------------------------------
	protected BaseWeaponManagerComponent GetWeaponManager(IEntity owner)	
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner);
		if (!character)
			return null;

		// Detect and store weapon manager
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return null;
		
		return weaponManager;
	}
		
	//---------------------------------------------------------------------------------------------------------
	//! Sets widget's image to an image or imageset
	protected static void SetWidgetImage(ImageWidget w, string imageOrImageset, string imageName = "", float scale = SCALE_DEFAULT)
	{
		if (!imageName.IsEmpty())
		{
			// Assume it's an image set
			w.LoadImageFromSet(0, imageOrImageset, imageName);
		}
		else if (!imageOrImageset.IsEmpty())
		{
			// Assume it's an image
			w.LoadImageTexture(0, imageOrImageset);
		}
		
		// Perform scaling
		int sx, sy;
		w.GetImageSize(0, sx, sy);
		w.SetSize(sx * scale, sy * scale);
	}
};