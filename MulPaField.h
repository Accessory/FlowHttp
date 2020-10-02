#pragma once

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include <FlowUtils/FlowParser.h>

using namespace std;

class MulPaField : public map<string, string> {
public:
    ~MulPaField() {
        if (_fileIsOpen) {
            _file->close();
            delete _file;
        }
    }

    string Header(const string &key) {
        auto itr = this->find(key);
        if (itr == this->end())
            return "";
        return itr->second;
    }

    void addHeader(const string &key, const string &value) {
        this->operator[](key) = value;
    }

    string Name() {
        if (_name.empty()) {
            _name = FlowParser::between(Header("Content-Disposition"), "name=\"", "\"");
        }

        return _name;
    }

    string FileName() {
        if (_filename.empty()) {
            _filename = FlowParser::between(Header("Content-Disposition"), "filename=\"", "\"");
        }

        return _filename;
    }

    ofstream *File() {
        return _file;
    }

    ofstream *OpenIn(const string &basePath, const std::function<std::string(MulPaField*)> &function) {
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

        _file = new ofstream(FullPath, ofstream::out | ofstream::binary);
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
    ofstream *_file;
    bool _fileIsOpen = false;
    string _name;
    string _filename;
};


