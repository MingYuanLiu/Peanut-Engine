#include "Peanut_Engine.h"

class ExampleLayer : public Peanut_Engine::Layer
{
public:
	ExampleLayer() : Layer("Example")
	{

	}

	void OnUpdate() override {
		PE_INFO("Example Layer's update function.");
	}

	void OnEvent(Peanut_Engine::Event& event) override {
		PE_WARN("{0}", event);
	}
};


class Sandbox : public Peanut_Engine::Application
{
public:
	Sandbox() : m_layer(new ExampleLayer()) {
		PushLayer(m_layer);
	}

	~Sandbox() {

	}
private:
	Peanut_Engine::Layer* m_layer;

};


Peanut_Engine::Application* Peanut_Engine::CreateApplication() {
	return new Sandbox();
}