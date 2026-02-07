[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EPoleCableType, "m_eCableType", "%1 Group")]
class SCR_PoleCableSlotGroup
{
	[Attribute(defvalue: SCR_EPoleCableType.POWER_LV.ToString(), desc: "Accepted cable type e.g ultra/high/low voltage, phone, etc", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EPoleCableType))]
	SCR_EPoleCableType m_eCableType;

	[Attribute(desc: "Slots on which wires of said type(s) can connect")]
	ref array<ref SCR_PoleCableSlot> m_aSlots;

	[Attribute(defvalue: "0 0 0", desc: "If different from 0 0 0, cable connection will use this point to check group proximity instead of an average of slot positions", params: "inf inf purpose=coords space=entity")]
	vector m_vAnchorOverride; // allow for multiple anchors? (e.g front-back, left-right etc)
}
