/*!
	Handels the showing of end screen and has a config with gameover screens
*/
[ComponentEditorProps(category: "GameScripted/GameOver", description: "")]
class SCR_GameOverScreenManagerComponentClass: SCR_BaseGameModeComponentClass
{
};

class SCR_GameOverScreenManagerComponent: SCR_BaseGameModeComponent
{
	[Attribute()]
	protected ref SCR_GameOverScreenConfig m_GameOverScreenConfig;
	
	[Attribute("1", desc: "Delay (in seconds) to lock player input and display the actual end screen after end screen has faded in")]
	protected float m_fShowEndscreenDelay;
	
	[Attribute("{AD9701BB476CB59A}UI/layouts/HUD/GameOver/EndScreen/GameOver_EndScreen.layout", params: "layout")]
	protected ResourceName m_sGameOverScreenBasePrefab;
	
	//Editor only
	protected bool m_bListeningToEditorCalledEndGame = false;
	protected EGameOverTypes m_iEditorSetGameOverType;
	protected ref array<int> m_aEditorSetFactions = new ref array<int>;
	
	//End game vars saved so it can be usesed in multiple function
	protected EGameOverTypes m_iEndGameType = EGameOverTypes.UNKNOWN;
	protected bool m_bIsFactionVictory = false;
	protected Faction m_FactionPlayer = null;
	protected int m_iPlayerId = -1;
	protected int m_iEndReason = 0;
	protected ref array<Faction> m_aWinningFactions = new ref array<Faction>;
	protected ref array<int> m_aWinningPlayers = new ref array<int>;
	protected bool m_bIsPlayingGameOverAudio = false;
	
	protected Widget m_EndScreenFade;
	
	/*!
	Get GameoverScreen Config
	\return m_GameOverScreenConfig
	*/
	SCR_GameOverScreenConfig GetGameOverConfig()
	{
		return m_GameOverScreenConfig;
	}
	
	//Called when end game is called on GameMode
	//Fade the endgame screen but retain control
	//Once the fade in is done the end screen will be shown
	protected void StartEndGameFade()
	{	
		SetGameOverVarriables(GetGameMode().GetEndGameData());
		m_EndScreenFade = GetGame().GetHUDManager().CreateLayout(m_sGameOverScreenBasePrefab, EHudLayers.ALWAYS_TOP, 0);
		SCR_FadeUIComponent fadeComponent = SCR_FadeUIComponent.Cast(m_EndScreenFade.FindHandler(SCR_FadeUIComponent));
		fadeComponent.GetOnFadeDone().Insert(OnEndScreenFadeDone);
		fadeComponent.FadeIn();
		
		//Play game over audio if has any and non are playing
		if (m_bIsPlayingGameOverAudio)
			return;

		SCR_BaseGameOverScreenInfo gameOverScreenInfo;
		if (!m_GameOverScreenConfig.GetGameOverScreenInfo(m_iEndGameType, gameOverScreenInfo))
			return;
		
		if (!gameOverScreenInfo.HasOptionalParams())
			return;
		
		if (m_bIsFactionVictory)
			PlayOneShotAudio(gameOverScreenInfo.GetOneShotAudio(m_FactionPlayer, m_aWinningFactions));
		else
			PlayOneShotAudio(gameOverScreenInfo.GetOneShotAudio(m_iPlayerId, m_aWinningPlayers));
	}
	
	//End screen fade is done show gameover dialog when delay is done
	protected void OnEndScreenFadeDone(SCR_FadeUIComponent fadeComponent, bool isFadeIn)
	{
		//Close all dialogs
		GetGame().GetMenuManager().CloseAllMenus();
		GetGame().GetCallqueue().CallLater(ShowGameOverScreen, m_fShowEndscreenDelay * 1000);
	}
	
