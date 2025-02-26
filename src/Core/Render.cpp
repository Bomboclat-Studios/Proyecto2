#include "Core/App.h"
#include "Core/Window.h"
#include "Core/Render.h"
#include "Gameplay/Entities/Entity.h"
#include "Core/DebugUI.h"

#include "Utils/Defs.h"
#include "Utils/Log.h"
#include <iostream>

#include <SDL_render.h>
#include <tracy/Tracy.hpp>
#include <imgui.h>
#include <sstream>
#include <regex>

#define VSYNC true

Render::Render() : Module()
{
	name.Create("renderer");
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

Render::Render(bool startEnabled) : Module(startEnabled)
{
	name.Create("renderer");
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render()
{}

// Called before render is available
bool Render::Awake(pugi::xml_node& config)
{
	LOG("Create SDL rendering context");
	bool ret = true;

	uint32_t flags = SDL_RENDERER_ACCELERATED;

	if (config.child("vsync").attribute("value").as_bool(true) == true)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
		LOG("Using vsync");
	}

	renderer = SDL_CreateRenderer(app->win->window, -1, flags);

	if (renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		// camera.w = app->win->screenSurface->w;
		// camera.h = app->win->screenSurface->h;
		SDL_RendererInfo info;
		SDL_GetRendererOutputSize(renderer, &camera.w, &camera.h);
		//camera.x = 0;
		//camera.y = 0;
	}


	//initialise the SDL_ttf library
	TTF_Init();

	//load a font into memory
	font = TTF_OpenFont("Assets/Fonts/GFSDidot.ttf", 25);
	//TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
	//TTF_SetFontOutline(font, 0);
	//TTF_SetFontKerning(font, 0);
	//TTF_SetFontHinting(font, TTF_HINTING_LIGHT_SUBPIXEL);

	return ret;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);

	overlayTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, camera.w, camera.h);
	SDL_SetTextureBlendMode(overlayTarget, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, overlayTarget);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(renderer, NULL);

	SDL_SetRenderTarget(renderer, NULL);

	lightingTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, camera.w, camera.h);
    SDL_SetTextureBlendMode(lightingTarget, SDL_BLENDMODE_MUL); // o SDL_BLENDMODE_MOD

	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	// OPTICK PROFILIN
	ZoneScoped;

	SDL_SetRenderTarget(renderer, overlayTarget);
	SDL_RenderClear(renderer);

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderClear(renderer);
	
	return true;
}

bool Render::Update(float dt)
{
	// OPTICK PROFILIN
	ZoneScoped;

	cameraInterpolation(camera.target, camera.lerpSpeed, dt, camera.offset);

	// Sort and draw all sprites
	std::stable_sort(sprites.begin(), sprites.end(),
	[](const Sprite& a, const Sprite& b)
	{
		if(a.layer != b.layer)
		{
			return a.layer < b.layer;
		}
		else
		{
			return a.pivot.y < b.pivot.y;
		}
	}
	);
	
	for(Sprite& s : sprites)
	{
		DrawSprite(s);
		
	}

	//Render all pivot points
	if(showPivots)
	{
		for(Sprite& s : sprites)
		{
			DrawRectangle({ s.pivot.x - 5, s.pivot.y - 5, 10, 10 }, 255, 0, 0, 255, true);
		}
	}


	// ------DrawLighting------- //

    // Draw the lightingTarget
    SDL_RenderCopy(renderer, lightingTarget, NULL, NULL);

	// Reset the blend mode to its default value
    SDL_SetRenderDrawBlendMode(app->render->renderer, SDL_BLENDMODE_BLEND);


	// ------Draw overlay------- //
	SDL_RenderCopy(renderer, overlayTarget, NULL, NULL);

	return true;
}

