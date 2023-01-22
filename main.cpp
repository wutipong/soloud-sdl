#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_CreateWindowAndRenderer(1024, 768, SDL_WINDOW_RESIZABLE, &window, &renderer);
    SDL_SetWindowTitle(window, "Soloud-SDL test application.");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    while (true)
    {
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {

            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
                break;
            }
        }

        SDL_RenderClear(renderer);
        ImGui_ImplSDL2_NewFrame(window);
        ImGui_ImplSDLRenderer_NewFrame();

        ImGui::NewFrame();

        {
            int width, height;
            SDL_GetWindowSize(window, &width, &height);

            ImGui::SetNextWindowPos({0, 0});
            ImGui::SetNextWindowSize({static_cast<float>(width), static_cast<float>(height)});

            ImGui::Begin("main window", nullptr,
                         ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoTitleBar);
            {
                if(ImGui::BeginMenuBar())
                {
                    if(ImGui::BeginMenu("File"))
                    {
                        ImGui::MenuItem("Open");
                        ImGui::Separator();
                        if (ImGui::MenuItem("Exit", "Alt+F4"))
                        {
                            SDL_Event quit_event = {.type = SDL_QUIT};
                            SDL_PushEvent(&quit_event);
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }
                ImGui::End();
            }
            ImGui::EndFrame();
        }
        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
        SDL_Delay(0);
    }

    SDL_Quit();

    return 0;
}
