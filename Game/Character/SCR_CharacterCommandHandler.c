[ComponentEditorProps(category: "GameScripted/Character", description: "Scripted character command handler", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterCommandHandlerComponentClass : CharacterCommandHandlerComponentClass
{
};

class SCR_CharacterCommandHandlerComponent : CharacterCommandHandlerComponent
{
	
	override bool HandleMelee(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		if (pInputCtx.GetMeleeAttack())
		{
			GetCommandModifier_Melee().Attack();
			pInputCtx.SetMeleeAttack(false);
			m_MeleeComponent.PerformAttack();
			return true;
		}
		if (m_MeleeComponent)
		{
			m_MeleeComponent.Update(pDt);
		}

		return GetCommandModifier_Melee().IsMeleeAttackTag();
	}
		
	override void OnCommandActivate(int pCmdId)
	{
		m_OnCommandActivate.Invoke(pCmdId);
		
		super.OnCommandActivate(pCmdId);
	}
		
	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		m_MovementState 		= new CharacterMovementState;
		m_OwnerEntity 			= ChimeraCharacter.Cast(owner);
		m_CharacterAnimComp 	= CharacterAnimationComponent.Cast(m_OwnerEntity.FindComponent(CharacterAnimationComponent));
		m_MeleeComponent 		= SCR_MeleeComponent.Cast(m_OwnerEntity.FindComponent(SCR_MeleeComponent));
		m_WeaponManager 		= BaseWeaponManagerComponent.Cast(m_OwnerEntity.FindComponent(BaseWeaponManagerComponent));
	}
	
	protected ChimeraCharacter 				m_OwnerEntity;
	protected ref CharacterMovementState 	m_MovementState;
	protected CharacterAnimationComponent	m_CharacterAnimComp
	protected CharacterControllerComponent  m_CharacterControllerComp;
	protected SCR_MeleeComponent			m_MeleeComponent;
	protected BaseWeaponManagerComponent 	m_WeaponManager;
	
	protected ref CharacterCommandMoveSettings m_MoveSettings = new CharacterCommandMoveSettings();
	protected ref CharacterCommandClimbSettings m_ClimbSettings = new CharacterCommandClimbSettings();
	
	ref ScriptInvoker m_OnCommandActivate = new ScriptInvoker();

};