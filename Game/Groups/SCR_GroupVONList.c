class SCR_CompareGroupRadioFreq : SCR_SortCompare<SCR_AIGroup>
{
	override static int Compare(SCR_AIGroup left, SCR_AIGroup right)
	{
		return left.GetRadioFrequency() < right.GetRadioFrequency();
	}
};

class SCR_GroupVONList: ScriptedWidgetComponent
{
	protected Widget m_wParentWidget;

	[Attribute()]
	protected ResourceName m_EntryLayout;

	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_ActiveGroupByFrequency;

	//------------------------------------------------------------------------------------------------
	void InitiateList(BaseTransceiver radioTransceiver)
	{
		if (!radioTransceiver)
			return;

		if (!m_wParentWidget)
			return;

		VerticalLayoutWidget entriesList = VerticalLayoutWidget.Cast(m_wParentWidget.FindAnyWidget("GroupList"));
		if (!entriesList)
			return;
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		Faction playerFaction = SCR_RespawnSystemComponent.GetInstance().GetPlayerFaction(GetGame().GetPlayerController().GetPlayerId());

		array<SCR_AIGroup> playableGroups = {};

		playableGroups.Copy(groupManager.GetPlayableGroupsByFaction(playerFaction));

		SCR_Faction milFaction = SCR_Faction.Cast(playerFaction);
		Widget groupEntry;
		RichTextWidget callsign, frequency;
		ImageWidget talkingImage;
		string company, platoon, squad, character, format;

		ClearList();

		groupEntry = GetGame().GetWorkspace().CreateWidgets(m_EntryLayout, entriesList);
		callsign = RichTextWidget.Cast(groupEntry.FindAnyWidget("Callsign"));
		if (!callsign)
			return;
		callsign.SetText("#AR-Comm_PlatoonChannel");
		frequency = RichTextWidget.Cast(groupEntry.FindAnyWidget("Frequency"));
		if (frequency)
			frequency.SetText(""+ milFaction.GetFactionRadioFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");
		
		if (radioTransceiver && milFaction.GetFactionRadioFrequency() == radioTransceiver.GetFrequency())
		{
			talkingImage = ImageWidget.Cast(groupEntry.FindAnyWidget("TalkingImage"));
			talkingImage.SetVisible(true);
			callsign.SetColor(m_ActiveGroupByFrequency);
		}

		SCR_Sorting<SCR_AIGroup, SCR_CompareGroupRadioFreq >.HeapSort(playableGroups);

		foreach (SCR_AIGroup group : playableGroups)
		{
			groupEntry = GetGame().GetWorkspace().CreateWidgets(m_EntryLayout, entriesList);
			callsign = RichTextWidget.Cast(groupEntry.FindAnyWidget("Callsign"));
			if (!callsign)
				continue;

			group.GetCallsigns(company, platoon, squad, character, format);
			callsign.SetTextFormat(format, company, platoon, squad, character);

			frequency = RichTextWidget.Cast(groupEntry.FindAnyWidget("Frequency"));
			if (frequency)
				frequency.SetText(""+group.GetRadioFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");
			if (radioTransceiver && group.GetRadioFrequency() == radioTransceiver.GetFrequency())
			{
				talkingImage = ImageWidget.Cast(groupEntry.FindAnyWidget("TalkingImage"));
				talkingImage.SetVisible(true);
				callsign.SetColor(m_ActiveGroupByFrequency);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override event void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wParentWidget = w;
	}

	//------------------------------------------------------------------------------------------------
	void ClearList()
	{
		VerticalLayoutWidget entriesList = VerticalLayoutWidget.Cast(m_wParentWidget.FindAnyWidget("GroupList"));
		if (!entriesList)
			return;
		Widget children = entriesList.GetChildren();
		while (children)
		{
			entriesList.RemoveChild(children);
			children = entriesList.GetChildren();
		}
	}
};
