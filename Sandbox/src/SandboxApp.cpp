#include "Peanut_Engine.h"

class Sandbox : public Peanut_Engine::Application
{
public:
	Sandbox() {

	}

	~Sandbox() {

	}

};


Peanut_Engine::Application* Peanut_Engine::CreateApplication() {
	return new Sandbox();
}