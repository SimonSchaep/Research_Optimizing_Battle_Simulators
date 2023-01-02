#pragma once

class AgentBase;

class MeleeAttack
{
public:
	MeleeAttack();
	~MeleeAttack();
	
	MeleeAttack(const MeleeAttack& other) = delete;
	MeleeAttack& operator=(const MeleeAttack& other) = delete;
	MeleeAttack(MeleeAttack&& other) = delete;
	MeleeAttack& operator=(MeleeAttack&& other) = delete;

	void Reset(float damage, float attackSpeed, float attackRange);

	void Update(float dt);

	void TryAttack(AgentBase* target);

	bool IsTargetInRange(AgentBase* target, const Elite::Vector2& ownPosition);

private:
	float m_Damage{};
	float m_AttackRange{};

	float m_AttackTimer{};
	float m_AttackDelay{};
};

