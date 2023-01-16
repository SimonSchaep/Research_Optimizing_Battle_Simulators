#pragma once
#include "Health.h"
#include "MeleeAttack.h"

class AgentBasePooler;

class AgentBase final
{
public:
	AgentBase();
	~AgentBase();

	AgentBase(const AgentBase& other) = delete;
	AgentBase& operator=(const AgentBase& other) = delete;
	AgentBase(AgentBase&& other) = delete;
	AgentBase& operator=(AgentBase&& other) = delete;
	
	void Enable(int teamId, const Elite::Vector2& position, float radius, const Elite::Color& color, float healthAmount, float damage, float attackSpeed, float attackRange, float speed);
	void Disable();
	bool GetIsEnabled() { return m_IsEnabled; };

	const Elite::Vector2& GetPosition() { return m_Position; };
	const Elite::Vector2& GetVelocity() { return m_Velocity; };
	int GetTeamId() { return m_TeamId; };

	void Update(float dt, AgentBasePooler* pAgentBasePooler, bool separation);
	void Render();

	void Damage(float damageAmount) { m_Health.Damage(damageAmount); };

private:
	//components
	Health m_Health{};
	MeleeAttack m_MeleeAttack{};

	//const data (not actually const cause can be changed when reenabling)
	int m_TeamId{};
	float m_Speed;

	float m_Radius{};
	Elite::Color m_Color{};

	//temp data
	AgentBase* m_pTargetAgent{};
	Elite::Vector2 m_TargetPosition{}; //used if no targetagent

	Elite::Vector2 m_Velocity{};
	Elite::Vector2 m_Position{};	

	bool m_IsEnabled{};

	std::vector<AgentBase*> m_Neighbors{};
	int m_NeighborCount{};


	void CalculateVelocity(float dt, bool separation);

	bool Move(float dt);

	void FindTarget(AgentBasePooler* pAgentBasePooler, bool findNeighbors);
};

