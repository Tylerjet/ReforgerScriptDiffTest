[EntityEditorProps(category: "GameScripted/BaseInteractiveLightComponent", description: "Handling behaviour of interactive (can be turned on / off) light on prefabs.")]
class SCR_BaseInteractiveLightComponentClass : SCR_BaseLightComponentClass
{
	[Attribute("", UIWidgets.Object, "", category: "")]
	private ref array<ref SCR_BaseLightData> m_aLightData;
	
	[Attribute("0", UIWidgets.CheckBox, "Gradual lighting?")]
	private bool m_bGradualLightning;

	[Attribute("0", UIWidgets.EditBox, "LV of light effect")]
	private float m_fLV;
	
	//------------------------------------------------------------------------------------------------
	float GetLightLV()
	{
		return m_fLV;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsGradualLightningOn()
	{
		return m_bGradualLightning;
	}
	
	//------------------------------------------------------------------------------------------------
	ref array<ref SCR_BaseLightData> GetLightData()
	{
		return m_aLightData;
	}
};

enum ELightState
{
	LIT = 0,
	LIT_ON_SPAWN = 1,
	OFF = 2,
};

class SCR_BaseInteractiveLightComponent : SCR_BaseLightComponent
{	
	protected ref array<LightEntity> m_aLights;
	protected SoundComponent m_SoundComponent;
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected ParametricMaterialInstanceComponent m_EmissiveMaterialComponent;
	protected float m_fCurLV;
	protected float m_fCurEM;
	protected float m_fLightEmisivityStep;
	protected float m_fSoundSignalStep;
	protected float m_fSoundSignalValue;
	protected bool m_bIsOn;
	
	private bool m_bUpdate;
	
