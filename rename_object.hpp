#ifndef RENAME_OBJECT_H
#define RENAME_OBJECT_H

#include <string>
#include <unordered_map>

class RenameObject {
public:
    void setLibraryId( uint64_t id );
    uint64_t getLibraryId() const;
    void setPresentedName( const std::string &name );
    const std::string &getPresentedName() const;
    void addCustomField( const std::string &key, const std::string &value );
    const std::unordered_map< std::string, std::string > &getCustomFields() const;
    void clearCustomFields();
    std::string toJson() const;
    void fromJson( const std::string &json );

private:
    uint64_t _library_id;
    std::string _name;
    std::unordered_map< std::string, std::string > _custom_fields;
};

#endif
