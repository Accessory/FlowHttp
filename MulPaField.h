#pragma once

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include <FlowUtils/FlowParser.h>


class MulPaField : public std::map<std::string, std::string> {
public:
    ~MulPaField() {
        if (_fileIsOpen) {
            _file->close();
            delete _file;
        }
    }

    std::string Header(const std::string &key) {
        auto itr = this->find(key);
        if (itr == this->end())
            return "";
        return itr->second;
    }

    void addHeader(const std::string &key, const std::string &value) {
        this->operator[](key) = value;
    }

    std::string Name() {
        if (_name.empty()) {
            _name = FlowParser::between(Header("Content-Disposition"), "name=\"", "\"");
        }

        return _name;
    }

    std::string FileName() {
        if (_filename.empty()) {
            _filename = FlowParser::between(Header("Content-Disposition"), "filename=\"", "\"");
        }

        return _filename;
    }

    std::ofstream *File() {
        return _file;
    }

    std::ofstream *OpenIn(const std::string &basePath, const std::function<std::string(MulPaField*)> &function) {
        _fileIsOpen = true;

        if (function != nullptr) {
            FullPath = function(this);
        } else {
            if (basePath.empty() || basePath.at(basePath.size() - 1) == '/' ||
                basePath.at(basePath.size() - 1) == '\\') {
                FullPath = basePath + FileName();
            } else {
                FullPath = basePath + '/' + FileName();
            }
        }

        _file = new std::ofstream(FullPath, std::ofstream::out | std::ofstream::binary);
        return _file;
    }

    bool FileIsOpen() {
        return _fileIsOpen;
    }

    void CloseFile() {
        if (_fileIsOpen) {
            _fileIsOpen = false;
            _file->close();
            delete _file;
        }
    }

    std::vector<unsigned char> data;
    std::string FullPath;

private:
    std::ofstream *_file;
    bool _fileIsOpen = false;
    std::string _name;
    std::string _filename;
};


