//------------------------------------------------------------------------------------------------
class SCR_VoiceoverSystem : GameSystem
{
	protected ref ScriptInvokerVoid m_OnFinished;

	protected ref SCR_VoiceoverData m_Data;

	protected ref SCR_VoiceoverSubtitles m_SubtitlesDisplay;

	protected ref array<ref SCR_VoiceoverLine> m_aQueue = {};
	protected ref array<IEntity> m_aActors = {};

	protected SCR_CommunicationSoundComponent m_PlayingSoundComponent;

	protected AudioHandle m_iAudioHandle;

	protected float m_fTimer;

	protected static const float PAUSE_BETWEEN_LINES = 0.5;

	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdate(ESystemPoint point)
	{
		if (!m_PlayingSoundComponent)
		{
			m_fTimer = 0;
			Enable(false);
			OnFinished();
			return;
		}

		if (!m_PlayingSoundComponent.IsFinishedPlaying(m_iAudioHandle))
			return;

		if (m_aQueue.IsEmpty())
		{
			m_fTimer = 0;
			Enable(false);
			OnFinished();
		}
		else
		{
			m_fTimer += GetWorld().GetFixedTimeSlice();

			if (m_fTimer >= PAUSE_BETWEEN_LINES)
			{
				m_fTimer = 0;
				PlayFromQueue();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override event bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_VoiceoverSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return SCR_VoiceoverSystem.Cast(world.FindSystem(SCR_VoiceoverSystem));
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when a line or sequence finishes playing
	ScriptInvokerVoid GetOnFinished()
	{
		if (!m_OnFinished)
			m_OnFinished = new ScriptInvokerVoid();

		return m_OnFinished;
	}

	//------------------------------------------------------------------------------------------------
	//! Loads voiceover data from the given SCR_VoiceoverData config resource
	//! Must be called before PlaySequence or PlayLine which both use data from this config!
	void SetData(ResourceName config)
	{
		Resource container = BaseContainerTools.LoadContainer(config);
		m_Data = SCR_VoiceoverData.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
	}

	//------------------------------------------------------------------------------------------------
	bool IsPlaying()
	{
		return IsEnabled();
	}

	//------------------------------------------------------------------------------------------------
	//! Plays a sequence of voiceover lines defined in loaded config (see SetData)
	// \param sequenceName Name of the sequence as defined in SCR_VoiceoverData
	// \param actor1 Entity playing the voiceover as defined in SCR_VoiceoverData by ACTOR enum (default: player character)
	void PlaySequence(string sequenceName, IEntity actor1 = null, IEntity actor2 = null, IEntity actor3 = null, IEntity actor4 = null, IEntity actor5 = null)
	{
		if (!m_Data)
			return;
		
		SCR_VoiceoverSequence sequence = m_Data.GetSequenceByName(sequenceName);

		if (!sequence)
			return;

		sequence.GetLines(m_aQueue);
		m_aActors = {actor1, actor2, actor3, actor4, actor5};

		PlayFromQueue();
	}

	//------------------------------------------------------------------------------------------------
	//! Plays a voiceover line defined in loaded config (see SetData)
	// \param lineName Name of the line as defined in SCR_VoiceoverData
	// \param actor1 Entity playing the voiceover as defined in SCR_VoiceoverData by ACTOR enum (default: player character)
	void PlayLine(string lineName, IEntity actor1 = null, IEntity actor2 = null, IEntity actor3 = null, IEntity actor4 = null, IEntity actor5 = null)
	{
		if (!m_Data)
			return;
		
		SCR_VoiceoverLineStandalone line = m_Data.GetLineByName(lineName);

		if (!line)
			return;

		m_aQueue = {line};
		m_aActors = {actor1, actor2, actor3, actor4, actor5};

		PlayFromQueue();
	}

	//------------------------------------------------------------------------------------------------
	//! Plays a custom voiceover line
	// \param soundEventName Sound event name as defined in the appropriate .acp file in the actor's SCR_CommunicationSoundComponent
	// \param actor Entity playing the voiceover (default: player character)
	void PlayCustomLine(string soundEventName, string subtitle, IEntity actor = null)
	{
		PlayLine(soundEventName, subtitle, actor);
	}

	//------------------------------------------------------------------------------------------------
	void Stop()
	{
		if (m_PlayingSoundComponent && m_iAudioHandle != AudioHandle.Invalid)
			m_PlayingSoundComponent.Terminate(m_iAudioHandle);
		
		m_iAudioHandle = AudioHandle.Invalid;
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayFromQueue()
	{
		if (m_aQueue.IsEmpty())
			return;

		SCR_VoiceoverLine line = m_aQueue[0];
		m_aQueue.RemoveOrdered(0);

		PlayLine(line.GetSoundEventName(), line.GetSubtitleText(), m_aActors[line.GetActor()]);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayLine(string eventName, string subtitle, IEntity actor = null)
	{
		if (!actor)
			actor = GetDefaultActor();

		if (!actor)
			return;

		m_PlayingSoundComponent = SCR_CommunicationSoundComponent.Cast(actor.FindComponent(SCR_CommunicationSoundComponent));

		if (!m_PlayingSoundComponent)
			return;

		if (m_iAudioHandle != AudioHandle.Invalid)
			AudioSystem.TerminateSound(m_iAudioHandle);

		m_iAudioHandle = m_PlayingSoundComponent.SoundEvent(eventName);

		if (m_iAudioHandle != AudioHandle.Invalid)
		{
			ShowSubtitle(subtitle);
			Enable(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowSubtitle(string subtitle)
	{
		if (!m_SubtitlesDisplay)
		{
			SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();

			if (!hudManager)
				return;

			m_SubtitlesDisplay = SCR_VoiceoverSubtitles.Cast(hudManager.FindInfoDisplay(SCR_VoiceoverSubtitles));

			if (!m_SubtitlesDisplay)
				return;
		}

		GetGame().GetCallqueue().Remove(m_SubtitlesDisplay.Show);
		m_SubtitlesDisplay.ShowSubtitle(subtitle);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GetDefaultActor()
	{
		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return null;

		return pc.GetControlledEntity();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFinished()
	{
		m_iAudioHandle = AudioHandle.Invalid;

		if (m_OnFinished)
			m_OnFinished.Invoke();

		if (!m_SubtitlesDisplay)
			return;

		GetGame().GetCallqueue().Remove(m_SubtitlesDisplay.Show);

		// Hide the subtitle after desired delay
		GetGame().GetCallqueue().CallLater(m_SubtitlesDisplay.Show, SCR_VoiceoverSubtitles.GetLingerDuration() * 1000, false, false, UIConstants.FADE_RATE_INSTANT, EAnimationCurve.LINEAR);
	}
}
