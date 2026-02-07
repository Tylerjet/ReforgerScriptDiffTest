//! Base class for handling selection menu entry widgets 
//! Handle all UI elemenents related to the entry in this dedicated class

//------------------------------------------------------------------------------------------------
class SCR_SelectionEntryWidgetComponent : ScriptedWidgetComponent 
{
	// Default widget name 
	const static string WIDGET_IMAGE_ICON = "ImgIcon";
	const static string WIDGET_PREVIEW_ITEM = "PreviewItem";
	const static string WIDGET_TEXT_LABEL = "TxtLabel";
	
	const static string WIDGET_DEFAULT_IMAGE = "{EB870A5DD7CE3A9A}UI/Textures/Editor/ContextMenu/ContextAction_Default.edds"; 
	
	// Widget editable name references
	[Attribute(WIDGET_IMAGE_ICON, UIWidgets.EditBox, "Reference to entry icon widget name.")]
	protected string m_sWidgetImageIcon;
	
	[Attribute(WIDGET_PREVIEW_ITEM, UIWidgets.EditBox, "Reference to entry item preview widget name for rendering items.")]
	protected string m_sWidgetRenderItem;
	
	[Attribute(WIDGET_TEXT_LABEL, UIWidgets.EditBox, "Reference to entry label name")]
	protected string m_sWidgetTextLabel;
	
	[Attribute(WIDGET_DEFAULT_IMAGE, UIWidgets.EditBox, "Default icon image texture")]
	protected ResourceName m_DefaultIconImage;
	
	[Attribute("", UIWidgets.EditBox, "Default icon label")]
	protected string m_sDefaultLabel;
	
	protected string m_sImageTexture;
	protected string m_sImageName;
	
	// Widgets 
	protected Widget m_wRoot;
	protected ImageWidget m_wImgIcon;
	protected ItemPreviewWidget m_wPreviewItem;
	protected TextWidget m_wTxtLabel;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wRoot = w;
		
		// Widget Setup 
		FindWidgets();
		SetupWidgetsDefaults();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Base function for general updates
	void UpdateData() {}
	
	//------------------------------------------------------------------------------------------------
	//! Find component widget references 
	protected void FindWidgets()
	{
		m_wImgIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sWidgetImageIcon));
		m_wPreviewItem = ItemPreviewWidget.Cast(m_wRoot.FindAnyWidget(m_sWidgetRenderItem));
		m_wTxtLabel = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sWidgetTextLabel));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set inital state for widgets 
	//! Should be called when attaching handler and reseting widgets to default state 
	void SetupWidgetsDefaults()
	{
		SetIcon(m_DefaultIconImage);
		if (m_wImgIcon)
			m_wImgIcon.SetVisible(true);
		
		SetLabelText(m_sDefaultLabel);
		
		if (m_wPreviewItem)
			m_wPreviewItem.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry icon by given image or image from set 
	//! Will be hidden if no icon is set 
	void SetIconFromData()
	{
		if (!m_wImgIcon)
			return;
		
		// No resource?
		if (m_sImageTexture.IsEmpty())
			return;
		
		// Apply specific image or image from set
		if (m_sImageTexture.EndsWith("imageset"))
			m_wImgIcon.LoadImageFromSet(0, m_sImageTexture, m_sImageName);
		else
			m_wImgIcon.LoadImageTexture(0, m_sImageTexture);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set texture data and icon
	void SetIcon(ResourceName imageResource, string imageName = "")
	{
		SetTextureData(imageResource, imageName);
		SetIconFromData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconSize(float w, float h,)
	{
		if (!m_wImgIcon)
			return;
		
		FrameSlot.SetSize(m_wImgIcon, w, h);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconFaded(bool faded, float opacity = 0.25)
	{
		if (!m_wImgIcon)
			return;
		
		if (faded)
			m_wImgIcon.SetOpacity(opacity);
		else
			m_wImgIcon.SetOpacity(1);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconVisible(bool visible)
	{
		if (m_wImgIcon)	
			m_wImgIcon.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepare preview item and render given entity 
	void SetPreviewItem(IEntity item)
	{
		if (!m_wPreviewItem)
			return;
		
		// Visibility 
		m_wPreviewItem.SetVisible(item != null);
		if (!item)
			return;

		// Get manager and render preview 
		ItemPreviewManagerEntity manager = GetGame().GetItemPreviewManager();
		if (!manager)
			return;
		
		ItemPreviewWidget renderPreview = ItemPreviewWidget.Cast( m_wPreviewItem );
		if (!renderPreview)
			return;
		
		// Set rendering and preview properties 
		manager.SetPreviewItem(renderPreview, item);
		m_wPreviewItem.SetResolutionScale(1, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLabelText(string text)
	{
		if (!m_wTxtLabel)
			return;
		
		m_wTxtLabel.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	TextWidget GetTxtLabel() { return m_wTxtLabel; }
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetIcon() { return m_wImgIcon; }
	
	//------------------------------------------------------------------------------------------------
	string GetImageTexture() { return m_sImageTexture; }
	
	//------------------------------------------------------------------------------------------------
	string GetImageName() { return m_sImageName; }
	
	//------------------------------------------------------------------------------------------------
	void SetTextureData(ResourceName imageResource, string imageName = "")
	{
		m_sImageTexture = imageResource;
		m_sImageName = imageName;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget() { return m_wRoot; }
};