[EntityEditorProps(category: "GameScripted/Gadgets", description: "Flashlight", color: "0 0 255 255")]
class SCR_FlashlightComponentClass: SCR_GadgetComponentClass
{
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), BaseContainerCustomTitleField("m_sDescription")]
class SCR_LenseColor
{
	[Attribute("", UIWidgets.EditBox, desc: "Lense type", category: "Flashlight")]
	string m_sDescription;
	
	[Attribute("1 1 1", UIWidgets.ColorPicker, desc: "Lense color", category: "Flashlight")]
	vector m_vLenseColor;
	
	[Attribute("5.0", UIWidgets.Slider, desc: "Light Value", "-8.0 20 1", category: "Flashlight")]
	float m_fLightValue;
	
	[Attribute("", UIWidgets.GraphDialog, desc: "Light EV during the day, abscissa is the day time and ordinate is the EV. EV is subtracted by 10 so the EV value is between [-10,10] instead of [0,20]", params: "24 20 0 0", category: "Flashlight")]
	ref Curve m_LightEVCurve;
};

//------------------------------------------------------------------------------------------------
class SCR_FlashlightComponent : SCR_GadgetComponent
{	
	const float SHADOW_NP_HAND = 0.5;
	const float SHADOW_NP_STRAPPED = 1;
	const float SHADOW_NP_VEHICLE = 0.1;
	
	[Attribute("50", UIWidgets.EditBox, desc: "Intensity of the emmissive texture", params: "0 1000", category: "Flashlight")]
	protected float m_fEmissiveIntensity;
			
	[Attribute("", UIWidgets.Object, desc: "Array of lense colors", category: "Flashlight")]
	protected ref array<ref SCR_LenseColor> m_LenseArray; 
	
	[Attribute("SOUND_FLASHLIGHT_ON", UIWidgets.EditBox, desc: "Activation sound", category: "Sound")]
	protected string m_sSoundActivation;
	
	[Attribute("SOUND_FLASHLIGHT_OFF", UIWidgets.EditBox, desc: "Deactivation sound", category: "Sound")]
	protected string m_sSoundDeactivation;
	
	protected bool m_bLastLightState;	// remember the last light state for cases where you put your flashlight into storage but there is no slot available -> true = active
	protected int m_iCurrentLenseColor = 0;	
	protected LightEntity m_Light;
	protected SoundComponent m_SoundComp;
	ParametricMaterialInstanceComponent m_EmissiveMaterial;
	protected SCR_CompartmentAccessComponent m_CompartmentComp;
					
	//------------------------------------------------------------------------------------------------
	override void OnToggleActive(bool state)
	{		
		m_bActivated = state;

		if (m_SoundComp)
		{
			if (m_bActivated)
				m_SoundComp.SoundEvent(m_sSoundActivation);
			else 
				m_SoundComp.SoundEvent(m_sSoundDeactivation);
		}
		
		if (state)
			SetEventMask(GetOwner(), EntityEvent.FRAME);
		else
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
		
		UpdateLightState();
	}
			
