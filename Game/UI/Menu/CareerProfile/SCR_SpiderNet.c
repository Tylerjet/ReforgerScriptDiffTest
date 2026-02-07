//------------------------------------------------------------------------------------------------
class SCR_SpiderNet: ScriptedWidgetComponent
{
	protected CanvasWidget m_wCanvasWidget;
	
	protected vector m_vCenter;
	protected float m_fRadius;
	protected float m_fLegendOffsetAsRadius;
	
	protected ref array<ref CanvasWidgetCommand> m_aCanvasCommands;
	protected ref array<float> m_aVertices;
	protected ref array<float> m_aLegendPositions;
	protected ref array<float> m_aLegendAlignment;
	protected ImageWidget m_wbackgroundImage;	
	
	[Attribute(defvalue: "0.761 0.386 0.08 0.059", desc: "Outline Graph Color")]
	protected ref Color m_iPolygonColor;
	
	[Attribute(defvalue: "0.761 0.386 0.08 1", desc: "Outline Graph Color")]
	protected ref Color m_iOutlineColor;
	
	[Attribute("0.03", "auto", "Value to display when progress is 0")]
	protected float m_fMinRepresentativeValue;
	
	[Attribute(params: "Legend layout")]
	protected ResourceName m_LegendLayout;
	
	protected ref array<ref FrameWidget> m_aLegends;
	protected int m_iSelectedLegend;
	
	protected Widget m_wParent, m_wSpiderNetFrame;
	protected WorkspaceWidget m_workspace;
	
	protected SCR_CareerProfileOverviewUI m_CareerProfileHandler = null;
	
	protected int m_iNumberOfPoints;
	protected ref array<float> m_aSpPoints;
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		if (!m_wParent)
			m_wParent = w;
		
		m_wCanvasWidget = CanvasWidget.Cast(w.FindAnyWidget("Canvas"));
		if (!m_wCanvasWidget)
			return;
		
		m_wbackgroundImage = ImageWidget.Cast(w.FindAnyWidget("Background"));
		if (!m_wbackgroundImage)
			return;
		
		m_fLegendOffsetAsRadius = 0.25;
		
