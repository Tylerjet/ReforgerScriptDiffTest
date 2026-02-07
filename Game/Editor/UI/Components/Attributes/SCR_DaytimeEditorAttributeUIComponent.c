/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_DaytimeEditorAttributeUIComponent: SCR_SliderEditorAttributeUIComponent
{
	override string GetSliderValueText(float value)
	{
		return SCR_Global.GetTimeFormattingHideSeconds(value, ETimeFormatParam.DAYS);
	}
};