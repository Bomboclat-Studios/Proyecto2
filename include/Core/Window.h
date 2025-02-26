#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Core/Module.h"

struct SDL_Window;
struct SDL_Surface;

class Window : public Module
{
public:

	Window();

	Window(bool startEnabled);

	// Destructor
	virtual ~Window();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called before quitting
	bool CleanUp();

	// Changae title
	void SetTitle(const char* title);

	// Retrive window size
	void GetWindowSize(uint& width, uint& height) const;

	// Retrieve window scale
	uint GetScale() const;

    void SetFullscreen(bool fullscreen);

    void SetBorderless(bool borderless);

    void SetFullscreenWindow(bool fullscreen_window);

    void SetResolution(int width, int height);

public:
	// The window we'll be rendering to
	SDL_Window* window;

	// The surface contained by the window
	SDL_Surface* screenSurface;

private:
	SString title;
	uint width;
	uint height;
	uint scale;
};

#endif // __WINDOW_H__