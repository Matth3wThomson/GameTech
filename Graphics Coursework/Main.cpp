#pragma comment(lib, "nclgl.lib")

#include "../nclgl/window.h"
#include "Renderer.h"

int main() {

	Window w("Matthew Thomson Coursework", 1280,720,false);
	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);//This handles all the boring OGL 3.2 initialisation stuff, and sets up our tutorial!
	if(!renderer.HasInitialised()) {
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);

	w.GetTimer()->GetTimedMS();

	while(w.UpdateWindow() && !Window::GetKeyboard()->KeyTriggered(KEYBOARD_ESCAPE)){
		renderer.UpdateScene(w.GetTimer()->GetTimedMS());
		renderer.RenderScene();
	}

	return 0;
}