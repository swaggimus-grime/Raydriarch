#include "raydpch.h"
#include "App.h"

int main() {
	ScopedPtr<App> app = MakeScopedPtr<App>();
	if(app) app->Run();
}