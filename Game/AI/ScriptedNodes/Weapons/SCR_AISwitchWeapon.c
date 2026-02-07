// Script File
class SCR_AISwitchWeapon: AITaskScripted
{
	static const string PORT_WEAPON_TYPE = "WeaponType";
	
	[Attribute("1", UIWidgets.EditBox, "Wanted weapon type", "", ParamEnumArray.FromEnum(EWeaponType) )]
	EWeaponType m_WeaponType;

	private GenericEntity m_Controlled = null;
	private BaseWeaponManagerComponent m_WpnManager = null;
	private SCR_CharacterControllerComponent m_Controller = null;
	private ref array<WeaponSlotComponent> m_Weapons;
	
	override void OnInit(AIAgent owner)
	{			
		m_Weapons = new array<WeaponSlotComponent>;
		m_Controlled = null;
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		int weaponType;
			
		if (!GetVariableIn(PORT_WEAPON_TYPE,weaponType))
			weaponType = m_WeaponType;
				
		IEntity contr = owner.GetControlledEntity();
		if (m_Controlled != contr)
		{
			m_Controlled = GenericEntity.Cast(contr);
			if (m_Controlled)
			{
				m_WpnManager = BaseWeaponManagerComponent.Cast(m_Controlled.FindComponent(BaseWeaponManagerComponent));
				m_Controller = SCR_CharacterControllerComponent.Cast(m_Controlled.FindComponent(SCR_CharacterControllerComponent));
			}
		}

		if (m_Controller.IsChangingItem())
		{
			return ENodeResult.RUNNING;
		}
		
		if (m_Controlled && m_WpnManager && m_Controller)
		{
			m_WpnManager.GetWeaponsSlots(m_Weapons);
			
			foreach (WeaponSlotComponent slot : m_Weapons)
			{
				//PrintFormat("Weapon %1 of type %2", slot, slot.GetWeaponType());
				if (slot.GetWeaponType() == weaponType)
				{
					if (m_Controller.SelectWeapon(slot))
					{
						return ENodeResult.SUCCESS;
					}
				}							
			}			
		}		
		return ENodeResult.FAIL;
	}
	
	override bool VisibleInPalette()
	{
		return true;
	}
	
	override bool CanReturnRunning()
	{
		return true;
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_WEAPON_TYPE
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;		
	}
};