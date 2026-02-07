//------------------------------------------------------------------------------------------------
//! Default ruleset
[BaseContainerProps()]
class SCR_NameTagRuleset : SCR_NameTagRulesetBase
{
	//------------------------------------------------------------------------------------------------
	override protected bool TestVisibility(SCR_NameTagData data, float timeSlice)
	{
		data.m_fTimeSliceCleanup = 0;	// dont cleanup using the distance check since group ruleset only handles small amount of tags
		return super.TestVisibility(data, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool TestVisibilityFiltered(SCR_NameTagData data, float timeSlice)
	{
		// obstructed, this is checked here in order for the LOS checks to have run for the smaller subset of tag data 
		if (data.m_Flags & ENameTagFlags.FADE_TIMER)
			data.m_fTimeSliceFade += timeSlice;
		
		if (data.m_Flags & ENameTagFlags.OBSTRUCTED)
			return false;
		
		// Reduce the angle required to show with distance -> the further is the entity, angle required to focus it gets smaller
		float distLerp = Math.InverseLerp(m_ZoneCfg.m_fFarthestZoneRangePow2, 0, data.m_fDistance);
		distLerp *= MAX_ANGLE/2;  // adjust for more standard ish FOV of 90 (45 radius) TODO: this should be taken from real FOV
		
		// out of visibility angle
		data.m_fAngleToScreenCenter = GetCameraToEntityAngle(data.m_vEntWorldPos, VERT_ANGLE_ADJUST);
		if (data.m_fAngleToScreenCenter < distLerp)
		{			
			// if within lerped angle and closer than the current focus priority 
			if ( data.m_fAngleToScreenCenter - NEAR_TAG_ANGLE < m_fFocusPrioAngle || m_fFocusPrioAngle == 0 )
			{
				// if angle difference is small && previous tag is closer, keep the (previous) closer target
				if ( m_ClosestAngleTag && (m_fFocusPrioAngle - data.m_fAngleToScreenCenter) < NEAR_TAG_ANGLE && m_ClosestAngleTag.m_fDistance < data.m_fDistance )
				{}
				else
				{
					m_ClosestAngleTag = data;
					m_fFocusPrioAngle = data.m_fAngleToScreenCenter;
				}
			}
						
			return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void DetermineVisibility(float timeSlice)
	{
		super.DetermineVisibility(timeSlice);

		int count = m_aCandidateTags.Count();
		for (int i = count - 1; i > -1; i--)
		{		
			SCR_NameTagData data = m_aCandidateTags.Get(i);

			UpdateVisibleTag(data, timeSlice);
		}
	}
};
