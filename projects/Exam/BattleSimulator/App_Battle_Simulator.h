#ifndef BATTLESIMULATOR_APPLICATION_H
#define BATTLESIMULATOR_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "AgentBasePooler.h"


//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_Battle_Simulator final : public IApp
{
public:
	//Constructor & Destructor
	App_Battle_Simulator() = default;
	virtual ~App_Battle_Simulator();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers

	AgentBasePooler* m_pAgentBasePooler{};

	Elite::Vector2 m_WorldDimensions{ 500,500 };

	bool m_IsPaused{true};

	float m_CurrentFps{};

	int m_SpawningUnitTeamID{};

	bool m_IsHoldingMouseDown{};
	Elite::Vector2 M_MousePos1{};
	Elite::Vector2 M_MousePos2{};

	bool m_RenderGrid{};

	int m_BenchmarkSpawnCount{ 500 };
	void SpawnBenchmark(int countPerTeam);

	void UpdateAndRenderUI();

	void ProcessInput();

	const std::vector<Elite::Color> m_TeamColors{ {1,0,0}, {0,0,1}, {0,1,0}, {1,1,0} };

	//C++ make the class non-copyable
	App_Battle_Simulator(const App_Battle_Simulator&) = delete;
	App_Battle_Simulator& operator=(const App_Battle_Simulator&) = delete;
};
#endif