class SimplePreload
{
	static const int PRELOAD_FRAME_TIME_MS = 100;
	static const int MIN_DURATION_MS = 1000;
	ref LoadingAnim m_LoadingAnim;
	vector m_Position;
	float m_fDuration;
	int m_iMaxDuration;
	int m_iRemainsTotal = -1;
	
	void SimplePreload() 
	{
		Print("SimplePreload: preload started.", LogLevel.VERBOSE);
	}
	
	void ~SimplePreload() 
	{
		m_LoadingAnim.Hide();
		Print("SimplePreload: preload finished (took: " + Math.Round(m_fDuration) + "s).", LogLevel.VERBOSE);
	}
	
	bool Update(float timeSlice)
	{
		int remains = g_Game.WaitPreload(PRELOAD_FRAME_TIME_MS);
		m_fDuration += timeSlice;
		
		if ((remains == 0 || m_fDuration >= (m_iMaxDuration * 0.001)) && (m_fDuration >= (MIN_DURATION_MS * 0.001)))
		{
			return true;
		}
		
		m_LoadingAnim.Update(timeSlice, Math.Min(remains / m_iRemainsTotal, 1.0) , Math.Min(m_fDuration / MIN_DURATION_MS * 0.001, 1.0));
		return false;
	}
	
	static SimplePreload Preload(vector position, float radius = 500, int max_duration_ms = 20000)
	{	
		if (g_Game.BeginPreload(g_Game.GetWorld(), position, radius))
		{
			int remains = g_Game.WaitPreload(PRELOAD_FRAME_TIME_MS);
			Print("SimplePreload: prelading " + remains + " resources.", LogLevel.VERBOSE);
			if (remains > 100)
			{
				// there is a lot of resoruces to be loaded, show loading anim
				SimplePreload preload = new SimplePreload();
				preload.m_LoadingAnim = g_Game.CreateLoadingAnim(g_Game.GetWorkspace());
				preload.m_LoadingAnim.Show();
				preload.m_iMaxDuration = max_duration_ms;
				preload.m_Position = position;
				preload.m_iRemainsTotal = remains;
				return preload;
			}
			else
			{
				// if there is not a lot of them, do it in blocikng wait without loading anim
				while (remains > 0)
				{
					remains = g_Game.WaitPreload(PRELOAD_FRAME_TIME_MS);
				}
				Print("SimplePreload: preload finished.", LogLevel.VERBOSE);
			}
		}
		
		return null;
	}
};