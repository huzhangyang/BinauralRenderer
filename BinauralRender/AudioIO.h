#pragma once

#include <cstdio>
#include <cstdlib>

#include <fmod.hpp>
#include <fmod_errors.h>

using namespace FMOD;

class AudioIO
{
public:
	void Init();
	void Update();
	void Release();
private:
	FMOD::System *system;
	FMOD_RESULT result;
	void ErrorHandle();

};