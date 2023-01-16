#pragma once
class Health final
{
public:
	Health();
	~Health();

	Health(const Health& other) = delete;
	Health& operator=(const Health& other) = delete;
	Health(Health&& other) = delete;
	Health& operator=(Health&& other) = delete;

	void Reset(float healthAmount);

	void Damage(float damageAmount);
	bool IsDead() { return m_CurrentHealth <= 0; };

private:
	float m_CurrentHealth;
	float m_MaxHealth;
};

