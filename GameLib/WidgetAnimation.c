enum WidgetAnimationType
{
	Opacity,
	Color,
	AlphaMask,
	LayoutFill,
	FrameSize,
	PaddingGrid,
	PaddingLayout,
	PaddingOverlay,
	PaddingButton,
	PaddingAlignable,
	Position,
	ImageRotation,
	ImageSaturation
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationBase
{
	Widget m_wWidget;
	WidgetAnimationType m_eAnimationType;
	
	protected bool m_bRepeat;
	protected float m_fSpeed;
	protected float m_fCurrentProgress = 0;
	//------------------------------------------------------------------------------------------------
	bool OnUpdate(float timeSlice)
	{
		m_fCurrentProgress += timeSlice * m_fSpeed;

		if (m_fCurrentProgress >= 1)
		{
			m_fCurrentProgress = 1;
			return true;
		}
		return false;
	}
	
	
	//------------------------------------------------------------------------------------------------
	bool Repeat()
	{
		m_fCurrentProgress = 0;
		return m_bRepeat;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationBase(Widget w, float speed)
	{
		m_wWidget = w;
		m_fSpeed = speed;
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationPadding : WidgetAnimationBase
{
	protected float m_fLeftTarget;
	protected float m_fTopTarget;
	protected float m_fRightTarget;
	protected float m_fBottomTarget;
	
	protected float m_fLeftDefault;
	protected float m_fTopDefault;
	protected float m_fRightDefault;
	protected float m_fBottomDefault;
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		if (!m_wWidget)
			return true;
		
		bool finished = super.OnUpdate(timeSlice);
		
		float left = Math.Lerp(m_fLeftDefault, m_fLeftTarget, m_fCurrentProgress);
		float top = Math.Lerp(m_fTopDefault, m_fTopTarget, m_fCurrentProgress);
		float right = Math.Lerp(m_fRightDefault, m_fRightTarget, m_fCurrentProgress);
		float bottom = Math.Lerp(m_fBottomDefault, m_fBottomTarget, m_fCurrentProgress);
		
		if (m_eAnimationType == WidgetAnimationType.PaddingGrid)
			GridSlot.SetPadding(m_wWidget, left, top, right, bottom);
		else if (m_eAnimationType == WidgetAnimationType.PaddingLayout)
			AlignableSlot.SetPadding(m_wWidget, left, top, right, bottom);
		else if (m_eAnimationType == WidgetAnimationType.PaddingOverlay)
			AlignableSlot.SetPadding(m_wWidget, left, top, right, bottom);
		else if (m_eAnimationType == WidgetAnimationType.PaddingButton)
			AlignableSlot.SetPadding(m_wWidget, left, top, right, bottom);
		else if (m_eAnimationType == WidgetAnimationType.PaddingAlignable)
			AlignableSlot.SetPadding(m_wWidget, left, top, right, bottom);
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationPadding(Widget w, float speed, WidgetAnimationType animationType, float left, float top, float right, float bottom, bool repeat = false)
	{
		m_eAnimationType = animationType;
		m_fLeftTarget = left;
		m_fTopTarget = top;
		m_fRightTarget = right;
		m_fBottomTarget = bottom;

		if (m_eAnimationType == WidgetAnimationType.PaddingGrid)
			GridSlot.GetPadding(w, m_fLeftDefault, m_fTopDefault, m_fRightDefault, m_fBottomDefault);
		else if (m_eAnimationType == WidgetAnimationType.PaddingLayout)
			AlignableSlot.GetPadding(w, m_fLeftDefault, m_fTopDefault, m_fRightDefault, m_fBottomDefault);
		else if (m_eAnimationType == WidgetAnimationType.PaddingOverlay)
			AlignableSlot.GetPadding(w, m_fLeftDefault, m_fTopDefault, m_fRightDefault, m_fBottomDefault);
		else if (m_eAnimationType == WidgetAnimationType.PaddingAlignable)
			AlignableSlot.GetPadding(w, m_fLeftDefault, m_fTopDefault, m_fRightDefault, m_fBottomDefault);
		
		m_bRepeat = repeat;
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationLayoutFill : WidgetAnimationBase
{
	float m_fValueDefault;
	float m_fValueTarget;
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		if (!m_wWidget)
			return true;
		
		bool finished = super.OnUpdate(timeSlice);
		float currentValue = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fCurrentProgress);
		LayoutSlot.SetFillWeight(m_wWidget, currentValue);
		
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationLayoutFill(Widget w, float speed, float targetValue, bool repeat = false)
	{
		m_eAnimationType = WidgetAnimationType.LayoutFill;
		m_fValueTarget = targetValue;
		m_fValueDefault = LayoutSlot.GetFillWeight(w);
		
		m_bRepeat = repeat;
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationOpacity : WidgetAnimationBase
{
	float m_fValueDefault;
	float m_fValueTarget;
	bool m_bChangeVisibileFlag;
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		if (!m_wWidget)
			return true;
		
		bool finished = super.OnUpdate(timeSlice);
		float currentValue = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fCurrentProgress);
		m_wWidget.SetOpacity(currentValue);
		
		if (finished && m_bChangeVisibileFlag)
			m_wWidget.SetVisible(currentValue != 0);
		
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationOpacity(Widget w, float speed, float targetValue, bool repeat = false, bool setVisibility = false)
	{
		m_eAnimationType = WidgetAnimationType.Opacity;
		m_fValueTarget = targetValue;
		m_fValueDefault = w.GetOpacity();
		
		m_bChangeVisibileFlag = setVisibility;
		m_bRepeat = repeat;
		
		if (setVisibility)
			w.SetVisible(true);
	}
};


//------------------------------------------------------------------------------------------------
class WidgetAnimationAlphaMask : WidgetAnimationBase
{
	float m_fValueDefault;
	float m_fValueTarget;
	float m_fValueCurrent;
	ImageWidget m_wImage;
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		if (!m_wImage)
			return true;
		
		bool finished = super.OnUpdate(timeSlice);
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fCurrentProgress);
		m_wImage.SetMaskProgress(m_fValueCurrent);
		
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationAlphaMask(Widget w, float speed, float targetValue, bool repeat = false)
	{
		m_wImage = ImageWidget.Cast(w);
		if (!m_wImage)
			return;
		
		m_eAnimationType = WidgetAnimationType.AlphaMask;
		m_fValueTarget = targetValue;
		m_fValueDefault = m_wImage.GetMaskProgress();
		
		m_bRepeat = repeat;
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationImageRotation : WidgetAnimationBase
{
	float m_fValueDefault;
	float m_fValueTarget;
	float m_fValueCurrent;
	ImageWidget m_wImage;
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		if (!m_wImage)
			return true;
		
		bool finished = super.OnUpdate(timeSlice);
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fCurrentProgress);
		m_wImage.SetRotation(m_fValueCurrent);
		
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationImageRotation(Widget w, float speed, float targetValue, bool repeat = false)
	{
		m_wImage = ImageWidget.Cast(w);
		if (!m_wImage)
			return;
		
		m_eAnimationType = WidgetAnimationType.ImageRotation;
		m_fValueTarget = targetValue;
		m_fValueDefault = m_wImage.GetRotation();
		
		m_bRepeat = repeat;
	}
};


//------------------------------------------------------------------------------------------------
class WidgetAnimationColor : WidgetAnimationBase
{
	ref Color m_ColorDefault;
	ref Color m_ColorTarget;
	ref Color m_ColorCurrent;
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		if (!m_wWidget)
			return true;
		
		bool finished = super.OnUpdate(timeSlice);
		m_ColorCurrent = m_ColorDefault.LerpNew(m_ColorTarget, m_fCurrentProgress);
		m_wWidget.SetColor(m_ColorCurrent);
		
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationColor(Widget w, float speed, Color targetColor, bool repeat = false)
	{
		m_eAnimationType = WidgetAnimationType.Color;
		m_ColorTarget = targetColor;
		m_ColorDefault = w.GetColor();
		
		m_bRepeat = repeat;
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationFrameSize : WidgetAnimationBase
{
	float m_fWidthDefault;
	float m_fHeightDefault;
	float m_fWidthTarget;
	float m_fHeightTarget;
	float m_fWidthCurrent;
	float m_fHeightCurrent;
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		if (!m_wWidget)
			return true;
		
		bool finished = super.OnUpdate(timeSlice);
		
		m_fWidthCurrent = Math.Lerp(m_fWidthDefault, m_fWidthTarget, m_fCurrentProgress);
		m_fHeightCurrent = Math.Lerp(m_fHeightDefault, m_fHeightTarget, m_fCurrentProgress);
		FrameSlot.SetSize(m_wWidget, m_fWidthCurrent, m_fHeightCurrent);
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationFrameSize(Widget w, float speed, float width, float height, bool repeat = false)
	{
		m_eAnimationType = WidgetAnimationType.FrameSize;
		m_fWidthTarget = width;
		m_fHeightTarget = height;
		
		m_fWidthDefault = FrameSlot.GetSizeX(w);
		m_fHeightDefault = FrameSlot.GetSizeY(w);
		
		m_bRepeat = repeat;
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationPosition : WidgetAnimationBase
{
	float m_fXDefault;
	float m_fYDefault;
	float m_fXTarget;
	float m_fYTarget;
	float m_fXCurrent;
	float m_fYCurrent;
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		if (!m_wWidget)
			return true;
		
		bool finished = super.OnUpdate(timeSlice);
		m_fXCurrent = Math.Lerp(m_fXDefault, m_fXTarget, m_fCurrentProgress);
		m_fYCurrent = Math.Lerp(m_fYDefault, m_fYTarget, m_fCurrentProgress);
		FrameSlot.SetPos(m_wWidget, m_fXCurrent, m_fYCurrent);
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationPosition(Widget w, float speed, float x, float y, bool repeat = false)
	{
		m_eAnimationType = WidgetAnimationType.Position;
		m_fXTarget = x;
		m_fYTarget = y;
		
		m_fXDefault = FrameSlot.GetPosX(w);
		m_fYDefault = FrameSlot.GetPosY(w);
		
		m_bRepeat = repeat;
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationImageSaturation : WidgetAnimationBase
{
	float m_fValueDefault;
	float m_fValueTarget;
	float m_fValueCurrent;
	ImageWidget m_wImage;
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		if (!m_wImage)
			return true;
		
		bool finished = super.OnUpdate(timeSlice);
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fCurrentProgress);
		m_wImage.SetSaturation(m_fValueCurrent);
		return finished;
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationImageSaturation(Widget w, float speed, float targetValue, bool repeat = false)
	{
		m_wImage = ImageWidget.Cast(w);
		if (!m_wImage)
			return;
		
		m_eAnimationType = WidgetAnimationType.ImageSaturation;
		m_fValueTarget = targetValue;
		m_fValueDefault = m_wImage.GetSaturation();
		
		m_bRepeat = repeat;
	}
};