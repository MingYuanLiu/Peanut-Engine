#pragma once

#ifdef PE_PLATFORM_WINDOWS

extern Peanut_Engine::Application* Peanut_Engine::CreateApplication();

int main(int argc, char** argv) 
{
	Peanut_Engine::Log::Init();
	PE_CORE_INFO("Welcom to Peanut Engine core logger!");
	PE_INFO("This is Client logger.");

	auto app = Peanut_Engine::CreateApplication();
	app->Run();
	delete app;

	return 0;
}
#else
#error Peanut Engine only supports windows
#endif
