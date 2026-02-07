//-------------------------------------------------------------------------------------------
//! Configurable dialog can be configured via .conf presets.
//! It has many options for button positions, title, message, colors, style, ...
//!
//! If you add buttons "confirm" or "cancel", they are automatically bound
//! to methods OnCancel and OnConfirm
class SCR_ConfigurableDialogUi: ScriptedWidgetComponent
{
	// Attributes
	[Attribute((1/UIConstants.FADE_RATE_FAST).ToString(), UIWidgets.Auto, "Duration of fade in and fade out animations")]
	protected float m_fFadeInTime;
	
	[Attribute("{08CF3B69CB1ACBC4}UI/layouts/WidgetLibrary/Buttons/WLib_NavigationButton.layout", UIWidgets.ResourceNamePicker, "Layout of the navigation button", params: "layout")]
	protected ResourceName m_sNavigationButtonLayout;
	
	[Attribute("ButtonsLeft", UIWidgets.Auto, "Widget name of left buttons layout")]
	protected string m_sWidgetNameButtonsLeft;
	
	[Attribute("ButtonsRight", UIWidgets.Auto, "Widget name of right buttons layout")]
	protected string m_sWidgetNameButtonsRight;
	
	[Attribute("ButtonsCenter", UIWidgets.Auto, "Widget name of center buttons layout")]
	protected string m_sWidgetNameButtonsCenter;
	
	// Script Invokers
	ref ScriptInvoker m_OnConfirm = new ScriptInvoker();	// (SCR_ConfigurableDialogUi dlg) - Called when Confirm button was clicked, if you don't override OnConfirm of this class
	ref ScriptInvoker m_OnCancel = new ScriptInvoker();		// (SCR_ConfigurableDialogUi dlg) - Called when Cancel button was clicked, if you don't override OnConfirm of this class
	ref ScriptInvoker m_OnClose = new ScriptInvoker();		// (SCR_ConfigurableDialogUi dlg) - Called when dialog is closed, AFTER the fade out animation is over
	
	// Widgets
	protected ImageWidget m_wImgTopLine;
	protected ImageWidget m_wImgTitleIcon;
	protected TextWidget m_wTitle;
	protected TextWidget m_wMessage;
	protected VerticalLayoutWidget m_wContentVerticalLayout;
	
	// Other
	protected ref map<string, SCR_NavigationButtonComponent> m_aButtonComponents = new map<string, SCR_NavigationButtonComponent>;
	protected Widget m_wRoot;
	
	// Menu handler which performs menu-specific actions.
	protected SCR_ConfigurableDialogUiProxy m_ProxyMenu;
	
	// Current dialog preset for data 
	protected ref SCR_ConfigurableDialogUiPreset m_DialogPreset;
	
	
	
	
	//-------------------------------------------------------------------------------------------------------------------------
	//                                             P U B L I C   A P I 
	//-------------------------------------------------------------------------------------------------------------------------
	
	
	
	
	//-----------------------------------------------------------------------
	//! Creates a dialog from preset
	static SCR_ConfigurableDialogUi CreateFromPreset(ResourceName presetsResourceName, string tag, SCR_ConfigurableDialogUi customDialogObj = null)
	{
		// Create presets
		Resource rsc = BaseContainerTools.LoadContainer(presetsResourceName);
		BaseContainer container = rsc.GetResource().ToBaseContainer();
		SCR_ConfigurableDialogUiPresets presets = SCR_ConfigurableDialogUiPresets.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		
		// Find preset
		SCR_ConfigurableDialogUiPreset preset = presets.FindPreset(tag);
		
		// Bail if preset is not found
		if (!preset)
		{
			Print(string.Format("[SCR_ConfigurableDialogUi] Preset was not found: %1, %2", presets, tag), LogLevel.ERROR);
			return null;
		}
		
		SCR_ConfigurableDialogUi dialog = CreateByPreset(preset, customDialogObj);
		
		return dialog;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateByPreset(SCR_ConfigurableDialogUiPreset preset, SCR_ConfigurableDialogUi customDialogObj = null)
	{
		// Open the proxy dialog
		SCR_ConfigurableDialogUiProxy proxyComp = SCR_ConfigurableDialogUiProxy.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ConfigurableDialog));
		
		// Create the actual layout inside proxy
		Widget internalWidget = GetGame().GetWorkspace().CreateWidgets(preset.m_sLayout, proxyComp.GetRootWidget());
		if (!internalWidget)
		{
			Print(string.Format("[SCR_ConfigurableDialogUi] internalWidget wans't created"), LogLevel.ERROR);
			return null;
		}
		
		SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.Cast(internalWidget.FindHandler(SCR_ConfigurableDialogUi));
		
