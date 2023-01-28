#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include <imfilebrowser.h>

#include <soloud.h>
#include <soloud_echofilter.h>
#include <soloud_freeverbfilter.h>
#include <soloud_lofifilter.h>
#include <soloud_sfxr.h>
#include <soloud_speech.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

#include <random>

#include "filesystemfile.hpp"
#include "joystickevent.hpp"

struct BackgroundMusic
{
    SoLoud::WavStream stream;
    SoLoud::handle handle = 0;
    FileSystemFile file;
    float volume = 1.0f;
};

struct SoundEffect
{
    SoLoud::Wav wav;
    FileSystemFile file;
};

struct Speech
{
    static constexpr size_t text_size = 1000;
    static constexpr unsigned int min_freq = 0;
    static constexpr unsigned int max_freq = 3000;

    SoLoud::Speech speech;
    SoLoud::handle handle = 0;
    float volume = 1.0f;
    float pan = 0.0f;
    char text[text_size]{};
    unsigned int base_frequency = 1330;
    float base_speed = 10;
    float base_declination = 0.5;
    KLATT_WAVEFORM base_waveform = KW_SAW;
};

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_CreateWindowAndRenderer(1024, 768, SDL_WINDOW_RESIZABLE, &window, &renderer);
    SDL_SetWindowTitle(window, "SoLoud-SDL Demo");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    std::random_device rd;
    std::uniform_int_distribution rand(INT_MIN, INT_MAX);

    SoLoud::Soloud soloud;
    soloud.init();
    soloud.setVisualizationEnable(true);

    BackgroundMusic bgm;

    ImGui::FileBrowser bgm_file_browser;
    bgm_file_browser.SetTitle("Open BGM");
    bgm_file_browser.SetTypeFilters({".ogg", ".wav", ".mp3", ".flac"});

    SoundEffect sfx;
    ImGui::FileBrowser sfx_file_browser;
    sfx_file_browser.SetTitle("Open SFX");
    sfx_file_browser.SetTypeFilters({".ogg", ".wav", ".mp3", ".flac"});

    Speech speech;

    constexpr size_t sfxr_count = 8;
    SoLoud::Sfxr sfxrs[sfxr_count] = {};
    SoLoud::Sfxr::SFXR_PRESETS sfxr_presets[sfxr_count] = {SoLoud::Sfxr::COIN};
    int sfxr_seeds[sfxr_count] = {rand(rd), rand(rd), rand(rd), rand(rd), rand(rd), rand(rd), rand(rd), rand(rd)};
    SoLoud::LofiFilter sfxr_lofi;
    sfxr_lofi.setParams(8000, 4);
    bool sfxr_lofi_enables[sfxr_count] = {false, false, false, false, false, false, false, false};

    SoLoud::Bus sfxr_bus;
    auto sfxr_bus_handle = soloud.play(sfxr_bus);
    float sfxr_bus_volume = 1.0f;

    SoLoud::FreeverbFilter sfxr_bus_freeverb;
    sfxr_bus_freeverb.setParams(0, 0.5f, 0.5f, 1);
    bool sfxr_bus_freeverb_enabled = false;

    SoLoud::EchoFilter sfxr_bus_echo;
    sfxr_bus_echo.setParams(0.300f);
    bool sfxr_bus_echo_enabled = false;

    float sfxr_x_pos = 0.0f, sfxr_y_pos = 0.0f, sfxr_z_pos = 0.0f;

    bool use_controller = false;
    SDL_GameController *controller = nullptr;
    if (SDL_NumJoysticks() >= 1)
    {
        controller = SDL_GameControllerOpen(0);
    }

    while (true)
    {
        JoyStickEvent joystick;

        bool quit = false;

        while (true)
        {
            SDL_JoystickUpdate();
            SDL_Event event;
            if (!SDL_PollEvent(&event))
                break;

            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (event.type == SDL_CONTROLLERBUTTONUP)
            {
                joystick.update_button(event);
            }
        }
        if (quit)
            break;

        if (use_controller)
        {
            sfxr_x_pos = static_cast<float>(joystick.x_axis) / static_cast<float>(SDL_MAX_SINT16);
            sfxr_y_pos = static_cast<float>(joystick.y_axis) / static_cast<float>(SDL_MAX_SINT16);
            sfxr_z_pos = static_cast<float>(joystick.z_axis) / static_cast<float>(SDL_MAX_SINT16);

            if (joystick.is_a_pressed)
            {
                sfxrs[0].loadPreset(sfxr_presets[0], sfxr_seeds[0]);
                sfxrs[0].setFilter(0, sfxr_lofi_enables[0] ? &sfxr_lofi : nullptr);
                sfxr_bus.play3d(sfxrs[0], sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
            }

            if (joystick.is_b_pressed)
            {
                sfxrs[1].loadPreset(sfxr_presets[1], sfxr_seeds[1]);
                sfxrs[1].setFilter(0, sfxr_lofi_enables[1] ? &sfxr_lofi : nullptr);
                sfxr_bus.play3d(sfxrs[1], sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
            }

            if (joystick.is_x_pressed)
            {
                sfxrs[2].loadPreset(sfxr_presets[2], sfxr_seeds[2]);
                sfxrs[2].setFilter(0, sfxr_lofi_enables[2] ? &sfxr_lofi : nullptr);
                sfxr_bus.play3d(sfxrs[2], sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
            }

            if (joystick.is_y_pressed)
            {
                sfxrs[3].loadPreset(sfxr_presets[3], sfxr_seeds[3]);
                sfxrs[3].setFilter(0, sfxr_lofi_enables[3] ? &sfxr_lofi : nullptr);
                sfxr_bus.play3d(sfxrs[3], sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
            }

            if (joystick.is_u_pressed)
            {
                sfxrs[4].loadPreset(sfxr_presets[4], sfxr_seeds[4]);
                sfxrs[4].setFilter(0, sfxr_lofi_enables[4] ? &sfxr_lofi : nullptr);
                sfxr_bus.play3d(sfxrs[4], sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
            }

            if (joystick.is_d_pressed)
            {
                sfxrs[5].loadPreset(sfxr_presets[5], sfxr_seeds[5]);
                sfxrs[5].setFilter(0, sfxr_lofi_enables[5] ? &sfxr_lofi : nullptr);
                sfxr_bus.play3d(sfxrs[5], sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
            }

            if (joystick.is_l_pressed)
            {
                sfxrs[6].loadPreset(sfxr_presets[6], sfxr_seeds[6]);
                sfxrs[6].setFilter(0, sfxr_lofi_enables[6] ? &sfxr_lofi : nullptr);
                sfxr_bus.play3d(sfxrs[6], sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
            }

            if (joystick.is_r_pressed)
            {
                sfxrs[7].loadPreset(sfxr_presets[7], sfxr_seeds[7]);
                sfxrs[7].setFilter(0, sfxr_lofi_enables[7] ? &sfxr_lofi : nullptr);
                sfxr_bus.play3d(sfxrs[7], sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
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
                    bgm_file_browser.Open();
                }
                ImGui::SameLine();
                if (ImGui::Button("Play"))
                {
                    soloud.stop(bgm.handle);
                    bgm.handle = soloud.play(bgm.stream);
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop"))
                {
                    soloud.stop(bgm.handle);
                }

                ImGui::LabelText("Filename", "%s", bgm.file.path().u8string().c_str());

                ImGui::SliderFloat("Volume", &bgm.volume, 0.0f, 1.0f);
                soloud.setVolume(bgm.handle, bgm.volume);
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
                    soloud.play3d(sfx.wav, sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
                }
                ImGui::LabelText("Filename", "%s", sfx.file.path().u8string().c_str());
            }
            ImGui::End();

            ImGui::Begin("Speech");
            {
                auto &[speech_instance, handle, volume, pan, text, base_frequency, base_speed, base_declination,
                       base_waveform] = speech;

                if (ImGui::Button("Play"))
                {
                    handle = soloud.play(speech_instance, volume, pan);
                    speech_instance.setParams(base_frequency, base_speed, base_declination);
                }

                if (ImGui::InputTextMultiline("Text", speech.text, Speech::text_size))
                {
                    speech_instance.setText(text);
                }

                if (ImGui::SliderFloat("Volume", &speech.volume, 0.0f, 1.0f))
                {
                    soloud.setVolume(handle, volume);
                }

                ImGui::SliderFloat("Panning", &pan, -1.0f, 1.0f);
                if (ImGui::CollapsingHeader("Base params"))
                {
                    ImGui::SliderScalar("Base freq", ImGuiDataType_U32, &base_frequency, &Speech::min_freq,
                                        &Speech::max_freq);
                    ImGui::SliderFloat("Base speed", &base_speed, 0.1f, 30);
                    ImGui::SliderFloat("Base declination", &base_declination, -3, 3);
                }
                if (ImGui::CollapsingHeader("Waveform"))
                {
                    if (ImGui::RadioButton("Sin", base_waveform == KW_SIN))
                    {
                        base_waveform = KW_SIN;
                    }
                    if (ImGui::RadioButton("Triangle", base_waveform == KW_TRIANGLE))
                    {
                        base_waveform = KW_TRIANGLE;
                    }
                    if (ImGui::RadioButton("Saw", base_waveform == KW_SAW))
                    {
                        base_waveform = KW_SAW;
                    }
                    if (ImGui::RadioButton("Square", base_waveform == KW_SQUARE))
                    {
                        base_waveform = KW_SQUARE;
                    }
                    if (ImGui::RadioButton("Pulse", base_waveform == KW_PULSE))
                    {
                        base_waveform = KW_PULSE;
                    }
                    if (ImGui::RadioButton("Warble", base_waveform == KW_WARBLE))
                    {
                        base_waveform = KW_WARBLE;
                    }
                    if (ImGui::RadioButton("Noise", base_waveform == KW_NOISE))
                    {
                        base_waveform = KW_NOISE;
                    }
                }
            }
            ImGui::End();

            ImGui::Begin("Sfxr");
            {
                if (ImGui::SliderFloat("Volume", &sfxr_bus_volume, 0.0f, 1.0f))
                {
                    soloud.setVolume(sfxr_bus_handle, sfxr_bus_volume);
                }

                if (ImGui::Checkbox("Echo", &sfxr_bus_echo_enabled))
                {
                    sfxr_bus.setFilter(0, sfxr_bus_echo_enabled ? &sfxr_bus_echo : nullptr);
                }

                if (ImGui::Checkbox("Reverb", &sfxr_bus_freeverb_enabled))
                {
                    sfxr_bus.setFilter(1, sfxr_bus_freeverb_enabled ? &sfxr_bus_freeverb : nullptr);
                }

                ImGui::SliderFloat("X", &sfxr_x_pos, -1.0f, 1.0f);
                ImGui::SliderFloat("Y", &sfxr_y_pos, -1.0f, 1.0f);
                ImGui::SliderFloat("Z", &sfxr_z_pos, -1.0f, 1.0f);

                constexpr const char sfxr_header_texts[sfxr_count][34]{
                    "SFXR #1 - Controller A Button",    "SFXR #2 - Controller B Button",
                    "SFXR #3 - Controller X Button",    "SFXR #4 - Controller Y Button",
                    "SFXR #5 - Controller UP Button",   "SFXR #6 - Controller Down Button",
                    "SFXR #7 - Controller Left Button", "SFXR #8 - Controller Right Button"};

                if (controller)
                {
                    ImGui::Checkbox("Use Controller", &use_controller);
                }
                for (size_t i = 0; i < sfxr_count; i++)
                {
                    if (ImGui::CollapsingHeader(sfxr_header_texts[i]))
                    {
                        if (ImGui::Button(std::format("Play ##{}", i).c_str()))
                        {
                            sfxrs[i].loadPreset(sfxr_presets[i], sfxr_seeds[i]);
                            sfxrs[i].setFilter(0, sfxr_lofi_enables[i] ? &sfxr_lofi : nullptr);
                            sfxr_bus.play3d(sfxrs[i], sfxr_x_pos, sfxr_y_pos, sfxr_z_pos);
                        }

                        if (ImGui::CollapsingHeader(std::format("Preset ##{}", i).c_str()))
                        {
                            if (ImGui::RadioButton("Coin", sfxr_presets[i] == SoLoud::Sfxr::COIN))
                            {
                                sfxr_presets[i] = SoLoud::Sfxr::COIN;
                            }

                            if (ImGui::RadioButton("Laser", sfxr_presets[i] == SoLoud::Sfxr::LASER))
                            {
                                sfxr_presets[i] = SoLoud::Sfxr::LASER;
                            }

                            if (ImGui::RadioButton("Explosion", sfxr_presets[i] == SoLoud::Sfxr::EXPLOSION))
                            {
                                sfxr_presets[i] = SoLoud::Sfxr::EXPLOSION;
                            }

                            if (ImGui::RadioButton("Power Up", sfxr_presets[i] == SoLoud::Sfxr::POWERUP))
                            {
                                sfxr_presets[i] = SoLoud::Sfxr::POWERUP;
                            }

                            if (ImGui::RadioButton("Hurt", sfxr_presets[i] == SoLoud::Sfxr::HURT))
                            {
                                sfxr_presets[i] = SoLoud::Sfxr::HURT;
                            }

                            if (ImGui::RadioButton("Jump", sfxr_presets[i] == SoLoud::Sfxr::JUMP))
                            {
                                sfxr_presets[i] = SoLoud::Sfxr::JUMP;
                            }

                            if (ImGui::RadioButton("Blip", sfxr_presets[i] == SoLoud::Sfxr::BLIP))
                            {
                                sfxr_presets[i] = SoLoud::Sfxr::BLIP;
                            }
                        }

                        ImGui::InputInt(std::format("Seed Number ##{}", i).c_str(), &sfxr_seeds[i]);
                        if (ImGui::Button(std::format("Random ##{}", i).c_str()))
                        {
                            sfxr_seeds[i] = rand(rd);
                        }

                        ImGui::Checkbox(std::format("Lo-Fi ##{}", i).c_str(), &sfxr_lofi_enables[i]);
                    }
                }
            }
            ImGui::End();

            ImGui::Begin("SoLoud");
            {
                ImGui::PlotLines("Visualization", soloud.getWave(), 256);
            }
            ImGui::End();
            bgm_file_browser.Display();
            if (bgm_file_browser.HasSelected())
            {
                soloud.stopAudioSource(bgm.stream);

                auto path = bgm_file_browser.GetSelected();
                bgm.file.open(path);

                bgm.stream.loadFile(&bgm.file);
                bgm_file_browser.ClearSelected();
            }

            sfx_file_browser.Display();
            if (sfx_file_browser.HasSelected())
            {
                auto path = sfx_file_browser.GetSelected();
                sfx.file.open(path);
                sfx.wav.loadFile(&sfx.file);
                sfx_file_browser.ClearSelected();
            }
        }
        ImGui::EndFrame();

        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);


        SDL_Delay(0);
    }

    bgm.file.close();
    sfx.file.close();

    soloud.stopAll();
    soloud.deinit();

    ImGui::DestroyContext();

    SDL_GameControllerClose(controller);
    SDL_Quit();

    return 0;
}
