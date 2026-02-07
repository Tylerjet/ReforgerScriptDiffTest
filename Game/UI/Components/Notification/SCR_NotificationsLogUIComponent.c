/** @ingroup Editor_UI Editor_UI_Components
Notification UI that displays the Notification Entires
Holds general functions such as scroll history and Init SCR_NotificationEntityUIComponent
Holds general data such as Notification Color
*/
class SCR_NotificationsLogComponent: MenuRootSubComponent
{	
	[Attribute("0", desc: "If false notifications can never be clicked on and do not have any key hints or teleport icons. This is used if the notifications are part of a menu")]
	protected bool m_bHasNotificationInput;
	
	protected SCR_NotificationsComponent m_NotificationsManager;
	protected SCR_EditorManagerEntity m_EditorManagerEntity;
	
	[Attribute("NotificationHolder")]
	protected string m_sNotificationHolderName;
	
	[Attribute("{C8272F3362E33359}UI/layouts/HUD/Notifications/NotificationMessage.layout")]
	protected ResourceName m_sNotificationEntityPrefab;
	
	[Attribute("{A059A7B4D6BCD553}UI/layouts/HUD/Notifications/NotificationMessageDouble.layout")]
	protected ResourceName m_sSplitNotificationEntityPrefab;
	
	[Attribute(desc: "A list of all sticky notifications to listen if they become active")]
	protected ref array<string> m_aStickyNotificationNames;
	
	[Attribute("15", desc: "Notification display time in seconds. This can be diffrent for each notification log. Can never be higher then SCR_NotificationsComponent.NOTIFICATION_DELETE_TIME")]
	protected float m_fNotificationDisplayTime;
	
	[Attribute("5", desc: "How many notifications will be displayed (not counting sticky notifications that do not effect the normal notifications) Can never be higher then SCR_NotificationsComponent.NOTIFICATION_HISTORY_LENGHT")]
	protected int m_iMaxNotifications;
	
	protected int m_iCurrentMaxNotifications;
	
	[Attribute(desc: "Notification Color array. Each enum can have it's own color")]
	protected ref array<ref SCR_NotificationDisplayColor> m_aNotificationDisplayColor;
	
	//References
	protected Widget m_wRoot;
	protected SCR_FadeUIComponent m_FadeUiComponent;
	protected Widget m_NotificationHolder;
	protected ref SCR_NotificationMessageUIComponent m_PrevOverFlowNotification;
	protected ref array<ref SCR_NotificationMessageUIComponent> m_aNotificationMessages = new ref array<ref SCR_NotificationMessageUIComponent>();
	
	//Notification Color Data Map
	ref map<ENotificationColor, SCR_NotificationDisplayColor> m_NotificationDisplayColorMap = new map<ENotificationColor, SCR_NotificationDisplayColor>;
	
	//InputType
	protected bool m_bIsUsingMouseAndKeyboard = true;
	
	protected ref ScriptInvoker Event_OnNewMessageHasPosition = new ref ScriptInvoker;
	protected ref ScriptInvoker EventOnInputDeviceChanged = new ref ScriptInvoker;
	
	
	
	//======================== ON NOTIFICATION ========================\\
	protected bool OnNotification(SCR_NotificationData data)
	{	
		if (!data)
			return false;
		
		float displayTimeLeft;
		
		//Check how much display time is left for this particular Notification log
		displayTimeLeft = m_fNotificationDisplayTime - (SCR_NotificationsComponent.NOTIFICATION_DELETE_TIME - data.GetNotificationTimeLeft());
		
		if (displayTimeLeft <= 0)
			return false;
		
		vector position;
		data.GetPosition(position);
		
		if (position != vector.Zero)
			Event_OnNewMessageHasPosition.Invoke();
		
		
		Widget newNotification;
		SCR_NotificationDisplayData displayData = data.GetDisplayData();
		if (!displayData)
			return false;
		
		SCR_UINotificationInfo uiInfo = displayData.GetNotificationUIInfo();
		if (!uiInfo)
			return false;
		
		SCR_SplitNotificationUIInfo splituiInfo = SCR_SplitNotificationUIInfo.Cast(uiInfo);
		
		//Spawn normal or split notification
		if (!splituiInfo)
			newNotification = GetGame().GetWorkspace().CreateWidgets(m_sNotificationEntityPrefab, m_NotificationHolder);
		else 
			newNotification = GetGame().GetWorkspace().CreateWidgets(m_sSplitNotificationEntityPrefab, m_NotificationHolder);
			
		if (!newNotification)
			return false;
		
		SCR_NotificationMessageUIComponent notificationMessage = SCR_NotificationMessageUIComponent.Cast(newNotification.FindHandler(SCR_NotificationMessageUIComponent));
		
		if (!notificationMessage)
			newNotification.RemoveFromHierarchy();
		
		m_aNotificationMessages.InsertAt(notificationMessage, 0);
		notificationMessage.GetOnDeleted().Insert(OnNotificationDeleted);
			
		notificationMessage.Init(data, this, displayTimeLeft * 1000);
		
		//Check if max notification amount is reached then delete latest
		if (m_aNotificationMessages.Count() > m_iCurrentMaxNotifications)
			RemoveOldestNotification();
		
		return true;
	}
	
