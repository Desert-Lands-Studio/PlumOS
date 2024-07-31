#include <SDL.h>
#include <iostream>

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

bool handleEvents(SDL_Rect& buttonRect, bool& colorToggle) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
	    if (event.type == SDL_QUIT) {
		return false;
	    } else if (event.type == SDL_MOUSEBUTTONDO
		int x, y;
		SDL_GetMouseState(&x, &y);
		if (x >= buttonRect.x && x <= buttonRect.x + buttonRect.w &&
		    y >= buttonRect.y && y <= buttonRect.y + buttonRect.h) {
		    colorToggle = !colorToggle;
		}
	    }
	}
	return true;

}

void render(SDL_Renderer* renderer, SDL_Rect& squareRect, bool colorToggle) {

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	if (colorToggle) {
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	} else {
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	}
	SDL_RenderFillRect(renderer, &squareRect);

	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	SDL_RenderFillRect(renderer, &buttonRect);

	SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO)
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Simple SDL2 GUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
