#ifndef XmlWriter_H
#define XmlWriter_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>      // std::stringstream
#include <boost/algorithm/string.hpp> // include Boost, a C++ library



class XmlWriter {
public:
    void init();
    void writeHeader();
    void writeOpenTag(const std::string);
    void writeOpenTagWithAttribute(const std::string openTag, const std::string attr);
    void writeCloseTag();
    void writeStartElementTag(const std::string);
    void writeWholeElement(const std::string);
    void writeEndElementTag();
    void writeAttribute(const std::string);
    void writeString(const std::string);
    std::string getOutString();

private:
    std::stringstream outStr;
    int indent;
    int openTags;
    int openElements;
    std::vector<std::string> tempOpenTag;
    std::vector<std::string> tempElementTag;
};

#endif
