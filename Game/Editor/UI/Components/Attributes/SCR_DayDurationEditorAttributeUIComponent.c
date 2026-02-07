//
/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_DayDurationEditorAttributeUIComponent: SCR_SliderEditorAttributeUIComponent
{
	[Attribute()]
	protected LocalizedString m_sDaydurationTooltip;
	
	override string GetSliderValueText(float value)
	{
		//Calculate how many real life seconds 1 in game hour is
		float realLifeSecondsFloat = Math.Round(3600 / value);
		
		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds((int)realLifeSecondsFloat, days, hours, minutes, seconds);
		
		OverrideDescription(true, m_sDaydurationTooltip, hours.ToString(), minutes.ToString(), seconds.ToString());
		
		return super.GetSliderValueText(value);
	}
};