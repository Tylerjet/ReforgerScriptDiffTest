[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionVoiceOverPlayLine : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Config with voice over data for this action.", params: "conf class=SCR_VoiceoverData")]
	ResourceName m_sVoiceOverDataConfig;
	
	[Attribute(desc: "Name of the line as defined in Voice Over Data Config")]
	string	m_sLineName;
	
	[Attribute(desc: "Entity playing the voiceover. If left empty, default will be player character. Reminder: Related .acp needs to be present in the SCR_CommunicationSoundComponent of target entity.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Actions that will be triggered once Sequence finishes playing")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_sVoiceOverDataConfig || m_sVoiceOverDataConfig.IsEmpty())
		{
			SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
			if (!manager)
				return;
			
			m_sVoiceOverDataConfig = manager.m_sVoiceOverDataConfig;
			
			if (!m_sVoiceOverDataConfig || m_sVoiceOverDataConfig.IsEmpty())
				return;
		}
		
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;
		
		SCR_VoiceoverSystem voiceoverSystem = SCR_VoiceoverSystem.GetInstance();
		if (!voiceoverSystem)
			return;
		
		voiceoverSystem.SetData(m_sVoiceOverDataConfig);
		voiceoverSystem.GetOnFinished().Insert(OnFinished);
		
		if (m_Getter)
			voiceoverSystem.PlayLine(m_sLineName, entity);
		else
			voiceoverSystem.PlayLine(m_sLineName);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFinished()
	{
		SCR_VoiceoverSystem voiceoverSystem = SCR_VoiceoverSystem.GetInstance();
		if (voiceoverSystem)
			voiceoverSystem.GetOnFinished().Remove(OnFinished);
		
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
	}
}