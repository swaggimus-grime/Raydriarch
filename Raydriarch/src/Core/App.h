#pragma once

#include "Core.h"
#include "Window.h"

class App {
public:
	App(const std::string& name = "Raid App");
	~App();
	App(App&) = delete;
	App& operator=(const App&) = delete;

	void Run();

private:
	ScopedPtr<Window> m_Window;
};