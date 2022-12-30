//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Battle_Simulator.h"

using namespace Elite;

//Destructor
App_Battle_Simulator::~App_Battle_Simulator()
{	
	SAFE_DELETE(m_pAgentBasePooler);
}

//Functions
void App_Battle_Simulator::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(55.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(m_WorldDimensions / 2);

	int size{ 500'000 };

	m_pAgentBasePooler = new AgentBasePooler{ size }; //500k agents ready to be spawned
}

void App_Battle_Simulator::Update(float deltaTime)
{
	m_CurrentFps = 1 / deltaTime;

	ProcessInput();

	UpdateAndRenderUI();

	if (m_IsPaused)
	{
		deltaTime = 0;
	}
	m_pAgentBasePooler->Update(deltaTime);
}

void App_Battle_Simulator::UpdateAndRenderUI()
{
	int id0Count{};
	int id1Count{};
	int id2Count{};
	int id3Count{};
	m_pAgentBasePooler->GetEnabledAgentCountsByTeamId(id0Count, id1Count, id2Count, id3Count);

	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Current");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / m_CurrentFps);
	ImGui::Text("%.1f FPS", m_CurrentFps);
	ImGui::Unindent();
	ImGui::Text("Average");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Units");
	ImGui::Text("%.0f Units Total", float(m_pAgentBasePooler->GetEnabledAgentsCount()));
	ImGui::Indent();
	ImGui::Text("%.0f Red", float(id0Count));
	ImGui::Text("%.0f Blue", float(id1Count));
	ImGui::Text("%.0f Green", float(id2Count));
	ImGui::Text("%.0f Yellow", float(id3Count));
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::InputInt("Count", &m_BenchmarkSpawnCount, 100);
	ImGui::Spacing();
	if (ImGui::Button("Start benchmark"))
	{
		SpawnBenchmark(m_BenchmarkSpawnCount);
		m_IsPaused = false;
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Checkbox(" Render Grid", &m_RenderGrid);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Checkbox(" Multithreading", &m_pAgentBasePooler->GetUsingMultiThreading());

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();

}

void App_Battle_Simulator::ProcessInput()
{
	if (INPUTMANAGER->IsKeyboardKeyDown(eScancode_Space))
	{
		m_IsPaused = !m_IsPaused;
	}

	if (INPUTMANAGER->IsKeyboardKeyDown(eScancode_Backspace))
	{
		//will remove units when drawing box
		m_SpawningUnitTeamID = -1;
	}
	else if (INPUTMANAGER->IsKeyboardKeyDown(eScancode_1))
	{
		m_SpawningUnitTeamID = 0;
	}
	else if (INPUTMANAGER->IsKeyboardKeyDown(eScancode_2))
	{
		m_SpawningUnitTeamID = 1;
	}
	else if (INPUTMANAGER->IsKeyboardKeyDown(eScancode_3))
	{
		m_SpawningUnitTeamID = 2;
	}
	else if (INPUTMANAGER->IsKeyboardKeyDown(eScancode_4))
	{
		m_SpawningUnitTeamID = 3;
	}

	if (INPUTMANAGER->IsMouseMoving())
	{
		//update second mouse position
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseMotion);
		M_MousePos2 = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}
	if (INPUTMANAGER->IsMouseButtonDown(InputMouseButton::eLeft))
	{
		//update first mouse position
		m_IsHoldingMouseDown = true;
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		M_MousePos1 = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}
	if (INPUTMANAGER->IsMouseButtonDown(InputMouseButton::eRight))
	{
		m_IsHoldingMouseDown = false;
	}
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_IsHoldingMouseDown)
	{
		m_IsHoldingMouseDown = false;
		float xMin{ min(M_MousePos1.x, M_MousePos2.x) };
		float xMax{ max(M_MousePos1.x, M_MousePos2.x) };
		float yMin{ min(M_MousePos1.y, M_MousePos2.y) };
		float yMax{ max(M_MousePos1.y, M_MousePos2.y) };
		
		//if removing agents
		if (m_SpawningUnitTeamID == -1)
		{
			std::vector<AgentBase*> agentsToDisable{};
			const std::vector<AgentBase*>& agents{ m_pAgentBasePooler->GetEnabledAgents() };
			for (int i{}; i < m_pAgentBasePooler->GetEnabledAgentsCount(); ++i)
			{
				//if agent is within drawed box
				if (agents[i]->GetPosition().x >= xMin && agents[i]->GetPosition().x <= xMax && agents[i]->GetPosition().y >= yMin && agents[i]->GetPosition().y <= yMax)
				{
					//add agent to vector of agents that need to be disabled
					agentsToDisable.push_back(agents[i]);
				}
			}
			//disable agents that need to be disabled
			for (AgentBase* pAgent : agentsToDisable)
			{
				pAgent->Damage(9999);
			}
		}
		else
		{
			Elite::Color color{ m_TeamColors[m_SpawningUnitTeamID] };

			if (int(xMax - xMin) != 0 && int(yMax - yMin) != 0) //to avoid division by 0
			{
				//spawn agents in random position in drawed box
				for (int i{}; i < (xMax - xMin) * (yMax - yMin) / 4; ++i)
				{
					m_pAgentBasePooler->SpawnNewAgent(m_SpawningUnitTeamID, { xMin + float(rand() % int(xMax - xMin)),yMin + float(rand() % int(yMax - yMin)) }, 1, color, 100, 10, 1, 5, 10);
				}
			}
		}			
	}
}

