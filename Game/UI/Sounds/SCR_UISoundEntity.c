[EntityEditorProps(category: "GameScripted/UI", description: "Handles sounds played by UI", color: "0 0 255 255")]
class SCR_UISoundEntityClass: GenericEntityClass
{

};

class SCR_UISoundEntity : GenericEntity
{
	static private SoundComponent m_SoundComponent;
	static private SCR_UISoundEntity m_Instance;

	static SCR_UISoundEntity GetInstance()
	{
		return m_Instance;
	}

	//------------------------------------------------------------------------------------------------
	static AudioHandle SoundEvent(string eventName, bool force = false)
	{
		if (!m_SoundComponent)
		{
			if (!m_Instance)
				return AudioHandle.Invalid;

			m_SoundComponent = SoundComponent.Cast(m_Instance.FindComponent(SoundComponent));
			if (!m_SoundComponent)
				return AudioHandle.Invalid;
		}
		
		if (force)
			m_SoundComponent.TerminateAll();
		
		return m_SoundComponent.SoundEvent(eventName);
	}

	//------------------------------------------------------------------------------------------------
	static void SetSignalValueStr(string signal, float value)
	{
		if (!m_SoundComponent)
		{
			if (!m_Instance)
				return;

			m_SoundComponent = SoundComponent.Cast(m_Instance.FindComponent(SoundComponent));
			if (!m_SoundComponent)
				return;
		}

		m_SoundComponent.SetSignalValueStr(signal, value);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_SoundComponent = SoundComponent.Cast(FindComponent(SoundComponent));
		
		// Insert radio protocol subtitles settings change event
		GetGame().OnUserSettingsChangedInvoker().Insert(SCR_CommunicationSoundComponent.SetSubtitiles);
		SCR_CommunicationSoundComponent.SetSubtitiles();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_UISoundEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
		m_Instance = this;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_UISoundEntity(IEntitySource src, IEntity parent)
	{
		// Remove radio protocol subtitles settings change event
		GetGame().OnUserSettingsChangedInvoker().Remove(SCR_CommunicationSoundComponent.SetSubtitiles);
	}
};
