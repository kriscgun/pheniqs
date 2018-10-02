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

#ifndef PHENIQS_URL_H
#define PHENIQS_URL_H

#include "include.h"
#include "json.h"

#define STANDARD_STREAM_ALIAS "-"
#define CANONICAL_STDIN_PATH "/dev/stdin"
#define CANONICAL_STDOUT_PATH "/dev/stdout"
#define CANONICAL_STDERR_PATH "/dev/stderr"
#define CANONICAL_NULL_DEVICE_PATH "/dev/null"
const char PATH_SEPARATOR('/');
const char EXTENSION_SEPARATOR('.');

enum class FormatType : uint8_t {
    UNKNOWN,
    NONE,
    FASTQ,
    SAM,
    BAM,
    BAI,
    CRAM,
    CRAI,
    VCF,
    BCF,
    CSI,
    GZI,
    TBI,
    BED,
    JSON,
};
string to_string(const FormatType& value);
bool from_string(const char* value, FormatType& result);
bool from_string(const string& value, FormatType& result);
ostream& operator<<(ostream& o, const FormatType& value);
void encode_key_value(const string& key, const FormatType& value, Value& container, Document& document);
template<> bool decode_value_by_key< FormatType >(const Value::Ch* key, FormatType& value, const Value& container);
template<> FormatType decode_value_by_key< FormatType >(const Value::Ch* key, const Value& container);

enum class FormatCompression : uint8_t {
    UNKNOWN,
    NONE,
    GZIP
};
string to_string(const FormatCompression& value);
bool from_string(const char* value, FormatCompression& result);
bool from_string(const string& value, FormatCompression& result);
ostream& operator<<(ostream& o, const FormatCompression& value);
void encode_key_value(const string& key, const FormatCompression& value, Value& container, Document& document);
template<> bool decode_value_by_key< FormatCompression >(const Value::Ch* key, FormatCompression& value, const Value& container);
template<> FormatCompression decode_value_by_key< FormatCompression >(const Value::Ch* key, const Value& container);

enum class FormatKind : uint8_t {
    UNKNOWN,
    DEV_NULL,
    FASTQ,
    HTS,
};
string to_string(const FormatKind& value);
bool from_string(const char* value, FormatKind& result);
void to_kstring(const FormatKind& value, kstring_t& result);
bool from_string(const string& value, FormatKind& result);
ostream& operator<<(ostream& o, const FormatKind& value);
void encode_key_value(const string& key, const FormatKind& value, Value& container, Document& document);

enum class IoDirection : uint8_t {
    IN,
    OUT,
    UNKNOWN,
};
string to_string(const IoDirection& value);
bool from_string(const char* value, IoDirection& result);
bool from_string(const string& value, IoDirection& result);
ostream& operator<<(ostream& o, const IoDirection& direction);
void encode_key_value(const string& key, const IoDirection& value, Value& container, Document& document);

string& expand_shell(string& expression);
void normalize_standard_stream(string& path, const IoDirection& direction);

class URL {
    friend ostream& operator<<(ostream& o, const URL& url);
    friend bool operator<(const URL& lhs, const URL& rhs);

