#pragma comment(lib, "nclgl.lib")

#include "../nclgl/window.h"
#include "MyGame.h"
#include "Renderer.h"
#include <thread>


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
	{
		if(!Window::Initialise("Game Technologies", 1600,900,false)) {
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
		Quit();

	}

	return 0;

}