	/*!
	Get script invoker if a new notification has a position
	\return ScriptInvoker Event_OnNewMessageHasPosition
	*/
	ScriptInvoker GetOnNewMessageHasPosition()
	{
		return Event_OnNewMessageHasPosition;
	}
	
	/*!
	Get script invoker if input device changed
	\return ScriptInvoker EventOnInputDeviceChanged
	*/
	ScriptInvoker GetOnInputDeviceChanged()
	{
		return EventOnInputDeviceChanged;
	}
	
	protected void OnNotificationDeleted(SCR_NotificationMessageUIComponent notificationMessage)
	{
		int index = m_aNotificationMessages.Find(notificationMessage);
		
		if (index >= 0)
		{
			m_aNotificationMessages.Remove(index);
		}
	}
	
	protected void RemoveOldestNotification()
	{
		if (m_aNotificationMessages.IsEmpty())
			return;
		
		int index = m_aNotificationMessages.Count() -1;
		
		//Make sure there is never more then 1 notification that was removed by overflow
		if (m_PrevOverFlowNotification)
			m_PrevOverFlowNotification.DeleteNotification();
		
		m_PrevOverFlowNotification = m_aNotificationMessages[index];
		m_aNotificationMessages[index].ForceRemoveNotification();
		
		//Can be that notification is already removed
		if (index < m_aNotificationMessages.Count())
			m_aNotificationMessages.Remove(index);
	}
	
	//======================== INPUT TYPE CHANGED ========================\\
	protected void CheckInputTypeChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{		
		//Only switched between mouse and keyboard
		if (SCR_Global.IsChangedMouseAndKeyboard(oldDevice, newDevice))
			return;
		
		m_bIsUsingMouseAndKeyboard = (newDevice == EInputDeviceType.KEYBOARD || newDevice == EInputDeviceType.MOUSE);
		EventOnInputDeviceChanged.Invoke(m_bIsUsingMouseAndKeyboard);
	}
	
	/*!
	Returns using mouse and keyboard or gampad
	\return bool if using mouse and keyboard
	*/
	bool GetIsUsingMouseAndKeyboard()
	{
		return m_bIsUsingMouseAndKeyboard;
	}
	
	/*!
	Returns EditorManagerEntity reference
	\return SCR_EditorManagerEntity m_EditorManagerEntity
	*/
	SCR_EditorManagerEntity GetEditorManager()
	{
		return m_EditorManagerEntity;
	}
	
	/*!
	Returns NotificationsManager reference
	\return SCR_NotificationsComponent m_NotificationsManager
	*/
	SCR_NotificationsComponent GetNotificationManager()
	{
		return m_NotificationsManager;
	}
	
	/*!
	Returns the color of the notification using the ENotificationColor enum
	\return color
	*/
	Color GetNotificationWidgetColor(ENotificationColor notificationColor)
	{
		if (m_NotificationDisplayColorMap.Contains(notificationColor))
		{
			return m_NotificationDisplayColorMap.Get(notificationColor).GetWidgetColor();
		}
		else 
		{
			Print("Notification color '" + typename.EnumToString(ENotificationColor, notificationColor) + "' has no color assigned to it in 'SCR_NotificationsLogComponent'", LogLevel.WARNING);
			return m_NotificationDisplayColorMap.Get(ENotificationColor.NEUTRAL).GetWidgetColor();
		}
	}
	
	/*!
	Returns the color of the notification using the ENotificationColor enum
	\return color
	*/
	Color GetNotificationTextColor(ENotificationColor notificationColor)
	{
		if (m_NotificationDisplayColorMap.Contains(notificationColor))
		{
			return m_NotificationDisplayColorMap.Get(notificationColor).GetTextColor();
		}
		else 
		{
			Print("Notification color '" + typename.EnumToString(ENotificationColor, notificationColor) + "' has no color assigned to it in 'SCR_NotificationsLogComponent'", LogLevel.WARNING);
			return m_NotificationDisplayColorMap.Get(ENotificationColor.NEUTRAL).GetTextColor();
		}
	}
	
	/*!
	Returns if the notifications have input (can be clicked on and have key hints)
	\return has notification input bool
	*/
	bool HasNotificationInput()
	{
		return m_bHasNotificationInput;
	}
	
	//======================== GENERATE COLOR MAP ========================\\
	protected void GenerateNotificationColorMap()
	{
		for(int i = 0; i < m_aNotificationDisplayColor.Count(); i++)
        {
			if (!m_NotificationDisplayColorMap.Contains(m_aNotificationDisplayColor[i].m_NotificationColor))
            	m_NotificationDisplayColorMap.Set(m_aNotificationDisplayColor[i].m_NotificationColor, m_aNotificationDisplayColor[i]);
			else
				Print("Notification Color in 'SCR_NotificationsLogComponent' has duplicate notification color key: '" + typename.EnumToString(ENotificationColor, m_aNotificationDisplayColor[i].m_NotificationColor) + "'. There should only be one of each key!", LogLevel.WARNING);
        }
	}
	
