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
};

//------------------------------------------------------------------------------------------------
class SCR_FlashlightComponent : SCR_GadgetComponent
{		
	[Attribute("50", UIWidgets.EditBox, desc: "Intensity of the emmissive texture", params: "0 1000", category: "Flashlight")]
	protected float m_fEmissiveIntensity;
			
	[Attribute("", UIWidgets.Object, desc: "Array of lense colors", category: "Flashlight")]
	protected ref array<ref SCR_LenseColor> m_LenseArray; 
	
	[Attribute("0.5", UIWidgets.EditBox, desc: "In meters, light ignores objects up to set distance to avoid unwanted shadows", params: "0.1 5", category: "Light near plane")]
	protected float m_fLightNearPlaneHand;
	
	[Attribute("1.2", UIWidgets.EditBox, desc: "In meters, light ignores objects up to set distance to avoid unwanted shadows", params: "0.1 5", category: "Light near plane")]
	protected float m_fLightNearPlaneStrapped;
	
	[Attribute("0.1", UIWidgets.EditBox, desc: "In meters, light ignores objects up to set distance to avoid unwanted shadows", params: "0.1 5", category: "Light near plane")]
	protected float m_fLightNearPlaneVehicle;
	
	[Attribute(SCR_SoundEvent.SOUND_FLASHLIGHT_ON, UIWidgets.EditBox, desc: "Activation sound", category: "Sound")]
	protected string m_sSoundActivation;
	
	[Attribute(SCR_SoundEvent.SOUND_FLASHLIGHT_OFF, UIWidgets.EditBox, desc: "Deactivation sound", category: "Sound")]
	protected string m_sSoundDeactivation;
	
	protected bool m_bLastLightState;	// remember the last light state for cases where you put your flashlight into storage but there is no slot available -> true = active
	protected bool m_bIsSlottedVehicle;	// char owner is in vehicle while flashlight is slotted
	protected int m_iCurrentLenseColor;	
	protected vector m_vAnglesCurrent;	// current local angles
	protected vector m_vAnglesTarget;	// target local angles used to align flashlight with aiming angles while strapped
	protected vector m_ItemMat[4];		// owner transformation matrix
	protected LightEntity m_Light;
	protected SoundComponent m_SoundComp;
	protected ParametricMaterialInstanceComponent m_EmissiveMaterial;
	protected SCR_CompartmentAccessComponent m_CompartmentComp;
	protected CharacterControllerComponent m_CharController;
					
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
	protected void SwitchLenses(int filter)
	{
		m_iCurrentLenseColor = filter;
		
		m_Light.SetColor(Color.FromVector(m_LenseArray[m_iCurrentLenseColor].m_vLenseColor), m_LenseArray[m_iCurrentLenseColor].m_fLightValue);
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
		
		DeactivateGadget();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set near plane of light shadow to avoid blocking the light with arm/weapon
	//! \param state determines mode: false - in hand / true - strapped
	//! \param isLeavingVehicle is signal for removing vehicle near plane mode
	protected void SetShadowNearPlane(bool state, bool isLeavingVehicle = false)
	{				
		float nearPlane;
		m_bIsSlottedVehicle = false;
		
		if (state)	// strapped mode
		{
			nearPlane = m_fLightNearPlaneStrapped;
			if (m_CompartmentComp && m_CompartmentComp.IsInCompartment() && !isLeavingVehicle)
			{
				if (m_CompartmentComp.IsInCompartment())
				{
					nearPlane = m_fLightNearPlaneVehicle;
					m_bIsSlottedVehicle = true;
				}
			}
			
			if (m_bActivated)
				ActivateGadget();
		}
		else		// in hand mode
			nearPlane = m_fLightNearPlaneHand;
		
		if (m_Light)
		{
			m_Light.SetNearPlane(nearPlane);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!m_bActivated)
			return;
		
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		IEntity occupant = compSlot.GetOccupant();
		if (occupant == m_CharacterOwner)
			SetShadowNearPlane(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!m_bActivated)
			return;
		
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		IEntity occupant = compSlot.GetOccupant();
		if (occupant == m_CharacterOwner)
			SetShadowNearPlane(true, true);
	}
			
	//------------------------------------------------------------------------------------------------
	override protected void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{		
		super.ModeSwitch(mode, charOwner);

		if (mode == EGadgetMode.ON_GROUND)
			m_CharController = null;
		else
			m_CharController = CharacterControllerComponent.Cast( m_CharacterOwner.FindComponent(CharacterControllerComponent) );
		
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
	override protected void ModeClear(EGadgetMode mode)
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
	override protected void ToggleActive(bool state)
	{
		if (m_iMode == EGadgetMode.IN_STORAGE && !m_bActivated)	// trying to activate flashlight hidden in inventory
			return;	
		
		super.ToggleActive(state);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ActivateAction()
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
	
	//------------------------------------------------------------------------------------------------
	override bool OnTicksOnRemoteProxy() 
	{ 
		return true; // proxies will only tick on owners without this
	};
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{		
		if (m_iMode != EGadgetMode.IN_SLOT)
			return;
		
		if (m_bIsSlottedVehicle || !m_CharController)
		{
			GetOwner().SetYawPitchRoll(vector.Zero); // vehicle aim angles are inconsistent, leave the local angles in default state
			return;
		}
		
		if (m_CharController.IsDead())	// deactivate frame on death without turning the light off
		{
			DeactivateGadget();
			return;
		}
				
		// ajdust angle of the flashlight to provide usable angle within various poses
		GetOwner().GetTransform(m_ItemMat);
		m_vAnglesCurrent = GetOwner().GetLocalYawPitchRoll();
		m_vAnglesTarget = ( m_CharController.GetAimingAngles() - Math3D.MatrixToAngles(m_ItemMat) ) + m_vAnglesCurrent;	 // diff between WS aiming and item angles, add local to the result
		m_vAnglesTarget[0] = fixAngle_180_180(m_vAnglesTarget[0]);		
		m_vAnglesTarget[1] = Math.Clamp(m_vAnglesTarget[1], 0, 30);	// only need too offset upwards, also avoid glitches with some stances
		m_vAnglesTarget[2] = 0;	// no roll		
		
		m_vAnglesTarget[0] = Math.Lerp(m_vAnglesCurrent[0], m_vAnglesTarget[0], timeSlice*4);
		m_vAnglesTarget[1] = Math.Lerp(m_vAnglesCurrent[1], m_vAnglesTarget[1], timeSlice*4);
		GetOwner().SetYawPitchRoll(m_vAnglesTarget); // sets local angles
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
};