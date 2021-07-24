#include "raydpch.h"
#include "App.h"

App::App(const std::string& name)
{
	Log::Init();
	m_Window = MakeScopedPtr<Window>(WindowProps{ "Raydriarch", 1280, 720 });
}

App::~App()
{
}

void App::Run()
{
	while (!m_Window->IsClosed()) {
		m_Window->Update();
	}
}
