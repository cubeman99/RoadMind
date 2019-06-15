#pragma once

#include "NodeGroup.h"

class RoadIntersection;


class TrafficLightPhase
{
public:
	TrafficLightPhase();

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
	void AddPhase(const TrafficLightPhase& phase);
	void BeginPhase(int index);
	void Udpate(Seconds dt);

private:
	Array<TrafficLightPhase> m_phases; // Ordered by priority
	TrafficLightPhase* m_currentPhase;
	TrafficLightPhase* m_nextPhase;
	int m_currentPhaseIndex;
	Seconds m_phaseTimer;

	Seconds m_yellowDuration;
	Seconds m_redDelay;
};