    public:
        URL();
        URL(const URL& other);
        URL(const string& path);
        void parse_file(const string& path);
        void parse_file(const string& path, const IoDirection& direction);
        void set_basename(const string& name);
        void set_dirname(const string& directory);
        void set_compression(const FormatCompression& compression);
        void set_type(const FormatType type);
        void relocate_child(const URL& base);
        void relocate_sibling(const URL& base);
        inline const string& path() const {
            return _path;
        };
        inline const string& basename() const {
            return _basename;
        };
        inline const string& dirname() const {
            return _dirname;
        };
        inline const string& extension() const {
            return _extension;
        };
        inline const FormatCompression& compression() const {
            return _compression;
        };
        inline const FormatType& type() const {
            return _type;
        };
        inline FormatKind kind() const {
            if(!is_dev_null()) {
                switch(_type) {
                    case FormatType::SAM:
                    case FormatType::BAM:
                    case FormatType::CRAM:
                        return FormatKind::HTS;
                        break;
                    case FormatType::FASTQ:
                        return FormatKind::FASTQ;
                        break;
                    default:
                        return FormatKind::UNKNOWN;
                        break;
                }
            } else { return FormatKind::DEV_NULL; }
        };
        inline bool empty() const {
            return _path.empty();
        };
        inline bool is_file() const {
            return !_basename.empty();
        };
        inline bool is_directory() const {
            return _basename.empty() && !_dirname.empty();
        };
        inline bool is_stdin() const {
            return _path == CANONICAL_STDIN_PATH;
        };
        inline bool is_stdout() const {
            return _path == CANONICAL_STDOUT_PATH;
        };
        inline bool is_stderr() const {
            return _path == CANONICAL_STDERR_PATH;
        };
        inline bool is_dev_null() const {
            return _path == CANONICAL_NULL_DEVICE_PATH;
        };
        inline bool is_standard_stream() const {
            return is_stdin() || is_stdout() || is_stderr() || is_dev_null();
        };
        inline bool is_absolute() const {
            return !_dirname.empty() && _dirname[0] == PATH_SEPARATOR;
        };
        inline const char* const hfile_name() const {
            if(is_file()) {
                if(is_stdout() || is_stdin()) {
                    return STANDARD_STREAM_ALIAS;
                } else if(is_stderr()) {
                    return NULL;

                } else if(is_dev_null()) {
                    return NULL;

                } else {
                    return _path.c_str();
                }
            } else {
                return NULL;
            }
        };
        inline void clear() {
            _path.clear();
            _basename.clear();
            _dirname.clear();
            _extension.clear();
            _compression = FormatCompression::UNKNOWN;
        };
        void expand() {
            expand_shell(_basename);
            expand_shell(_dirname);
            refresh();
        };
        void normalize(const IoDirection& direction) {
            string buffer(_path);
            normalize_standard_stream(buffer, direction);
            parse_file(buffer);
        };
        bool is_readable() const;
        bool is_writable() const;
        const char* const c_str() const;
        const size_t size() const;
        bool operator==(const URL& other) const;
        URL& operator=(const URL& other);
        operator string() const;
        string description() const;

    private:
        string _path;
        string _basename;
        string _dirname;
        string _extension;
        FormatCompression _compression;
        FormatType _type;
        void refresh();
};
bool operator<(const URL& lhs, const URL& rhs);

namespace std {
    template <> struct hash< URL > {
        size_t operator()(const URL& url) const {
            return hash< string >()(url.path());
        };
    };
};

template<> URL decode_value(const Value& container);
template<> URL decode_value_by_key(const Value::Ch* key, const Value& container);
template<> bool decode_value< URL >(URL& value, const Value& container);

template<> list< URL > decode_value_by_key(const Value::Ch* key, const Value& container);
template<> bool decode_value_by_key< URL >(const Value::Ch* key, URL& value, const Value& container);
template<> bool decode_value_by_key< list< URL > >(const Value::Ch* key, list< URL >& value, const Value& container);

void encode_value(const URL& value, Value& container, Document& document);
bool encode_key_value(const string& key, const URL& value, Value& container, Document& document);
bool encode_key_value(const string& key, const list< URL >& value, Value& container, Document& document);

void expand_url_value(Value& container, Document& document, const IoDirection& direction=IoDirection::UNKNOWN);
void expand_url_value_by_key(const Value::Ch* key, Value& container, Document& document, const IoDirection& direction=IoDirection::UNKNOWN);
void expand_url_array_by_key(const Value::Ch* key, Value& container, Document& document, const IoDirection& direction=IoDirection::UNKNOWN);
void relocate_url_array_by_key(const Value::Ch* key, Value& container, Document& document, const URL& base);

#endif /* PHENIQS_URL_H */
