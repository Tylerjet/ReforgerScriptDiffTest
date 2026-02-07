[EntityEditorProps(category: "GameScripted/UI", description: "Helper entity handling processing widget animations. Do not insert into world.", color: "0 0 255 255")]
class WidgetAnimatorClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class WidgetAnimator : GenericEntity
{
	static const float FADE_RATE_SUPER_FAST = 20;
	static const float FADE_RATE_FAST = 10;
	static const float FADE_RATE_DEFAULT = 5; // Used for near instant actions
	static const float FADE_RATE_SLOW = 1; // Used for fading out elements that should be visible for some time
	static const float FADE_RATE_SUPER_SLOW = 0.2; // Very slow fade out
	
	ref array<ref WidgetAnimationBase> m_aAnimations = new ref array<ref WidgetAnimationBase>;
	private static WidgetAnimator s_Instance = null;

	//------------------------------------------------------------------------------------------------
	static sealed WidgetAnimator GetInstance() 
	{
		return s_Instance;
	}

	/*!
	Stop either all animations on the widget, or an animation of a specific type
	*/
	//------------------------------------------------------------------------------------------------
	static bool StopAnimation(Widget w, WidgetAnimationType animationType = -1)
	{
		bool success = false;
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (animator && w)
		{
			int count = animator.m_aAnimations.Count();
			for (int i = count - 1; i >= 0; i--)
			{
				WidgetAnimationBase current = animator.m_aAnimations.Get(i);
				int currentAnimationType = current.m_eAnimationType;
				if (current.m_wWidget == w)
				{
					if (animationType == -1)
					{
						animator.m_aAnimations.Remove(i);
						success = true;
					}
					else if (animationType == currentAnimationType)
					{
						animator.m_aAnimations.Remove(i);
						success = true;
					}
				}
			}
		}
		
		return success;
	}
	//------------------------------------------------------------------------------------------------
	/*!
	Stop animation
	\param w Target widget
	\param anim Animation class
	\return True if the animation was removed
	*/
	static bool StopAnimation(Widget w, WidgetAnimationBase anim)
	{
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (!animator)
			return false;
		
		int index = animator.m_aAnimations.Find(anim);
		if (index == -1)
			return false;
		
		animator.m_aAnimations.Remove(index);
		return true;
	}
		//------------------------------------------------------------------------------------------------
	/*!
	Stop all animation on given widget.
	\param w Target widget
	*/
	static void StopAllAnimations(Widget w)
	{
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (!animator)
			return;
		
		for (int i = animator.m_aAnimations.Count() - 1; i >= 0; i--)
		{
			if (animator.m_aAnimations[i].m_wWidget == w)
				animator.m_aAnimations.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsAnimating(Widget w)
	{
		if (!w)
			return false;
		
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (animator)
		{
			foreach (WidgetAnimationBase animation: animator.m_aAnimations)
			{
				if (animation.m_wWidget == w)
					return true;
			}
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Play animation
	\param w Target widget
	\param anim Animation class
	*/
	static void PlayAnimation(notnull WidgetAnimationBase anim)
	{
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (animator)
			animator.m_aAnimations.Insert(anim);
	}

	// Used for opacity and alpha mask animation
	//------------------------------------------------------------------------------------------------
	static void PlayAnimation(Widget w, WidgetAnimationType animationType, float targetValue, float speed, bool repeat = false, bool changeVisibilityFlag = false)
	{
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (!animator || !w || speed <= 0)
			return;
		
		StopAnimation(w, animationType);
		WidgetAnimationBase anim; 
		
		if (animationType == WidgetAnimationType.Opacity && targetValue != w.GetOpacity())
		{
			anim = new WidgetAnimationOpacity(w, speed, targetValue, repeat, changeVisibilityFlag);
		}
		else if (animationType == WidgetAnimationType.AlphaMask)
		{
			ImageWidget image = ImageWidget.Cast(w);
			if (image && image.GetMaskProgress() != targetValue)
				anim = new WidgetAnimationAlphaMask(image, speed, targetValue, repeat);
		}
		else if (animationType == WidgetAnimationType.LayoutFill && LayoutSlot.GetFillWeight(w) != targetValue)
		{
			anim = new WidgetAnimationLayoutFill(w, speed, targetValue, repeat);
		}
		else if (animationType == WidgetAnimationType.ImageRotation)
		{
			anim = new WidgetAnimationImageRotation(w, speed, targetValue, repeat);
		}
		else if (animationType == WidgetAnimationType.ImageSaturation)
		{
			anim = new WidgetAnimationImageSaturation(w, speed, targetValue, repeat);
		}
		
		if (anim)
			animator.m_aAnimations.Insert(anim);
	}
	
	// Used for color animation
	//------------------------------------------------------------------------------------------------
	static void PlayAnimation(Widget w, WidgetAnimationType animationType, Color targetColor, float speed, bool repeat = false)
	{
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (!animator || !w || speed <= 0)
			return;
		
		if (w.GetColor() == targetColor)
			return;
		
		StopAnimation(w, animationType);
		ref WidgetAnimationColor anim = new WidgetAnimationColor(w, speed, targetColor, repeat);
		animator.m_aAnimations.Insert(anim);
	}

	// TEMP disabled
	/*
	//------------------------------------------------------------------------------------------------
	static void PlayAnimationSequence(Widget w, WidgetAnimationType animationType, WidgetAnimation animation)
	{
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (!animator)
			return;
		
		StopAnimation(w, animationType);
		ref WidgetAnimationInstance anim = new WidgetAnimationInstance(w, animationType, 1, 1, null, animation);
		animator.m_aAnimations.Insert(anim);
	}
	*/

	//------------------------------------------------------------------------------------------------
	static void PlayAnimation(Widget w, WidgetAnimationType animationType, float speed, float paddingLeft, float paddingTop, float paddingRight, float paddingBottom, bool repeat = false)
	{
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (!animator || !w || speed <= 0)
			return;
		
		StopAnimation(w, animationType);
		ref WidgetAnimationPadding anim = new WidgetAnimationPadding(w, speed, animationType, paddingLeft, paddingTop, paddingRight, paddingBottom, repeat);
		animator.m_aAnimations.Insert(anim);
	}

	// Used for frame size animation
	//------------------------------------------------------------------------------------------------
	static void PlayAnimation(Widget w, WidgetAnimationType animationType, float speed, float x, float y, bool repeat = false)
	{
		WidgetAnimator animator = WidgetAnimator.GetInstance();
		if (!animator || !w || speed <= 0)
			return;
		
		StopAnimation(w, animationType);
		ref WidgetAnimationBase anim; 

		if (animationType == WidgetAnimationType.FrameSize && (x != FrameSlot.GetSizeX(w) || y != FrameSlot.GetSizeY(w)))
			anim = new WidgetAnimationFrameSize(w, speed, x, y, repeat);
		else if (animationType == WidgetAnimationType.Position && (x != FrameSlot.GetPosX(w) || y != FrameSlot.GetPosY(w)))
			anim = new WidgetAnimationPosition(w, speed, x, y, repeat);

		if (anim)
			animator.m_aAnimations.Insert(anim);
	}
	
	//------------------------------------------------------------------------------------------------
	override private void EOnFrame(IEntity owner, float timeSlice)
	{
		// Tick all animations
		int count = m_aAnimations.Count();
		for (int i = count - 1; i >=0; i--)
		{
			bool finished = m_aAnimations[i].OnUpdate(timeSlice);
			if (finished)
			{
				if (!m_aAnimations[i].Repeat())
				{
					m_aAnimations.Remove(i);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void WidgetAnimator(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~WidgetAnimator()
	{
		if (s_Instance == this)
			s_Instance = null;

		m_aAnimations.Clear();
		m_aAnimations = null;
	}
};

