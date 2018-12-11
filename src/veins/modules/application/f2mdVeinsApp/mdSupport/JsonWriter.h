#ifndef JsonWriter_H
#define JsonWriter_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>      // std::stringstream
#include <boost/algorithm/string.hpp> // include Boost, a C++ library



class JsonWriter {
public:
    JsonWriter();
    void writeHeader();
    void addElement(const std::string element);
    void writeFooter();
    std::string addNeededindent(const std::string outString, int indentCnt);
    void openJsonElement(const std::string openTag, bool noName);
    void openJsonElementList(const std::string openTag);
    void addTagToElement(const std::string openTag, const std::string addTag);
    void addFinalTagToElement(const std::string openTag, const std::string addTag);
    std::string getJsonElement(const std::string openTag);
    std::string getJsonElementList(const std::string openTag);
    std::string getSimpleTag(const std::string tag, const std::string res, bool intStr);
    std::string getOutString();

private:
    std::stringstream outStr;
    int openTags;
    std::vector<std::string> tempOpenTag;
    std::vector<std::string> elementString;
};

#endif