	protected const static float MATERIAL_EMISSIVITY_STEP = 0.1;
	protected const static int MATERIAL_EMISSIVITY_ON = 30;
	protected const static float LIGHT_EMISSIVITY_START = 0.1;
	protected const static int MATERIAL_EMISSIVITY_START = 1;
	protected const static int LIGHT_EMISSIVITY_OFF = 0;	
	protected const static int UPDATE_LIGHT_TIME = 100;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ELightState))]
	private ELightState m_eInitialLightState;
		
	//------------------------------------------------------------------------------------------------
	bool GetInitialState()
	{		
		switch (m_eInitialLightState)
		{
			case ELightState.LIT:
				return true;
			
			case ELightState.LIT_ON_SPAWN:
				if (GetOwner().IsLoaded())
					return false;
				return true;
				
			case ELightState.OFF:
				return false;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_bIsOn;
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleLight(bool turnOn, bool skipTransition = false, bool playSound = true)
	{
		if (!GetGame().InPlayMode() || m_bIsOn == turnOn)
			return;	
		
		SCR_BaseInteractiveLightComponentClass componentData = SCR_BaseInteractiveLightComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData) 
			return;
		
		if (turnOn)
		{			
			TurnOn(componentData, skipTransition, playSound);
		}
		else
		{
			TurnOff(componentData, playSound);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TurnOn(notnull SCR_BaseInteractiveLightComponentClass componentData, bool skipTransition, bool playSound)
	{
		if (!System.IsConsoleApp() && playSound && m_SoundComponent)
		{
			m_SoundComponent.SoundEvent(SCR_SoundEvent.SOUND_TURN_ON);
		}

		LightEntity light;
		IEntity owner = GetOwner();
		vector lightOffset;
		vector lightDirection;
		vector pos;
		
		if (!m_aLights)
			m_aLights = {};
		
		for (int i = 0, count = componentData.GetLightData().Count(); i < count; i++)
		{			
			if (!componentData.GetLightData()[i])
				continue;
			
			vector mat[4];
			owner.GetTransform(mat);
			
			lightOffset = componentData.GetLightData()[i].GetLightOffset();
			lightDirection = (componentData.GetLightData()[i].GetLightConeDirection().Multiply4(mat) - lightOffset.Multiply4(mat)).Normalized();
			pos = owner.GetOrigin() + lightOffset;
			
			light = CreateLight(componentData.GetLightData()[i], pos, lightDirection, LIGHT_EMISSIVITY_START);
			if (!light)
				continue;
						
			owner.AddChild(light, -1, EAddChildFlags.AUTO_TRANSFORM | EAddChildFlags.RECALC_LOCAL_TRANSFORM);
			light.SetFlags(EntityFlags.PROXY);
			m_aLights.Insert(light);
		}

		m_bIsOn = true;
		m_bUpdate = true;

		// Skip transition phase of the light.		
		if (skipTransition || !componentData.IsGradualLightningOn())
		{
			if (m_EmissiveMaterialComponent)
				m_EmissiveMaterialComponent.SetEmissiveMultiplier(MATERIAL_EMISSIVITY_ON);

			for (int i = 0, count = m_aLights.Count(); i < count; i++)
			{
				m_aLights[i].SetColor(Color.FromVector(componentData.GetLightData()[i].GetLightColor()), componentData.GetLightLV());
			}

			if (m_SignalsManagerComponent)
				m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("Trigger"), 1);

			return;
		}
		
		m_fCurLV = LIGHT_EMISSIVITY_START;
		m_fCurEM = MATERIAL_EMISSIVITY_START;
		m_fLightEmisivityStep = componentData.GetLightLV() / ((MATERIAL_EMISSIVITY_ON - MATERIAL_EMISSIVITY_START) / MATERIAL_EMISSIVITY_STEP);
		m_fSoundSignalStep = 1 / ((MATERIAL_EMISSIVITY_ON - MATERIAL_EMISSIVITY_START) / MATERIAL_EMISSIVITY_STEP);
		m_fSoundSignalValue = 0;
		
		SetEventMask(GetOwner(), EntityEvent.VISIBLE);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TurnOff(notnull SCR_BaseInteractiveLightComponentClass componentData, bool playSound)
	{
		if (!System.IsConsoleApp() && playSound && m_SoundComponent)
		{
			m_SoundComponent.SoundEvent(SCR_SoundEvent.SOUND_TURN_OFF);
			if (m_SignalsManagerComponent)
				m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("Trigger"), 0);
		}
		
		if (!componentData.GetLightData().IsEmpty())
		{				
			RemoveLights();
				
			if (m_EmissiveMaterialComponent)
				m_EmissiveMaterialComponent.SetEmissiveMultiplier(LIGHT_EMISSIVITY_OFF);
		}
			
		ClearEventMask(GetOwner(), EntityEvent.VISIBLE);
		m_bIsOn = false;
		m_bUpdate = false;
	}	
	
	//------------------------------------------------------------------------------------------------
	override void EOnVisible(IEntity owner, int frameNumber)
	{
		if (!m_bUpdate)
			return;
		
		SCR_BaseInteractiveLightComponentClass componentData = SCR_BaseInteractiveLightComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData) 
			return;		
		
		if (m_SignalsManagerComponent)
		{
			m_fSoundSignalValue += m_fSoundSignalStep;
			m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("Trigger"), m_fSoundSignalValue);
		}

		if (!m_aLights)
			return;
		
		bool shouldUpdate = true;
		
		for (int i = 0, count = Math.Min(componentData.GetLightData().Count(), m_aLights.Count()); i < count; i++)
		{
			if (m_fLightEmisivityStep > 0.0 && m_fCurLV < componentData.GetLightLV())
				UpdateLightEmissivity(componentData.GetLightData()[i], i);
			
			if (m_EmissiveMaterialComponent && m_fCurEM < MATERIAL_EMISSIVITY_ON)
				UpdateMaterialEmissivity();
			
			shouldUpdate &= !(m_fCurLV >= componentData.GetLightLV() && (!m_EmissiveMaterialComponent || m_fCurEM > MATERIAL_EMISSIVITY_ON));
		}
		
		m_bUpdate = shouldUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	private void UpdateLightEmissivity(notnull SCR_BaseLightData lightData, int lightArrayIndex)
	{
		m_fCurLV += m_fLightEmisivityStep;
		m_aLights[lightArrayIndex].SetColor(Color.FromVector(lightData.GetLightColor()), m_fCurLV);
	}
	
	//------------------------------------------------------------------------------------------------
	private void UpdateMaterialEmissivity()
	{
		m_fCurEM += MATERIAL_EMISSIVITY_STEP;
		m_EmissiveMaterialComponent.SetEmissiveMultiplier(m_fCurEM);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		super.RplSave(writer);
		
		writer.WriteBool(IsOn());
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		super.RplLoad(reader);
		
		bool isOn;
		reader.ReadBool(isOn);
		ToggleLight(isOn, true, false);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{		
		super.OnPostInit(owner);
		
		m_SoundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		m_EmissiveMaterialComponent = ParametricMaterialInstanceComponent.Cast(owner.FindComponent(ParametricMaterialInstanceComponent));
		
		ToggleLight(GetInitialState(), true, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveLights()
	{
		if (m_aLights && !m_aLights.IsEmpty())
		{
			for (int i = 0, count = m_aLights.Count(); i < count; i++)
			{
				delete m_aLights[i];
			}
		}
		
		if (m_aLights)
		{
			m_aLights.Clear();
			m_aLights = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		RemoveLights();
	}
};