		// Create a new dialog object, or apply the provided one, if the dialog obj was not found in the layout.
		if (!dialog)
		{
			if (customDialogObj)
				dialog = customDialogObj;
			else
				dialog = new SCR_ConfigurableDialogUi();
			dialog.InitAttributedVariables();
			internalWidget.AddHandler(dialog);
		}
		
		dialog.Init(internalWidget, preset, proxyComp);
		proxyComp.Init(dialog);
		
		// Set action context
		if (!preset.m_sActionContext.IsEmpty())
			proxyComp.SetActionContext(preset.m_sActionContext);
		
		// Call dialog's events manually
		dialog.OnMenuOpen(preset);
		
		return dialog;
	}
	
	
	//------------------------------------------------------------------------------------------------
	// Closes the dialogue with a fade-out animation
	void Close()
	{
		AnimateWidget.Opacity(GetRootWidget(), 0, 1 / m_fFadeInTime);
		GetGame().GetCallqueue().CallLater(Internal_Close, m_fFadeInTime * 1000.0);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	void SetTitle(string text)
	{
		if (m_wTitle)
			m_wTitle.SetText(text);
	}
	

	
	//------------------------------------------------------------------------------------------------
	void SetMessage(string text)
	{
		if (m_wMessage)
		{
			m_wMessage.SetVisible(true);
			m_wMessage.SetText(text);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	TextWidget GetMessageWidget() { return m_wMessage; }
	
	//------------------------------------------------------------------------------------------------
	string GetMessageStr()
	{
		if (m_wMessage)
			return m_wMessage.GetText();
		
		return "";
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Set title icons with custom image 
	void SetTitleIcon(ResourceName image, string imageName)
	{
		if (!m_wImgTitleIcon)
			return;
		
		//  Set image by input 
		if (image.EndsWith("imageset"))
			m_wImgTitleIcon.LoadImageFromSet(0, image, imageName);
		else
			m_wImgTitleIcon.LoadImageTexture(0, image);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Sets colors based on style
	void SetStyle(EConfigurableDialogStyle type)
	{	
		// Check widgets 
		if (!m_wImgTopLine || !m_wImgTitleIcon)
			return;
		
		// Select color
		Color color = Color.White;
		
		switch (type)
		{
			case EConfigurableDialogStyle.ACTION:
			color = UIColors.CONTRAST_CLICKED_HOVERED;
			break;
			
			case EConfigurableDialogStyle.WARNING:
			color = UIColors.WARNING;
			break;
		}
		
		// Set colors 
		m_wImgTopLine.SetColor(color);
		m_wImgTitleIcon.SetColor(color);
	}
	
	
	
	
	//-------------------------------------------------------------------------------------------------------------------------
	//                                       P R O T E C T E D   M E T H O D S
	//-------------------------------------------------------------------------------------------------------------------------
	
	
	
	
	
	//-------------------------------------------------------------------------------------------------------------------------
	// C A N   B E   O V E R R I D E N   I N   I N H E R I T E D   C L A S S E S 
	//-------------------------------------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------
	protected void OnConfirm()
	{
		Close();
		m_OnConfirm.Invoke(this);
	}
	
	
	//----------------------------------------------------------------------------------------
	protected void OnCancel()
	{
		Close();
		m_OnCancel.Invoke(this);
	}
	
	
	//----------------------------------------------------------------------------------------
	//! Called last of all, after all the initialization of main element done.
	//! Here you can perform custom initialization.
	protected void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset);
	
	
	//----------------------------------------------------------------------------------------
	void OnMenuUpdate(float tDelta);

	//
	//
	//-------------------------------------------------------------------------------------------------------------------------
	
	
	
	
	
	
	//----------------------------------------------------------------------------------------
	//! Returns a button with given tag
	void CreateButton(SCR_ConfigurableDialogUiButtonPreset buttonPreset)
	{
		HorizontalLayoutWidget wLayout;
		const float buttonPadding = 8.0;
		float paddingLeft, paddingRight;
		switch (buttonPreset.m_eAlign)
		{
			case EConfigurableDialogUiButtonAlign.LEFT:
				wLayout = HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsLeft));
				paddingRight = buttonPadding;
				break;
			
			case EConfigurableDialogUiButtonAlign.RIGHT:
				wLayout = HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsRight));
				paddingLeft = buttonPadding;
				break;
			
			default:
				wLayout = HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsCenter)); break;
		}
		
		Widget wButton = GetGame().GetWorkspace().CreateWidgets(m_sNavigationButtonLayout, wLayout);
		
		SCR_NavigationButtonComponent comp = SCR_NavigationButtonComponent.Cast(wButton.FindHandler(SCR_NavigationButtonComponent));
			
		if (!comp)
			return;
		
		comp.SetLabel(buttonPreset.m_sLabel);
		comp.SetAction(buttonPreset.m_sActionName);
		
		comp.SetHoverSound(buttonPreset.m_sSoundHovered);
		comp.SetClickedSound(buttonPreset.m_sSoundClicked);
		
		m_aButtonComponents.Insert(buttonPreset.m_sTag, comp);
	}
	
	//----------------------------------------------------------------------------------------
	//! Returns a button with given tag
	SCR_NavigationButtonComponent FindButton(string tag)
	{
		return m_aButtonComponents.Get(tag);
	}
	
	//----------------------------------------------------------------------------------------
	void ClearButtons()
	{
		m_aButtonComponents.Clear();
	}
	
	
	
	//----------------------------------------------------------------------------------------
	protected void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, SCR_ConfigurableDialogUiProxy proxyMenu)
	{
		m_ProxyMenu = proxyMenu;
		m_DialogPreset = preset;
		m_wRoot = root;		
		
		InitWidgets();
		
		// Set title
		SetTitle(preset.m_sTitle);
		
		// Set message
		if (!preset.m_sMessage.IsEmpty())
		{
			SetMessage(preset.m_sMessage);
		}
		
		// Set style
		SetStyle(preset.m_eVisualStyle);
		
		// Set title image
		if (!preset.m_sTitleIconTexture.IsEmpty())
		{
			SetTitleIcon(preset.m_sTitleIconTexture, preset.m_sTitleIconImageName);
		}
			
		// Create buttons
		HorizontalLayoutWidget wLeft =		HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsLeft));
		HorizontalLayoutWidget wRight =		HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsRight));
		HorizontalLayoutWidget wCenter =	HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsCenter));
		
		foreach (SCR_ConfigurableDialogUiButtonPreset buttonPreset : preset.m_aButtons)
		{
			/*HorizontalLayoutWidget wLayout;
			const float buttonPadding = 8.0;
			float paddingLeft, paddingRight;
			switch (buttonPreset.m_eAlign)
			{
				case EConfigurableDialogUiButtonAlign.LEFT:
					wLayout = wLeft;
					paddingRight = buttonPadding;
					break;
				
				case EConfigurableDialogUiButtonAlign.RIGHT:
					wLayout = wRight;
					paddingLeft = buttonPadding;
					break;
				
				default:
					wLayout = wCenter; break;
			}
			
			Widget wButton = GetGame().GetWorkspace().CreateWidgets(m_sNavigationButtonLayout, wLayout);
			
			if (!wButton)
				break;
			
			HorizontalLayoutSlot.SetPadding(wButton, paddingLeft, 0, paddingRight, 0);
			
			SCR_NavigationButtonComponent comp = SCR_NavigationButtonComponent.Cast(wButton.FindHandler(SCR_NavigationButtonComponent));
			
			if (!comp)
				break;
			
			comp.SetLabel(buttonPreset.m_sLabel);
			comp.SetAction(buttonPreset.m_sActionName);
			
			comp.SetHoverSound(buttonPreset.m_sSoundHovered);
			comp.SetClickedSound(buttonPreset.m_sSoundClicked);
			
			m_aButtonComponents.Insert(buttonPreset.m_sTag, comp);*/
			
			CreateButton(buttonPreset);
		}
		
		// Bind actions to default buttons (if they were created)
		SCR_NavigationButtonComponent buttonConfirm = FindButton("confirm");
		if (buttonConfirm)
			buttonConfirm.m_OnActivated.Insert(OnConfirm);
		
		SCR_NavigationButtonComponent buttonCancel = FindButton("cancel");
		if (buttonCancel)
			buttonCancel.m_OnActivated.Insert(OnCancel);
	}
	
	
	//----------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		Widget w = m_wRoot;
		
		// Images 
		m_wImgTopLine = ImageWidget.Cast(w.FindAnyWidget("Separator"));
		m_wImgTitleIcon = ImageWidget.Cast(w.FindAnyWidget("ImgTitleIcon"));

		// Texts 
		m_wTitle = TextWidget.Cast(w.FindAnyWidget("Title"));
		m_wMessage = TextWidget.Cast(w.FindAnyWidget("Message"));
		
		// Verical layout widget
		m_wContentVerticalLayout = VerticalLayoutWidget.Cast(w.FindAnyWidget("ContentVerticalLayout"));
		
		// Play animation
		w.SetOpacity(0);
		AnimateWidget.Opacity(w, 1, 1 / m_fFadeInTime);
	}
	
	
	//----------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//----------------------------------------------------------------------------------------
	SCR_ConfigurableDialogUiPreset GetDialogPreset()
	{
		return m_DialogPreset;
	}
	
	//----------------------------------------------------------------------------------------
	protected void Internal_Close()
	{
		if (!m_ProxyMenu)
			return;
		
		m_ProxyMenu.Close();
		m_OnClose.Invoke(this);
	}
	
	//----------------------------------------------------------------------------------------
	//! Verifies that all attributed variables are set up.
	//! When we create this object with 'new' keyword, the game doesn't assign default values to attributed variables.
	protected void InitAttributedVariables()
	{	
		if (m_sNavigationButtonLayout.IsEmpty())
			m_sNavigationButtonLayout = "{08CF3B69CB1ACBC4}UI/layouts/WidgetLibrary/Buttons/WLib_NavigationButton.layout";
		
		if (m_sWidgetNameButtonsLeft.IsEmpty())
			m_sWidgetNameButtonsLeft = "ButtonsLeft";
		
		if (m_sWidgetNameButtonsRight.IsEmpty())
			m_sWidgetNameButtonsRight = "ButtonsRight";
		
		if (m_sWidgetNameButtonsCenter.IsEmpty())
			m_sWidgetNameButtonsCenter = "ButtonsCenter";
		
		if (m_fFadeInTime == 0)
			m_fFadeInTime = 1/UIConstants.FADE_RATE_FAST;
	}
};



