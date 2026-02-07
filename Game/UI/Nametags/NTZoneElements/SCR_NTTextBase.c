//------------------------------------------------------------------------------------------------
//! Base nametag element for text
[BaseContainerProps()]
class SCR_NTTextBase : SCR_NTElementBase
{			
	[Attribute("14", UIWidgets.CheckBox, "Scaled text size at the beginning of the zone \n pixels")]
	protected int m_fTextSizeMax;
	
	[Attribute("14", UIWidgets.CheckBox, "Scaled text size at the end of the zone \n pixels")]
	protected int m_fTextSizeMin;
	
	[Attribute("{E2CBA6F76AAA42AF}UI/Fonts/Roboto/Roboto_Regular.fnt", UIWidgets.ResourceNamePicker, desc: "Font type", params: "fnt")]
	protected ResourceName m_FontResource;
		
	//------------------------------------------------------------------------------------------------
	//!  Get text from this element
	//! \param data is nametag struct
	//! \return current text
	protected string GetText(SCR_NameTagData data)
	{				
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set text for this element
	//! \param data is nametag struct
	protected void SetText(SCR_NameTagData data, string text, int index)
	{
		TextWidget.Cast( data.m_aNametagElements[index] ).SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ScaleElement(SCR_NameTagData data, int index)
	{
		Widget widget = data.m_aNametagElements[index];
		if (!widget)
			return;
		
		//At least one zone always need to be defined
		SCR_NameTagZone zone = SCR_NameTagDisplay.GetNametagZones().Get(data.m_iZoneID);
		if (!zone)
			return;
		
		float diff = m_fTextSizeMax - m_fTextSizeMin;	
		
		// avoid 0 as a starting point
		int zoneStart = zone.GetZoneStart();
		if ( zoneStart < 1 )
			zoneStart = 1;
	
		float lerp = Math.InverseLerp(zoneStart, zone.m_iZoneEnd, data.m_fDistance);
		diff = diff * lerp;
		
		float x = FrameSlot.GetSizeX(widget);
		float y = FrameSlot.GetSizeY(widget);
		float ratio = x / y;
		
		float scaledY = m_fTextSizeMax - diff;
		
		FrameSlot.SetSizeY(widget, scaledY);
		FrameSlot.SetSizeX(widget, scaledY * ratio);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
		
		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		tWidget.SetFont(m_FontResource);
		
		if (!m_bScaleElement)
			tWidget.SetExactFontSize(m_fTextSizeMax);
		
		tWidget.SetColor(stateConf.m_vColor);
		tWidget.SetShadow( stateConf.m_fShadowSize, stateConf.m_vShadowColor.PackToInt(), stateConf.m_fShadowOpacity, 0, 0);
		
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions		
	}
};

//------------------------------------------------------------------------------------------------
//! Nametag element for name text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class SCR_NTName : SCR_NTTextBase
{		
	//------------------------------------------------------------------------------------------------
 	override string GetText(SCR_NameTagData data)
	{	
		return data.GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);
		
		if (data.m_Flags & ENameTagFlags.NAME_UPDATE)
		{
			string name = GetText(data);
			
			if (name == string.Empty)
				SetText(data, "GETNAME_ERROR", index);
			else
			{
				SetText(data, name, index);
				data.m_Flags &= ~ENameTagFlags.NAME_UPDATE;
			}
		}
	}
};

//------------------------------------------------------------------------------------------------
//! Nametag element for distance debug text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class SCR_NTTextDebug : SCR_NTTextBase
{	
	//------------------------------------------------------------------------------------------------
 	override string GetText(SCR_NameTagData data)
	{
		return Math.Round(Math.Sqrt(data.m_fDistance)).ToString();
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{		
		super.UpdateElement(data, index);
		
		SetText(data, GetText(data), index);	
	}
};