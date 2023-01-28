#include <SDL.h>

struct JoyStickEvent
{
    bool is_a_pressed = false;
    bool is_b_pressed = false;
    bool is_x_pressed = false;
    bool is_y_pressed = false;
    bool is_u_pressed = false;
    bool is_d_pressed = false;
    bool is_l_pressed = false;
    bool is_r_pressed = false;

    Sint16 x_axis = 0;
    Sint16 y_axis = 0;
    Sint16 z_axis = 0;

    void update_button(const SDL_Event &event)
    {
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A)
        {
            is_a_pressed = true;
        }

        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B)
        {
            is_b_pressed = true;
        }

        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X)
        {
            is_x_pressed = true;
        }

        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_Y)
        {
            is_y_pressed = true;
        }

        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
        {
            is_u_pressed = true;
        }

        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
        {
            is_d_pressed = true;
        }

        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
        {
            is_l_pressed = true;
        }

        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
        {
            is_r_pressed = true;
        }
    }

    void update_axis(SDL_GameController *&controller)
    {
        x_axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        y_axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        z_axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) -
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    }
};