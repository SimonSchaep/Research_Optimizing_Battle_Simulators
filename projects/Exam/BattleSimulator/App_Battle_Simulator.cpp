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

	m_pAgentBasePooler = new AgentBasePooler{ size, m_WorldDimensions }; //500k agents ready to be spawned
}

void App_Battle_Simulator::Update(float deltaTime)
{
	m_CurrentFps = 1 / deltaTime;

	ProcessInput();

	UpdateAndRenderUI();

	m_pAgentBasePooler->Update(deltaTime * m_TimeScale);
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
	if (ImGui::CollapsingHeader("Info", (const char*)0, true, true))
	{
		ImGui::Indent();
		ImGui::Text("Click 'spawn agents' to ");
		ImGui::Text("spawn the given amount of");
		ImGui::Text("agents for each team");
		ImGui::Text("Or drag to spawn agents of");
		ImGui::Text("the selected team in an");
		ImGui::Text("area");
		ImGui::Text("Change the selected team");
		ImGui::Text("by pressing '1', '2', '3'");
		ImGui::Text("or '4'");
		ImGui::Text("Erase units by pressing");
		ImGui::Text("backspace and dragging");
		ImGui::Text("Or click 'Clear all'");
		ImGui::Unindent();
	}

	if (ImGui::CollapsingHeader("Stats", (const char*)0, true, true))
	{
		ImGui::Indent();
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
		ImGui::Unindent();
	}

	if (ImGui::CollapsingHeader("Settings", (const char*)0, true, true))
	{
		ImGui::Indent();
		ImGui::Checkbox(" Separation", &m_pAgentBasePooler->GetUsingSeparation());
		ImGui::Spacing();
		ImGui::Checkbox(" Multithreading", &m_pAgentBasePooler->GetUsingMultiThreading());
		ImGui::Spacing();
		ImGui::Checkbox(" Render Grid", &m_RenderGrid);
		ImGui::Spacing();
		ImGui::Text("TimeScale");
		ImGui::SliderFloat("", &m_TimeScale, 0.f, 10.f);
		ImGui::Spacing();
		ImGui::Unindent();
	}

	if (ImGui::CollapsingHeader("Spawning", (const char*)0, true, true))
	{
		ImGui::Indent();
		ImGui::Text("Amounts per team");
		ImGui::Spacing();
		ImGui::Text("Red");
		ImGui::InputInt(" ", &m_AgentSpawnCounts[0], 100);
		ImGui::Spacing();
		ImGui::Text("Blue");
		ImGui::InputInt("  ", &m_AgentSpawnCounts[1], 100);
		ImGui::Spacing();
		ImGui::Text("Green");
		ImGui::InputInt("   ", &m_AgentSpawnCounts[2], 100);
		ImGui::Spacing();
		ImGui::Text("Yellow");
		ImGui::InputInt("    ", &m_AgentSpawnCounts[3], 100);
		ImGui::Spacing();
		if (ImGui::Button("Spawn agents"))
		{
			SpawnAgents();
		}
		ImGui::Spacing();
		if (ImGui::Button("Clear all"))
		{
			const std::vector<AgentBase*>& agents{ m_pAgentBasePooler->GetEnabledAgents() };
			for (int i{}; i < m_pAgentBasePooler->GetEnabledAgentsCount(); ++i)
			{
				agents[i]->Damage(99999);
			}
		}
		ImGui::Unindent();
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();

}

void App_Battle_Simulator::ProcessInput()
{
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
		//if spawning agents
		else
		{
			if (int(xMax - xMin) != 0 && int(yMax - yMin) != 0) //to avoid division by 0
			{
				//spawn agents in random position in drawed box
				for (int i{}; i < (xMax - xMin) * (yMax - yMin) / 4; ++i)
				{
					SpawnAgent(m_SpawningUnitTeamID, m_TeamColors[m_SpawningUnitTeamID], xMin, xMax, yMin, yMax);
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

		Elite::Color color{ 1,1,1 };
		if (m_SpawningUnitTeamID != -1)
		{
			color = m_TeamColors[m_SpawningUnitTeamID];
		}
		std::vector<Elite::Vector2> points{ {xMin, yMin},{xMin,yMax},{xMax,yMax},{xMax,yMin} };
		DEBUGRENDERER2D->DrawPolygon(&points[0], 4, color, -1);
	}

	//render agents and grid (which is member of AgentBasePooler)
	m_pAgentBasePooler->Render(m_RenderGrid);
}

void App_Battle_Simulator::SpawnAgents()
{
	//team 0
	float xMin{ 150 };
	float xMax{ m_WorldDimensions.x - 150 };
	float yMin{ m_WorldDimensions.y - 150 };
	float yMax{ m_WorldDimensions.y - 50 };

	for (int i{}; i < m_AgentSpawnCounts[0]; ++i)
	{
		SpawnAgent(0, m_TeamColors[0], xMin, xMax, yMin, yMax);
	}


	//team 1
	xMin = 150;
	xMax = m_WorldDimensions.x - 150;
	yMin = 50;
	yMax = 150;

	for (int i{}; i < m_AgentSpawnCounts[1]; ++i)
	{
		SpawnAgent(1, m_TeamColors[1], xMin, xMax, yMin, yMax);
	}


	//team 2
	xMin = m_WorldDimensions.x - 150;
	xMax = m_WorldDimensions.x - 50;
	yMin = 150;
	yMax = m_WorldDimensions.y - 150;

	for (int i{}; i < m_AgentSpawnCounts[2]; ++i)
	{
		SpawnAgent(2, m_TeamColors[2], xMin, xMax, yMin, yMax);
	}


	//team 3
	xMin = 50;
	xMax = 150;
	yMin = 150;
	yMax = m_WorldDimensions.y - 150;

	for (int i{}; i < m_AgentSpawnCounts[3]; ++i)
	{
		SpawnAgent(3, m_TeamColors[3], xMin, xMax, yMin, yMax);
	}
}

void App_Battle_Simulator::SpawnAgent(int teamId, const Elite::Color& color, float xMin, float xMax, float yMin, float yMax)
{
	m_pAgentBasePooler->SpawnNewAgent(teamId, { xMin + float(rand() % int(xMax - xMin)),yMin + float(rand() % int(yMax - yMin)) }, 1, color, 100, 10, 1, 5, 10);
}