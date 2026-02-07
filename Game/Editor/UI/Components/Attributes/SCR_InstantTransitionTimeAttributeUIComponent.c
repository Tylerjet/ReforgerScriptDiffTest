class SCR_InstantTransitionTimeAttributeUIComponent: SCR_SliderEditorAttributeUIComponent
{
	override string GetSliderValueText(float value)
	{
		return SCR_Global.GetTimeFormatting(value, ETimeFormatParam.DAYS);
	}
};