	//On each sticky notification active make sure to shrink the list and grow it again once they are inactive. 
	//This is only called by the sticky notification if it affects the list size
	protected void OnStickyNotificationChanged(bool newActive)
	{		
		if (newActive)
		{
			m_iCurrentMaxNotifications--;
			RemoveOldestNotification();
		}
		else 
		{
			m_iCurrentMaxNotifications++;
		}
	}
	
	//======================== INHERENTED ========================\\
	override void HandlerAttachedScripted(Widget w)
	{			
		m_wRoot = w;
		m_FadeUiComponent = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		
		if (m_iMaxNotifications > SCR_NotificationsComponent.NOTIFICATION_HISTORY_LENGHT)
		{
			Print("Max display Notification in notification log is higher than total notification history in notification component and thus are set the same", LogLevel.ERROR);
			m_iMaxNotifications = SCR_NotificationsComponent.NOTIFICATION_HISTORY_LENGHT;
		}
		
		m_iCurrentMaxNotifications = m_iMaxNotifications;	
		
		m_NotificationsManager = SCR_NotificationsComponent.GetInstance();
		if (!m_NotificationsManager)
		{
			Print("SCR_NotificationsLogComponent requires SCR_NotificationsComponent on PlayerController!", LogLevel.WARNING);
			w.SetVisible(false);
			return;
		}
		
		m_NotificationHolder = w.FindAnyWidget(m_sNotificationHolderName);
		if (!m_NotificationHolder)
			return;
		
		if (m_fNotificationDisplayTime > SCR_NotificationsComponent.NOTIFICATION_DELETE_TIME)
		{
			Print("Notification display time in notification log is higher than delete time in notification component and thus are set the same time", LogLevel.ERROR);
			m_fNotificationDisplayTime = SCR_NotificationsComponent.NOTIFICATION_DELETE_TIME;
		}
		
		//Fade logics. 
		m_NotificationsManager.GetOnNotification().Insert(OnNotification);
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(CheckInputTypeChanged);
			
		EInputDeviceType inputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		m_bIsUsingMouseAndKeyboard = (inputDevice == EInputDeviceType.KEYBOARD || inputDevice == EInputDeviceType.MOUSE);
		m_EditorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		
		//Generate Color map data
		GenerateNotificationColorMap();
		
		Widget stickyNotificationWidget;
		SCR_StickyNotificationUIComponent stickyNotificationComponent;
		
		foreach(string s: m_aStickyNotificationNames)
		{
			stickyNotificationWidget = w.FindAnyWidget(s);
			if (!stickyNotificationWidget)
			{
				Print(string.Format("NotificationsLog could not find stickNotification: %1", s));
				continue;
			}
				
			stickyNotificationComponent = SCR_StickyNotificationUIComponent.Cast(stickyNotificationWidget.FindHandler(SCR_StickyNotificationUIComponent));
			if (!stickyNotificationComponent)
			{
				Print(string.Format("NotificationsLog could not find SCR_StickyNotificationUIComponent on: %1", s));
				continue;
			}
			
			//Init the sticky notification
			stickyNotificationComponent.OnInit(this);
			
			if (!stickyNotificationComponent.InfluenceNotificationListSize())
				continue;
			
			stickyNotificationComponent.GetOnStickyActiveChanged().Insert(OnStickyNotificationChanged);
			
			if (stickyNotificationComponent.isStickyActive())
				OnStickyNotificationChanged(true);
		}
		
		array<SCR_NotificationData> notificationHistory = new array<SCR_NotificationData>;
		int count = m_NotificationsManager.GetHistoryOldToNew(notificationHistory, m_iCurrentMaxNotifications);
		
		//Add the notifications from old to new to set the order in UI correctly
		for(int i = 0; i < count; i++)
        {
			OnNotification(notificationHistory[i]);
        }
	}
	
	
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (!m_NotificationsManager) return;
			
		m_NotificationsManager.GetOnNotification().Remove(OnNotification);
		GetGame().OnInputDeviceUserChangedInvoker().Remove(CheckInputTypeChanged);	
	}
};

/*!
Class that saves the color data for specific ENotificationColor enums for the notification system
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotificationColor, "m_NotificationColor")]
class SCR_NotificationDisplayColor
{
	[Attribute("0", UIWidgets.ComboBox, "Color Enum", "", ParamEnumArray.FromEnum(ENotificationColor) )]
	ENotificationColor m_NotificationColor;
	
	[Attribute(defvalue: "255,255,255,255", desc: "Color of images within the notification message")]
	protected ref Color m_cWidgetNotificationColor;
	
	[Attribute(defvalue: "255,255,255,255", desc: "Color of message text. Only relevant with Split notifications")]
	protected ref Color m_TextNotificationColor;
	
	/*!
	Returns the color saved in data
	\return color for coloring widgets
	*/
	Color GetWidgetColor()
	{
		return m_cWidgetNotificationColor;
	}
	
	/*!
	Returns the color saved in data
	\return color for coloring text
	*/
	Color GetTextColor()
	{
		return m_TextNotificationColor;
	}
};