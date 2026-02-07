//------------------------------------------------------------------------------------------------
class SCR_ButtonImageComponent : SCR_ButtonBaseComponent 
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Select edds texture or imageset", "edds imageset")]
	ResourceName m_sTexture;
	
	[Attribute()]
	string m_sImageName;
	
	ImageWidget m_wImage;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wImage = ImageWidget.Cast(w.FindAnyWidget("Image"));
		SetImage(m_sTexture, m_sImageName);
	}

	//------------------------------------------------------------------------------------------------
	ImageWidget GetImageWidget()
	{
		return m_wImage;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetImage(ResourceName texture)
	{
		if (!m_wImage)
			return;
		
		bool show = texture != string.Empty;
		m_wImage.SetVisible(show);
		if (show)
		{
			int x, y;
			m_wImage.LoadImageTexture(0, texture);
			m_wImage.GetImageSize(0, x, y);
			m_wImage.SetSize(x, y);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetImage(ResourceName imageSet, string imageName)
	{
		if (!m_wImage)
			return;
		
		bool show = imageSet != string.Empty;
		m_wImage.SetVisible(show);
		
		if (!show)
			return;
		
		if (imageSet.EndsWith("imageset"))
			m_wImage.LoadImageFromSet(0, imageSet, imageName);
		else
			m_wImage.LoadImageTexture(0, imageSet);

		// Resize
		int x, y;
		m_wImage.GetImageSize(0, x, y);
		m_wImage.SetSize(x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetImage(out string imageSet)
	{
		if (m_sImageName == string.Empty)
			return m_sTexture;
		imageSet = m_sTexture;
		return m_sImageName;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ButtonImageComponent GetButtonImage(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_ButtonImageComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_ButtonImageComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
};