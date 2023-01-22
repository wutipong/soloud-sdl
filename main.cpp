#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include <imfilebrowser.h>

#include <soloud.h>
#include <soloud_wavstream.h>

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

    SoLoud::Soloud soloud;
    SoLoud::WavStream wavs_stream;
    SoLoud::handle handle{};
    bool wav_valid = false;

    soloud.init();

    ImGui::FileBrowser file_browser;
    file_browser.SetTitle("Open");
    file_browser.SetTypeFilters({".ogg", ".wav", "mp3", ".flac"});

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
                        if(ImGui::MenuItem("Open"))
                        {
                            file_browser.Open();
                        }
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

                file_browser.Display();
                if (file_browser.HasSelected())
                {
                    auto path = file_browser.GetSelected();
                    if (wav_valid)
                    {
                        soloud.stopAudioSource(wavs_stream);
                    }
                    auto result = wavs_stream.load(path.string().c_str());
                    wav_valid = true;
                    handle = soloud.play(wavs_stream);
                    file_browser.ClearSelected();
                }
            }
            ImGui::EndFrame();
        }
        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
        SDL_Delay(0);
    }
    soloud.stopAll();
    soloud.deinit();
    SDL_Quit();

    return 0;
}
