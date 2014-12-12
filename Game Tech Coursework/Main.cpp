#pragma comment(lib, "nclgl.lib")

#include "../nclgl/window.h"
#include "MyGame.h"
#include "Renderer.h"
#include <thread>

//TODO: REMOVE THIS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

bool quit = false;

int Quit(bool pause = false, const string &reason = "") {
	PhysicsSystem::Destroy();
	Window::Destroy();
	Renderer::Destroy();

	if(pause) {
		std::cout << reason << std::endl;
		system("PAUSE");
	}

	return 0;
}

void UpdatePhysicsEngine(MyGame* game){

	GameTimer gt = GameTimer();

	while (!quit){
		game->UpdatePhysics(gt.GetTimedMS());
	}

}


int main() {
	// Enable memory leak detection
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	{
		if(!Window::Initialise("Game Technologies", 1280,720,false)) {
			return Quit(true, "Window failed to initialise!");
		}

		if(!Renderer::Initialise()) {
			return Quit(true, "Renderer failed to initialise!");
		}

		PhysicsSystem::Initialise();

		MyGame* game = new MyGame();

		Window::GetWindow().LockMouseToWindow(true);
		Window::GetWindow().ShowOSPointer(false);

		Window::GetWindow().GetTimer()->GetTimedMS();

		std::thread physThread(UpdatePhysicsEngine, game);

		while(Window::GetWindow().UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
			float msec = Window::GetWindow().GetTimer()->GetTimedMS();	//How many milliseconds since last update?

			game->UpdateGraphics(msec);
			game->UpdateGame(msec);
		}

		quit = true;
		physThread.join();

		delete game;

	}

	// Any point in your code you can dump all the memory allocations
	_CrtDumpMemoryLeaks(); // dump memory leaks to the output window

	return 0;

}