class SCR_ServicesStatusDialogComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected ref array<ref SCR_ServicesStatusDialogComponent_Service> m_aServices;

	[Attribute()]
	protected ref array<ref SCR_ServicesStatusDialogComponent_Status> m_aStatuses;

	[Attribute(defvalue: "{D6EA742398E63066}UI/layouts/Menus/Dialogs/ServiceStatusLine.layout", params: "layout")]
	protected ResourceName m_LineLayout;

	[Attribute(defvalue: "MOTD")]
	protected string m_sMOTDWidget;

	[Attribute(defvalue: "Statuses")]
	protected string m_sLinesParentWidget;

	protected Widget m_wMOTDWidget;
	protected Widget m_wLinesParentWidget;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		CreateLines(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateLines(Widget w)
	{
		if (!m_LineLayout || !m_aServices || m_aServices.IsEmpty())
			return;

		// MOTD
		m_wMOTDWidget = SCR_WidgetHelper.GetWidgetOrChild(w, m_sMOTDWidget);

		// lines
		m_wLinesParentWidget = SCR_WidgetHelper.GetWidgetOrChild(w, m_sLinesParentWidget);

		if (!m_wLinesParentWidget)
			m_wLinesParentWidget = w;

		if (!m_wLinesParentWidget)
			return;

		SCR_ServicesStatusDialogComponent_Status status;
		if (m_aStatuses && !m_aStatuses.IsEmpty())
			status = m_aStatuses[0];

		Widget line;
		TextWidget titleWidget;
		foreach (SCR_ServicesStatusDialogComponent_Service service : m_aServices)
		{
			line = GetGame().GetWorkspace().CreateWidgets(m_LineLayout, m_wLinesParentWidget);
			if (!line)
				continue;

			line.SetName(service.m_sId);

			titleWidget = TextWidget.Cast(line.FindAnyWidget("Text"));
			if (titleWidget)
				titleWidget.SetText(service.m_sTitle);

			if (status)
				SetServiceState(service.m_sId, status.m_Status);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetMOTD(string message = string.Empty)
	{
		if (!m_wMOTDWidget)
			return;

		TextWidget motdText = TextWidget.Cast(m_wMOTDWidget.FindWidget("MOTDText"));
		if (!motdText)
			return;

		message.TrimInPlace();

		if (message.IsEmpty())
		{
			m_wMOTDWidget.SetVisible(false);
		}
		else
		{
			m_wMOTDWidget.SetVisible(true);
			motdText.SetText(message);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetServiceState(string serviceId, EServiceStatus status)
	{
		Widget line = m_wLinesParentWidget.FindAnyWidget(serviceId);
		if (!line)
			return;

		ImageWidget backgroundWidget = ImageWidget.Cast(line.FindAnyWidget("Background"));
		if (backgroundWidget)
			SetStatusBackground(backgroundWidget, status);

		ImageWidget iconWidget = ImageWidget.Cast(line.FindAnyWidget("Icon"));
		if (iconWidget)
			SetStatusImage(iconWidget, status);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetStatusBackground(ImageWidget backgroundWidget, EServiceStatus serviceStatus)
	{
		SCR_ServicesStatusDialogComponent_Status status = GetStatus(serviceStatus);
		if (!status)
			return;

		backgroundWidget.SetColor(status.m_sBackgroundColor);
	}

	//------------------------------------------------------------------------------------------------
	void SetStatusImage(ImageWidget iconWidget, EServiceStatus serviceStatus)
	{
		SCR_ServicesStatusDialogComponent_Status status = GetStatus(serviceStatus);
		if (!status)
			return;

		iconWidget.LoadImageFromSet(0, status.m_ImageSet, status.m_sIcon);
		iconWidget.SetColor(status.m_sIconColor);
	}

	//------------------------------------------------------------------------------------------------
	void SetStatusImageColor(ImageWidget iconWidget, EServiceStatus serviceStatus)
	{
		SCR_ServicesStatusDialogComponent_Status status = GetStatus(serviceStatus);
		if (!status)
			return;

		iconWidget.SetColor(status.m_sIconColor);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetStatusText(TextWidget textWidget, EServiceStatus status, string serviceId)
	{
		SCR_ServicesStatusDialogComponent_Service service = GetService(serviceId);
		if (!service)
			return;

		foreach (SCR_ServicesStatusDialogComponent_Service_StatusMessage message : service.m_aStatusMessages)
		{
			if (message.status == status)
			{
				textWidget.SetText(message.m_sMessage);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ServicesStatusDialogComponent_Service GetService(string serviceId)
	{
		if (!m_aServices)
			return null;

		foreach (SCR_ServicesStatusDialogComponent_Service service : m_aServices)
		{
			if (service.m_sId.ToLower() == serviceId.ToLower())
				return service;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ServicesStatusDialogComponent_Status GetStatus(EServiceStatus serviceStatus)
	{
		if (!m_aStatuses)
			return null;

		foreach (SCR_ServicesStatusDialogComponent_Status status : m_aStatuses)
		{
			if (status.m_Status == serviceStatus)
				return status;
		}
		return null;
	}
};

[BaseContainerProps()]
class SCR_ServicesStatusDialogComponent_Status
{
	[Attribute(defvalue: "{1F0A6C9C19E131C6}UI/Textures/Icons/icons_wrapperUI.imageset", params: "imageset")]
	ResourceName m_ImageSet;

	[Attribute(defvalue: SCR_Enum.GetDefault(EServiceStatus.RUNNING), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EServiceStatus))]
	EServiceStatus m_Status;

	[Attribute(defvalue: "circle-check")]
	string m_sIcon;

	[Attribute(defvalue: "1 1 1 1")]
	ref Color m_sIconColor;

	// no textColor for now

	[Attribute(defvalue: "0 0 0 0")]
	ref Color m_sBackgroundColor;
};

[BaseContainerProps()]
class SCR_ServicesStatusDialogComponent_Service
{
	[Attribute()]
	string m_sId;

	[Attribute(defvalue: "#AR-ServicesStatus_Service_XXX")]
	string m_sTitle;

	[Attribute()]
	ref array<ref SCR_ServicesStatusDialogComponent_Service_StatusMessage> m_aStatusMessages;
};

[BaseContainerProps()]
class SCR_ServicesStatusDialogComponent_Service_StatusMessage
{
	[Attribute(defvalue: SCR_Enum.GetDefault(EServiceStatus.WARNING), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EServiceStatus))]
	EServiceStatus status;

	[Attribute(defvalue: "#AR-ServicesStatus_Service_XXX_Warning")]
	string m_sMessage;
};
