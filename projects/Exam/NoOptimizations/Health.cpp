//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "Health.h"

Health::Health()
{
}

Health::~Health()
{
}

void Health::Reset(float healthAmount)
{
	m_CurrentHealth = healthAmount;
	m_MaxHealth = healthAmount;
}

void Health::Damage(float damageAmount)
{
	if (IsDead()) return;
	m_CurrentHealth -= damageAmount;	
}
