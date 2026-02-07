[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_CommunicationSoundComponentClass : CommunicationSoundComponentClass
{
	// prefab properties here
};

enum ECP_VehicleRoles
{
	DRIVER,
	GUNNER,
	COMMANDER,
	PILOT,
	COPILOT,
	OPERATOR,
	PASSANGER
};

enum ECP_Characters
{
	MAN,
	SOLDIER,
	MACHINE_GUNNER,
	SNIPER,
	RECON,
	AT_SOLDIER,
	OFFICER,
	RADIO_MAN
};

class SCR_CommunicationSoundComponent : CommunicationSoundComponent
{
	protected SignalsManagerComponent m_SignalsManagerComponent;
	private static const int SUBTITLES_MAX_DISTANCE = 20;
	
	static bool m_bShowSubtitles;
	
	void ShowSubtitles(array<string> metadata)
	{
		int size = metadata.Count();
		
		if (size > 10)
		{
			Print("Too many metadata. Metadata size = " + size.ToString(), LogLevel.ERROR);
			return;
		}
		
		// Get text		
		int textIdx = -1;
		
		for (int i = size - 1; i >= 0; i--)
		{
			if (!metadata[i].IsEmpty())
			{
				textIdx = i;
				break;
			}
		}
		
		// Do not show subtitles if metadata is empty
		if (textIdx == -1)
			return;
		
		// Get parameters		
		string param[9];		
		int paramIndex;
			
		for (int i = 0; i < textIdx; i++)
		{
			if (!metadata[i].IsEmpty())
			{
				param[paramIndex] = metadata[i];
				paramIndex++;
			}
		}

		// Show subtitles
		// Subtitles printed to the chat - old way
		string localizedText = WidgetManager.Translate(metadata[textIdx], param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7], param[8]);

		SCR_ChatComponent.RadioProtocolMessage(localizedText);
	}
	
	// Inserted to OnUserSettingsChangedInvoker() on SCR_UISoundEntity
	static void SetSubtitiles()
	{	
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (settings)
			settings.Get("m_bShowRadioProtocolText", m_bShowSubtitles);
	}
	
	//
	void SetCallsignSignals(int company, int platoon, int squad, int character, int characterRole)
	{
		if (!m_SignalsManagerComponent)
			return;	
		
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("CompanyCaller"), company);
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("PlattonCaller"), platoon);
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("SquadCaller"), squad);
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("SoldierCaller"), character -1);
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_CallsignBaseComponent callsignBaseComponent = SCR_CallsignBaseComponent.Cast(owner.FindComponent(SCR_CallsignBaseComponent));
		
		if (callsignBaseComponent)
			callsignBaseComponent.GetOnCallsignChanged().Insert(SetCallsignSignals);
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleMetadata(array<string> metadata, int priority, float distance)
	{
		// Hotfix: Disabled "subtitles"
		/*
		if (m_bShowSubtitles && distance < SUBTITLES_MAX_DISTANCE)
			ShowSubtitles(metadata);
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSoundEventFinished(string eventName, AudioHandle handle, int priority, bool terminated)
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CommunicationSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CommunicationSoundComponent()
	{
	}
};