	//------------------------------------------------------------------------------------------------
	//! Update state of light ON/OFF
	protected void UpdateLightState()
	{
		if (m_bActivated)
			EnableLight();
		else 
			DisableLight();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Switch flashlight lenses
	void SwitchLenses(int filter)
	{
		// Change color on activation
		if (m_iCurrentLenseColor == filter)
			return;
		
		m_iCurrentLenseColor = filter;
		
		// Switch only
		if ( !m_Light.IsEnabled() )
			return;
		
		// Switch and toggle
		UpdateLightValue();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn light entity
	protected void EnableLight()
	{	
		if (m_Light)	
			m_Light.SetEnabled(true);
		
		if (m_EmissiveMaterial)
			m_EmissiveMaterial.SetEmissiveMultiplier(m_fEmissiveIntensity);
		
		SwitchLenses(0);	// TODO lens switch support
		
		if (m_iMode == EGadgetMode.IN_SLOT)
			SetShadowNearPlane(true);
		else if (m_iMode == EGadgetMode.IN_HAND)
			SetShadowNearPlane(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove light
	protected void DisableLight()
	{
		if (m_Light)
			m_Light.SetEnabled(false);
		
		if (m_EmissiveMaterial)
			m_EmissiveMaterial.SetEmissiveMultiplier(0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set near plane of light shadow to avoid blocking the light with arm/weapon
	//! \param state determines mode: true - in hand / false - strapped
	//! \param isLeavingVehicle is signal for removing vehicle near plane mode
	protected void SetShadowNearPlane(bool state, bool isLeavingVehicle = false)
	{				
		float nearPlane;
		
		if (state)	// strapped mode
		{
			nearPlane = SHADOW_NP_STRAPPED;
			if (m_CompartmentComp && m_CompartmentComp.IsInCompartment() && !isLeavingVehicle)
			{
				if (m_CompartmentComp.IsInCompartment())
					nearPlane = SHADOW_NP_VEHICLE;
			}
		}
		else		// in hand mode
			nearPlane = SHADOW_NP_HAND;
		
		if (m_Light)
		{
			m_Light.SetNearPlane(nearPlane);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!m_bActivated)
			return;
		
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID);
		IEntity occupant = compSlot.GetOccupant();
		if (occupant == m_CharacterOwner)
			SetShadowNearPlane(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!m_bActivated)
			return;
		
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID);
		IEntity occupant = compSlot.GetOccupant();
		if (occupant == m_CharacterOwner)
			SetShadowNearPlane(true, true);
	}
			
	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{		
		super.ModeSwitch(mode, charOwner);
		
		if (mode == EGadgetMode.ON_GROUND || mode == EGadgetMode.IN_STORAGE)
		{
			m_bLastLightState = m_bActivated;
			
			if (m_bActivated)
				ToggleActive(false);
			
			if (mode == EGadgetMode.ON_GROUND)	// always set the last state as false when removing from inventory
				m_bLastLightState = false;
				
		}
		else if (mode == EGadgetMode.IN_SLOT)
		{					
			m_CompartmentComp = SCR_CompartmentAccessComponent.Cast(m_CharacterOwner.FindComponent(SCR_CompartmentAccessComponent));
			if (m_CompartmentComp)
			{				
				m_CompartmentComp.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
				m_CompartmentComp.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
			}
			
			SetShadowNearPlane(true);
		}
		else if (mode == EGadgetMode.IN_HAND)
		{			
			SetShadowNearPlane(false);
			
			if (m_bLastLightState)
				ToggleActive(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{
		if (mode == EGadgetMode.IN_SLOT)
		{					
			if (m_CompartmentComp)
			{				
				m_CompartmentComp.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
				m_CompartmentComp.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
			}
			
			SetShadowNearPlane(true);
		}
		
		super.ModeClear(mode);
	}
			
	//------------------------------------------------------------------------------------------------
	override void ToggleActive(bool state)
	{
		if (m_iMode == EGadgetMode.IN_STORAGE && !m_bActivated)	// trying to activate flashlight hidden in inventory
			return;	
		
		super.ToggleActive(state);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ActivateAction()
	{
		ToggleActive(!m_bActivated);
	}
		
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.FLASHLIGHT;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeToggled()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsVisibleEquipped()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool RplSave(ScriptBitWriter writer)
    {
		if (!super.RplSave(writer))
			return false;
		
        writer.WriteBool(m_bActivated);
       
		return true;
    }
     
	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		if (!super.RplLoad(reader))
			return false;
		
        reader.ReadBool(m_bActivated);
				
		UpdateLightState();
		
        return true;
    }
	
	protected void UpdateLightValue()
	{
		if (m_LenseArray.Count() == 0)
			return;
		
		TimeAndWeatherManagerEntity timeManager = GetGame().GetTimeAndWeatherManager();
		if (!timeManager)
			return;
		
		m_Light.SetColor(Color.FromVector(m_LenseArray[m_iCurrentLenseColor].m_vLenseColor), m_LenseArray[m_iCurrentLenseColor].m_fLightValue);
		float lightEV = GetPoint(timeManager.GetTimeOfTheDay(), m_LenseArray[m_iCurrentLenseColor].m_LightEVCurve);
		m_Light.SetIntensityEVClip(lightEV - 10.0);
	}
	
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		UpdateLightValue();
	}
					
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{		
		m_Light =  LightEntity.Cast( owner.GetChildren() );
		m_SoundComp = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		m_EmissiveMaterial = ParametricMaterialInstanceComponent.Cast(owner.FindComponent(ParametricMaterialInstanceComponent));

		// Insert default entry if none are set
		if (m_LenseArray.Count() == 0)
		{
			SCR_LenseColor defColor = new SCR_LenseColor();
			defColor.m_sDescription = "CONFIGURE ME";
			m_LenseArray.Insert(defColor);
		}
	}
	
	protected float GetPoint(float time, Curve curve)
	{		
		time = Math.Clamp(time, 0.0, 24.0);
		
		int size = curve.Count();
		if (size == 0)
			return 0.0;
		
		if (size == 1)
			return curve[0][1];
		
		if (time < curve[0][0])
			return curve[0][1];
		
		int i;
		for (i = 0; i < size - 1; i++)
		{
			float timeMin = curve[i][0];
			float timeMax = curve[i+1][0];
			if (time >= timeMin && time <= timeMax)
				return Math.Lerp(curve[i][1], curve[i+1][1], (time - timeMin) / (timeMax - timeMin));
		}
		
		return curve[i][1];
	}
};