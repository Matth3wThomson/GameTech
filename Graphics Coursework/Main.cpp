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

	while(w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		renderer.UpdateScene(w.GetTimer()->GetTimedMS());
		renderer.RenderScene();

		////Test "resetting the simulation"
		//if (Window::GetKeyboard()->KeyDown(KEYBOARD_CONTROL)
		//	&& Window::GetKeyboard()->KeyDown(KEYBOARD_F1)){
		//		renderer = Renderer(w);
		//}
	}

	return 0;
}