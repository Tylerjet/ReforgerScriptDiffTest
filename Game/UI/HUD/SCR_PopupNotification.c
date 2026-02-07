//------------------------------------------------------------------------------------------------
//! List of status notifications with non-fixed duration
enum eCampaignStatusMessage
{
	SEIZING_ENEMY,
	SEIZING_YOU,
	CANT_SEIZE,
	SESSION_RESTART
};

//------------------------------------------------------------------------------------------------
enum SCR_EPopupMsgFilter
{
	ALL,
	NONE,
	TUTORIAL
};

//------------------------------------------------------------------------------------------------
//! Popup message priorities sorted from lowest to highest
enum SCR_ECampaignPopupPriority
{
	DEFAULT,
	SUPPLIES_HANDLED,
	TASK_AVAILABLE,
	RELAY_DETECTED,
	TASK_PROGRESS,
	TASK_DONE,
	BASE_OVERRUN,
	BASE_LOST,
	MHQ,
	BASE_UNDER_ATTACK,
	RESPAWN,
	MATCH_END
};

//------------------------------------------------------------------------------------------------
class SCR_PopUpNotificationClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
/*class SCR_PopupMessage
{
	private string m_sText;
	private string m_sSubtitle;
	private float m_fDuration;
	private SCR_ECampaignPopupPriority m_ePrio;
	private string m_sSound;
	private ref array<string> m_aTextParams = {};
	private ref array<string> m_aSubtitleParams = {};
	
	void SCR_PopupMessage(string text, string subtitle = "", float duration = SCR_PopUpNotification.DEFAULT_DURATION)
	{
		m_aTextParams.Clear();
		m_aSubtitleParams.Clear();
		m_aTextParams = null;
		m_aSubtitleParams = null;
	}
	
	void ~SCR_PopupMessage()
	{
		m_aTextParams.Clear();
		m_aSubtitleParams.Clear();
		m_aTextParams = null;
		m_aSubtitleParams = null;
	}
}*/

//------------------------------------------------------------------------------------------------
//! Takes care of dynamic and static onscreen popups
class SCR_PopUpNotification: GenericEntity
{
	protected static SCR_PopUpNotification s_Instance = null;
	static const float DEFAULT_DURATION = 4;
	static const string TASKS_KEY_IMAGE_FORMAT = "<color rgba='226,168,79,255'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'><action name='TasksOpen'/></shadow></color>";
	protected static SCR_EPopupMsgFilter m_sFilter = SCR_EPopupMsgFilter.ALL;
	
	protected ref array<string> m_aPopupMsgTextsQueue = new ref array<string>();
	protected ref array<string> m_aPopupMsgTextsSmallQueue = new ref array<string>();
	protected ref array<float> m_aPopupMsgTimersQueue = new ref array<float>();
	protected ref array<int> m_aPopupMsgPrioQueue = new ref array<int>();
	protected ref array<string> m_aPopupMsgSoundQueue = new ref array<string>();
	protected ref array<int> m_aActiveStatusMsgs = new ref array<int>();
	protected ref array<ref array<string>> m_aParamsArrayTextQueue = new ref array<ref array<string>>();
	protected ref array<ref array<string>> m_aParamsArrayText2Queue = new ref array<ref array<string>>();
	
	protected RichTextWidget m_wPopupMsg;
	protected RichTextWidget m_wPopupMsgSmall;
	protected ProgressBarWidget m_wStatusProgress;
	protected ImageWidget m_wLine;
	
	protected eCampaignStatusMessage m_ePrevMsg = -1;
	
	protected bool m_bStatusMsgRefresh = false;
	protected bool m_bStatusMsgActive = false;
	protected bool m_bPopupQueueRefresh = true;
	