//-------------------------------------------------------------------------------------------
//! It is here to expose the Menu API to the configurable dialog instance.
class SCR_ConfigurableDialogUiProxy : ChimeraMenuBase
{
	protected SCR_ConfigurableDialogUi m_Dlg;
	
	void Init(SCR_ConfigurableDialogUi dlg)
	{
		m_Dlg = dlg;
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		m_Dlg.OnMenuUpdate(tDelta);
	}
};



//-------------------------------------------------------------------------------------------
//! Configuration for button alignment
enum EConfigurableDialogUiButtonAlign
{
	LEFT = 0,
	RIGHT,
	CENTER
};



//------------------------------------------------------------------------------------------------
//! Style of the dialog
enum EConfigurableDialogStyle
{
	NEUTRAL,
	ACTION,
	WARNING,
};








//-------------------------------------------------------------------------------------------
//		C O N F I G U R A T I O N   C L A S S E S 
//-------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------
//! Configuration for a button
[BaseContainerProps()]
class SCR_ConfigurableDialogUiButtonPreset
{
	[Attribute("", UIWidgets.Auto, "Custom tag, used for finding this button at run time")]
	string m_sTag;
	
	[Attribute("", UIWidgets.Auto, "Action name the button will listen to")]
	string m_sActionName;
	