		m_aSpPoints = {};
	}
	
	void RegisterCareerProfileHandler(SCR_CareerProfileOverviewUI instance)
	{
		m_CareerProfileHandler = instance;
	}
	
	void SetSpPoints(array<int> specializations)
	{
		m_aSpPoints.Clear();
		
		for (int i = 0, count = specializations.Count(); i < count; i++)
			m_aSpPoints.Insert(specializations[i] / 1000000);
	}
	
	//------------------------------------------------------------------------------------------------
	void DrawSpiderNet()
	{
		if (m_aSpPoints.IsEmpty())
			return;
		
		float x, y;
		m_wbackgroundImage.GetScreenSize(x, y);
		if (x <= 0 && y <=  0) //If the layout and widgets have not been initialized yet, do nothing
		{
			GetGame().GetCallqueue().CallLater(DrawSpiderNet,10);
			return;
		}
		
		m_workspace = GetGame().GetWorkspace();
		
		m_fRadius = Math.Sqrt((x/4) * (x/4) + (y/4) * (y/4));
		m_vCenter = {};
		m_vCenter[0] = x/2; //center: x
		m_vCenter[1] = y/2; //center: y
		
		CalculateVertices(m_aSpPoints);
		DrawVertices();
		DrawLegends();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! To navigate a CanvasWidget we do minus operation to move upwards on the vertical axis
	//! Because the CanvasWidget has the 0,0 coordinate at the top-left corner of the widget
	//! 
	//! That's why to calculate position we do x=center+h*cos(angle), y=center-h*sin(angle). Because the sin is inverted.
	//! So we need to invert the sum operation when working with sin if we want to invert the vertical direction
	//! and move upwards by decreasing on the y
	protected void CalculateVertices(array<float> points)
	{
		m_aVertices = {};
		m_aLegendPositions = {};
		m_aLegendAlignment = {};
		m_iNumberOfPoints = points.Count();
		
		float angle, length, lengthToLegend;
		
		for (int i=0; i<m_iNumberOfPoints;i++)
		{
			//Distribute the 2*PI angles equidistantly for all specializations
			angle = ( (2 * Math.PI) / m_iNumberOfPoints) *i;
			
			//m_iMinRepresentativeValue of offset so 0 doesn't look like nothing at all
			if (points[i] < m_fMinRepresentativeValue)
				points[i] = m_fMinRepresentativeValue;
			
			//Length of the line = % of points on this Specialization.
			length = m_fRadius * points[i];
			
			//Distance to Legend from Center
			lengthToLegend = m_fRadius + m_fRadius * m_fLegendOffsetAsRadius;
			
			//Vertices so we know where to draw.
			m_aVertices.Insert(m_vCenter[0] + length * Math.Cos(angle)); //x
			m_aVertices.Insert(m_vCenter[1] - length * Math.Sin(angle)); //y
			
			//Legend position so we know where to put the legend
			m_aLegendPositions.Insert(m_vCenter[0] + lengthToLegend * Math.Cos(angle)); //x
			m_aLegendPositions.Insert(m_vCenter[1] - lengthToLegend * Math.Sin(angle)); //y
			
			//Legend text alignment so we can make it look aligned with the vertice
			//We need to translate the [-1, 1] range of the cos into a [1, 0] range of text alignment. 0.5-x/2 does the job
			//We need to translate the [-1, 1] range of the sin into a [0, 1] range of text alignment. 0.5 + x/2 does the job
			m_aLegendAlignment.Insert(0.5 - (Math.Cos(angle) / 2)); //x
			m_aLegendAlignment.Insert(0.5 + (Math.Sin(angle) / 2)); //y
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create inner polygon
	protected void DrawVertices()
	{
		m_aCanvasCommands = {};
		
		m_aCanvasCommands.Insert(InnerArea());
		m_aCanvasCommands.Insert(OuterArea());
		
		m_wCanvasWidget.SetDrawCommands(m_aCanvasCommands);
	}
	
	//------------------------------------------------------------------------------------------------
	protected CanvasWidgetCommand InnerArea()
	{
		PolygonDrawCommand polygon = new PolygonDrawCommand();
		
		polygon.m_iColor = m_iPolygonColor.PackToInt();
		polygon.m_Vertices = m_aVertices;
		
		return polygon;
	}
	
	//------------------------------------------------------------------------------------------------
	protected CanvasWidgetCommand OuterArea()
	{
		LineDrawCommand surface = new LineDrawCommand();
		
		surface.m_Vertices = m_aVertices;
		surface.m_fWidth = 2;
		surface.m_fOutlineWidth = 5;
		surface.m_iOutlineColor = m_iOutlineColor.PackToInt();
		surface.m_bShouldEnclose = true;
		
		return surface;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DrawLegends()
	{
		if (!m_wSpiderNetFrame)
			m_wSpiderNetFrame = m_wParent.FindAnyWidget("SpiderNetFrame");
		
		if (!m_wSpiderNetFrame)
			return;
		
		m_aLegends = {};
		
		string name;
		
		for (int i = 0; i < m_iNumberOfPoints; i++)
		{
			switch (i)
			{
				case 0: name = "1"; break;
				case 1: name = "2"; break;
				case 2: name = "3"; break;
				default: name = "Undefined ID"; break;
			}
			
			CreateLegend(m_aLegendPositions[i * 2], m_aLegendPositions[i * 2 + 1], m_aLegendAlignment[i * 2], m_aLegendAlignment[i * 2 + 1], name, i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateLegend(float positionX, float positionY, float horizontalAlignment, float verticalAlignment, string title, int id)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		
		FrameWidget legendWidget = FrameWidget.Cast(workspace.CreateWidgets(m_LegendLayout, m_wSpiderNetFrame));
		if (!legendWidget)
			return;
		
		ButtonWidget buttonWidget = ButtonWidget.Cast(legendWidget.FindAnyWidget("LegendButton"));
		if (!buttonWidget)
			return;
		
		SCR_ButtonLegendComponent handler = SCR_ButtonLegendComponent.Cast(buttonWidget.FindHandler(SCR_ButtonLegendComponent));
		if (!handler)
			return;
		
		handler.GetOnClicked().Insert(OnButtonClick);
		handler.SetButtonId(id);
		
		TextWidget wText = handler.GetTextWidget();
		ImageWidget wImage = handler.GetImageWidget();
		
		handler.SetText(title);
		
		positionX = m_workspace.DPIUnscale(positionX);
		positionY = m_workspace.DPIUnscale(positionY);
		
		FrameSlot.SetPos(legendWidget, positionX, positionY);
		FrameSlot.SetAlignment(legendWidget, horizontalAlignment, verticalAlignment);
		
		if (horizontalAlignment < 0.6)
		{
			//Image left, Text right
			AlignableSlot.SetHorizontalAlign(wImage, LayoutHorizontalAlign.Left);
			wText.SetFlags(WidgetFlags.RALIGN);
		}
		else
		{
			//Image right, Text left which's set as default on editor rn
			AlignableSlot.SetHorizontalAlign(wImage, LayoutHorizontalAlign.Right);
		}
		
		
		//Default legend
		m_iSelectedLegend = 0;
		if (m_aLegends.IsEmpty())
			handler.ActivateLegend();
		
		m_aLegends.Insert(legendWidget);
	}
	
	//When a legend is selected, deselect other legends
	//------------------------------------------------------------------------------------------------
	void OnButtonClick(SCR_ButtonLegendComponent buttonHandler)
	{	
		if (!buttonHandler)
			return;
		
		int newSelection = buttonHandler.GetButtonId();
		
		ButtonWidget buttonWidget;
		SCR_ButtonLegendComponent handler;
		
		for (int i = m_aLegends.Count() - 1; i >= 0; i--)
		{
			buttonWidget = ButtonWidget.Cast(m_aLegends.Get(i).FindAnyWidget("LegendButton"));
			handler = SCR_ButtonLegendComponent.Cast(buttonWidget.FindHandler(SCR_ButtonLegendComponent));
			
			if (!handler)
				continue;
			
			if (handler.GetButtonId() != newSelection)
				handler.DeactivateLegend();
			else
				handler.ActivateLegend();
		}
		
		m_iSelectedLegend = newSelection;
		
		if (m_CareerProfileHandler)
			m_CareerProfileHandler.UpdateSpecializationStats(m_iSelectedLegend);
	}
};