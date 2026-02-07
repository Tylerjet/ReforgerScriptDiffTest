[ComponentEditorProps(category: "GameScripted/Effects", description: "Scripted fireplace component", color: "0 0 255 255", icon: HYBRID_COMPONENT_ICON)]
class SCR_ScriptedFireplaceComponentClass: ScriptEffectComponentClass
{
};

class SCR_ScriptedFireplaceComponent : ScriptEffectComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Should effect be active from the start")]
	bool m_bStartAutomatically;
	[Attribute("0", UIWidgets.CheckBox, "Should effect emit light")]
	bool m_bHasLight;
	[Attribute("1", UIWidgets.EditBox, "What's the initial strength of effect")]
	float m_fInitialBirthRateCoef;
	[Attribute("1", UIWidgets.EditBox, "Position of light relative to origin of owner")]
	vector m_vLightOffset;
	
	
	override void EOnInit(IEntity owner) 
	{		
		if (m_bStartAutomatically)
		{
			if (m_bHasLight)
			{
				vector pos = owner.GetOrigin() + m_vLightOffset;
				StartLight(pos);
			}
			SetEffectBirthRateCoeff(m_fInitialBirthRateCoef);
		}
	}
};