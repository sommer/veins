#include <veins/modules/application/f2md/mdSupport/XmlWriter.h>


void XmlWriter::init() {
        indent = 0;
        openTags = 0;
        openElements = 0;
}

void XmlWriter::writeHeader() {
        outStr << "<!--XML Document-->\n";
        outStr << "<?xml version='1.0' encoding='us-ascii'>\n";
}

void XmlWriter::writeOpenTag(const std::string openTag) {
        for (int i = 0; i < indent; i++) {
            outStr << "\t";
        }
        tempOpenTag.resize(openTags + 1);
        outStr << "<" << openTag << ">\n";
        tempOpenTag[openTags] = openTag;
        indent += 1;
        openTags += 1;
}

void XmlWriter::writeOpenTagWithAttribute(const std::string openTag, const std::string attr) {
        for (int i = 0; i < indent; i++) {
            outStr << "\t";
        }
        tempOpenTag.resize(openTags + 1);
        outStr << "<" << openTag <<" "<< attr << ">\n";
        tempOpenTag[openTags] = openTag;
        indent += 1;
        openTags += 1;
}

void XmlWriter::writeCloseTag() {
        indent -= 1;
        for (int i = 0; i < indent; i++) {
            outStr << "\t";
        }
        outStr << "</" << tempOpenTag[openTags - 1] << ">\n";
        tempOpenTag.resize(openTags - 1);
        openTags -= 1;
}

void XmlWriter::writeStartElementTag(const std::string elementTag) {
        for (int i = 0; i < indent; i++) {
            outStr << "\t";
        }
        tempElementTag.resize(openElements + 1);
        tempElementTag[openElements] = elementTag;
        openElements += 1;
        outStr << "<" << elementTag;

}
void XmlWriter::writeAttribute(const std::string outAttribute) {
        outStr << " " << outAttribute;
}

void XmlWriter::writeString(const std::string outString) {
        outStr << ">" << outString;
}

void XmlWriter::writeEndElementTag() {
        outStr << "</" << tempElementTag[openElements - 1] << ">\n";
        tempElementTag.resize(openElements - 1);
        openElements -= 1;
}

void XmlWriter::writeWholeElement(const std::string outString) {
    std::string indentS = "";
    for (int i = 0; i < indent; i++) {
        indentS = indentS + "\t";
    }

    std::string replaceStr = outString;
    boost::replace_all(replaceStr, "\n", "\n"+indentS);
    replaceStr = indentS + replaceStr;
    replaceStr = replaceStr.substr (0,replaceStr.size()-indentS.size());
    outStr << replaceStr;
}


std::string XmlWriter::getOutString(){
    return outStr.str();
}



