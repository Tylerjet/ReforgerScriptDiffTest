/*
Helper system to call widget animations updates
*/

class AnimateWidgetSystem : GameSystem
{
	protected ref AnimateWidget m_Animator;

	//------------------------------------------------------------------------------------------------
	override void OnInit()
	{
		super.OnInit();
		
		m_Animator = AnimateWidget.GetInstance();
		if (!m_Animator)
			m_Animator = new AnimateWidget();

		m_Animator.m_OnAnimatingStarted.Insert(OnAnimatingStarted);
		m_Animator.m_OnAnimatingCompleted.Insert(OnAnimatingCompleted);
		
		Enable(m_Animator.IsActive())
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);
		
		m_Animator.UpdateAnimations(GetWorld().GetTimeSlice());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ShouldBeEnabledInEditMode()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAnimatingCompleted()
	{
		Enable(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnimatingStarted()
	{
		Enable(true);
	}
}