#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include <imfilebrowser.h>

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_CreateWindowAndRenderer(1024, 768, SDL_WINDOW_RESIZABLE, &window, &renderer);
    SDL_SetWindowTitle(window, "SoLoud-SDL test application.");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    SoLoud::Soloud soloud;
    SoLoud::WavStream bgm_stream;
    SoLoud::Wav sfx_wav;
    SoLoud::handle bgm_handle = 0;
    float bgm_volume = 1.0f;
    float sfx_x_pos = 0.0f, sfx_y_pos = 0.0f, sfx_z_pos = 0.0f;

    soloud.init();

    ImGui::FileBrowser bgm_file_browser;
    bgm_file_browser.SetTitle("Open BGM");
    bgm_file_browser.SetTypeFilters({".ogg", ".wav", "mp3", ".flac"});

    ImGui::FileBrowser sfx_file_browser;
    sfx_file_browser.SetTitle("Open BGM");
    sfx_file_browser.SetTypeFilters({".ogg", ".wav", "mp3", ".flac"});

    SDL_GameController *controller = nullptr;
    if (SDL_NumJoysticks() >= 1)
    {
        controller = SDL_GameControllerOpen(0);
    }

    bool is_a_pressed = false;
    bool is_using_controller = true;

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
            if (event.type == SDL_CONTROLLERBUTTONUP)
            {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A)
                {
                    is_a_pressed = true;
                }
            }
        }
        SDL_JoystickUpdate();

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
                    bgm_file_browser.Open();
                }
                ImGui::SameLine();
                if (ImGui::Button("Play"))
                {
                    bgm_handle = soloud.play(bgm_stream);
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop"))
                {
                    soloud.stop(bgm_handle);
                }

                ImGui::LabelText("Filename", "%s", bgm_stream.mFilename);

                ImGui::SliderFloat("Volume", &bgm_volume, 0.0f, 1.0f);
                soloud.setVolume(bgm_handle, bgm_volume);
            }
            ImGui::End();

            ImGui::Begin("SFX");
            {
                ImGui::NewLine();

                ImGui::SameLine();
                if (ImGui::Button("Open"))
                {
                    sfx_file_browser.Open();
                }
                ImGui::SameLine();
                if (ImGui::Button("Play"))
                {
                    soloud.play(sfx_wav);
                }

                if (controller)
                {
                    ImGui::Checkbox("Use Controller", &is_using_controller);
                    if (is_using_controller)
                    {
                        auto left_x_value = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
                        auto left_y_value = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
                        auto trigger_value = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) -
                            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

                        sfx_x_pos = static_cast<float>(left_x_value) / static_cast<float>(SDL_MAX_SINT16);
                        sfx_y_pos = static_cast<float>(left_y_value) / static_cast<float>(SDL_MAX_SINT16);
                        sfx_z_pos = static_cast<float>(trigger_value) / static_cast<float>(SDL_MAX_SINT16);

                        if (is_a_pressed)
                        {
                            soloud.play3d(sfx_wav, sfx_x_pos, sfx_y_pos, sfx_z_pos);

                            is_a_pressed = false;
                        }
                    }
                }

                ImGui::SliderFloat("X", &sfx_x_pos, -1.0f, 1.0f);
                ImGui::SliderFloat("Y", &sfx_y_pos, -1.0f, 1.0f);
                ImGui::SliderFloat("Z", &sfx_z_pos, -1.0f, 1.0f);
            }
            ImGui::End();


            bgm_file_browser.Display();
            if (bgm_file_browser.HasSelected())
            {
                soloud.stopAudioSource(bgm_stream);

                auto path = bgm_file_browser.GetSelected();
                bgm_stream.load(path.string().c_str());
                bgm_file_browser.ClearSelected();
            }

            sfx_file_browser.Display();
            if (sfx_file_browser.HasSelected())
            {
                auto path = sfx_file_browser.GetSelected();

                sfx_wav.load(path.string().c_str());
                sfx_file_browser.ClearSelected();
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

    SDL_GameControllerClose(controller);
    SDL_Quit();

    return 0;
}
