/*
    Pheniqs : PHilology ENcoder wIth Quality Statistics
    Copyright (C) 2018  Lior Galanti
    NYU Center for Genetics and System Biology

    Author: Lior Galanti <lior.galanti@nyu.edu>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "json.h"

#include <cstdio>
#include <rapidjson/encodedstream.h>
#include <rapidjson/filereadstream.h>

using namespace rapidjson;

void print_json(const Value& node, const char* path, const int32_t& precision) {
    FILE* file_handle = fopen(path, "w");
    char buffer[65536];
    FileWriteStream output_stream(file_handle, buffer, sizeof(buffer));
    PrettyWriter< FileWriteStream > writer(output_stream);
    writer.SetMaxDecimalPlaces(precision);
    node.Accept(writer);
    fclose(file_handle);
};
void print_json(const Value& node, ostream& o, const int32_t& precision) {
    StringBuffer buffer;
    PrettyWriter< StringBuffer > writer(buffer);
    writer.SetMaxDecimalPlaces(precision);
    node.Accept(writer);
    o << buffer.GetString() << endl;
};
// Document* load_json(const string& path) {
//     Document* document(NULL);
//     if(access(path.c_str(), R_OK) != -1) {
//         document = new Document();
//         ifstream file(path);
//         const string content((istreambuf_iterator< char >(file)), istreambuf_iterator< char >());
//         file.close();
//         if(document->Parse(content.c_str()).HasParseError()) {
//             string message(GetParseError_En(document->GetParseError()));
//             message += " at position ";
//             message += to_string(document->GetErrorOffset());
//             throw ConfigurationError(message);
//         }
//     }
//     return document;
// };

Document encode_validation_error(const SchemaValidator& validator, const Value& schema, const Value& container) {
    Document error(kObjectType);
    StringBuffer buffer;

    Value violation(kObjectType);

    Pointer invalid_element_pointer(validator.GetInvalidDocumentPointer());
    invalid_element_pointer.Stringify(buffer);
    string invalid_element_path(buffer.GetString());
    buffer.Clear();
    encode_key_value("invalid element path", invalid_element_path, violation, error);

    const Value* element_reference(invalid_element_pointer.Get(container));
    if(element_reference != NULL) {
        Value invalid_element(*element_reference, error.GetAllocator());
        violation.AddMember(Value("element").Move(), invalid_element.Move(), error.GetAllocator());
    }

    Pointer invalid_schema_pointer(validator.GetInvalidSchemaPointer());
    invalid_schema_pointer.Stringify(buffer);
    string invalid_schema_path(buffer.GetString());
    buffer.Clear();
    encode_key_value("invalid schema path", invalid_schema_path, violation, error);

    const Value* schema_reference(invalid_schema_pointer.Get(schema));
    if(schema_reference != NULL) {
        Value invalid_schema(*schema_reference, error.GetAllocator());
        violation.AddMember(Value("schema").Move(), invalid_schema.Move(), error.GetAllocator());
    }

    string keyword(validator.GetInvalidSchemaKeyword());
    encode_key_value("keyword", keyword, violation, error);

    error.AddMember(Value("violation").Move(), violation.Move(), error.GetAllocator());

    return error;
};

template <> bool decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsBool()) {
                    return reference->value.GetBool();
                } else { throw ConfigurationError(string(key) + " element is not a boolean"); }
            } else { return false; }
        } else { return false; }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> uint8_t decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                unsigned value;
                if(reference->value.IsUint() && (value = reference->value.GetUint()) <= numeric_limits< uint8_t >::max()) {
                    return static_cast< uint8_t >(value);
                } else { throw ConfigurationError(string(key) + " element is not an 8 bit unsigned integer"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> vector< uint8_t > decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsArray()) {
                    unsigned buffer;
                    size_t index(0);
                    vector< uint8_t > value(reference->value.Size());
                    for(const auto& element : reference->value.GetArray()) {
                        if(element.IsUint() && (buffer = element.GetUint()) <= numeric_limits< uint8_t >::max()) {
                            value[index] = static_cast < uint8_t >(buffer);
                            ++index;
                        } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not an 8 bit unsigned integer"); }
                    }
                    value.shrink_to_fit();
                    return value;
                } else { throw ConfigurationError(string(key) + " element is not an array"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> int32_t decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsInt()) {
                    return reference->value.GetInt();
                } else { throw ConfigurationError(string(key) + " element is not a 32 bit integer"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> vector< int32_t > decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsArray()) {
                    size_t index(0);
                    vector< int32_t > value(reference->value.Size());
                    for(const auto& element : reference->value.GetArray()) {
                        if(element.IsInt()) {
                            value[index] = element.GetInt();
                            ++index;
                        } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not a 32 bit integer"); }
                    }
                    value.shrink_to_fit();
                    return value;
                } else { throw ConfigurationError(string(key) + " element is not an array"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> uint32_t decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsUint()) {
                    return reference->value.GetUint();
                } else { throw ConfigurationError(string(key) + " element is not a 32 bit unsigned integer"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> int64_t decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsInt64()) {
                    return reference->value.GetInt64();
                } else { throw ConfigurationError(string(key) + " element is not a 64 bit integer"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> uint64_t decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsUint64()) {
                    return reference->value.GetUint64();
                } else { throw ConfigurationError(string(key) + " element is not a 64 bit unsigned integer"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> vector< uint64_t > decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsArray()) {
                    size_t index(0);
                    vector< uint64_t > value(reference->value.Size());
                    for(const auto& element : reference->value.GetArray()) {
                        if(element.IsUint64()) {
                            value[index] = element.GetUint64();
                            ++index;
                        } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not an unsigned 64 bit integer"); }
                    }
                    value.shrink_to_fit();
                    return value;
                } else { throw ConfigurationError(string(key) + " element is not an array"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> double decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsNumber()) {
                    return reference->value.GetDouble();
                } else { throw ConfigurationError(string(key) + " element is not numeric"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> vector< double > decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsArray()) {
                    size_t index(0);
                    vector< double > value(reference->value.Size());
                    for(const auto& element : reference->value.GetArray()) {
                        if(element.IsNumber()) {
                            value[index] = element.GetDouble();
                            ++index;
                        } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not numeric"); }
                    }
                    value.shrink_to_fit();
                    return value;
                } else { throw ConfigurationError(string(key) + " element is not an array"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> string decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsString()) {
                    return string(reference->value.GetString(), reference->value.GetStringLength());
                } else { throw ConfigurationError(string(key) + " element is not a string"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> list< string > decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsArray()) {
                    size_t index(0);
                    list< string > value;
                    for(const auto& element : reference->value.GetArray()) {
                        if(element.IsString()) {
                            value.emplace_back(element.GetString(), element.GetStringLength());
                            ++index;
                        } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not a string"); }
                    }
                    return value;
                } else { throw ConfigurationError(string(key) + " element is not an array"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};
template <> kstring_t decode_value_by_key(const Value::Ch* key, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsString()) {
                    if(reference->value.GetStringLength() < numeric_limits< size_t >::max()) {
                        kstring_t value({ 0, 0, NULL });
                        ks_put_string(reference->value.GetString(), static_cast< size_t >(reference->value.GetStringLength()), value);
                        return value;
                    } else { throw ConfigurationError(string(key) + " element must be a string shorter than " + to_string(numeric_limits< int >::max())); }
                } else { throw ConfigurationError(string(key) + " element is not a string"); }
            } else { throw ConfigurationError(string(key) + " is null"); }
        } else { throw ConfigurationError(string(key) + " not found"); }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
};

template<> bool decode_value_by_key< bool >(const Value::Ch* key, bool& value, const Value& container) {
    if(container.IsObject()) {
        value = false;
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd()) {
            if(!reference->value.IsNull()) {
                if(reference->value.IsBool()) {
                    value = reference->value.GetBool();
                } else { throw ConfigurationError(string(key) + " element is not a boolean"); }
            }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return true;
};
template<> bool decode_value_by_key< uint8_t >(const Value::Ch* key, uint8_t& value, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            unsigned buffer;
            if(reference->value.IsUint() && (buffer = reference->value.GetUint()) <= numeric_limits< uint8_t >::max()) {
                value = static_cast< uint8_t >(buffer);
                return true;
            } else { throw ConfigurationError(string(key) + " element is not an 8 bit unsigned integer"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return false;
};
template<> bool decode_value_by_key< vector< uint8_t > >(const Value::Ch* key, vector< uint8_t >& value, const Value& container) {
    bool result(false);
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsArray()) {
                unsigned buffer;
                size_t index(0);
                value.resize(reference->value.Size());
                for(const auto& element : reference->value.GetArray()) {
                    if(element.IsUint() && (buffer = element.GetUint()) <= numeric_limits< uint8_t >::max()) {
                        value[index] = static_cast < uint8_t >(buffer);
                        ++index;
                        result = true;
                    } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not an 8 bit unsigned integer"); }
                }
                value.shrink_to_fit();
            } else { throw ConfigurationError(string(key) + " element is not an array"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return result;
};
template<> bool decode_value_by_key< int32_t >(const Value::Ch* key, int32_t& value, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsInt()) {
                value = reference->value.GetInt();
                return true;
            } else { throw ConfigurationError(string(key) + " element is not a 32 bit integer"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return false;
};
template<> bool decode_value_by_key< vector< int32_t > >(const Value::Ch* key, vector< int32_t >& value, const Value& container) {
    bool result(false);
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsArray()){
                size_t index(0);
                value.resize(reference->value.Size());
                for(const auto& element : reference->value.GetArray()) {
                    if(element.IsInt()) {
                        value[index] = static_cast < uint8_t >(element.GetInt());
                        ++index;
                        result = true;
                    } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not a 32 bit integer"); }
                }
                value.shrink_to_fit();
            } else { throw ConfigurationError(string(key) + " element is not an array"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return result;
};
template<> bool decode_value_by_key< uint32_t >(const Value::Ch* key, uint32_t& value, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsUint()) {
                value = reference->value.GetUint();
                return true;
            } else { throw ConfigurationError(string(key) + " element is not a 32 bit unsigned integer"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return false;
};
template<> bool decode_value_by_key< int64_t >(const Value::Ch* key, int64_t& value, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsInt64()) {
                value = reference->value.GetInt64();
                return true;
            } else { throw ConfigurationError(string(key) + " element is not a 64 bit integer"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return false;
};
template<> bool decode_value_by_key< uint64_t >(const Value::Ch* key, uint64_t& value, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsUint64()) {
                value = reference->value.GetUint64();
                return true;
            } else { throw ConfigurationError(string(key) + " element is not a 64 bit unsigned integer"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return false;
};
template<> bool decode_value_by_key< vector< uint64_t > >(const Value::Ch* key, vector< uint64_t >& value, const Value& container) {
    bool result(false);
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsArray()){
                size_t index(0);
                value.resize(reference->value.Size());
                for(const auto& element : reference->value.GetArray()) {
                    if(element.IsUint64()) {
                        value[index] = element.GetUint64();
                        ++index;
                        result = true;
                    } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not an unsigned 64 bit integer"); }
                }
                value.shrink_to_fit();
            } else { throw ConfigurationError(string(key) + " element is not an array"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return result;
};
template<> bool decode_value_by_key< double >(const Value::Ch* key, double& value, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsNumber()) {
                value = reference->value.GetDouble();
                return true;
            } else { throw ConfigurationError(string(key) + " element is not numeric"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return false;
};
template<> bool decode_value_by_key< vector< double > >(const Value::Ch* key, vector< double >& value, const Value& container) {
    bool result(false);
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsArray()) {
                size_t index(0);
                value.resize(reference->value.Size());
                for(const auto& element : reference->value.GetArray()) {
                    if(element.IsNumber()) {
                        value[index] = element.GetDouble();
                        ++index;
                        result = true;
                    } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not numeric"); }
                }
                value.shrink_to_fit();
            } else { throw ConfigurationError(string(key) + " element is not an array"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return result;
};
template<> bool decode_value_by_key< string >(const Value::Ch* key, string& value, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsString()) {
                value.assign(reference->value.GetString(), reference->value.GetStringLength());
                return true;
            } else { throw ConfigurationError(string(key) + " element is not a string"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return false;
};
template<> bool decode_value_by_key< list< string > >(const Value::Ch* key, list< string >& value, const Value& container) {
    bool result(false);
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsArray()) {
                size_t index(0);
                value.clear();
                for(const auto& element : reference->value.GetArray()) {
                    if(element.IsString()) {
                        value.emplace_back(element.GetString(), element.GetStringLength());
                        ++index;
                        result = true;
                    } else { throw ConfigurationError(string(key) + " element at position " + to_string(index) + " is not a string"); }
                }
            } else { throw ConfigurationError(string(key) + " element is not an array"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return result;
};
template<> bool decode_value_by_key< kstring_t >(const Value::Ch* key, kstring_t& value, const Value& container) {
    if(container.IsObject()) {
        Value::ConstMemberIterator reference = container.FindMember(key);
        if(reference != container.MemberEnd() && !reference->value.IsNull()) {
            if(reference->value.IsString()) {
                if(reference->value.GetStringLength() < numeric_limits< size_t >::max()) {
                    ks_put_string(reference->value.GetString(), static_cast< size_t >(reference->value.GetStringLength()), value);
                    return true;
                } else { throw ConfigurationError(string(key) + " element is too long"); }
            } else { throw ConfigurationError(string(key) + " element is not a string"); }
        }
    } else { throw ConfigurationError(string(key) + " container is not a dictionary"); }
    return false;
};

void ValidationError::compile() {
    message.clear();
    message += name;
    string example;
    Value::ConstMemberIterator reference = ontology.FindMember("violation");
    if(reference != ontology.MemberEnd()) {
        const Value& violation(reference->value);

        try {
            string invalid_element_path(decode_value_by_key< string >("invalid element path", violation));
            string invalid_schema_path(decode_value_by_key< string >("invalid schema path", violation));

            try {
                const Value& schema(find_value_by_key("schema", violation));

                string title;
                if(decode_value_by_key< string >("title", title, schema)) {
                    message += " in ";
                    message += title;
                    message += " directive";
                }
                message += "\n";

                string description;
                if(decode_value_by_key< string >("description", description, schema)) {
                    message += "Directive description: ";
                    message += description;
                    message += "\n";
                }

                string keyword(decode_value_by_key< string >("keyword", violation));
                const Value& schema_keyword_value(find_value_by_key(keyword.c_str(), schema));
                const Value& element_value(find_value_by_key("element", violation));

                message += "Error description: ";

                if(keyword == "type") {
                    message += "Expected type ";
                    message += string(schema_keyword_value.GetString(), schema_keyword_value.GetStringLength());
                    message += " but actual is ";
                    switch (element_value.GetType()) {
                        case Type::kNullType: {
                            message += "null";
                            break;
                        };
                        case Type::kFalseType:
                        case Type::kTrueType: {
                            message += "boolean";
                            break;
                        };
                        case Type::kObjectType: {
                            message += "object";
                            break;
                        };
                        case Type::kArrayType: {
                            message += "array";
                            break;
                        };
                        case Type::kStringType: {
                            message += "string";
                            break;
                        };
                        case Type::kNumberType: {
                            message += "number";
                            break;
                        };
                        default:
                            break;
                    }

                } else if(keyword == "required") {
                    list< string > expected(decode_value_by_key< list< string > >(keyword.c_str(), schema));
                    if(expected.size() > 0) {
                        message += "Missing ";
                        if(expected.size() > 1) {
                            message += " one or more mandatory elements: ";
                            for(auto& option : expected) {
                                message += " ";
                                message += option;
                            }
                        } else {
                            message += "mandatory element: ";
                            message += expected.front();
                        }
                        message += ".";
                    }

                } else if(keyword == "pattern") {
                    string actual(decode_value_by_key< string >("element", violation));
                    string expected(decode_value_by_key< string >(keyword.c_str(), schema));
                    message += actual;
                    message += " does not match pattern ";
                    message += expected;

                } else if(keyword == "enum") {
                    string actual(decode_value_by_key< string >("element", violation));
                    list< string > expected(decode_value_by_key< list< string > >(keyword.c_str(), schema));
                    message += actual;
                    message += " is not one of:";
                    for(auto& option : expected) {
                        message += " ";
                        message += option;
                    }
                    message += ".";

                } else if(keyword == "minimum") {
                    string type(decode_value_by_key< string >("type", schema));
                    if(type == "integer") {
                        int64_t actual(decode_value_by_key< int64_t >("element", violation));
                        int64_t expected(decode_value_by_key< int64_t >(keyword.c_str(), schema));
                        message += to_string(actual);
                        message += " is smaller than ";
                        message += to_string(expected);
                    } else {
                        double actual(decode_value_by_key< double >("element", violation));
                        double expected(decode_value_by_key< double >(keyword.c_str(), schema));
                        message += to_string(actual);
                        message += " is smaller than ";
                        message += to_string(expected);
                    }
                    message += ".";

                } else if(keyword == "maximum") {
                    string type(decode_value_by_key< string >("type", schema));
                    if(type == "integer") {
                        int64_t actual(decode_value_by_key< int64_t >("element", violation));
                        int64_t expected(decode_value_by_key< int64_t >(keyword.c_str(), schema));
                        message += to_string(actual);
                        message += " is bigger than ";
                        message += to_string(expected);
                    } else {
                        double actual(decode_value_by_key< double >("element", violation));
                        double expected(decode_value_by_key< double >(keyword.c_str(), schema));
                        message += to_string(actual);
                        message += " is bigger than ";
                        message += to_string(expected);
                    }
                    message += ".";

                } else if(keyword == "minLength") {
                    int64_t actual(element_value.GetStringLength());
                    int64_t expected(decode_value_by_key< int64_t >(keyword.c_str(), schema));
                    message += "Expected at least ";
                    message += to_string(expected);
                    if(expected > 1) {
                        message += " characters";
                    } else {
                        message += " character";
                    }
                    message += " in the string but found ";
                    message += to_string(actual);
                    message += ".";

                } else if(keyword == "maxLength") {
                    int64_t actual(element_value.GetStringLength());
                    int64_t expected(decode_value_by_key< int64_t >(keyword.c_str(), schema));
                    message += "Expected at most ";
                    message += to_string(expected);
                    if(expected > 1) {
                        message += " characters";
                    } else {
                        message += " character";
                    }
                    message += " in the string but found ";
                    message += to_string(actual);
                    message += ".";

                } else if(keyword == "minItems") {
                    int64_t actual(element_value.Size());
                    int64_t expected(decode_value_by_key< int64_t >(keyword.c_str(), schema));
                    message += "Expected at least ";
                    message += to_string(expected);
                    if(expected > 1) {
                        message += " items";
                    } else {
                        message += " item";
                    }
                    message += " in the array but found ";
                    message += to_string(actual);
                    message += ".";

                } else if(keyword == "maxItems") {
                    int64_t actual(element_value.Size());
                    int64_t expected(decode_value_by_key< int64_t >(keyword.c_str(), schema));
                    message += "Expected no more than ";
                    message += to_string(expected);
                    if(expected > 1) {
                        message += " items";
                    } else {
                        message += " item";
                    }
                    message += " in array but found ";
                    message += to_string(actual);
                    message += ".";

                } else {
                    message += " uknown.";
                }

                reference = schema.FindMember("examples");
                if(reference != ontology.MemberEnd()) {
                    if(reference->value.IsArray() && !reference->value.Empty()) {
                        example += "\nExample:";
                        for(auto& e : reference->value.GetArray()) {
                            example += "\n";
                            StringBuffer string_buffer;
                            PrettyWriter< StringBuffer > writer(string_buffer);
                            e.Accept(writer);
                            example += string(string_buffer.GetString());
                        }
                    }
                }

            } catch(ConfigurationError& error) {
                // too bad...
            }

            message += "\nPath in document: ";
            message += invalid_element_path;

            string url;
            if(decode_value_by_key< string >("url", url, ontology)) {
                message += "\nDocument URL: ";
                message += url;
            }

            if(!example.empty()) {
                message += example;
            }
        } catch(ConfigurationError& error) {
            message += " : unknown JSON schema violation";
        }
    } else {
        message += " : unknown JSON schema violation";
    }
};
ostream& ValidationError::describe(ostream& o) const {
    o << message << endl;
    return o;
};

void merge_json_value(const Value& base, Value& ontology, Document& document) {
    if(!base.IsNull()) {
        if(!ontology.IsNull()) {
            if(base.IsObject()) {
                if(ontology.IsObject()) {
                    for(auto& element : base.GetObject()) {
                        Value::MemberIterator reference = ontology.FindMember(element.name);
                        if(reference != ontology.MemberEnd()) {
                            try {
                                merge_json_value(element.value, reference->value, document);
                            } catch(ConfigurationError& error) {
                                throw ConfigurationError(string(element.name.GetString(), element.name.GetStringLength()) + " " + error.message);
                            }
                        } else {
                            Value key(element.name, document.GetAllocator());
                            Value value(element.value, document.GetAllocator());
                            ontology.AddMember(key.Move(), value.Move(), document.GetAllocator());
                        }
                    }
                } else { throw ConfigurationError("element is not a dictionary"); }
            }
        } else { ontology.CopyFrom(base, document.GetAllocator()); }
    }
};
void project_json_value(const Value& base, const Value& ontology, Value& container, Document& document) {
    container.SetNull();
    if(!base.IsNull() && !ontology.IsNull()) {
        if(base.IsObject()) {
            if(ontology.IsObject()) {
                container.SetObject();
                for(auto& record : base.GetObject()) {
                    Value child;
                    Value::ConstMemberIterator element = ontology.FindMember(record.name);
                    if(element != ontology.MemberEnd()) {
                        project_json_value(record.value, element->value, child, document);
                    } else {
                        child.CopyFrom(record.value, document.GetAllocator());
                    }
                    container.AddMember(Value(record.name, document.GetAllocator()).Move(), child.Move(), document.GetAllocator());
                }
            } else if(ontology.IsArray()) {
                container.SetArray();
                for(const auto& element : ontology.GetArray()) {
                    Value child;
                    project_json_value(base, element, child, document);
                    container.PushBack(child.Move(), document.GetAllocator());
                }
            }
        }
    }
    if(!ontology.IsNull() && container.IsNull()) {
        container.CopyFrom(ontology, document.GetAllocator());
    }
};
void clean_json_value(Value& ontology, Document& document) {
    switch (ontology.GetType()) {
        case Type::kNullType:
        case Type::kTrueType:
        case Type::kNumberType: {
            break;
        };
        case Type::kFalseType: {
            ontology.SetNull();
            break;
        };
        case Type::kObjectType: {
            Value clean(kObjectType);
            for(auto& record : ontology.GetObject()) {
                clean_json_value(record.value, document);
                if(!record.value.IsNull()) {
                    clean.AddMember(record.name.Move(), record.value.Move(), document.GetAllocator());
                }
            }
            if(clean.ObjectEmpty()) { clean.SetNull(); }
            ontology.Swap(clean);
            break;
        };
        case Type::kArrayType: {
            Value clean(kArrayType);
            for(auto& element : ontology.GetArray()) {
                clean_json_value(element, document);
                if(!element.IsNull()) {
                    clean.PushBack(element.Move(), document.GetAllocator());
                }
            }
            if(clean.Empty()) { clean.SetNull(); }
            ontology.Swap(clean);
            break;
        };
        case Type::kStringType: {
            if(ontology.GetStringLength() < 1) { ontology.SetNull(); }
            break;
        };
    }
};
void sort_json_value(Value& ontology, Document& document) {
    if(ontology.IsObject()) {
        map< string, Value* > dictionary;
        for(auto& record : ontology.GetObject()) {
            sort_json_value(record.value, document);
            dictionary.emplace(make_pair(string(record.name.GetString(), record.name.GetStringLength()), &record.value));
        }
        Value sorted(kObjectType);
        for(auto& record : dictionary) {
            sorted.AddMember(Value(record.first.c_str(), record.first.size(), document.GetAllocator()).Move(), record.second->Move(), document.GetAllocator());
        }
        ontology.Swap(sorted);

    } else if(ontology.IsArray()) {
        for(auto& element : ontology.GetArray()) {
            sort_json_value(element, document);
        }
    }
};
void clean_json_object(Value& ontology, Document& document) {
    clean_json_value(ontology, document);
    if(ontology.IsNull()) {
        ontology.SetObject();
    }
};
void overlay_json_object(Document& ontology, const Value& overlay) {
    if(!overlay.IsNull()) {
        if(overlay.IsObject()) {
            if(!overlay.ObjectEmpty()) {
                Document overlaid;
                overlaid.CopyFrom(overlay, overlaid.GetAllocator());
                merge_json_value(ontology, overlaid, overlaid);
                ontology.Swap(overlaid);
            }
        } else { throw ConfigurationError("Overlay ontology root must be a dictionary"); }
    }
};
bool remove_disabled_from_json_value(Value& ontology, Document& document) {
    if(ontology.IsObject()) {
        if(!decode_value_by_key< bool >("disabled", ontology)) {
            Value clean(kObjectType);
            for(auto& record : ontology.GetObject()) {
                if(!remove_disabled_from_json_value(record.value, document)) {
                    clean.AddMember(record.name.Move(), record.value.Move(), document.GetAllocator());
                }
            }
            ontology.Swap(clean);
            return ontology.ObjectEmpty();
        } else { return true; }

    } else if(ontology.IsArray()) {
        Value clean(kArrayType);
        for(auto& element : ontology.GetArray()) {
            if(!remove_disabled_from_json_value(element, document)) {
                clean.PushBack(element.Move(), document.GetAllocator());
            }
        }
        ontology.Swap(clean);
        return ontology.Empty();

    } else { return false; }
};
/*
    switch (value.GetType()) {
        case Type::kNullType: {
            break;
        };
        case Type::kFalseType: {
            break;
        };
        case Type::kTrueType: {
            break;
        };
        case Type::kObjectType: {
            break;
        };
        case Type::kArrayType: {
            break;
        };
        case Type::kStringType: {
            break;
        };
        case Type::kNumberType: {
            break;
        };
        default:
            break;
    }
*/

