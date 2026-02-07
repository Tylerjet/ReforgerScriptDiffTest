//------------------------------------------------------------------------------------------------
class SCR_MultipleStatesIconsButtonComponent : SCR_ButtonComponent 
{
	
	[Attribute()]
	protected ResourceName m_wIconSetTexture;
	
	[Attribute()]
	ref array<string> m_aStateIcons;
	
	[Attribute()]
	protected int m_iStateSelected;
	
	protected ImageWidget m_wIcon;
	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("ContentImage"));

		Init();
	}
		
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		if (m_aStateIcons.Count()>0)
			ChangeState(m_iStateSelected);
			
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeState(int state, bool color = false)
	{
		if (state >= 0 && state < m_aStateIcons.Count())
		{
			m_iStateSelected = state;
			m_wIcon.LoadImageFromSet(0, m_wIconSetTexture, m_aStateIcons[state]);
			if (color)
				WidgetAnimator.PlayAnimation(m_wIcon, WidgetAnimationType.Color, COLOR_BACKGROUND_CLICKED, m_fAnimationRate);
			else
				WidgetAnimator.PlayAnimation(m_wIcon, WidgetAnimationType.Color, COLOR_CONTENT_DEFAULT, m_fAnimationRate);

		}
		
	}
			
	//------------------------------------------------------------------------------------------------
	int GetSelectedItem()
	{
		return m_iStateSelected;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		
		if (GetGame().GetWorkspace().GetFocusedWidget() != w)
		{
			WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, COLOR_BACKGROUND_HOVERED, m_fAnimationRate);	
		
		}
		
		return false;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{

		WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Color, COLOR_BACKGROUND_DEFAULT, m_fAnimationRate);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{	

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void VirtualClick(Widget w)
	{
		m_OnClicked.Invoke(w);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ColorizeWidgets(Color colorBackground, Color colorContent, float speed = -1)
	{
	}
	
		//------------------------------------------------------------------------------------------------
	void ShowButton()
	{
		WidgetAnimator.PlayAnimation(m_wRoot, WidgetAnimationType.Opacity, 1, WidgetAnimator.FADE_RATE_DEFAULT);
		WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Opacity, 1, m_fAnimationRate);
	}
	
	//------------------------------------------------------------------------------------------------
	void HideButton()
	{
		WidgetAnimator.PlayAnimation(m_wBackground, WidgetAnimationType.Opacity, 0, m_fAnimationRate);
		if (m_iStateSelected == 0)
			WidgetAnimator.PlayAnimation(m_wRoot, WidgetAnimationType.Opacity, 0, WidgetAnimator.FADE_RATE_DEFAULT);
	
	}
	
	
};