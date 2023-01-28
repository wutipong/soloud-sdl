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
    SoLoud::WavStream source;
    SoLoud::handle handle = 0;
    FileSystemFile file;
    float volume = 1.0f;
};

struct SoundEffect
{
    SoLoud::Wav source;
    FileSystemFile file;
};

struct Speech
{
    static constexpr size_t text_size = 1000;
    static constexpr unsigned int min_freq = 0;
    static constexpr unsigned int max_freq = 3000;

    SoLoud::Speech source;
    SoLoud::handle handle = 0;
    float volume = 1.0f;
    float pan = 0.0f;
    char text[text_size]{};
    unsigned int base_frequency = 1330;
    float base_speed = 10;
    float base_declination = 0.5;
    KLATT_WAVEFORM base_waveform = KW_SAW;
};

struct SFXRSoundEffect
{
    static constexpr size_t count = 8;

    SoLoud::Sfxr sources[count] = {};
    SoLoud::Sfxr::SFXR_PRESETS presets[count] = {SoLoud::Sfxr::COIN};
    int seeds[count] = {};

    SoLoud::LofiFilter lofi_filter;
    bool lofi_enableds[count] = {false, false, false, false, false, false, false, false};

    SoLoud::Bus bus;
    float bus_volume = 1.0f;
    SoLoud::handle bus_handle = 0;

    SoLoud::FreeverbFilter bus_freeverb_filter;
    bool bus_freeverb_enabled = false;

    SoLoud::EchoFilter bus_echo_filter;
    bool bus_echo_enabled = false;

    float position_x = 0.0f, position_y = 0.0f, position_z = 0.0f;

private:
    std::random_device rd_;
    std::uniform_int_distribution<int> rand_{INT_MIN, INT_MAX};

public:
    SFXRSoundEffect()
    {
        for (auto &seed : seeds)
        {
            seed = rand_(rd_);
        }
        lofi_filter.setParams(8000, 4);
        bus_freeverb_filter.setParams(0, 0.5f, 0.5f, 1);
        bus_echo_filter.setParams(0.300f);
    }

    SFXRSoundEffect(const SFXRSoundEffect &) = delete;
    SFXRSoundEffect &operator=(const SFXRSoundEffect &) = delete;

    void random_seed(const int &index) { seeds[index] = rand_(rd_); }
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

    SFXRSoundEffect sfxr;
    sfxr.bus_handle = soloud.play(sfxr.bus);

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
            sfxr.position_x = static_cast<float>(joystick.x_axis) / static_cast<float>(SDL_MAX_SINT16);
            sfxr.position_y = static_cast<float>(joystick.y_axis) / static_cast<float>(SDL_MAX_SINT16);
            sfxr.position_z = static_cast<float>(joystick.z_axis) / static_cast<float>(SDL_MAX_SINT16);

            int index = SFXRSoundEffect::count;

            if (joystick.is_a_pressed)
            {
                index = 0;
            }

            else if (joystick.is_b_pressed)
            {
                index = 1;
            }

            else if (joystick.is_x_pressed)
            {
                index = 2;
            }

            else if (joystick.is_y_pressed)
            {
                index = 3;
            }

            else if (joystick.is_u_pressed)
            {
                index = 4;
            }

            else if (joystick.is_d_pressed)
            {
                index = 5;
            }

            else if (joystick.is_l_pressed)
            {
                index = 6;
            }

            else if (joystick.is_r_pressed)
            {
                index = 7;
            }

