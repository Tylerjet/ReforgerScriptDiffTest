class SCR_ActionUIInfo : UIInfo
{
	[Attribute(UIConstants.ICONS_IMAGE_SET, UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sInteractionIcon;
	
	[Attribute(desc: "If 'Icon' is an ImageSet, define the name of the wanted Icon here.")]
	protected string m_sIconName;
	
	//------------------------------------------------------------------------------------------------
	//! Set icon to given image widgets.
	//! \param[in] ImageWidget Target image widget
	//! \return True when the image was set
	bool SetIconTo(ImageWidget imageWidget)
	{
		if (!imageWidget)
			return false;
		
		if (!m_sInteractionIcon && !m_sIconName)
			return false;

		string ext;
		FilePath.StripExtension(m_sInteractionIcon, ext);
		if (ext == "imageset")
			imageWidget.LoadImageFromSet(0, m_sInteractionIcon, m_sIconName);
		else
			imageWidget.LoadImageTexture(0, m_sInteractionIcon);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the icon name if defined
	//! \return string IconName
	string GetIconName()
	{
		return m_sIconName;
	}
}