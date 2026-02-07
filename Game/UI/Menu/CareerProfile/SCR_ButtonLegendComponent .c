//------------------------------------------------------------------------------------------------
class SCR_ButtonLegendComponent : SCR_WLibComponentBase 
{
	[Attribute()]
	protected LocalizedString m_sSpecializationText;
	
	protected TextWidget m_wSpecializationText;
	protected ImageWidget m_wSpecializationImage;
	
	protected int m_ibuttonId;

	[Attribute()]
	protected bool m_bIsActive;
	
	[Attribute("1 1 1 0.1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorInactive;
	
	[Attribute("1 1 1 0.3", UIWidgets.ColorPicker)]
	protected ref Color m_ColorInactiveHovered;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorActive;
	
	// Arguments passed: this
	protected ref ScriptInvoker m_OnClicked = new ref ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wSpecializationText = TextWidget.Cast(w.FindAnyWidget("SpecializationText"));
		if (!m_wSpecializationText)
			return;
		
		m_wSpecializationImage = ImageWidget.Cast(w.FindAnyWidget("SpecializationImage"));
		if(!m_wSpecializationImage)
			return;
		
		m_wSpecializationText.SetText(m_sSpecializationText);
		m_wSpecializationText.SetColor(m_ColorInactive);
		m_wSpecializationImage.SetColor(m_ColorInactive);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnClicked()
	{
		return m_OnClicked;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w,x,y,button);
		if (button != 0)
			return false;
		
		m_OnClicked.Invoke(this);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		
		if (!m_bIsActive)
		{
			AnimateWidget.Color(m_wSpecializationImage, m_ColorInactiveHovered, m_fAnimationRate);
			AnimateWidget.Color(m_wSpecializationText, m_ColorInactiveHovered, m_fAnimationRate);
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		
		if (!m_bIsActive)
		{
			AnimateWidget.Color(m_wSpecializationImage, m_ColorInactive, m_fAnimationRate);
			AnimateWidget.Color(m_wSpecializationText, m_ColorInactive, m_fAnimationRate);
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ActivateLegend(bool animate = true)
	{
		if (m_bIsActive)
			return;
		
		m_bIsActive = true;
		PlaySound(m_sSoundClicked);
		
		if (animate)
		{
			AnimateWidget.Color(m_wSpecializationImage, m_ColorActive, m_fAnimationRate);
			AnimateWidget.Color(m_wSpecializationText, m_ColorActive, m_fAnimationRate);
		}
		else
		{
			AnimateWidget.StopAnimation(m_wSpecializationImage, WidgetAnimationColor);
			m_wSpecializationImage.SetColor(m_ColorActive);
			
			AnimateWidget.StopAnimation(m_wSpecializationText, WidgetAnimationColor);
			m_wSpecializationImage.SetColor(m_ColorActive);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void DeactivateLegend(bool animate = true)
	{
		if (!m_bIsActive)
			return;
		
		m_bIsActive = false;
		PlaySound(m_sSoundClicked);
		
		if (animate)
		{
			AnimateWidget.Color(m_wSpecializationImage, m_ColorInactive, m_fAnimationRate);
			AnimateWidget.Color(m_wSpecializationText, m_ColorInactive, m_fAnimationRate);
		}
		else
		{
			AnimateWidget.StopAnimation(m_wSpecializationImage, WidgetAnimationColor);
			m_wSpecializationImage.SetColor(m_ColorInactive);
			
			AnimateWidget.StopAnimation(m_wSpecializationText, WidgetAnimationColor);
			m_wSpecializationImage.SetColor(m_ColorInactive);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		if (!m_wSpecializationText)
			return;
		
		if (m_sSpecializationText == text)
			return;
		
		m_sSpecializationText = text;
		m_wSpecializationText.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sSpecializationText;
	}
	
	//------------------------------------------------------------------------------------------------
	TextWidget GetTextWidget()
	{
		return m_wSpecializationText;
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetImageWidget()
	{
		return m_wSpecializationImage;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetButtonId(int n)
	{
		m_ibuttonId = n;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetButtonId()
	{
		return m_ibuttonId;
	}
};