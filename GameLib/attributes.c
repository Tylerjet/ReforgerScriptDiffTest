class CinematicTrackAttribute
{
	string m_Name; //<! Custom track name if you don't want to use class name
	string m_Description; //<! Description of usage
	
	void CinematicTrackAttribute(string name = "", string description = "")
	{
		m_Name = name;
		m_Description = description;
	}
}

class CinematicEventAttribute
{
	string m_Name; //<! Custom name if you don't want to use name of the method
	
	void CinematicEventAttribute(string name = "")
	{
		m_Name = name;
	}
}
