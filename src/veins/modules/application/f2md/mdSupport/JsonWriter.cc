#include <veins/modules/application/f2md/mdSupport/JsonWriter.h>


JsonWriter::JsonWriter() {
        openTags = 0;
}

void JsonWriter::writeHeader() {
    outStr << "{";
}

void JsonWriter::addElement(const std::string element) {

    //     std::string elementTemp = addNeededindent(element, 1);
        outStr << element;
}

void JsonWriter::writeFooter() {
    outStr << "}\n";
}


std::string JsonWriter::addNeededindent(const std::string outString, int indentCnt) {
    std::string indentS = "";
    for (int i = 0; i < indentCnt; i++) {
        indentS = indentS + "\t";
    }
    std::string replaceStr = outString;
    boost::replace_all(replaceStr, "\n", "\n"+indentS);
    replaceStr = indentS + replaceStr;
    replaceStr = replaceStr.substr (0,replaceStr.size()-indentS.size());
    return replaceStr;
}

void JsonWriter::openJsonElement(const std::string openTag, bool noName) {
    int elementIndex = -1;
    for (int var = 0; var < openTags; ++var) {
        if(!tempOpenTag[var].compare(openTag)){
            elementIndex =var;
            break;
        }
    }
    if(elementIndex == -1){
        std::string element = "";

        tempOpenTag.resize(openTags + 1);
        elementString.resize(openTags + 1);

        if(noName){
            element = "{\n";
        }else{
            element = "\"" + openTag + "\":{\n";
        }

        tempOpenTag[openTags] = openTag;
        elementString[openTags] = element;

        openTags += 1;
    }else{
        std::string element = "";
        if(noName){
            element = "{\n";
        }else{
            element = "\"" + openTag + "\":{\n";
        }
        elementString[elementIndex] = element;
    }
}

void JsonWriter::openJsonElementList(const std::string openTag) {
    int elementIndex = -1;
    for (int var = 0; var < openTags; ++var) {
        if(!tempOpenTag[var].compare(openTag)){
            elementIndex =var;
            break;
        }
    }
    if(elementIndex == -1){
        std::string element = "";

        tempOpenTag.resize(openTags + 1);
        elementString.resize(openTags + 1);
        element = "\"" + openTag + "\":[\n";

        tempOpenTag[openTags] = openTag;
        elementString[openTags] = element;

        openTags += 1;
    }else{
        std::string element = "";
        element = "\"" + openTag + "\":[\n";

        elementString[elementIndex] = element;
    }
}


void JsonWriter::addTagToElement(const std::string openTag, const std::string addTag) {
    int elementIndex = -1;
    for (int var = 0; var < openTags; ++var) {
        if(!tempOpenTag[var].compare(openTag)){
            elementIndex =var;
            break;
        }
    }
    std::string addString = addNeededindent(addTag+",\n",1);
    elementString[elementIndex] = elementString[elementIndex] + addString;
}


void JsonWriter::addFinalTagToElement(const std::string openTag, const std::string addTag) {
    int elementIndex = -1;
    for (int var = 0; var < openTags; ++var) {
        if(!tempOpenTag[var].compare(openTag)){
            elementIndex =var;
            break;
        }
    }
    std::string addString = addNeededindent(addTag+"\n",1);
    elementString[elementIndex] = elementString[elementIndex] + addString;
}

std::string JsonWriter::getJsonElement(const std::string openTag) {
    int elementIndex = -1;
    for (int var = 0; var < openTags; ++var) {
        if(!tempOpenTag[var].compare(openTag)){
            elementIndex = var;
            break;
        }
    }
    elementString[elementIndex] = elementString[elementIndex]+ "}";
    return elementString[elementIndex];
}

std::string JsonWriter::getJsonElementList(const std::string openTag) {
    int elementIndex = -1;
    for (int var = 0; var < openTags; ++var) {
        if(!tempOpenTag[var].compare(openTag)){
            elementIndex = var;
            break;
        }
    }
    elementString[elementIndex] = elementString[elementIndex]+ "]";
    return elementString[elementIndex];
}

std::string JsonWriter::getSimpleTag(const std::string tag, const std::string res, bool intStr){
    std::string out = "";
    if(intStr){
        out = "\"" + tag + "\": " + res + "";
    }else{
        out = "\"" + tag + "\": \"" + res + "\"";
    }

    return out;
}

std::string JsonWriter::getOutString(){
    return outStr.str();
}



