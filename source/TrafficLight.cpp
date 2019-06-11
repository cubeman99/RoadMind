#include "TrafficLight.h"

TrafficLightProgramState::TrafficLightProgramState()
	: m_duration(6.0f)
{
}

bool TrafficLightProgramState::IsTriggered() const
{
	if (m_triggers.empty())
	{
		return true;
	}
	else
	{
		return true;
		//for (Node* node : m_triggers)
		//	if (node->IsTriggered())
		//		return true;
		//return false;
	}
}

TrafficLightSignal TrafficLightProgramState::GetNodeSignal(const Node* node) const
{
	if (m_signals.find((Node*) node) != m_signals.end())
		return m_signals.at((Node*) node);
	return TrafficLightSignal::STOP;
}

Seconds TrafficLightProgramState::GetDuration() const
{
	return m_duration;
}

void TrafficLightProgramState::Clear()
{
	m_signals.clear();
	m_triggers.clear();
}

void TrafficLightProgramState::SetSignal(Node* node, TrafficLightSignal signal)
{
	m_signals[node] = signal;
}

void TrafficLightProgramState::AddTrigger(Node * node)
{
	m_triggers.insert(node);
}

void TrafficLightProgramState::SetDuration(Seconds duration)
{
	m_duration = duration;
}



TrafficLightProgram::TrafficLightProgram()
	: m_currentState(nullptr)
	, m_nextState(nullptr)
	, m_stateTimer(0.0f)
	, m_yellowDuration(2.0f)
	, m_redDelay(1.0f)
{
}

TrafficLightProgram::~TrafficLightProgram()
{
}

TrafficLightSignal TrafficLightProgram::GetSignal(const Node* node) const
{
	if (m_currentState == nullptr)
	{
		return TrafficLightSignal::GO;
	}
	else if (m_nextState != nullptr)
	{
		TrafficLightSignal prev = m_currentState->GetNodeSignal(node);
		TrafficLightSignal next = m_nextState->GetNodeSignal(node);
		if (prev == next)
		{
			return prev;
		}
		else if (next == TrafficLightSignal::STOP)
		{
			if (m_stateTimer < m_yellowDuration)
				return TrafficLightSignal::YELLOW;
			else
				return TrafficLightSignal::STOP;
		}
		else
		{
			return TrafficLightSignal::STOP;
		}
	}
	else
	{
		return m_currentState->GetNodeSignal(node);
	}
}

void TrafficLightProgram::AddState(const TrafficLightProgramState& state)
{
	m_states.push_back(state);
}

void TrafficLightProgram::BeginState(int index)
{
	m_currentStateIndex = index;
	m_nextState = &m_states[index];
	if (m_currentState == nullptr)
		m_currentState = m_nextState;
	m_stateTimer = 0.0f;
}

void TrafficLightProgram::Udpate(Seconds dt)
{
	if (m_states.empty())
		return;
	if (m_currentState == nullptr)
		BeginState(0);


	m_stateTimer += dt;

	if (m_nextState != nullptr)
	{
		if (m_stateTimer >= m_yellowDuration + m_redDelay)
		{
			m_currentState = m_nextState;
			m_nextState = nullptr;
		}
	}
	else if (m_stateTimer >= m_currentState->GetDuration())
	{
		// Change to the next state
		for (int i = 1; i < (int) m_states.size(); i++)
		{
			int index = (m_currentStateIndex + i) % m_states.size();
			if (m_states[index].IsTriggered())
			{
				BeginState(index);
				break;
			}
		}
	}
}
