#include "TrafficLight.h"

TrafficLightPhase::TrafficLightPhase()
	: m_duration(6.0f)
{
}

bool TrafficLightPhase::IsTriggered() const
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

TrafficLightSignal TrafficLightPhase::GetNodeSignal(const Node* node) const
{
	if (m_signals.find((Node*) node) != m_signals.end())
		return m_signals.at((Node*) node);
	return TrafficLightSignal::STOP;
}

Seconds TrafficLightPhase::GetDuration() const
{
	return m_duration;
}

void TrafficLightPhase::Clear()
{
	m_signals.clear();
	m_triggers.clear();
}

void TrafficLightPhase::SetSignal(Node* node, TrafficLightSignal signal)
{
	m_signals[node] = signal;
}

void TrafficLightPhase::AddTrigger(Node * node)
{
	m_triggers.insert(node);
}

void TrafficLightPhase::SetDuration(Seconds duration)
{
	m_duration = duration;
}



TrafficLightProgram::TrafficLightProgram()
	: m_currentPhase(nullptr)
	, m_nextPhase(nullptr)
	, m_phaseTimer(0.0f)
	, m_yellowDuration(2.0f)
	, m_redDelay(1.0f)
{
}

TrafficLightProgram::~TrafficLightProgram()
{
}

TrafficLightSignal TrafficLightProgram::GetSignal(const Node* node) const
{
	if (m_currentPhase == nullptr)
	{
		return TrafficLightSignal::GO;
	}
	else if (m_nextPhase != nullptr)
	{
		TrafficLightSignal prev = m_currentPhase->GetNodeSignal(node);
		TrafficLightSignal next = m_nextPhase->GetNodeSignal(node);
		if (prev == next)
		{
			return prev;
		}
		else if (next == TrafficLightSignal::STOP)
		{
			if (m_phaseTimer < m_yellowDuration)
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
		return m_currentPhase->GetNodeSignal(node);
	}
}

void TrafficLightProgram::AddPhase(const TrafficLightPhase& phase)
{
	m_phases.push_back(phase);
}

void TrafficLightProgram::BeginPhase(int index)
{
	m_currentPhaseIndex = index;
	m_nextPhase = &m_phases[index];
	if (m_currentPhase == nullptr)
		m_currentPhase = m_nextPhase;
	m_phaseTimer = 0.0f;
}

void TrafficLightProgram::Udpate(Seconds dt)
{
	if (m_phases.empty())
		return;
	if (m_currentPhase == nullptr)
		BeginPhase(0);


	m_phaseTimer += dt;

	if (m_nextPhase != nullptr)
	{
		if (m_phaseTimer >= m_yellowDuration + m_redDelay)
		{
			m_currentPhase = m_nextPhase;
			m_nextPhase = nullptr;
		}
	}
	else if (m_phaseTimer >= m_currentPhase->GetDuration())
	{
		// Change to the next phase
		for (int i = 1; i < (int) m_phases.size(); i++)
		{
			int index = (m_currentPhaseIndex + i) % m_phases.size();
			if (m_phases[index].IsTriggered())
			{
				BeginPhase(index);
				break;
			}
		}
	}
}
