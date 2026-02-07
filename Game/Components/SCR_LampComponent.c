[EntityEditorProps(category: "GameScripted", color: "0 0 255 255")]
class SCR_LampComponentClass : SCR_BaseInteractiveLightComponentClass
{
	[Attribute("{40318DDE45FF4CC3}Particles/Enviroment/Lamp_fire_normal.ptc", UIWidgets.ResourceNamePicker, "Prefab of fire particle used for a fire action.", "ptc")]
	protected ResourceName m_sParticle;	
	[Attribute("0 0.1 0", UIWidgets.EditBox, "Particle offset in local space from the origin of the entity")]
	protected vector m_vParticleOffset;
	
	ResourceName GetParticle()
	{
		return m_sParticle;
	}
	
	vector GetParticleOffset()
	{
		return m_vParticleOffset;
	}
};

class SCR_LampComponent : SCR_BaseInteractiveLightComponent
{
	
	private SCR_ParticleEmitter m_pFireParticle;
	protected SCR_BaseInteractiveLightComponentClass m_ComponentData;
		
	//------------------------------------------------------------------------------------------------
	override void ToggleLight(bool turnOn, bool skipTransition = false, bool playSound = true)
	{	
		super.ToggleLight(turnOn, skipTransition, playSound);
		
		// don't continue if DS server is running in console (particles, EOnFrame, decals)
		if (System.IsConsoleApp())
			return;		
		
		if (turnOn)
		{			
			TurnOn();
		}	
		else
		{
			TurnOff();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void TurnOn()
	{
		if (m_pFireParticle)
			m_pFireParticle.UnPause();
		else
		{
			SCR_LampComponentClass componentData = SCR_LampComponentClass.Cast(GetComponentData(GetOwner()));
			if (componentData) 
			{
				m_pFireParticle = SCR_ParticleEmitter.CreateAsChild(componentData.GetParticle(), GetOwner(), componentData.GetParticleOffset());
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void TurnOff()
	{
		if (m_pFireParticle)
			m_pFireParticle.Pause();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		// Can't relay on RplLoad method of base class as with the lamp we have to execute ToggleLight method in both on / off state.
		bool isOn = false;
		reader.ReadBool(isOn);
		ToggleLight(isOn, true);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_LampComponent()
	{
		RemoveLights();
	}
};