	/*!
	Call end screen using SCR_GameModeEndData. This is generally called after fade but can also be called instantly if desired
	\param SCR_GameModeEndData end game data. Will get endgame date from gamemode if non given
	\return SCR_GameOverScreenUIComponent game over screen widget
	*/
	SCR_GameOverScreenUIComponent ShowGameOverScreen(SCR_GameModeEndData endData = null)
	{		
		//Remove the fade UI
		if (m_EndScreenFade)
			m_EndScreenFade.RemoveFromHierarchy();
			
		if (!endData)
			endData = GetGameMode().GetEndGameData();
		
		if (m_iEndGameType == EGameOverTypes.UNKNOWN)
			SetGameOverVarriables(endData);
		
		SCR_BaseGameOverScreenInfo gameOverScreenInfo;
				
		//Get game over type
		if (!m_GameOverScreenConfig.GetGameOverScreenInfo(m_iEndGameType, gameOverScreenInfo))
		{
			Print(string.Format("'SCR_GameOverScreenConfig': Could not find '%1' endscreen in array!", typename.EnumToString(EGameOverTypes, m_iEndGameType)));
			return null;
		}
		
		if (gameOverScreenInfo.HasOptionalParams())
		{
			//Show end screen
			if (m_bIsFactionVictory)
			{
				if (!m_bIsPlayingGameOverAudio)
					PlayOneShotAudio(gameOverScreenInfo.GetOneShotAudio(m_FactionPlayer, m_aWinningFactions));
				
				SCR_GameOverScreenUIComponent gameOverScreen = OpenGameOverScreen(gameOverScreenInfo.GetGameOverContentLayout(), endData, gameOverScreenInfo.GetTitle(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetSubtitle(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetDebriefing(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetImage(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetVignetteColor(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetTitleParam(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetSubtitleParam(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetDebriefingParam(m_FactionPlayer, m_aWinningFactions));
				gameOverScreenInfo.GameOverUICustomization(gameOverScreen, m_FactionPlayer, m_aWinningFactions);
				return gameOverScreen;
			}
			else 
			{
				if (!m_bIsPlayingGameOverAudio)
					PlayOneShotAudio(gameOverScreenInfo.GetOneShotAudio(m_iPlayerId, m_aWinningPlayers));

				SCR_GameOverScreenUIComponent gameOverScreen = OpenGameOverScreen(gameOverScreenInfo.GetGameOverContentLayout(), endData, gameOverScreenInfo.GetTitle(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetSubtitle(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetDebriefing(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetImage(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetVignetteColor(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetTitleParam(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetSubtitleParam(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetDebriefingParam(m_iPlayerId, m_aWinningPlayers));
				gameOverScreenInfo.GameOverUICustomization(gameOverScreen, m_iPlayerId, m_aWinningPlayers);
				return gameOverScreen;
			}
		}
		else
		{
			if (m_bIsFactionVictory)
			{
				SCR_GameOverScreenUIComponent gameOverScreen = OpenGameOverScreen(gameOverScreenInfo.GetGameOverContentLayout(), endData, gameOverScreenInfo.GetTitle(m_FactionPlayer, m_aWinningFactions));
				gameOverScreenInfo.GameOverUICustomization(gameOverScreen, m_FactionPlayer, m_aWinningFactions);
				return gameOverScreen;
			}
			else 
			{
				SCR_GameOverScreenUIComponent gameOverScreen = OpenGameOverScreen(gameOverScreenInfo.GetGameOverContentLayout(), endData, gameOverScreenInfo.GetTitle(m_iPlayerId, m_aWinningPlayers));
				gameOverScreenInfo.GameOverUICustomization(gameOverScreen, m_iPlayerId, m_aWinningPlayers);
				return gameOverScreen;
			}
		}
		
		//Makes sure set game over varriables are set again next time end game is called
		m_iEndGameType = EGameOverTypes.UNKNOWN;
	}
	
	//Sat all varriables needed to set the correct game over screen
	protected void SetGameOverVarriables(SCR_GameModeEndData endData = null)
	{
		m_iEndGameType = EGameOverTypes.UNKNOWN;
		m_bIsPlayingGameOverAudio = false;
		m_FactionPlayer = null;
		m_iPlayerId = -1;
		m_iEndReason = 0;
		m_aWinningFactions.Clear();
		m_aWinningPlayers.Clear();
		m_bIsFactionVictory = false;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		PlayerManager playerManager = GetGame().GetPlayerManager();
		
		if (!endData)
			endData = GetGameMode().GetEndGameData();
		
		if (endData != null) 
		{		
			m_iEndReason = endData.GetEndReason();
				
			
			if (factionManager)
			{
				array<int> winningFactionIds = new array<int>;
				endData.GetFactionWinnerIds(winningFactionIds);
				
				if (winningFactionIds)
				{
					Faction factionVictor;
					
					foreach(int id: winningFactionIds)
					{
						factionVictor = factionManager.GetFactionByIndex(id);
						if (factionVictor)
							m_aWinningFactions.Insert(factionVictor);
					}
				}
			}
				
			endData.GetWinnerIds(m_aWinningPlayers);
			
			m_bIsFactionVictory = (m_aWinningFactions != null && !m_aWinningFactions.IsEmpty());
		}
		
		if (playerManager)
		{
			SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (pc)
			{
				m_iPlayerId = pc.GetPlayerId();
				m_FactionPlayer = SCR_RespawnSystemComponent.GetLocalPlayerFaction();
			}
		}
		
		//Get the type of game over using given data
		m_iEndGameType = GetGameOverType(m_iEndReason, m_iPlayerId, m_aWinningPlayers, m_FactionPlayer, m_aWinningFactions, m_bIsFactionVictory);
	}
	
	//This function checks if the player is part of the winning faction (if any), if it is the winning player (if any) and return the correct GameOverType (Eg: Won, lost ect)
	protected EGameOverTypes GetGameOverType(int endReason, int playerId, array<int> winningPlayers, Faction factionPlayer, array<Faction> factionsVictor, bool isFactionVictory)
	{
		//// ==== Default end reasons ==== \\\\
		
		//-1: ENDREASON_UNDEFINED: Undefined
		if (endReason == SCR_GameModeEndData.ENDREASON_UNDEFINED)
		{
			return EGameOverTypes.NEUTRAL;
		}
		//-2: ENDREASON_TIMELIMIT: Time limit reached
		else if (endReason == SCR_GameModeEndData.ENDREASON_TIMELIMIT)
		{
			if (isFactionVictory)
			{	
				if (!factionPlayer)
					return EGameOverTypes.FACTION_NEUTRAL;
				else if (factionsVictor.Contains(factionPlayer))
					return EGameOverTypes.FACTION_VICTORY_TIME;
				else 
					return EGameOverTypes.FACTION_DEFEAT_TIME;	
			}
			else 
			{
				if (winningPlayers.Contains(playerId))
					return EGameOverTypes.DEATHMATCH_VICTORY_TIME;
				else 
					return EGameOverTypes.DEATHMATCH_DEFEAT_TIME;
			}
			
		}
		//-3: ENDREASON_SCORELIMIT: Score limit reached
		else if (endReason == SCR_GameModeEndData.ENDREASON_SCORELIMIT)
		{
			if (isFactionVictory)
			{	
				if (!factionPlayer)
					return EGameOverTypes.FACTION_NEUTRAL;
				else if (factionsVictor.Contains(factionPlayer))
					return EGameOverTypes.FACTION_VICTORY_SCORE;
				else 
					return EGameOverTypes.FACTION_DEFEAT_SCORE;	
			}
			else 
			{
				if (winningPlayers.Contains(playerId))
					return EGameOverTypes.DEATHMATCH_VICTORY_SCORE;
				else 
					return EGameOverTypes.DEATHMATCH_DEFEAT_SCORE;
			}
		}
		//-4: ENDREASON_DRAW
		else if (endReason == SCR_GameModeEndData.ENDREASON_DRAW)
		{
			if (isFactionVictory)
				return EGameOverTypes.FACTION_DRAW;
			else 
				return EGameOverTypes.DEATHMATCH_DRAW;
		}
		
		//// ==== Shared End Reasons ==== \\\\
		else if (endReason == SCR_GameModeEndData.ENDREASON_SERVER_RESTART)
		{
			return EGameOverTypes.SERVER_RESTART;
		}
		
		//// ==== Editor End Reasons ==== \\\\
		
		//Editor Neutral
		else if (endReason == EGameOverTypes.EDITOR_NEUTRAL)
		{
			return EGameOverTypes.EDITOR_NEUTRAL;
		}
		//Editor Victory
		else if (endReason == EGameOverTypes.EDITOR_FACTION_VICTORY)
		{
			if (!factionPlayer)
				return EGameOverTypes.EDITOR_FACTION_NEUTRAL;
			else if (factionsVictor.Contains(factionPlayer))
				return EGameOverTypes.EDITOR_FACTION_VICTORY;
			else 
				return EGameOverTypes.EDITOR_FACTION_DEFEAT;
		}
		//Non specific Editor end reasons
		else if (endReason == EGameOverTypes.EDITOR_NEUTRAL || endReason == EGameOverTypes.EDITOR_NEUTRAL || endReason == EGameOverTypes.EDITOR_FACTION_DRAW)
		{
			return endReason;
		}
		//Editor victory
		else if (endReason == EGameOverTypes.EDITOR_FACTION_VICTORY)
		{
			if (!factionPlayer)
				return EGameOverTypes.EDITOR_FACTION_NEUTRAL;
			else if (factionsVictor.Contains(factionPlayer))
				return EGameOverTypes.EDITOR_FACTION_VICTORY;
			else 
				return EGameOverTypes.EDITOR_FACTION_DEFEAT;
		}
		else 
		{
			Print(string.Format("'SCR_GameOverScreenConfig': Game over end reason: '%1' is invalid! A neutral end screen is shown instead", endReason.ToString()));
			return EGameOverTypes.NEUTRAL;
		}
	}
	
	protected void PlayOneShotAudio(string audioOneShot)
	{
		if (audioOneShot.IsEmpty())
			return;
		
		SCR_MusicManager musicManager = SCR_MusicManager.GetInstance();
		if (!musicManager)
			return;
		
		musicManager.PlayMusicOneShot(audioOneShot, true, true, 1000);
		m_bIsPlayingGameOverAudio = true;
	}
	
	/*!
	Open end screen
	\param title title of game over screen
	\param subtitle subtitle of game over screen
	\param debriefing debriefing text of game over screen
	\param imageTexture Image shown in game over screen
	\param vignetteColor will set the color of the gameover screen background overlay
	\param titleParam title param %1
	\param subtitleParam subtitle param %1
	\param debriefingParam debriefing param %1
	\return Widget endscreen widget
	*/
	SCR_GameOverScreenUIComponent OpenGameOverScreen(ResourceName contentLayout, SCR_GameModeEndData endData, LocalizedString title = string.Empty, LocalizedString subtitle = string.Empty, LocalizedString debriefing = string.Empty, ResourceName imageTexture = ResourceName.Empty, Color vignetteColor = null, string titleParam = string.Empty, string subtitleParam = string.Empty, string debriefingParam = string.Empty)
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return null;
		
		//This is just temporary hopefully. This bypasses a lot of the code here and in other files. If one wants to change the layout, do it in Menu Config.
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.EndgameScreen, 0, true);
		
		Widget screen = Widget.Cast(menu.GetRootWidget());
				
		SCR_GameOverScreenUIComponent screenUIComponent = SCR_GameOverScreenUIComponent.Cast(screen.FindHandler(SCR_GameOverScreenUIComponent));
		
		if (screenUIComponent)
			screenUIComponent.InitGameOverScreen(contentLayout, endData, title, subtitle, debriefing, imageTexture, vignetteColor, titleParam, subtitleParam, debriefingParam);
		
		return screenUIComponent;
	}
	
	
	//// ==== Editor Attribute Game over logic ==== \\\
	protected void OnEditorEndGameApplyDelayDone()
	{
		m_bListeningToEditorCalledEndGame = false;
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;
		
		SCR_BaseGameOverScreenInfo gameOverInfo = m_GameOverScreenConfig.GetGameOverInfo(m_iEditorSetGameOverType);
		if (!gameOverInfo)
			return;
		
		SCR_BaseGameOverScreenInfoEditor optionalEditorParam = gameOverInfo.GetEditorOptionalParams();
		
		if (!optionalEditorParam || !optionalEditorParam.m_bNeedsPlayableFactions)
			gamemode.EndGameMode(SCR_GameModeEndData.CreateSimple(m_iEditorSetGameOverType));
		else
			gamemode.EndGameMode(SCR_GameModeEndData.Create(m_iEditorSetGameOverType, null, m_aEditorSetFactions));
		
		//Clear vars
		m_aEditorSetFactions.Clear();
		m_iEditorSetGameOverType = EGameOverTypes.EDITOR_NEUTRAL;
	}
	
	
	/*!
	Set the gameover type for editor gameover used in the gameover screen
	\param gameOverType EGameOverTypes gameover type
	*/
	void SetEditorGameOverType(EGameOverTypes gameOverType)
	{
		m_iEditorSetGameOverType = gameOverType;
		
		if (!m_bListeningToEditorCalledEndGame)
		{
			m_bListeningToEditorCalledEndGame = true;
			GetGame().GetCallqueue().CallLater(OnEditorEndGameApplyDelayDone, 1);
		}
	}
	
	/*!
	Set the factions for editor gameover used in the gameover screen (Generally winning factions)
	\param factions array of factions 
	*/
	void SetEditorGameOverFactions(notnull array<int> factions)
	{
		m_aEditorSetFactions = factions;
		
		if (!m_bListeningToEditorCalledEndGame)
		{
			m_bListeningToEditorCalledEndGame = true;
			GetGame().GetCallqueue().CallLater(OnEditorEndGameApplyDelayDone, 1);
		}
	}
	
	//When game ends
	override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		//If called on init HUD Manager might not exist so call it with a delay
		if (!GetGame().GetHUDManager())
		{
			GetGame().GetCallqueue().CallLater(OnInitDelayedEndGame, 1000);
			return;
		}
		StartEndGameFade();
	}
	
	protected void OnInitDelayedEndGame()
	{
		StartEndGameFade();
	}
	
	override void OnPostInit(IEntity owner)
	{	
		
		
	}
};

