#pragma once

#include "NodeGroup.h"

class RoadIntersection;


class TrafficLightProgramState
{
public:
	TrafficLightProgramState();

	// Getters 
	bool IsTriggered() const;
	TrafficLightSignal GetNodeSignal(const Node* node) const;
	Seconds GetDuration() const;

	// Setters

	void Clear();
	void SetSignal(Node* node, TrafficLightSignal signal);
	void AddTrigger(Node* node);
	void SetDuration(Seconds duration);

private:
	Map<Node*, TrafficLightSignal> m_signals;
	Set<Node*> m_triggers;
	Seconds m_duration;
};


class TrafficLightProgram
{
public:
	friend class RoadNetwork;

public:
	// Constructors
	TrafficLightProgram();
	~TrafficLightProgram();

	// Getters
	TrafficLightSignal GetSignal(const Node* node) const;

	// Setters
	void AddState(const TrafficLightProgramState& state);
	void BeginState(int index);
	void Udpate(Seconds dt);

private:
	Array<TrafficLightProgramState> m_states; // Ordered by priority
	TrafficLightProgramState* m_currentState;
	TrafficLightProgramState* m_nextState;
	int m_currentStateIndex;
	Seconds m_stateTimer;

	Seconds m_yellowDuration;
	Seconds m_redDelay;
};

