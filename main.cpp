#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include <imfilebrowser.h>

#include <soloud.h>
#include <soloud_file.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
#include <soloud_sfxr.h>
#include <soloud_speech.h>

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <soloud_lofifilter.h>

class FileSystemFile : public SoLoud::File
{
public:
    FileSystemFile() = default;
    FileSystemFile(const FileSystemFile &) = delete;
    FileSystemFile &operator=(const FileSystemFile &) = delete;

    void open(std::filesystem::path path);
    void close();

    std::filesystem::path &path() { return path_; }

    int eof() override;
    unsigned int read(unsigned char *aDst, unsigned int aBytes) override;
    unsigned int length() override;
    void seek(int aOffset) override;
    unsigned int pos() override;

private:
    std::basic_ifstream<unsigned char> stream_;
    std::filesystem::path path_;
};

void FileSystemFile::open(const std::filesystem::path path)
{
    if (stream_.is_open())
    {
        close();
    }
    path_ = path;
    stream_.open(path, std::ios::in | std::ios::binary);

    stream_.seekg(0, std::ios::beg);
}

void FileSystemFile::close() { stream_.close(); }

int FileSystemFile::eof() { return stream_.eof(); }

unsigned int FileSystemFile::read(unsigned char *aDst, unsigned int aBytes)
{
    stream_.read(aDst, aBytes);
    if (stream_.eof())
    {
        return static_cast<unsigned int>(stream_.gcount());
    }
    return aBytes;
}

unsigned int FileSystemFile::length() { return static_cast<unsigned int>(std::filesystem::file_size(path_)); }

void FileSystemFile::seek(int aOffset) { stream_.seekg(aOffset); }

unsigned int FileSystemFile::pos() { return static_cast<unsigned int>(stream_.tellg()); }

constexpr size_t text_size = 1000;

struct SpeechInfo
{
    SoLoud::Speech speech;
    SoLoud::handle handle = 0;
    float volume = 1.0f;
    float pan = 0.0f;
    char text[text_size]{};
    unsigned int base_freq = 1330;
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
    SDL_SetWindowTitle(window, "SoLoud-SDL test application.");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    std::srand(static_cast<unsigned>(std::time(nullptr))); 

    SoLoud::Soloud soloud;
    soloud.init();

    SoLoud::WavStream bgm_stream;
    SoLoud::handle bgm_handle = 0;
    float bgm_volume = 1.0f;
    ImGui::FileBrowser bgm_file_browser;
    bgm_file_browser.SetTitle("Open BGM");
    bgm_file_browser.SetTypeFilters({".ogg", ".wav", ".mp3", ".flac"});
    FileSystemFile bgm_file;

    SoLoud::Wav sfx_wav;
    float sfx_x_pos = 0.0f, sfx_y_pos = 0.0f, sfx_z_pos = 0.0f;
    ImGui::FileBrowser sfx_file_browser;
    sfx_file_browser.SetTitle("Open SFX");
    sfx_file_browser.SetTypeFilters({".ogg", ".wav", ".mp3", ".flac"});
    FileSystemFile sfx_file;

    SoLoud::Bus speech_bus;
    SoLoud::handle speechBusHandle = soloud.play(speech_bus);

    std::vector<SpeechInfo> speeches;

    SoLoud::Sfxr sfxr;
    SoLoud::Sfxr::SFXR_PRESETS sfxr_preset = SoLoud::Sfxr::COIN;
    int sfxr_seed = std::rand();
    SoLoud::LofiFilter sfxr_lofi;
    sfxr_lofi.setParams(8000, 4);
    bool sfxr_lofi_enable = false;

    SDL_GameController *controller = nullptr;
    if (SDL_NumJoysticks() >= 1)
    {
        controller = SDL_GameControllerOpen(0);
    }

    bool is_a_pressed = false;
    bool is_using_controller = true;
    bool is_speech_paused = false;

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

                ImGui::LabelText("Filename", "%s", bgm_file.path().u8string().c_str());

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
                ImGui::LabelText("Filename", "%s", sfx_file.path().u8string().c_str());
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

