[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_VotingUIInfo: SCR_UIInfo
{
	[Attribute(desc: "Text used when starting a vote about something.")]
	protected LocalizedString m_sStartVotingName;
	
	[Attribute(desc: "Text used when canceling your vote about something.")]
	protected LocalizedString m_sCancelVotingName;
	
	[Attribute()]
	protected LocalizedString m_sStickyNotificationText;
	
	[Attribute("0", desc: "The Start notification id to which the voting type is linked. Leave UNKNOWN to not have a notification", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_iStartNotificationId;
	
	[Attribute("0", desc: "The Succeed notification id to which the voting type is linked. Leave UNKNOWN to not have a notification", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_iSucceededNotificationId;
	
	[Attribute("0", desc: "The Fail notification id to which the voting type is linked. Leave UNKNOWN to not have a notification", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_iFailedNotificationId;
	
	/*!
	\return Name used when starting a new vote.
	*/
	string GetStartVotingName()
	{
		if (m_sStartVotingName)
			return m_sStartVotingName;
		else
			return GetName();
	}
	/*!
	\return Name used when withdrawing a vote.
	*/
	string GetCancelVotingName()
	{
		if (m_sCancelVotingName)
			return m_sCancelVotingName;
		else
			return GetName();
	}
	
	/*!
	Get sticky notification text shown in the notification log when the specific voting type is active.
	\return string stricky notification text
	*/
	string GetStickyNotificationText()
	{
		return m_sStickyNotificationText;
	}
	
	/*!
	Get Start notification ID which is send to the Notification Component to show the specific notification when the voting starts
	\return ENotification Notification ID of Start voting
	*/
	ENotification GetVotingStartNotification()
	{
		return m_iStartNotificationId;
	}	
	
	/*!
	Get Succeed notification ID which is send to the Notification Component to show the specific notification when voting ends and succeeds
	\return ENotification Notification ID of successful voting
	*/
	ENotification GetVotingSucceedNotification()
	{
		return m_iSucceededNotificationId;
	}	
	
	/*!
	Get Fail notification ID which is send to the Notification Component to show the specific notification when voting ends and fails
	\return ENotification Notification ID of failed voting
	*/
	ENotification GetVotingFailNotification()
	{
		return m_iFailedNotificationId;
	}
};