void App_Battle_Simulator::Render(float deltaTime) const
{
	if (m_IsHoldingMouseDown)
	{
		//draw box to visualize where we are spwning agents

		float xMin{ min(M_MousePos1.x, M_MousePos2.x) };
		float xMax{ max(M_MousePos1.x, M_MousePos2.x) };
		float yMin{ min(M_MousePos1.y, M_MousePos2.y) };
		float yMax{ max(M_MousePos1.y, M_MousePos2.y) };

		Elite::Color color{1,1,1};
		if (m_SpawningUnitTeamID != -1)
		{
			color = m_TeamColors[m_SpawningUnitTeamID];
		}
		std::vector<Elite::Vector2> points{ {xMin, yMin},{xMin,yMax},{xMax,yMax},{xMax,yMin} };
		DEBUGRENDERER2D->DrawPolygon(&points[0], 4, color, -1);
	}	
	
	m_pAgentBasePooler->Render(m_RenderGrid);
}

void App_Battle_Simulator::SpawnBenchmark(int countPerTeam)
{
	float xMin{ 150 };
	float xMax{ 350 };
	float yMin{ 350 };
	float yMax{ 450 };

	Elite::Color color{ m_TeamColors[0] };

	//spawn agents in random position in drawed box
	for (int i{}; i < countPerTeam; ++i)
	{
		m_pAgentBasePooler->SpawnNewAgent(0, { xMin + float(rand() % int(xMax - xMin)),yMin + float(rand() % int(yMax - yMin)) }, 1, color, 100, 10, 1, 5, 10);
	}


	xMin = 150;
	xMax = 350;
	yMin = 50;
	yMax = 150;

	color = m_TeamColors[1];

	//spawn agents in random position in drawed box
	for (int i{}; i < countPerTeam; ++i)
	{
		m_pAgentBasePooler->SpawnNewAgent(1, { xMin + float(rand() % int(xMax - xMin)),yMin + float(rand() % int(yMax - yMin)) }, 1, color, 100, 10, 1, 5, 10);
	}


	xMin = 350;
	xMax = 450;
	yMin = 150;
	yMax = 350;

	color = m_TeamColors[2];

	//spawn agents in random position in drawed box
	for (int i{}; i < countPerTeam; ++i)
	{
		m_pAgentBasePooler->SpawnNewAgent(2, { xMin + float(rand() % int(xMax - xMin)),yMin + float(rand() % int(yMax - yMin)) }, 1, color, 100, 10, 1, 5, 10);
	}

	xMin = 50;
	xMax = 150;
	yMin = 150;
	yMax = 350;

	color = m_TeamColors[3];

	//spawn agents in random position in drawed box
	for (int i{}; i < countPerTeam; ++i)
	{
		m_pAgentBasePooler->SpawnNewAgent(3, { xMin + float(rand() % int(xMax - xMin)),yMin + float(rand() % int(yMax - yMin)) }, 1, color, 100, 10, 1, 5, 10);
	}
}

