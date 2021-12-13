#include "raydpch.h"
#include "App.h"

App::App(const std::string& name)
{
	Log::Init();
	m_Window = MakeScopedPtr<Window>(WindowProps{ "Raydriarch", 1280, 720 });
	
	Graphics::Init(m_Window);
}

App::~App()
{
	Graphics::Shutdown();
}

void App::Run()
{
	while (!m_Window->IsClosed()) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();

		m_Window->Update();
		Graphics::Present(m_Window, deltaTime);
	}

}
