#include "rename_object.hpp"
#include "functions.hpp"
#include <sstream>
#include <rapidjson/document.h>
#include <iostream>

void RenameObject::setLibraryId( uint64_t id ) {
    _library_id = id;
}

uint64_t RenameObject::getLibraryId() const {
    return _library_id;
}

void RenameObject::setPresentedName( const std::string &name ) {
    _name = name;
}

const std::string &RenameObject::getPresentedName() const {
    return _name;
}

void RenameObject::addCustomField( const std::string &key,
                                   const std::string &value ) {
    _custom_fields[key] = value;
}

const std::unordered_map< std::string, std::string > &RenameObject::getCustomFields() const {
    return _custom_fields;
}

void RenameObject::clearCustomFields() {
    _custom_fields.clear();
}

std::string RenameObject::toJson() const {
    std::ostringstream result;
    result << "{\n  \"library_id\": " << _library_id << ",\n";
    result << "  \"name\": \"" << safeJson(_name) << "\",\n";
    result << "  \"custom_fields\": {\n";
    if ( _custom_fields.size() > 0 ) {
        for ( auto &entry : _custom_fields ) {
            result << "    \"" << safeJson(entry.first) << "\": \"" << safeJson(entry.second)
                   << "\",\n";
        }
        result.seekp( -2, std::ios_base::end );
        result << "\n";
    }
    result << "  }\n}";
    return result.str();
}

void invalidJson() {
    throw "Invalid JSON";
}

void RenameObject::fromJson( const std::string &json ) {
    rapidjson::Document doc;
    doc.Parse( json.c_str() );
    if( doc.HasParseError() ) {
        invalidJson();
        return;
    }
    if( doc.FindMember("library_id") != doc.MemberEnd() && doc["library_id"].IsUint64()) {
        setLibraryId(doc["library_id"].GetUint64());
    }
    if( doc.FindMember("name") != doc.MemberEnd() && doc["name"].IsString()) {
        setPresentedName(doc["name"].GetString());
    }
    if( doc.FindMember("custom_fields") != doc.MemberEnd() && doc["custom_fields"].IsObject()) {
        auto custom_obj = doc["custom_fields"].GetObject();
        for(auto it = custom_obj.begin(); it != custom_obj.end(); it++) {
            addCustomField(it->name.GetString(), it->value.GetString());
        }
    }
}
