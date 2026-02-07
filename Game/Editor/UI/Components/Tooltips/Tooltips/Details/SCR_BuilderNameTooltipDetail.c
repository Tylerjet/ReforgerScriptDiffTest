
[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_BuilderNameTooltipDetail : SCR_EntityTooltipDetail
{
	protected const string AUTHOR_TEXT_WIDGET_NAME = "Text";
	protected const string PLATFORM_ICON_WIDGET_NAME = "PlatformIcon";

	//---- REFACTOR NOTE START: Searching for "Text" dirrectly 
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{		
		TextWidget text = TextWidget.Cast(widget.FindAnyWidget(AUTHOR_TEXT_WIDGET_NAME));
		if (!text)
			return false;
		
		BaseGameMode gameMode = GetGame().GetGameMode();	
		if (!gameMode)
			return false;
				
		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entity.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return false;
		
		PlatformKind playerPlatformKind;
		string name = GetPlayerName(compositionComponent.GetBuilderId(), playerPlatformKind);
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(name))
			return false;
		
		text.SetText(name);

		ImageWidget platformIcon = ImageWidget.Cast(widget.FindAnyWidget(PLATFORM_ICON_WIDGET_NAME));
		if (platformIcon && playerPlatformKind != PlatformKind.NONE)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.SetPlatformImageToKind(playerPlatformKind, platformIcon, showOnPC: true, showOnXbox: true);
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetPlayerName(int playerID, out PlatformKind playerPlatformKind = PlatformKind.NONE)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return string.Empty;
		
		string playerName = playerManager.GetPlayerName(playerID);
		playerPlatformKind = playerManager.GetPlatformKind(playerID);
		
		//Player name not found
		if (playerName.IsEmpty())
		{
			SCR_NotificationsComponent notificationsManager = SCR_NotificationsComponent.GetInstance();
			if (notificationsManager)
				playerName = notificationsManager.GetPlayerNameFromHistory(playerID);
		}
			
		return playerName;
	}
}
