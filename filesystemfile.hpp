#ifndef FILESYSTEMFILE_HPP
#define FILESYSTEMFILE_HPP

#include <filesystem>
#include <fstream>
#include <soloud_file.h>

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

#endif