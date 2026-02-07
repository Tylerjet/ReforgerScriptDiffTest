//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ConsumableType
{
	[Attribute( defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "", enums: ParamEnumArray.FromEnum( EConsumableType ) )]
	protected EConsumableType 	m_eConsumableType;	
	
	[Attribute( defvalue: "0", uiwidget: UIWidgets.EditBox, desc: "" )]
	protected float 			m_fValue;
	
	EConsumableType 			GetType() { return m_eConsumableType; }
	float 						GetValue() { return m_fValue; }
};