/*
Value project_json_on_namespace(const Value& ontology, Document& document, const string& uri, const unordered_map< string, Value >& namespace_by_uri) {
    auto ns_record = namespace_by_uri.find(uri);
    if(ns_record == namespace_by_uri.end()) {
        const Value& ns(ns_record->second);

        Value projection(kObjectType);
        Value projection(kNullType);
        if(!ontology.IsNull()) {
            Value::ConstMemberIterator reference = ns.FindMember("element");
            if(reference != ns.MemberEnd()) {
                const Value& ns_element_dictionary(reference->value);
                for(auto& ns_element_record : ns_element_dictionary.GetObject()) {
                    string name(ns_element_record.name.GetString(), ns_element_record.name.GetStringLength());
                    string type(decode_value_by_key< string >("type", ns_element_record.value));
                    string plural(decode_value_by_key< bool >("plural", ns_element_record.value));

                    if(!ontology.IsNull()) {
                        reference = ontology.FindMember(ns_element_record.name);
                        if(reference != ontology.MemberEnd()) {
                            Value key(ns_element_record.name.GetString(), ns_element_record.name.GetStringLength(), document.GetAllocator());
                            if(type == "object") {
                                string object_namespace_uri(decode_value_by_key< string >("namespace", ns_element_record.value));
                                Value value(project_json_on_namespace(reference->value, document, object_namespace_uri, namespace_by_uri));
                                projection.AddMember(key.Move(), value.Move(), document.GetAllocator());
                            } else {
                                if(!reference->value.IsNull()) {
                                    Value value(reference->value, document.GetAllocator());
                                    projection.AddMember(key.Move(), value.Move(), document.GetAllocator());
                                } else {
                                    reference = ns_element_record.value.FindMember("default");
                                    if(reference != ns_element_record.value.MemberEnd()) {
                                        Value value(reference->value, document.GetAllocator());
                                        projection.AddMember(key.Move(), value.Move(), document.GetAllocator());
                                    }
                                }
                            }
                        } else {
                            reference = ns_element_record.value.FindMember("default");
                            if(reference != ns_element_record.value.MemberEnd()) {
                                Value value(reference->value, document.GetAllocator());
                                projection.AddMember(key.Move(), value.Move(), document.GetAllocator());
                            }
                        }
                    }
                }
            }
        }
    }
};
*/
