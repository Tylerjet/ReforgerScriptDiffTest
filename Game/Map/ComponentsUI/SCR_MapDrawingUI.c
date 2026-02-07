//------------------------------------------------------------------------------------------------
//! Drawn line width
enum eMapDrawLineWidth
{
	THIN = 4,
	MEDIUM = 8,
	THICK = 12
};

//------------------------------------------------------------------------------------------------
//! Drawn line colors
class SCR_MapDrawingUILineColor
{
	const ref Color WHITE = 	new Color(1.0, 1.0, 1.0, 1.0);
	const ref Color BLACK = 	new Color(0.0, 0.0, 0.0, 1.0);
	const ref Color RED = 		new Color(1.0, 0.0, 0.0, 1.0);
	const ref Color GREEN = 	new Color(0.0, 1.0, 0.0, 1.0);
	const ref Color BLUE = 		new Color(0.0, 0.0, 1.0, 1.0);
};

//------------------------------------------------------------------------------------------------
//! Map drawing UI configuration
class SCR_MapDrawingUI: SCR_MapUIBaseComponent
{	
	protected bool m_bVisibility;
	protected Widget m_Palette;
	
	protected static int m_fLineWidth = eMapDrawLineWidth.THIN;
	protected static ref Color m_LineColor = SCR_MapDrawingUILineColor.BLACK;
	
	//------------------------------------------------------------------------------------------------
	//! Get line width
	static float GetLineWidth() { return m_fLineWidth; }
	//! Get line color
	static Color GetLineColor() { return m_LineColor; }
	
	//! Line selection callbacks
	protected void SetLineWidthThin() { m_fLineWidth = eMapDrawLineWidth.THIN; }
	protected void SetLineWidthMedium() { m_fLineWidth = eMapDrawLineWidth.MEDIUM; }
	protected void SetLineWidthThick() { m_fLineWidth = eMapDrawLineWidth.THICK; }
	
	//! Color selection callbacks
	protected void SetLineColorWhite() { m_LineColor = SCR_MapDrawingUILineColor.WHITE; }
	protected void SetLineColorBlack()  { m_LineColor = SCR_MapDrawingUILineColor.BLACK; }
	protected void SetLineColorRed() { m_LineColor = SCR_MapDrawingUILineColor.RED; }
	protected void SetLineColorGreen() { m_LineColor = SCR_MapDrawingUILineColor.GREEN; }
	protected void SetLineColorBlue() { m_LineColor = SCR_MapDrawingUILineColor.BLUE; }
		
	//------------------------------------------------------------------------------------------------
	//! Toggle pallete UI
	void ToggleVisibility()
	{
		if (m_bVisibility)
			ClosePalette();
		else 
			OpenPalette();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Open pallete UI
	void OpenPalette()
	{
		m_bVisibility = true;
		m_Palette.SetVisible(m_bVisibility);				
	}
	
	//------------------------------------------------------------------------------------------------
	//! Close pallete UI
	void ClosePalette()
	{
		m_bVisibility = false;
		m_Palette.SetVisible(m_bVisibility);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Context callback	
	protected void DrawModeReset()
	{}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		m_Palette = m_RootWidget.FindWidget("DrawingPalette");
		m_Palette.AddHandler(this);
		
		// Set callbacks
		AddOrFindClickEvent("Thin", m_Palette).AddCallback(SetLineWidthThin);
		AddOrFindClickEvent("Medium", m_Palette).AddCallback(SetLineWidthMedium);
		AddOrFindClickEvent("Thick", m_Palette).AddCallback(SetLineWidthThick);
		AddOrFindClickEvent("ResetDrawing", m_Palette).AddCallback(DrawModeReset);
		
		AddOrFindClickEvent("White", m_Palette).AddCallback(SetLineColorWhite);
		AddOrFindClickEvent("Black", m_Palette).AddCallback(SetLineColorBlack);
		AddOrFindClickEvent("Red", m_Palette).AddCallback(SetLineColorRed);
		AddOrFindClickEvent("Green", m_Palette).AddCallback(SetLineColorGreen);
		AddOrFindClickEvent("Blue", m_Palette).AddCallback(SetLineColorBlue);
		
		// Add ctx menu entry
		/*SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapContextualMenuUI));
		if (ctxMenu)
			ctxMenu.ContextRegisterDynamic("debug: Drawing palette", true).m_OnClick.Insert(ToggleVisibility);*/
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		if (m_Palette)
			m_Palette.RemoveHandler(this);
		
		super.OnMapClose(config);
	}
	
};