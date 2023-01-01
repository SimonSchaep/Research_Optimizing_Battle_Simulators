//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "MeleeAttack.h"
#include "AgentBase.h"

MeleeAttack::MeleeAttack()
{
}

MeleeAttack::~MeleeAttack()
{
}

void MeleeAttack::Reset(float damage, float attackSpeed, float attackRange)
{
	m_Damage = damage;
	m_AttackRange = attackRange;
	m_AttackDelay = 1 / attackSpeed;
	m_AttackTimer = 0;
}

void MeleeAttack::Update(float dt)
{
	m_AttackTimer += dt;
}

void MeleeAttack::TryAttack(AgentBase* target)
{
	//no need to check for range, since it is done when checking if the agent needs to move
	if (m_AttackTimer >= m_AttackDelay)
	{
		target->Damage(m_Damage);
		m_AttackTimer = 0;
	}
	
}

bool MeleeAttack::IsTargetInRange(AgentBase* target, Elite::Vector2 ownPosition)
{
	return target->GetPosition().DistanceSquared(ownPosition) <= (m_AttackRange * m_AttackRange);
}