	protected float m_fCurPopupMsgFadeInStart = -1;
	protected float m_fCurPopupMsgDurationStart;
	protected float m_fCurPopupMsgFadeOutStart;
	protected float m_fCurPopupMsgFadeOutEnd;
	protected float m_fPopupMsgDefaultAlpha;
	protected float m_fLineDefaultAlpha;
	protected float m_fCurStatusMsgFadeInStart = -1;
	protected float m_fCurStatusMsgDurationStart;
	protected float m_fCurStatusMsgFadeOutStart;
	protected float m_fCurStatusMsgFadeOutEnd;
	protected float m_fProgressBarStart = -1;
	protected float m_fProgressBarFinish = -1;
	protected float m_fQueueProcessorTimer = -1;
	
	protected int m_iLastQueueSize = 0;
	protected int m_iHighestPrioIndex = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Returns an instance of this manager.
	static SCR_PopUpNotification GetInstance()
	{
		if (!s_Instance)
		{
			BaseWorld world = GetGame().GetWorld();
			
			if (world)
				s_Instance = SCR_PopUpNotification.Cast(GetGame().SpawnEntity(SCR_PopUpNotification, world));
		}
		
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	static void SetFilter(SCR_EPopupMsgFilter filter)
	{
		m_sFilter = filter;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether the pop up notification is being shown or not.
	bool IsShowing()
	{
		if (m_wPopupMsg && m_wPopupMsg.IsVisibleInHierarchy())
		{
			float opacityEpsilon = 0.001;
			if (m_wPopupMsg.GetOpacity() > opacityEpsilon)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Queue new popup notification 
	//! \param text Text to be shown 
	//! \param duration How long the text should stay on screen (excluding fade in / fade out)
	//! \param fade How long the fade transitions should be
	//! \param text2 Secondary (smaller) text
	void PopupMsg(string text, float duration = DEFAULT_DURATION, float fade = 0.5, string text2 = "", int prio = -1, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string text2param1 = "", string text2param2 = "", string text2param3 = "", string text2param4 = "", string sound = "", SCR_EPopupMsgFilter category = SCR_EPopupMsgFilter.ALL)
	{
		// Don't show UI on headless
		if (System.IsConsoleApp())
			return;
		
		if (m_sFilter == SCR_EPopupMsgFilter.NONE || (m_sFilter == SCR_EPopupMsgFilter.TUTORIAL && category != SCR_EPopupMsgFilter.TUTORIAL))
			return;
		
		if ((text.IsEmpty() && text2.IsEmpty()) || !GetWorld())
			return;
		
		m_aPopupMsgTextsQueue.Insert(text);
		m_aPopupMsgTextsSmallQueue.Insert(text2);
		
		array<string> params = {param1, param2, param3, param4, param5};
		array<string> params2 = {text2param1, text2param2, text2param3, text2param4};
	
		m_aParamsArrayTextQueue.Insert(params);
		m_aParamsArrayText2Queue.Insert(params2);
		
		m_aPopupMsgTimersQueue.Insert(duration);
		m_aPopupMsgTimersQueue.Insert(fade);
		m_aPopupMsgPrioQueue.Insert(prio);
		m_aPopupMsgSoundQueue.Insert(sound);
	}
	
	//------------------------------------------------------------------------------------------------
	void HideCurrentPopupMsg()
	{
		m_fCurPopupMsgFadeOutEnd = -float.MAX;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AlphaLerp(Widget widget, float startedAt, float time, float duration, float defaultAlpha, bool fadeIn = true)
	{
		float start = 0;
		float end = defaultAlpha;
		
		if (!fadeIn)
		{
			start = defaultAlpha;
			end = 0;
		};
		
		float alpha = Math.Lerp(start, end, (time - startedAt) / duration);
		widget.SetOpacity(alpha);
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleStatusMsg(int msgID, bool show, float progressStart = -1, float progressEnd = -1, SCR_EPopupMsgFilter category = SCR_EPopupMsgFilter.ALL)
	{
		// Don't show UI on headless
		if (System.IsConsoleApp())
			return;
		
		// Method called too early, init not yet processed. Terminate.
		if (!m_wPopupMsg)
			return;
		
		if (m_sFilter == SCR_EPopupMsgFilter.NONE || (m_sFilter == SCR_EPopupMsgFilter.TUTORIAL && category != SCR_EPopupMsgFilter.TUTORIAL))
			return;
		
		if (show)
		{
			BaseWorld world = GetGame().GetWorld();
			
			if (world)
			{
				float timeDiff = Replication.Time() - world.GetWorldTime();
				m_fProgressBarStart = progressStart - timeDiff;
				m_fProgressBarFinish = progressEnd - timeDiff;
				m_wStatusProgress.SetVisible(false);		// Needed to reset the progress bar limits
			}
			
			if (m_aActiveStatusMsgs.Find(msgID) == -1)
			{
				int prevFirstElement = -1;
				
				if (m_aActiveStatusMsgs.Count() != 0)
					prevFirstElement = m_aActiveStatusMsgs[0];
				
				m_aActiveStatusMsgs.Insert(msgID);
				m_aActiveStatusMsgs.Sort();
				
				if (m_aActiveStatusMsgs[0] != prevFirstElement)
					m_bStatusMsgRefresh = true;
			}
		} else {
			int elementID = m_aActiveStatusMsgs.Find(msgID);
			if (elementID != -1)
			{
				m_aActiveStatusMsgs.Remove(elementID);
				m_bStatusMsgRefresh = true;
			};
		};
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessInit()
	{
		if (!GetGame().GetHUDManager())
			return;
		
		// Init can be safely processed
		GetGame().GetCallqueue().Remove(ProcessInit);
		
		// Initialize popups UI
		Widget root = GetGame().GetHUDManager().CreateLayout("{D74D24696C4F32F0}UI/layouts/HUD/CampaignMP/CampaignPopupUI.layout", EHudLayers.MEDIUM, 0);
		m_wPopupMsg = RichTextWidget.Cast(root.FindWidget("Popup"));
		m_wPopupMsgSmall = RichTextWidget.Cast(root.FindWidget("PopupSmall"));
		m_wLine = ImageWidget.Cast(root.FindWidget("Line"));
		m_fPopupMsgDefaultAlpha = m_wPopupMsg.GetColor().A();
		m_fLineDefaultAlpha = m_wLine.GetColor().A();
		m_wPopupMsg.SetOpacity(0);
		m_wPopupMsgSmall.SetVisible(false);
		m_wLine.SetVisible(false);
		
		// Initialize status UI
		m_wStatusProgress = ProgressBarWidget.Cast(root.FindWidget("Progress"));
		m_wStatusProgress.SetVisible(false);
		
		SetEventMask(EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		
		if (mapEntity)
		{
			MapConfiguration config = mapEntity.GetMapConfig();
			if (!config)
				return;
			
			Widget mapWidget = config.RootWidgetRef;
			if (mapWidget)
				root.SetZOrder(mapWidget.GetZOrder() - 1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		BaseWorld world = GetGame().GetWorld();
		
		if (!world)
			return;
		
		float curTime = world.GetWorldTime();
		int queueCnt = m_aPopupMsgTextsQueue.Count();
		
		// Popup msg queue
		if (queueCnt != 0)
		{
			// Status msg is active, ignore popups
			if (m_bStatusMsgActive)
			{
				if (m_fCurPopupMsgFadeInStart != -1)
				{
					m_wPopupMsgSmall.SetVisible(false);
					m_wLine.SetVisible(false);
					m_fCurPopupMsgFadeInStart = -1;
					m_aPopupMsgTextsQueue.Remove(m_iHighestPrioIndex);
					m_aPopupMsgTextsSmallQueue.Remove(m_iHighestPrioIndex);
					m_aParamsArrayTextQueue.Remove(m_iHighestPrioIndex);
					m_aParamsArrayText2Queue.Remove(m_iHighestPrioIndex);
					m_aPopupMsgPrioQueue.Remove(m_iHighestPrioIndex);
					m_aPopupMsgSoundQueue.Remove(m_iHighestPrioIndex);
					m_aPopupMsgTimersQueue.Remove(m_iHighestPrioIndex * 2);
					m_aPopupMsgTimersQueue.Remove(m_iHighestPrioIndex * 2);
				}
			}
			else
			{
				if (m_iLastQueueSize == 0)
				{
					m_fQueueProcessorTimer = curTime + 250;
					m_iLastQueueSize = queueCnt;
					return;
				}
	
				m_iLastQueueSize = queueCnt;
				
				if (curTime < m_fQueueProcessorTimer)
					return;
	
				if (m_bPopupQueueRefresh)
				{
					m_bPopupQueueRefresh = false;
					m_iHighestPrioIndex = 0;
				
					foreach (int i, int prio: m_aPopupMsgPrioQueue)
					{
						if (prio > m_aPopupMsgPrioQueue[m_iHighestPrioIndex])
							m_iHighestPrioIndex = i;
					}
				}
	
				string text = m_aPopupMsgTextsQueue[m_iHighestPrioIndex];
				string textSmall = m_aPopupMsgTextsSmallQueue[m_iHighestPrioIndex];
				array<string> params = m_aParamsArrayTextQueue[m_iHighestPrioIndex];
				array<string> params2 = m_aParamsArrayText2Queue[m_iHighestPrioIndex];
				float duration = m_aPopupMsgTimersQueue[m_iHighestPrioIndex * 2] * 1000;
				float fade = m_aPopupMsgTimersQueue[(m_iHighestPrioIndex * 2) + 1] * 1000;
				string sound = m_aPopupMsgSoundQueue[m_iHighestPrioIndex];
				bool isMapOpen = false;
				SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
				
				if (mapEntity)
					isMapOpen = mapEntity.IsOpen();
				
				// Map is open, increase the popup timers
				if (m_fCurPopupMsgFadeOutEnd > curTime && isMapOpen)
				{
					float toAdd = timeSlice * 1000;
					m_fCurPopupMsgFadeInStart += toAdd;
					m_fCurPopupMsgDurationStart += toAdd;
					m_fCurPopupMsgFadeOutStart += toAdd;
					m_fCurPopupMsgFadeOutEnd += toAdd;
				}
				
				if (m_fCurPopupMsgFadeInStart == -1)
				{
					m_wPopupMsg.SetTextFormat(text, params[0], params[1], params[2], params[3], params[4]);
					m_wPopupMsgSmall.SetTextFormat(textSmall, params2[0], params2[1], params2[2], params2[3]);
	
					if (duration > 0)
					{
						AlphaLerp(m_wPopupMsg, curTime, curTime, fade, m_fPopupMsgDefaultAlpha);
						AlphaLerp(m_wPopupMsgSmall, curTime, curTime, fade, m_fPopupMsgDefaultAlpha);
						AlphaLerp(m_wLine, curTime, curTime, fade, m_fLineDefaultAlpha);
					};
					
					m_wPopupMsgSmall.SetVisible(true);
					m_wLine.SetVisible(true);
					m_fCurPopupMsgFadeInStart = curTime;
					m_fCurPopupMsgDurationStart = m_fCurPopupMsgFadeInStart + fade;
					m_fCurPopupMsgFadeOutStart = m_fCurPopupMsgDurationStart + duration;
					m_fCurPopupMsgFadeOutEnd = m_fCurPopupMsgFadeOutStart + fade;
					
					if (!sound.IsEmpty())
						SCR_UISoundEntity.SoundEvent(sound);
				}
				else
				{
					if (curTime >= m_fCurPopupMsgFadeOutEnd)
					{
						m_wPopupMsg.SetOpacity(0);
						m_wPopupMsgSmall.SetVisible(false);
						m_wLine.SetVisible(false);
						m_fCurPopupMsgFadeInStart = -1;
						m_aPopupMsgTextsQueue.Remove(m_iHighestPrioIndex);
						m_aPopupMsgTextsSmallQueue.Remove(m_iHighestPrioIndex);
						m_aParamsArrayTextQueue.Remove(m_iHighestPrioIndex);
						m_aParamsArrayText2Queue.Remove(m_iHighestPrioIndex);
						m_aPopupMsgPrioQueue.Remove(m_iHighestPrioIndex);
						m_aPopupMsgSoundQueue.Remove(m_iHighestPrioIndex);
						m_aPopupMsgTimersQueue.Remove(m_iHighestPrioIndex * 2);
						m_aPopupMsgTimersQueue.Remove(m_iHighestPrioIndex * 2);
						m_bPopupQueueRefresh = true;
					}
					else
					{
						if (curTime < m_fCurPopupMsgDurationStart)
						{
							AlphaLerp(m_wPopupMsg, m_fCurPopupMsgFadeInStart, curTime, fade, m_fPopupMsgDefaultAlpha);
							AlphaLerp(m_wPopupMsgSmall, m_fCurPopupMsgFadeInStart, curTime, fade, m_fPopupMsgDefaultAlpha);
							AlphaLerp(m_wLine, m_fCurPopupMsgFadeInStart, curTime, fade, m_fLineDefaultAlpha);
						}
						else
						{
							if (curTime >= m_fCurPopupMsgFadeOutStart)
							{
								AlphaLerp(m_wPopupMsg, m_fCurPopupMsgFadeOutStart, curTime, fade, m_fPopupMsgDefaultAlpha, false);
								AlphaLerp(m_wPopupMsgSmall, m_fCurPopupMsgFadeOutStart, curTime, fade, m_fPopupMsgDefaultAlpha, false);
								AlphaLerp(m_wLine, m_fCurPopupMsgFadeOutStart, curTime, fade, m_fLineDefaultAlpha, false);
							}
						}
					}
				}
			}
		}
		
		// Refresh status msg 
		if (m_bStatusMsgRefresh && curTime > m_fCurStatusMsgFadeOutEnd && curTime > m_fCurStatusMsgDurationStart)
		{
			m_bStatusMsgRefresh = false;
			
			switch(m_aActiveStatusMsgs.Count())
			{
				case 0:
					m_fCurStatusMsgFadeOutStart = curTime;
					m_fCurStatusMsgFadeOutEnd = m_fCurStatusMsgFadeOutStart + 500;
					break;
				case 1:
					m_fCurStatusMsgFadeInStart = curTime;
					m_fCurStatusMsgDurationStart = m_fCurStatusMsgFadeInStart + 500;
					break;
				case 2:
					m_fCurStatusMsgFadeOutStart = curTime;
					m_fCurStatusMsgFadeOutEnd = m_fCurStatusMsgFadeOutStart + 500;
					m_fCurStatusMsgFadeInStart = m_fCurStatusMsgFadeOutEnd;
					m_fCurStatusMsgDurationStart = m_fCurStatusMsgFadeInStart + 500;
					break;
			};
		};
		
		if (curTime > m_fCurStatusMsgFadeOutEnd && m_fCurStatusMsgFadeOutEnd > m_fCurStatusMsgDurationStart && m_bStatusMsgActive)
		{
			m_bStatusMsgActive = false;
			m_bPopupQueueRefresh = true;
			m_wPopupMsg.SetOpacity(0);
			m_wStatusProgress.SetVisible(false);
		};
		
		if (m_aActiveStatusMsgs.Count() != 0 && m_fCurStatusMsgFadeOutEnd < m_fCurStatusMsgDurationStart && (!m_bStatusMsgActive || m_ePrevMsg != m_aActiveStatusMsgs[0]))
		{
			m_bStatusMsgActive = true;
			string text;
			
			switch (m_aActiveStatusMsgs[0])
			{
				case eCampaignStatusMessage.SEIZING_ENEMY:
					text = "#AR-Campaign_SeizingEnemy-UC";
					break;

				case eCampaignStatusMessage.SEIZING_YOU:
					text = "#AR-Campaign_SeizingFriendly-UC";
					break;
					
				case eCampaignStatusMessage.SESSION_RESTART:
					text = "#AR-Campaign_Restarting-UC";
					break;
			}

			m_wPopupMsg.SetTextFormat(text);
		};
		
		// Process status msg 
		if (m_wPopupMsg && m_bStatusMsgActive)
		{
			if (m_fCurStatusMsgFadeOutEnd >= curTime)
			{
				AlphaLerp(m_wPopupMsg, m_fCurStatusMsgFadeOutStart, curTime, 500, m_fPopupMsgDefaultAlpha, false);
				AlphaLerp(m_wStatusProgress, m_fCurStatusMsgFadeOutStart, curTime, 500, m_fPopupMsgDefaultAlpha, false);
			} else {
				if (m_fCurStatusMsgDurationStart >= curTime)
				{
					AlphaLerp(m_wPopupMsg, m_fCurStatusMsgFadeInStart, curTime, 500, m_fPopupMsgDefaultAlpha);
					AlphaLerp(m_wStatusProgress, m_fCurStatusMsgFadeInStart, curTime, 500, m_fPopupMsgDefaultAlpha);
				}
			};
			
			if (curTime < m_fProgressBarFinish)
			{
				if (!m_wStatusProgress.IsVisible() || (m_aActiveStatusMsgs.Count() > 0 && m_aActiveStatusMsgs[0] != m_ePrevMsg))
				{
					m_wStatusProgress.SetVisible(true);
					m_wStatusProgress.SetMin(m_fProgressBarStart);
					m_wStatusProgress.SetMax(m_fProgressBarFinish);
				}
				
				if (m_fCurStatusMsgFadeOutEnd < curTime)
					m_wStatusProgress.SetCurrent(curTime);
				
			} else {
				if (m_wStatusProgress.IsVisible())
					m_wStatusProgress.SetVisible(false);
			}
		};
		
		if (m_aActiveStatusMsgs.Count() > 0)
			m_ePrevMsg = m_aActiveStatusMsgs[0];
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_PopUpNotification(IEntitySource src, IEntity parent)
	{
		// Don't show UI on headless
		if (System.IsConsoleApp())
			return;
		
		// Make sure init can be processed properly (when HUD Manager is ready, check in ProcessInit())
		GetGame().GetCallqueue().CallLater(ProcessInit, 1000, true)
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_PopUpNotification()
	{
		m_aPopupMsgTextsQueue = null;
		m_aPopupMsgTextsSmallQueue = null;
		m_aPopupMsgTimersQueue = null;
		m_aActiveStatusMsgs = null;
		m_aPopupMsgPrioQueue = null;
		m_aPopupMsgSoundQueue = null;
		
		int cnt = m_aParamsArrayTextQueue.Count();
		
		for (int i = cnt - 1; i >= 0; i--)
		{
			if (m_aParamsArrayTextQueue[i])
			{
				m_aParamsArrayTextQueue[i].Clear();
				m_aParamsArrayTextQueue[i] = null;
			}
			
			m_aParamsArrayTextQueue.Remove(i);
		}
	
		m_aParamsArrayTextQueue = null;
		
		cnt = m_aParamsArrayText2Queue.Count();
		
		for (int i = cnt - 1; i >= 0; i--)
		{
			if (m_aParamsArrayText2Queue[i])
			{
				m_aParamsArrayText2Queue[i].Clear();
				m_aParamsArrayText2Queue[i] = null;
			}
			
			m_aParamsArrayText2Queue.Remove(i);
		}
	
		m_aParamsArrayText2Queue = null;
		
		if (m_wPopupMsg)
			m_wPopupMsg.GetParent().RemoveFromHierarchy();
	}
};