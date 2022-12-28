//=== General Includes ===
#include "stdafx.h"
#include "EFiniteStateMachine.h"
using namespace Elite;

FiniteStateMachine::FiniteStateMachine(FSMState* startState, Blackboard* pBlackboard)
    : m_pCurrentState(nullptr),
    m_pBlackboard(pBlackboard)
{
    ChangeState(startState);
}

FiniteStateMachine::~FiniteStateMachine()
{
    SAFE_DELETE(m_pBlackboard);
}

void FiniteStateMachine::AddTransition(FSMState* startState, FSMState* toState, FSMCondition* condition)
{
    auto it = m_Transitions.find(startState);
    if (it == m_Transitions.end())
    {
        m_Transitions[startState] = Transitions();
    }
   
    m_Transitions[startState].push_back(std::make_pair(condition, toState));
}

void FiniteStateMachine::Update(float deltaTime)
{
    //TODO 4: Look if 1 or more transitions exists for the current state that we are in
    //Tip: Check the transitions map for a TransitionState pair
    if (!m_pCurrentState)
    {
        return;
    }

    //TODO 5: if a TransitionState exists
    Transitions transitions{ m_Transitions[m_pCurrentState] }; //no need to check since [] will insert default transitions vector if key is invalid
  
    //TODO 6: Loop over all the TransitionState pairs
    for (const TransitionStatePair& transitionStatePair : transitions)
    {
        //TODO 7: If the Evaluate function of the FSMCondition returns true => transition to the new corresponding state
        if (transitionStatePair.first->Evaluate(m_pBlackboard))
        {
            //TODO 8: Update the current state (if one exists)
            ChangeState(transitionStatePair.second);
            return;
        }
    }
}

Blackboard* FiniteStateMachine::GetBlackboard() const
{
    return m_pBlackboard;
}

void FiniteStateMachine::ChangeState(FSMState* newState)
{
    //TODO 1. If currently in a state => make sure the OnExit of that state gets called
    if (m_pCurrentState)
    {
        m_pCurrentState->OnExit(m_pBlackboard);
    }

    //TODO 2. Change the current state to the new state
    m_pCurrentState = newState;


    //TODO 3. Call the OnEnter of the new state
    m_pCurrentState->OnEnter(m_pBlackboard);
        
}
