#ifndef _SIMULATION_H
#define _SIMULATION_H

#include "rgtypes.h"
#include "kinematicsmodel.h"
#include "animation.h"
#include "animator.h"

typedef struct SimulationSetupInfo {
	String model;
	String animation;
} SimulationSetupInfo;

class Simulation {
	public:
		Simulation() {}
		~Simulation() {}

		Bool Setup(const SimulationSetupInfo* info);
		void Free();
		Bool Step(Float64 dt);

	public:
		Engine::KinematicsModel* model;
		Engine::Animation* animation;
		Engine::Animator* animator;
};

#endif