	[Attribute("", UIWidgets.Auto, "Label of the button")]
	string m_sLabel;
	
	[Attribute("0", UIWidgets.ComboBox, "Alignment of the button", "", ParamEnumArray.FromEnum(EConfigurableDialogUiButtonAlign))]
	EConfigurableDialogUiButtonAlign m_eAlign;
	
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER, UIWidgets.EditBox, "")]
	string m_sSoundHovered;

	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	string m_sSoundClicked;
};



//-------------------------------------------------------------------------------------------
//! Configuration for a dialog
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sTag")]
class SCR_ConfigurableDialogUiPreset
{
	[Attribute("{E6B607B27BCC1477}UI/layouts/Menus/Dialogs/ConfigurableDialog.layout", UIWidgets.ResourceNamePicker, ".layout of the dialog", params: "layout")]
	ResourceName m_sLayout;
	
	[Attribute("", UIWidgets.Auto, "Custom tag, used for finding this preset at run time.")]
	string m_sTag;
	
	[Attribute("0", UIWidgets.ComboBox, "Visual style. Affects color of elements.", "", ParamEnumArray.FromEnum(EDialogType))]
	EDialogType m_eVisualStyle;
	
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", UIWidgets.ResourcePickerThumbnail, "Texture or imageset for title icon", "edds imageset")]
	ResourceName m_sTitleIconTexture;
	
	[Attribute("misc", UIWidgets.Auto)]
	string m_sTitleIconImageName;
	
	[Attribute("", UIWidgets.Auto, "Message to be displayed inside, if messaage widget is present.")]
	string m_sMessage;
	
	[Attribute("", UIWidgets.Auto, "Title of the dialog")]
	string m_sTitle;
	
	[Attribute()]
	ref array<ref SCR_ConfigurableDialogUiButtonPreset> m_aButtons;
	
	[Attribute(string.Empty, UIWidgets.EditBox, "Action context of the menu. If empty, MenuContext is used, same as in standard menu manager.")]
	string m_sActionContext;
};



//-------------------------------------------------------------------------------------------
//! Class for a .conf file with multiple presets.
[BaseContainerProps(configRoot : true)]
class SCR_ConfigurableDialogUiPresets
{
	[Attribute()]
	ref array<ref SCR_ConfigurableDialogUiPreset> m_aPresets;
	
	
	//-------------------------------------------------------------------------------------------
	//! Finds a preset by tag
	SCR_ConfigurableDialogUiPreset FindPreset(string tag)
	{
		foreach (auto i : m_aPresets)
		{
			if (i.m_sTag == tag)
				return i;
		}
		
		return null;
	}
};