            if (index != SFXRSoundEffect::count)
            {
                sfxr.sources[index].loadPreset(sfxr.presets[index], sfxr.seeds[index]);
                sfxr.sources[index].setFilter(0, sfxr.lofi_enableds[index] ? &sfxr.lofi_filter : nullptr);
                sfxr.bus.play3d(sfxr.sources[index], sfxr.position_x, sfxr.position_y, sfxr.position_z);
            }
        }

        SDL_RenderClear(renderer);
        ImGui_ImplSDL2_NewFrame(window);
        ImGui_ImplSDLRenderer_NewFrame();

        ImGui::NewFrame();
        {
            ImGui::Begin("BGM");
            {
                ImGui::SameLine();
                if (ImGui::Button("Open"))
                {
                    bgm_file_browser.Open();
                }
                ImGui::SameLine();
                if (ImGui::Button("Play"))
                {
                    soloud.stop(bgm.handle);
                    bgm.handle = soloud.play(bgm.source);
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
                    soloud.play3d(sfx.source, sfxr.position_x, sfxr.position_y, sfxr.position_z);
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
                if (ImGui::SliderFloat("Volume", &sfxr.bus_volume, 0.0f, 1.0f))
                {
                    soloud.setVolume(sfxr.bus_handle, sfxr.bus_volume);
                }

                if (ImGui::Checkbox("Echo", &sfxr.bus_echo_enabled))
                {
                    sfxr.bus.setFilter(0, sfxr.bus_echo_enabled ? &sfxr.bus_echo_filter : nullptr);
                }

                if (ImGui::Checkbox("Reverb", &sfxr.bus_freeverb_enabled))
                {
                    sfxr.bus.setFilter(1, sfxr.bus_freeverb_enabled ? &sfxr.bus_freeverb_filter : nullptr);
                }

                ImGui::Text("Position");

                ImGui::SliderFloat("X", &sfxr.position_x, -1.0f, 1.0f);
                ImGui::SliderFloat("Y", &sfxr.position_y, -1.0f, 1.0f);
                ImGui::SliderFloat("Z", &sfxr.position_z, -1.0f, 1.0f);

                constexpr char sfxr_header_texts[SFXRSoundEffect::count][34]{
                    "SFXR #1 - Controller A Button",    "SFXR #2 - Controller B Button",
                    "SFXR #3 - Controller X Button",    "SFXR #4 - Controller Y Button",
                    "SFXR #5 - Controller UP Button",   "SFXR #6 - Controller Down Button",
                    "SFXR #7 - Controller Left Button", "SFXR #8 - Controller Right Button"};

                if (controller)
                {
                    ImGui::Checkbox("Use Controller", &use_controller);
                }
                for (size_t i = 0; i < SFXRSoundEffect::count; i++)
                {
                    if (ImGui::CollapsingHeader(sfxr_header_texts[i]))
                    {
                        if (ImGui::Button(std::format("Play ##{}", i).c_str()))
                        {
                            sfxr.sources[i].loadPreset(sfxr.presets[i], sfxr.seeds[i]);
                            sfxr.sources[i].setFilter(0, sfxr.lofi_enableds[i] ? &sfxr.lofi_filter : nullptr);
                            sfxr.bus.play3d(sfxr.sources[i], sfxr.position_x, sfxr.position_y, sfxr.position_z);
                        }

                        if (ImGui::CollapsingHeader(std::format("Preset ##{}", i).c_str()))
                        {
                            if (ImGui::RadioButton("Coin", sfxr.presets[i] == SoLoud::Sfxr::COIN))
                            {
                                sfxr.presets[i] = SoLoud::Sfxr::COIN;
                            }

                            if (ImGui::RadioButton("Laser", sfxr.presets[i] == SoLoud::Sfxr::LASER))
                            {
                                sfxr.presets[i] = SoLoud::Sfxr::LASER;
                            }

                            if (ImGui::RadioButton("Explosion", sfxr.presets[i] == SoLoud::Sfxr::EXPLOSION))
                            {
                                sfxr.presets[i] = SoLoud::Sfxr::EXPLOSION;
                            }

                            if (ImGui::RadioButton("Power Up", sfxr.presets[i] == SoLoud::Sfxr::POWERUP))
                            {
                                sfxr.presets[i] = SoLoud::Sfxr::POWERUP;
                            }

                            if (ImGui::RadioButton("Hurt", sfxr.presets[i] == SoLoud::Sfxr::HURT))
                            {
                                sfxr.presets[i] = SoLoud::Sfxr::HURT;
                            }

                            if (ImGui::RadioButton("Jump", sfxr.presets[i] == SoLoud::Sfxr::JUMP))
                            {
                                sfxr.presets[i] = SoLoud::Sfxr::JUMP;
                            }

                            if (ImGui::RadioButton("Blip", sfxr.presets[i] == SoLoud::Sfxr::BLIP))
                            {
                                sfxr.presets[i] = SoLoud::Sfxr::BLIP;
                            }
                        }

                        ImGui::InputInt(std::format("Seed Number ##{}", i).c_str(), &sfxr.seeds[i]);
                        if (ImGui::Button(std::format("Random ##{}", i).c_str()))
                        {
                            sfxr.random_seed(i);
                        }

                        ImGui::Checkbox(std::format("Lo-Fi ##{}", i).c_str(), &sfxr.lofi_enableds[i]);
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
                soloud.stopAudioSource(bgm.source);

                auto path = bgm_file_browser.GetSelected();
                bgm.file.open(path);

                bgm.source.loadFile(&bgm.file);
                bgm_file_browser.ClearSelected();
            }

            sfx_file_browser.Display();
            if (sfx_file_browser.HasSelected())
            {
                auto path = sfx_file_browser.GetSelected();
                sfx.file.open(path);
                sfx.source.loadFile(&sfx.file);
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