            ImGui::Begin("Speech");
            {
                ImGui::SameLine();
                if (ImGui::Button("Pause All"))
                {
                    is_speech_paused = !is_speech_paused;
                    soloud.setPause(speechBusHandle, is_speech_paused);
                }
                ImGui::SameLine();
                if (ImGui::Button("Add One"))
                {
                    speeches.emplace_back();
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove Last"))
                {
                    auto &speech = speeches.back();
                    speech.speech.stop();
                    speeches.pop_back();
                }

                for (size_t i = 0; i < speeches.size(); i++)
                {
                    auto &[speech, handle, volume, pan, text, base_freq, base_speed, base_declination, base_waveform] =
                        speeches[i];

                    if (ImGui::CollapsingHeader(std::format("Speech {}", i).c_str()))
                    {
                        if (ImGui::Button(std::format("Play {}", i).c_str()))
                        {
                            handle = speech_bus.play(speech, volume, pan);
                            speech.setParams(static_cast<unsigned int>(floor(base_freq)), base_speed, base_declination);
                        }

                        if (ImGui::InputTextMultiline(std::format("Text {}", i).c_str(), text, text_size))
                        {
                            speech.setText(text);
                        }

                        if (ImGui::SliderFloat(std::format("Volume {}", i).c_str(), &volume, 0.0f, 1.0f))
                        {
                            speech.setVolume(volume);
                        }

                        ImGui::SliderFloat(std::format("Panning {}", i).c_str(), &pan, -1.0f, 1.0f);
                        if (ImGui::CollapsingHeader(std::format("Base params {}", i).c_str()))
                        {
                            constexpr unsigned int min_freq = 0;
                            constexpr unsigned int max_freq = 3000;

                            ImGui::SliderScalar("Base freq", ImGuiDataType_U32, &base_freq, &min_freq, &max_freq);
                            ImGui::SliderFloat("Base speed", &base_speed, 0.1f, 30);
                            ImGui::SliderFloat("Base declination", &base_declination, -3, 3);
                        }
                        if (ImGui::CollapsingHeader(std::format("Waveform {}", i).c_str()))
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
                }
            }
            ImGui::End();

            ImGui::Begin("Sfxr");
            {
                if (ImGui::Button("Play"))
                {
                    sfxr.loadPreset(sfxr_preset, sfxr_seed);
                    sfxr.setFilter(0, sfxr_lofi_enable? &sfxr_lofi: nullptr);
                    soloud.play(sfxr);
                }

                if (ImGui::CollapsingHeader("Preset"))
                {
                    if (ImGui::RadioButton("Coin", sfxr_preset == SoLoud::Sfxr::COIN))
                    {
                        sfxr_preset = SoLoud::Sfxr::COIN;
                    }

                    if (ImGui::RadioButton("Laser", sfxr_preset == SoLoud::Sfxr::LASER))
                    {
                        sfxr_preset = SoLoud::Sfxr::LASER;
                    }

                    if (ImGui::RadioButton("Explosion", sfxr_preset == SoLoud::Sfxr::EXPLOSION))
                    {
                        sfxr_preset = SoLoud::Sfxr::EXPLOSION;
                    }

                    if (ImGui::RadioButton("Power Up", sfxr_preset == SoLoud::Sfxr::POWERUP))
                    {
                        sfxr_preset = SoLoud::Sfxr::POWERUP;
                    }

                    if (ImGui::RadioButton("Hurt", sfxr_preset == SoLoud::Sfxr::HURT))
                    {
                        sfxr_preset = SoLoud::Sfxr::HURT;
                    }

                    if (ImGui::RadioButton("Jump", sfxr_preset == SoLoud::Sfxr::JUMP))
                    {
                        sfxr_preset = SoLoud::Sfxr::JUMP;
                    }

                    if (ImGui::RadioButton("Blip", sfxr_preset == SoLoud::Sfxr::BLIP))
                    {
                        sfxr_preset = SoLoud::Sfxr::BLIP;
                    }
                }
                
                ImGui::InputInt("Seed Number", &sfxr_seed);
                if (ImGui::Button("Random"))
                {
                    sfxr_seed = std::rand();
                }

                ImGui::Checkbox("Lo-Fi", &sfxr_lofi_enable);
            }
            ImGui::End();

            bgm_file_browser.Display();
            if (bgm_file_browser.HasSelected())
            {
                soloud.stopAudioSource(bgm_stream);

                auto path = bgm_file_browser.GetSelected();
                bgm_file.open(path);

                bgm_stream.loadFile(&bgm_file);
                bgm_file_browser.ClearSelected();
            }

            sfx_file_browser.Display();
            if (sfx_file_browser.HasSelected())
            {
                auto path = sfx_file_browser.GetSelected();
                sfx_file.open(path);
                sfx_wav.loadFile(&sfx_file);
                sfx_file_browser.ClearSelected();
            }
        }
        ImGui::EndFrame();

        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);


        SDL_Delay(0);
    }

    bgm_file.close();
    sfx_file.close();

    soloud.stopAll();
    soloud.deinit();

    SDL_GameControllerClose(controller);
    SDL_Quit();

    return 0;
}
