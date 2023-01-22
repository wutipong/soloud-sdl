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
    SoLoud::WavStream wav_stream;

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
            ImGui::Begin("BGM");
            {
                ImGui::NewLine();

                ImGui::SameLine();
                if (ImGui::Button("Open"))
                {
                    file_browser.Open();
                }
                ImGui::SameLine();
                if (ImGui::Button("Play"))
                {
                    soloud.play(wav_stream);
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop"))
                {
                    soloud.stopAudioSource(wav_stream);
                }

                ImGui::LabelText("Filename", "%s", wav_stream.mFilename);
                
            }
            ImGui::End();


            file_browser.Display();
            if (file_browser.HasSelected())
            {
                auto path = file_browser.GetSelected();

                soloud.stopAudioSource(wav_stream);

                wav_stream.load(path.string().c_str());
                file_browser.ClearSelected();
            }

        }
        ImGui::EndFrame();

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