void Render::DrawImGui()
{
	if(app->debugUI->renderInfo)
	{
		ImGui::Begin("Render", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Camera Position:"); ImGui::SameLine();
		ImGui::DragInt2("##Camera Position", &camera.x, 1.0f);

		ImGui::Checkbox("Camera Interpolation", &camera.useInterpolation);

		ImGui::Text("camera lerp speed: %f", camera.lerpSpeed);
		ImGui::SliderFloat("Camera Lerp Speed", &camera.lerpSpeed, 0.0f, 16.0f);
		ImGui::Text("camera target: %s", camera.target != nullptr ? camera.target->name.GetString() : "null");
		ImGui::Checkbox("Free Cam", &freeCam);
		ImGui::Text("Vsync: %s", vsyncEnabled ? "Enabled" : "Disabled");

		ImGui::Checkbox("Show Pivots", &showPivots);

		ImGui::Separator();
		//Sprite info
		ImGui::Text("Sprites: %d", sprites.size());

		// Create a child region of fixed width and enable horizontal scrollbar
		if (ImGui::BeginChild("Sprite List", ImVec2(0, 200), true))
		{
			for (Sprite& s : sprites)
			{
				// Display sprite information
				if (ImGui::TreeNode(&s, "Sprite %d", &s - &sprites[0]))
				{
					ImGui::Text("Position: %d, %d", s.position.x, s.position.y);
					ImGui::Text("Speed: %f", s.speed);
					ImGui::Text("Angle: %f", s.angle);
					ImGui::Text("Size: %f", s.size);
					ImGui::Text("Layer: %d", s.layer);
					ImGui::Text("Flip: %d", s.flip);
					ImGui::Text("Pivot: %d, %d", s.pivot.x, s.pivot.y);
					// Add more sprite properties here...

					ImGui::TreePop();
				}
			}
			ImGui::EndChild();
		}
		
		ImGui::End();
	}
}

bool Render::PostUpdate()
{
	// OPTICK PROFILIN
	ZoneScoped;

	sprites.clear();

	SetViewPort({
		0,
		0,
		camera.w,
		camera.h
		});
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.b, background.a);
	SDL_RenderPresent(renderer);

	FrameMark;
	
	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{	
	//SDL_RenderSetClipRect
	SDL_RenderSetViewport(renderer, &rect);
	
}

void Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

/// @brief Camera interpolation
/// @return TODO remove hardcoded 16 and calculate the right offset
/// @param target Entity to follow
/// @param lerpSpeed the camera’s movement speed. Lower values result in a “lazier” camera.
/// @param offset position of the camera relative to the target. (0,0) is the center.
/// @param dt delta time
void Render::cameraInterpolation(Entity* target, float lerpSpeed, float dt, iPoint offset)
{	
	//DEBUG
	if(freeCam)
	{
		target = nullptr;
		camera.useInterpolation = false;
	}
	else
	{
		camera.useInterpolation = true;//TODO: Arreglar para que no setee cada frame
	}

	if(target != nullptr)
	{
		if(camera.useInterpolation)
		{
			camera.x = std::ceil(std::lerp(camera.x, (int)(camera.w / 2 / app->win->GetScale()) - 16 - target->position.x - offset.x, dt * lerpSpeed / 1000));
			camera.y = std::ceil(std::lerp(camera.y, (int)(camera.h / 2 / app->win->GetScale()) - 16 - target->position.y - offset.y, dt * lerpSpeed / 1000));
		}
		else
		{
			camera.x = -target->position.x + camera.w / app->win->GetScale() / 2 - 16 - offset.x;
			camera.y = -target->position.y + camera.h / app->win->GetScale() / 2 - 16 - offset.y;
		}
	}
}

// Blit to screen
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, SDL_Rect* section, float speed, double angle, float size, uint layer, SDL_RendererFlip flip, int pivotX, int pivotY)
{
	bool ret = true;

	if(section != NULL)
	{
		Sprite sprite = Sprite(texture, x, y, *section, speed, angle, size, layer, flip, pivotX, pivotY);
		sprites.push_back(sprite);
	}
	else
	{
		Sprite sprite = Sprite(texture, x, y, {0,0,0,0}, speed, angle, size, layer, flip, pivotX, pivotY);
		sprites.push_back(sprite);
	}

	return ret;
}

