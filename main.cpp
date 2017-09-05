#include "Draw.hpp"
#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>

#include <chrono>
#include <iostream>

int main(int argc, char **argv) {
	//Configuration:
	struct {
		std::string title = "Game0: Tennis For One";
		glm::uvec2 size = glm::uvec2(800, 640);
	} config;

	//------------  initialization ------------

	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		config.size.x, config.size.y,
		SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI*/
	);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	#ifdef _WIN32
	//On windows, load OpenGL extensions:
	if (!init_gl_shims()) {
		std::cerr << "ERROR: failed to initialize shims." << std::endl;
		return 1;
	}
	#endif

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	//Hide mouse cursor (note: showing can be useful for debugging):
	SDL_ShowCursor(SDL_DISABLE);

	//------------  game state ------------
  int score = 0;
  int lives = 3;

  // based on https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
  srand((unsigned)time(0));
  float target_size = 1.2f;
  float target_y = (rand()/(float)RAND_MAX)*(2.0f - target_size) + target_size/2.0f - 1.0f;

  glm::vec2 target = glm::vec2(-1.0f, target_y);
	glm::vec2 paddle = glm::vec2(1.0f, 0.0f);
	glm::vec2 ball = glm::vec2(0.0f, 0.0f);
	glm::vec2 ball_velocity;
  
	//------------  game loop ------------

	auto previous_time = std::chrono::high_resolution_clock::now();
	bool should_quit = false;
  bool ball_moving = false;
  bool game_over = false;
	while (true) {
		static SDL_Event evt;
		while (SDL_PollEvent(&evt) == 1) {
			//handle input:
			if (evt.type == SDL_MOUSEMOTION) {
        if (!game_over) {
				  paddle.y = (evt.motion.y + 0.5f) / float(config.size.y) *-2.0f + 1.0f;
        }
			} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
        if (!game_over) {
          if (!ball_moving) {
				    ball_velocity = glm::vec2(1.5f, (rand()/(float)RAND_MAX)*3.0f-1.5f); 
            ball_moving = true;
          }
        } else {
          should_quit = true;
        }
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
				should_quit = true;
			} else if (evt.type == SDL_QUIT) {
				should_quit = true;
				break;
			}
		}
		if (should_quit) break;

		auto current_time = std::chrono::high_resolution_clock::now();
		float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
		previous_time = current_time;

		{ //update game state:
      if (ball_moving) {
			  ball += elapsed * ball_velocity;
        
        //target collision
        float target_offset = ball.y - target.y;
        if (ball.x <= -0.94f && std::abs(target_offset) <= (target_size / 2.0f + 0.02f)) {
          if (++score == 99) {
            game_over = true;
          }
          ball = glm::vec2(0.0f, 0.0f);
          target_size = std::max(target_size*0.9, 0.05);
          target_y = (rand()/(float)RAND_MAX)*(2.0f - target_size) + target_size/2.0f - 1.0f;
          target = glm::vec2(-1.0f, target_y);
          ball_moving = false;
        }
 
        //paddle collision
        float paddle_offset = ball.y - paddle.y;
        if (ball.x >= 0.94f && std::abs(paddle_offset) <= (0.17f)) {
          ball_velocity.x = -ball_velocity.x;
          ball_velocity.y = 1.5f * paddle_offset / 0.15f;
        }

        //wall collision
        if (ball.x < -1.0f) {
          ball_velocity.x = std::abs(ball_velocity.x);
        }
			  if (ball.x > 1.0f) {
          if (--lives == 0) {
            game_over = true;
          } 
	        ball = glm::vec2(0.0f, 0.0f);
          ball_moving = false;
        }
        if (ball.y < -1.0f) {
          ball_velocity.y = std::abs(ball_velocity.y);
        }
        if (ball.y > 1.0f) {
          ball_velocity.y = -std::abs(ball_velocity.y);
        }

		  }
    }
		//draw output:
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		{ //draw game state:
			Draw draw;
			if (!game_over) {
        //draw objects
        draw.add_rectangle(paddle + glm::vec2(-0.04f,-0.15f), paddle + glm::vec2(0.0f, 0.15f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
			  draw.add_rectangle(ball + glm::vec2(-0.02f,-0.02f), ball + glm::vec2(0.02f, 0.02f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
        draw.add_rectangle(target + glm::vec2(0.0f, -target_size/2.0f), target + glm::vec2(0.04f, target_size/2.0f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
       
        { //draw lives
          switch (lives) {
            case 0:
              draw.add_rectangle(glm::vec2(0.95f, 0.96f), glm::vec2(0.96f, 0.98f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.95f, 0.93f), glm::vec2(0.96f, 0.95f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.96f, 0.92f), glm::vec2(0.98f, 0.93f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.98f, 0.93f), glm::vec2(0.99f, 0.95f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.98f, 0.96f), glm::vec2(0.99f, 0.98f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.96f, 0.98f), glm::vec2(0.98f, 0.99f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              break;
            case 1:
              draw.add_rectangle(glm::vec2(0.98f, 0.93f), glm::vec2(0.99f, 0.95f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.98f, 0.96f), glm::vec2(0.99f, 0.98f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              break;
            case 2:
              draw.add_rectangle(glm::vec2(0.95f, 0.93f), glm::vec2(0.96f, 0.95f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.96f, 0.92f), glm::vec2(0.98f, 0.93f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.98f, 0.96f), glm::vec2(0.99f, 0.98f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.96f, 0.98f), glm::vec2(0.98f, 0.99f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.96f, 0.95f), glm::vec2(0.98f, 0.96f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              break;
            case 3:
              draw.add_rectangle(glm::vec2(0.96f, 0.92f), glm::vec2(0.98f, 0.93f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.98f, 0.93f), glm::vec2(0.99f, 0.95f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.98f, 0.96f), glm::vec2(0.99f, 0.98f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.96f, 0.98f), glm::vec2(0.98f, 0.99f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              draw.add_rectangle(glm::vec2(0.96f, 0.95f), glm::vec2(0.98f, 0.96f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
              break;
          }
        }

        //draw score
        int units = score % 10;
        int tens = score / 10;

        { //draw tens
          switch (tens) {
            case 0:
              draw.add_rectangle(glm::vec2(-0.99f, 0.96f), glm::vec2(-0.98f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.99f, 0.93f), glm::vec2(-0.98f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.92f), glm::vec2(-0.96f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.93f), glm::vec2(-0.95f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.96f), glm::vec2(-0.95f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.98f), glm::vec2(-0.96f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 1:
              draw.add_rectangle(glm::vec2(-0.96f, 0.93f), glm::vec2(-0.95f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.96f), glm::vec2(-0.95f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 2:
              draw.add_rectangle(glm::vec2(-0.99f, 0.93f), glm::vec2(-0.98f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.92f), glm::vec2(-0.96f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.96f), glm::vec2(-0.95f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.98f), glm::vec2(-0.96f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.95f), glm::vec2(-0.96f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 3:
              draw.add_rectangle(glm::vec2(-0.98f, 0.92f), glm::vec2(-0.96f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.93f), glm::vec2(-0.95f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.96f), glm::vec2(-0.95f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.98f), glm::vec2(-0.96f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.95f), glm::vec2(-0.96f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 4:
              draw.add_rectangle(glm::vec2(-0.99f, 0.96f), glm::vec2(-0.98f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.93f), glm::vec2(-0.95f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.96f), glm::vec2(-0.95f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.95f), glm::vec2(-0.96f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 5:
              draw.add_rectangle(glm::vec2(-0.99f, 0.96f), glm::vec2(-0.98f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.92f), glm::vec2(-0.96f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.93f), glm::vec2(-0.95f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.98f), glm::vec2(-0.96f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.95f), glm::vec2(-0.96f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 6:
              draw.add_rectangle(glm::vec2(-0.99f, 0.96f), glm::vec2(-0.98f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.99f, 0.93f), glm::vec2(-0.98f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.92f), glm::vec2(-0.96f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.93f), glm::vec2(-0.95f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.98f), glm::vec2(-0.96f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.95f), glm::vec2(-0.96f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 7:
              draw.add_rectangle(glm::vec2(-0.96f, 0.93f), glm::vec2(-0.95f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.96f), glm::vec2(-0.95f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.98f), glm::vec2(-0.96f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 8:
              draw.add_rectangle(glm::vec2(-0.99f, 0.96f), glm::vec2(-0.98f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.99f, 0.93f), glm::vec2(-0.98f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.92f), glm::vec2(-0.96f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.93f), glm::vec2(-0.95f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.96f), glm::vec2(-0.95f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.98f), glm::vec2(-0.96f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.95f), glm::vec2(-0.96f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 9:
              draw.add_rectangle(glm::vec2(-0.99f, 0.96f), glm::vec2(-0.98f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.92f), glm::vec2(-0.96f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.93f), glm::vec2(-0.95f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.96f, 0.96f), glm::vec2(-0.95f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.98f), glm::vec2(-0.96f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.98f, 0.95f), glm::vec2(-0.96f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
          }
        }

        { //draw units
          switch (units) {
            case 0:
              draw.add_rectangle(glm::vec2(-0.94f, 0.96f), glm::vec2(-0.93f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.94f, 0.93f), glm::vec2(-0.93f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.92f), glm::vec2(-0.91f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.93f), glm::vec2(-0.90f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.96f), glm::vec2(-0.90f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.98f), glm::vec2(-0.91f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
            case 1:
              draw.add_rectangle(glm::vec2(-0.91f, 0.93f), glm::vec2(-0.90f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.96f), glm::vec2(-0.90f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
            case 2:
              draw.add_rectangle(glm::vec2(-0.94f, 0.93f), glm::vec2(-0.93f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.92f), glm::vec2(-0.91f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.96f), glm::vec2(-0.90f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.98f), glm::vec2(-0.91f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.95f), glm::vec2(-0.91f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
            case 3:
              draw.add_rectangle(glm::vec2(-0.93f, 0.92f), glm::vec2(-0.91f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.93f), glm::vec2(-0.90f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.96f), glm::vec2(-0.90f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.98f), glm::vec2(-0.91f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.95f), glm::vec2(-0.91f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
            case 4:
              draw.add_rectangle(glm::vec2(-0.94f, 0.96f), glm::vec2(-0.93f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.93f), glm::vec2(-0.90f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.96f), glm::vec2(-0.90f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.95f), glm::vec2(-0.91f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
            case 5:
              draw.add_rectangle(glm::vec2(-0.94f, 0.96f), glm::vec2(-0.93f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.92f), glm::vec2(-0.91f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.93f), glm::vec2(-0.90f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.98f), glm::vec2(-0.91f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.95f), glm::vec2(-0.91f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
            case 6:
              draw.add_rectangle(glm::vec2(-0.94f, 0.96f), glm::vec2(-0.93f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.94f, 0.93f), glm::vec2(-0.93f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.92f), glm::vec2(-0.91f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.93f), glm::vec2(-0.90f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.98f), glm::vec2(-0.91f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.95f), glm::vec2(-0.91f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
            case 7:
              draw.add_rectangle(glm::vec2(-0.91f, 0.93f), glm::vec2(-0.90f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.96f), glm::vec2(-0.90f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.98f), glm::vec2(-0.91f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
            case 8:
              draw.add_rectangle(glm::vec2(-0.94f, 0.96f), glm::vec2(-0.93f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.94f, 0.93f), glm::vec2(-0.93f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.92f), glm::vec2(-0.91f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.93f), glm::vec2(-0.90f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.96f), glm::vec2(-0.90f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.98f), glm::vec2(-0.91f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.95f), glm::vec2(-0.91f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
            case 9:
              draw.add_rectangle(glm::vec2(-0.94f, 0.96f), glm::vec2(-0.93f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.92f), glm::vec2(-0.91f, 0.93f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.93f), glm::vec2(-0.90f, 0.95f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.91f, 0.96f), glm::vec2(-0.90f, 0.98f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.98f), glm::vec2(-0.91f, 0.99f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.93f, 0.95f), glm::vec2(-0.91f, 0.96f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break; 
          }
        }
        
      } else {
        if (score == 99) {
          //W
          draw.add_rectangle(glm::vec2(-0.45f, 0.3f), glm::vec2(-0.35f, 0.7f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.55f, 0.2f), glm::vec2(-0.45f, 0.3f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.65f, 0.3f), glm::vec2(-0.55f, 0.5f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.75f, 0.2f), glm::vec2(-0.65f, 0.3f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.85f, 0.3f), glm::vec2(-0.75f, 0.7f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));

          //I
          draw.add_rectangle(glm::vec2(-0.25f, 0.6f), glm::vec2(0.25f, 0.7f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.05f, 0.3f), glm::vec2(0.05f, 0.6f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.25f, 0.2f), glm::vec2(0.25f, 0.3f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));

          //N
          draw.add_rectangle(glm::vec2(0.35f, 0.2f), glm::vec2(0.45f, 0.6f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.45f, 0.6f), glm::vec2(0.55f, 0.7f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.55f, 0.3f), glm::vec2(0.65f, 0.6f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.65f, 0.2f), glm::vec2(0.75f, 0.3f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.75f, 0.3f), glm::vec2(0.85f, 0.7f), glm::u8vec4(0x00, 0xff, 0x00, 0xff));

        } else {
          //L
          draw.add_rectangle(glm::vec2(-0.87f, 0.2f), glm::vec2(-0.55f, 0.28f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.95f, 0.28f), glm::vec2(-0.87f, 0.6f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));

          //O
          draw.add_rectangle(glm::vec2(-0.37f, 0.2f), glm::vec2(-0.12f, 0.28f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.37f, 0.52f), glm::vec2(-0.12f, 0.6f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.45f, 0.28f), glm::vec2(-0.37f, 0.52f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(-0.12f, 0.28f), glm::vec2(-0.05f, 0.52f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));

          //S
          draw.add_rectangle(glm::vec2(0.13f, 0.2f), glm::vec2(0.37f, 0.28f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.13f, 0.36f), glm::vec2(0.37f, 0.44f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.13f, 0.52f), glm::vec2(0.37f, 0.6f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.05f, 0.44f), glm::vec2(0.13f, 0.52f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.37f, 0.28f), glm::vec2(0.45f, 0.36f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));

          //S
          draw.add_rectangle(glm::vec2(0.63f, 0.2f), glm::vec2(0.87f, 0.28f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.63f, 0.36f), glm::vec2(0.87f, 0.44f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.63f, 0.52f), glm::vec2(0.87f, 0.6f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.55f, 0.44f), glm::vec2(0.63f, 0.52f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));
          draw.add_rectangle(glm::vec2(0.87f, 0.28f), glm::vec2(0.95f, 0.36f), glm::u8vec4(0xff, 0x00, 0x00, 0xff));

        } 
        
        //draw score
        int units = score % 10;
        int tens = score / 10;

        { //draw tens
          switch (tens) {
            case 0 :
              draw.add_rectangle(glm::vec2(-0.5f, -0.3f), glm::vec2(-0.4f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.5f, -0.6f), glm::vec2(-0.4f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.7f), glm::vec2(-0.2f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.6f), glm::vec2(-0.1f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.3f), glm::vec2(-0.1f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.1f), glm::vec2(-0.2f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 1 :
              draw.add_rectangle(glm::vec2(-0.2f, -0.6f), glm::vec2(-0.1f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.3f), glm::vec2(-0.1f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 2 :
              draw.add_rectangle(glm::vec2(-0.5f, -0.6f), glm::vec2(-0.4f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.7f), glm::vec2(-0.2f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.3f), glm::vec2(-0.1f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.1f), glm::vec2(-0.2f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.4f), glm::vec2(-0.2f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 3 :
              draw.add_rectangle(glm::vec2(-0.4f, -0.7f), glm::vec2(-0.2f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.6f), glm::vec2(-0.1f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.3f), glm::vec2(-0.1f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.1f), glm::vec2(-0.2f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.4f), glm::vec2(-0.2f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 4 :
              draw.add_rectangle(glm::vec2(-0.5f, -0.3f), glm::vec2(-0.4f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.6f), glm::vec2(-0.1f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.3f), glm::vec2(-0.1f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.4f), glm::vec2(-0.2f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 5 :
              draw.add_rectangle(glm::vec2(-0.5f, -0.3f), glm::vec2(-0.4f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.7f), glm::vec2(-0.2f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.6f), glm::vec2(-0.1f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.1f), glm::vec2(-0.2f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.4f), glm::vec2(-0.2f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 6 :
              draw.add_rectangle(glm::vec2(-0.5f, -0.3f), glm::vec2(-0.4f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.5f, -0.6f), glm::vec2(-0.4f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.7f), glm::vec2(-0.2f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.6f), glm::vec2(-0.1f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.1f), glm::vec2(-0.2f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.4f), glm::vec2(-0.2f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 7 :
              draw.add_rectangle(glm::vec2(-0.2f, -0.6f), glm::vec2(-0.1f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.3f), glm::vec2(-0.1f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.1f), glm::vec2(-0.2f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 8 :
              draw.add_rectangle(glm::vec2(-0.5f, -0.3f), glm::vec2(-0.4f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.5f, -0.6f), glm::vec2(-0.4f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.7f), glm::vec2(-0.2f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.6f), glm::vec2(-0.1f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.3f), glm::vec2(-0.1f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.1f), glm::vec2(-0.2f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.4f), glm::vec2(-0.2f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 9 :
              draw.add_rectangle(glm::vec2(-0.5f, -0.3f), glm::vec2(-0.4f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.7f), glm::vec2(-0.2f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.6f), glm::vec2(-0.1f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.2f, -0.3f), glm::vec2(-0.1f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.1f), glm::vec2(-0.2f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(-0.4f, -0.4f), glm::vec2(-0.2f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
          }
        }

        { //draw units
          switch (units) {
            case 0 :
              draw.add_rectangle(glm::vec2(0.1f, -0.3f), glm::vec2(0.2f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.1f, -0.6f), glm::vec2(0.2f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.7f), glm::vec2(0.4f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.6f), glm::vec2(0.5f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.3f), glm::vec2(0.5f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.1f), glm::vec2(0.4f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 1 :
              draw.add_rectangle(glm::vec2(0.4f, -0.6f), glm::vec2(0.5f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.3f), glm::vec2(0.5f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 2 :
              draw.add_rectangle(glm::vec2(0.1f, -0.6f), glm::vec2(0.2f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.7f), glm::vec2(0.4f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.3f), glm::vec2(0.5f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.1f), glm::vec2(0.4f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.4f), glm::vec2(0.4f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 3 :
              draw.add_rectangle(glm::vec2(0.2f, -0.7f), glm::vec2(0.4f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.6f), glm::vec2(0.5f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.3f), glm::vec2(0.5f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.1f), glm::vec2(0.4f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.4f), glm::vec2(0.4f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 4 :
              draw.add_rectangle(glm::vec2(0.1f, -0.3f), glm::vec2(0.2f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.6f), glm::vec2(0.5f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.3f), glm::vec2(0.5f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.4f), glm::vec2(0.4f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 5 :
              draw.add_rectangle(glm::vec2(0.1f, -0.3f), glm::vec2(0.2f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.7f), glm::vec2(0.4f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.6f), glm::vec2(0.5f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.1f), glm::vec2(0.4f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.4f), glm::vec2(0.4f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 6 :
              draw.add_rectangle(glm::vec2(0.1f, -0.3f), glm::vec2(0.2f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.1f, -0.6f), glm::vec2(0.2f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.7f), glm::vec2(0.4f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.6f), glm::vec2(0.5f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.1f), glm::vec2(0.4f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.4f), glm::vec2(0.4f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 7 :
              draw.add_rectangle(glm::vec2(0.4f, -0.6f), glm::vec2(0.5f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.3f), glm::vec2(0.5f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.1f), glm::vec2(0.4f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 8 :
              draw.add_rectangle(glm::vec2(0.1f, -0.3f), glm::vec2(0.2f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.1f, -0.6f), glm::vec2(0.2f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.7f), glm::vec2(0.4f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.6f), glm::vec2(0.5f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.3f), glm::vec2(0.5f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.1f), glm::vec2(0.4f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.4f), glm::vec2(0.4f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
            case 9 :
              draw.add_rectangle(glm::vec2(0.1f, -0.3f), glm::vec2(0.2f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.7f), glm::vec2(0.4f, -0.6f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.6f), glm::vec2(0.5f, -0.4f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.4f, -0.3f), glm::vec2(0.5f, -0.1f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.1f), glm::vec2(0.4f, 0.0f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              draw.add_rectangle(glm::vec2(0.2f, -0.4f), glm::vec2(0.4f, -0.3f), glm::u8vec4(0x00, 0x00, 0xff, 0xff));
              break;
          }
        }

      }
			
      draw.draw();
		}

		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------

	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	return 0;
}
