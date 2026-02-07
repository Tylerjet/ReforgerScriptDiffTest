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

	[Attribute(defvalue: "MOTDText")]
	protected string m_sMOTDTextWidget;

	[Attribute(defvalue: "Ping")]
	protected string m_sPingWidget;

	[Attribute(defvalue: "Statuses")]
	protected string m_sLinesParentWidget;

	[Attribute(defvalue: "999", params: "1 inf 1")]
	protected int m_iMaxPing;

	[Attribute(defvalue: " - ")]
	protected string m_sNoPing;

	[Attribute(defvalue: " ... ")]
	protected string m_sUpdatingPing;

	[Attribute(defvalue: "%1+", desc: "Can use %1 for Max Ping display (default \"%1+\" e.g \"999+\")")]
	protected string m_sBigPing;

	protected Widget m_wMOTDWidget;
	protected TextWidget m_wMOTDTextWidget;
	protected TextWidget m_wPingWidget;
	protected Widget m_wLinesParentWidget;
	protected string m_sPingText;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!m_LineLayout || !m_aServices || m_aServices.IsEmpty())
			return;

		// MOTD
		m_wMOTDWidget = SCR_WidgetHelper.GetWidgetOrChild(w, m_sMOTDWidget);
		if (m_wMOTDWidget)
			m_wMOTDTextWidget = TextWidget.Cast(m_wMOTDWidget.FindWidget(m_sMOTDTextWidget));

		// ping
		m_wPingWidget = TextWidget.Cast(SCR_WidgetHelper.GetWidgetOrChild(w, m_sPingWidget));
		if (m_wPingWidget)
			m_sPingText = m_wPingWidget.GetText();

		// lines
		m_wLinesParentWidget = SCR_WidgetHelper.GetWidgetOrChild(w, m_sLinesParentWidget);
		if (!m_wLinesParentWidget)
			m_wLinesParentWidget = w;
		if (!m_wLinesParentWidget)
			return;

		CreateLines(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateLines(Widget w)
	{
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
		if (!m_wMOTDWidget || !m_wMOTDTextWidget)
			return;

		message.TrimInPlace();

		if (message.IsEmpty())
		{
			m_wMOTDWidget.SetVisible(false);
		}
		else
		{
			m_wMOTDWidget.SetVisible(true);
			m_wMOTDTextWidget.SetText(message);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set the ping value (formats -1 to " - ", 0 to " ..." and >999 value to "999+")
	//! \param pingInMs ping in milliseconds
	void SetPing(int pingInMs)
	{
		if (!m_wPingWidget)
			return;

		string sPing;
		if (pingInMs == -1)
			sPing = m_sNoPing;
		else if (pingInMs == 0)
			sPing = m_sUpdatingPing;
		else if (pingInMs < 0 || pingInMs > m_iMaxPing)
			sPing = string.Format(m_sBigPing, m_iMaxPing);
		else
			sPing = pingInMs.ToString();

		m_wPingWidget.SetTextFormat(m_sPingText, sPing);
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_ServicesStatusDialogComponent_Service> GetAllServices()
	{
		return m_aServices;
	}

	//------------------------------------------------------------------------------------------------
	void SetAllServicesState(EServiceStatus status)
	{
		foreach (SCR_ServicesStatusDialogComponent_Service serviceInfo : m_aServices)
			SetServiceState(serviceInfo.m_sId, status);
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
			SetStatusImageAndColor(iconWidget, status);
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
	void SetStatusImageAndColor(ImageWidget iconWidget, EServiceStatus serviceStatus)
	{
		SCR_ServicesStatusDialogComponent_Status status = GetStatus(serviceStatus);
		if (!status)
			return;

		iconWidget.LoadImageFromSet(0, status.m_sImageSet, status.m_sIcon);
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
	[Attribute(defvalue: "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", params: "imageset")]
	ResourceName m_sImageSet;

	[Attribute(defvalue: SCR_Enum.GetDefault(EServiceStatus.RUNNING), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EServiceStatus))]
	EServiceStatus m_Status;

	[Attribute(defvalue: "okCircle")]
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

	[Attribute()]
	string m_sServiceId;

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