bool Render::DrawTextureLegacy(SDL_Texture* texture, int x, int y, SDL_Rect* section, float speed, double angle, float size, SDL_RendererFlip flip, int pivotX, int pivotY, bool overlayTarget)
{
	bool ret = true;

	if(overlayTarget)SDL_SetRenderTarget(app->render->renderer, app->render->overlayTarget);

	uint scale = app->win->GetScale();

	SDL_Rect rect;
	rect.x = ((int)(camera.x * speed) + x) * scale;
	rect.y = ((int)(camera.y * speed) + y) * scale;

	if(section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w *= size;
	rect.h *= size;

	rect.w *= scale;
	rect.h *= scale;

	if(texture == NULL)
	{
		LOG("Cannot blit to screen. Texture is nullptr");
		return true;
	}

	if(SDL_RenderCopyEx(renderer, texture, section, &rect, angle, NULL, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	if(overlayTarget)SDL_SetRenderTarget(app->render->renderer, NULL);

	return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;

	SDL_SetRenderTarget(renderer, overlayTarget);

	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = ((int)(camera.x + rect.x) * scale);
		rec.y = ((int)(camera.y + rect.y) * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	SDL_SetRenderTarget(renderer, NULL);

	return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;

	SDL_SetRenderTarget(renderer, overlayTarget);

	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer, (camera.x + x1) * scale, (camera.y + y1) * scale, (camera.x + x2) * scale, (camera.y + y2) * scale);
	else
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	SDL_SetRenderTarget(renderer, NULL);

	return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;

	SDL_SetRenderTarget(renderer, overlayTarget);

	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	for(uint i = 0; i < 360; ++i)
	{
		points[i].x = ((int)(x + camera.x) * scale + radius * cos(i * factor) * scale);
		points[i].y = ((int)(y + camera.y) * scale + radius * sin(i * factor) * scale);
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	SDL_SetRenderTarget(renderer, NULL);

	return ret;
}

bool Render::DrawText(const char* text, int posx, int posy, int w, int h, SDL_Color color) {

    SDL_SetRenderTarget(renderer, overlayTarget);

    TTF_SetFontSize(app->render->font, h * 0.75);

	std::string textRaw = text;
	std::string normalText = std::regex_replace(textRaw, std::regex("\\\\n"), "\n");

    std::istringstream stream(normalText);
    std::string line;
    int lineNum = 0;
    while (std::getline(stream, line)) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(app->render->font, line.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        uint scale = app->win->GetScale();

        SDL_Rect rect;
        rect.x = ((int)posx + w / 4) * scale;
        rect.y = ((int)posy + lineNum * h) * scale; // Adjust the y position for each line

        SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);

        rect.w *= scale;
        rect.h *= scale;

        SDL_Rect dstrect = { rect.x,  rect.y, rect.w, rect.h };

        SDL_RenderCopy(renderer, texture, NULL, &dstrect);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);

        lineNum++;
    }

	//Reset font size
	TTF_SetFontSize(app->render->font, 25);

	SDL_SetRenderTarget(renderer, NULL);

	return true;
}
// for now load camera's x and y
bool Render::LoadState(pugi::xml_node node) {

	camera.x = node.child("camera").attribute("x").as_int();
	camera.y = node.child("camera").attribute("y").as_int();

	return true;
}

// using append_child and append_attribute
bool Render::SaveState(pugi::xml_node node) {

	pugi::xml_node camNode = node.append_child("camera");
	camNode.append_attribute("x").set_value(camera.x);
	camNode.append_attribute("y").set_value(camera.y);

	return true;
}

void Render::SetVsync(bool vsync)
{
	pugi::xml_document configFile;
	pugi::xml_parse_result result = configFile.load_file("config.xml");

	if (result) {
		pugi::xml_node rendererNode = configFile.child("config").child("renderer");
		pugi::xml_node vsyncNode = rendererNode.child("vsync");

		vsyncNode.attribute("value").set_value(vsync ? "true" : "false");
		vsyncEnabled = vsync;

		configFile.save_file("config.xml");
	} else {
		// TODO Handle error
		LOG("Error loading config.xml");
	}
}
bool Render::DrawSprite(Sprite &sprite) const
{
	SDL_Texture* texture = sprite.texture; //TODO: Check if texture is nullptr
	int x = sprite.position.x;
	int y = sprite.position.y;
	SDL_Rect* section = &sprite.section;
	if(section->w == 0 && section->h == 0)
	{
		section = NULL;
	}
	float speed = sprite.speed;
	double angle = sprite.angle;
	float size = sprite.size;
	SDL_RendererFlip flip = sprite.flip;
	int pivotX = sprite.pivotX;
	int pivotY = sprite.pivotY;

	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_Rect rect;
	rect.x = ((int)(camera.x * speed) + x) * scale;
	rect.y = ((int)(camera.y * speed) + y) * scale;

	if(section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w *= size;
	rect.h *= size;

	rect.w *= scale;
	rect.h *= scale;

	if(texture == NULL)
	{
		//LOG("Cannot blit to screen. Texture is nullptr");
		return true;
	}

	if(SDL_RenderCopyEx(renderer, texture, section, &rect, angle, NULL, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}