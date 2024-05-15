#include <regex>

#include "bencode.h"
#include "log.h"

BItemVisitor::BItemVisitor() = default;
BItemVisitor::~BItemVisitor() = default;

BItem::BItem() = default;
BItem::~BItem() = default;

BInteger::BInteger(ValueType value) : _value(value)
{
}

std::unique_ptr<BInteger> BInteger::create(ValueType value)
{
    return std::unique_ptr<BInteger>(new BInteger(value));
}

auto BInteger::value() const -> ValueType
{
    return _value;
}

void BInteger::setValue(ValueType value)
{
    _value = value;
}

void BInteger::accept(BItemVisitor *visitor)
{
    visitor->visit(this);
}

BString::BString(ValueType value) : _value(value)
{
}

std::unique_ptr<BString> BString::create(ValueType value)
{
    return std::unique_ptr<BString>(new BString(value));
}

auto BString::value() const -> ValueType
{
    return _value;
}

void BString::setValue(ValueType value)
{
    _value = value;
}

auto BString::length() const -> ValueType::size_type
{
    return _value.length();
}

void BString::accept(BItemVisitor *visitor)
{
    visitor->visit(this);
}

bool BDictionary::BStringByValueComparator::operator()(const std::shared_ptr<BString> &lhs,
                                                       const std::shared_ptr<BString> &rhs) const
{
    return lhs->value() < rhs->value();
}

BDictionary::BDictionary() = default;

BDictionary::BDictionary(std::initializer_list<value_type> items) : itemMap(items)
{
}

std::unique_ptr<BDictionary> BDictionary::create()
{
    return std::unique_ptr<BDictionary>(new BDictionary());
}

std::unique_ptr<BDictionary> BDictionary::create(std::initializer_list<value_type> items)
{
    return std::unique_ptr<BDictionary>(new BDictionary(items));
}

BDictionary::size_type BDictionary::size() const
{
    return itemMap.size();
}

bool BDictionary::empty() const
{
    return itemMap.empty();
}

BDictionary::mapped_type &BDictionary::operator[](const key_type &key)
{
    return itemMap[key];
}

BDictionary::iterator BDictionary::begin()
{
    return itemMap.begin();
}

BDictionary::iterator BDictionary::end()
{
    return itemMap.end();
}

BDictionary::const_iterator BDictionary::begin() const
{
    return itemMap.begin();
}

BDictionary::const_iterator BDictionary::end() const
{
    return itemMap.end();
}

BDictionary::const_iterator BDictionary::cbegin() const
{
    return itemMap.cbegin();
}

BDictionary::const_iterator BDictionary::cend() const
{
    return itemMap.cend();
}

void BDictionary::accept(BItemVisitor *visitor)
{
    visitor->visit(this);
}

BList::BList() = default;

BList::BList(std::initializer_list<value_type> items) : itemList(items)
{
}

std::unique_ptr<BList> BList::create()
{
    return std::unique_ptr<BList>(new BList());
}

std::unique_ptr<BList> BList::create(std::initializer_list<value_type> items)
{
    return std::unique_ptr<BList>(new BList(items));
}

BList::size_type BList::size() const
{
    return itemList.size();
}

bool BList::empty() const
{
    return itemList.empty();
}

void BList::push_back(const value_type &bItem)
{
    assert(bItem && "cannot add a null item to the list");

    itemList.push_back(bItem);
}

void BList::pop_back()
{
    assert(!empty() && "cannot call pop_back() on an empty list");

    itemList.pop_back();
}

BList::reference BList::front()
{
    assert(!empty() && "cannot call front() on an empty list");

    return itemList.front();
}

BList::const_reference BList::front() const
{
    assert(!empty() && "cannot call front() on an empty list");

    return itemList.front();
}

BList::reference BList::back()
{
    assert(!empty() && "cannot call back() on an empty list");

    return itemList.back();
}

BList::const_reference BList::back() const
{
    assert(!empty() && "cannot call back() on an empty list");

    return itemList.back();
}

BList::iterator BList::begin()
{
    return itemList.begin();
}

BList::iterator BList::end()
{
    return itemList.end();
}

BList::const_iterator BList::begin() const
{
    return itemList.begin();
}

BList::const_iterator BList::end() const
{
    return itemList.end();
}

BList::const_iterator BList::cbegin() const
{
    return itemList.cbegin();
}

BList::const_iterator BList::cend() const
{
    return itemList.cend();
}

void BList::accept(BItemVisitor *visitor)
{
    visitor->visit(this);
}

std::shared_ptr<BItem> BList::getValue(const std::string &key)
{
    for (const auto &item : *this)
    {
        if (typeid(*item) == typeid(BDictionary))
        {
            std::shared_ptr<BDictionary> subDictionary = std::dynamic_pointer_cast<BDictionary>(item);
            auto potentialValue = subDictionary->getValue(key);
            if (potentialValue)
                return potentialValue;
        }
    }
    return std::shared_ptr<BItem>();
}

std::shared_ptr<BItem> BDictionary::getValue(const std::string &key)
{
    for (const auto &item : *this)
    {
        if (item.first->value() == key)
        {
            return item.second;
        }

        if (typeid(*item.second) == typeid(BDictionary))
        {
            std::shared_ptr<BDictionary> subDictionary = std::dynamic_pointer_cast<BDictionary>(item.second);
            auto potentialValue = subDictionary->getValue(key);
            if (potentialValue)
            {
                return potentialValue;
            }
        }
        else if (typeid(*item.second) == typeid(BList))
        {
            std::shared_ptr<BList> subList = std::dynamic_pointer_cast<BList>(item.second);
            auto potentialValue = subList->getValue(key);
            if (potentialValue)
            {
                return potentialValue;
            }
        }
    }
    return std::shared_ptr<BItem>();
}

bool readUpTo(std::istream &stream, std::string &readData, char sentinel)
{
    // Do not use std::getline() because it eats the sentinel from the stream.
    while (stream.peek() != std::char_traits<char>::eof() && stream.peek() != sentinel)
    {
        readData += stream.get();
    }
    return stream && stream.peek() == sentinel;
}
bool readUntil(std::istream &stream, std::string &readData, char last)
{
    char c;
    while (stream.get(c))
    {
        readData += c;
        if (c == last)
        {
            return true;
        }
    }
    return false;
}

std::string replace(const std::string &str, char what, const std::string &withWhat)
{
    std::string result;
    for (auto c : str)
    {
        if (c == what)
        {
            result += withWhat;
        }
        else
        {
            result += c;
        }
    }
    return result;
}

PrettyPrinter::PrettyPrinter() = default;

std::unique_ptr<PrettyPrinter> PrettyPrinter::create()
{
    return std::unique_ptr<PrettyPrinter>(new PrettyPrinter());
}

std::string PrettyPrinter::getPrettyRepr(std::shared_ptr<BItem> data, const std::string &indent)
{
    prettyRepr.clear();
    indentLevel = indent;
    currentIndent.clear();
    data->accept(this);
    return prettyRepr;
}

void PrettyPrinter::storeCurrentIndent()
{
    prettyRepr += currentIndent;
}

void PrettyPrinter::increaseIndentLevel()
{
    currentIndent += indentLevel;
}

void PrettyPrinter::decreaseIndentLevel()
{
    currentIndent = currentIndent.substr(0, currentIndent.size() - indentLevel.size());
}

void PrettyPrinter::visit(BDictionary *bDictionary)
{
    prettyRepr += "{\n";
    increaseIndentLevel();
    bool putComma = false;
    for (auto &item : *bDictionary)
    {
        if (putComma)
        {
            prettyRepr += ",\n";
        }
        storeCurrentIndent();
        item.first->accept(this);
        prettyRepr += ": ";
        item.second->accept(this);
        putComma = true;
    }
    if (!bDictionary->empty())
    {
        prettyRepr += "\n";
    }
    decreaseIndentLevel();
    storeCurrentIndent();
    prettyRepr += "}";
}

void PrettyPrinter::visit(BInteger *bInteger)
{
    prettyRepr += std::to_string(bInteger->value());
}

void PrettyPrinter::visit(BList *bList)
{

    prettyRepr += "[\n";
    increaseIndentLevel();
    bool putComma = false;
    for (auto bItem : *bList)
    {
        if (putComma)
        {
            prettyRepr += ",\n";
        }
        storeCurrentIndent();
        bItem->accept(this);
        putComma = true;
    }
    if (!bList->empty())
    {
        prettyRepr += "\n";
    }
    decreaseIndentLevel();
    storeCurrentIndent();
    prettyRepr += "]";
}

void PrettyPrinter::visit(BString *bString)
{
    prettyRepr += '"' + replace(bString->value(), '"', std::string(R"(\")")) + '"';
}

std::string getPrettyRepr(std::shared_ptr<BItem> data, const std::string &indent)
{
    auto prettyPrinter = PrettyPrinter::create();
    return prettyPrinter->getPrettyRepr(data, indent);
}

Encoder::Encoder()
{
}

std::unique_ptr<Encoder> Encoder::create()
{
    return std::unique_ptr<Encoder>(new Encoder());
}

std::string Encoder::encode(std::shared_ptr<BItem> data)
{
    data->accept(this);
    return encodedData;
}

void Encoder::visit(BDictionary *bDictionary)
{
    encodedData += "d";
    for (auto item : *bDictionary)
    {
        item.first->accept(this);
        item.second->accept(this);
    }
    encodedData += "e";
}

void Encoder::visit(BInteger *bInteger)
{
    std::string encodedInteger("i" + std::to_string(bInteger->value()) + "e");
    encodedData += encodedInteger;
}

void Encoder::visit(BList *bList)
{
    encodedData += "l";
    for (auto bItem : *bList)
    {
        bItem->accept(this);
    }
    encodedData += "e";
}

void Encoder::visit(BString *bString)
{
    std::string encodedString(std::to_string(bString->length()) + ":" + bString->value());
    encodedData += encodedString;
}

std::string encode(std::shared_ptr<BItem> data)
{
    auto encoder = Encoder::create();
    return encoder->encode(data);
}

DecodingError::DecodingError(const std::string &what) : std::runtime_error(what)
{
}

Decoder::Decoder()
{
}

std::unique_ptr<Decoder> Decoder::create()
{
    return std::unique_ptr<Decoder>(new Decoder());
}
std::unique_ptr<BItem> Decoder::decode(const std::string &data)
{
    std::istringstream input(data);
    auto decodedData = decode(input);
    validateInputDoesNotContainUndecodedCharacters(input);
    return decodedData;
}

std::unique_ptr<BItem> Decoder::decode(std::istream &input)
{
    switch (input.peek())
    {
    case 'd':
        return decodeDictionary(input);
    case 'i':
        return decodeInteger(input);
    case 'l':
        return decodeList(input);
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return decodeString(input);
    default:
        throw DecodingError(std::string("unexpected character: '") + static_cast<char>(input.peek()) + "'");
    }

    assert(false && "should never happen");
    return std::unique_ptr<BItem>();
}

void Decoder::readExpectedChar(std::istream &input, char expected_char) const
{
    int c = input.get();
    if (c != expected_char)
    {
        throw DecodingError(std::string("expected '") + expected_char + "', got '" + static_cast<char>(c) + "'");
    }
}

std::unique_ptr<BDictionary> Decoder::decodeDictionary(std::istream &input)
{
    readExpectedChar(input, 'd');
    auto bDictionary = decodeDictionaryItemsIntoDictionary(input);
    readExpectedChar(input, 'e');
    return bDictionary;
}

std::unique_ptr<BDictionary> Decoder::decodeDictionaryItemsIntoDictionary(std::istream &input)
{
    auto bDictionary = BDictionary::create();
    while (input && input.peek() != 'e')
    {
        std::shared_ptr<BString> key(decodeDictionaryKey(input));
        std::shared_ptr<BItem> value(decodeDictionaryValue(input));
        (*bDictionary)[key] = value;
    }
    return bDictionary;
}

std::shared_ptr<BString> Decoder::decodeDictionaryKey(std::istream &input)
{
    std::shared_ptr<BItem> key(decode(input));
    // A dictionary key has to be a string.
    std::shared_ptr<BString> keyAsBString(key->as<BString>());
    if (!keyAsBString)
    {
        throw DecodingError("found a dictionary key that is not a bencoded string");
    }
    return keyAsBString;
}

std::unique_ptr<BItem> Decoder::decodeDictionaryValue(std::istream &input)
{
    return decode(input);
}
std::unique_ptr<BInteger> Decoder::decodeInteger(std::istream &input) const
{
    return decodeEncodedInteger(readEncodedInteger(input));
}
std::string Decoder::readEncodedInteger(std::istream &input) const
{
    std::string encodedInteger;
    bool encodedIntegerReadCorrectly = readUntil(input, encodedInteger, 'e');
    if (!encodedIntegerReadCorrectly)
    {
        throw DecodingError("error during the decoding of an integer near '" + encodedInteger + "'");
    }

    return encodedInteger;
}

std::unique_ptr<BInteger> Decoder::decodeEncodedInteger(const std::string &encodedInteger) const
{
    std::regex integerRegex("i([-+]?(0|[1-9][0-9]*))e");
    std::smatch match;
    bool valid = std::regex_match(encodedInteger, match, integerRegex);
    if (!valid)
    {
        throw DecodingError("encountered an encoded integer of invalid format: '" + encodedInteger + "'");
    }

    BInteger::ValueType integerValue;
    strToNum(match[1].str(), integerValue);
    return BInteger::create(integerValue);
}
std::unique_ptr<BList> Decoder::decodeList(std::istream &input)
{
    readExpectedChar(input, 'l');
    auto bList = decodeListItemsIntoList(input);
    readExpectedChar(input, 'e');
    return bList;
}

std::unique_ptr<BList> Decoder::decodeListItemsIntoList(std::istream &input)
{
    auto bList = BList::create();
    while (input && input.peek() != 'e')
    {
        bList->push_back(decode(input));
    }
    return bList;
}
std::unique_ptr<BString> Decoder::decodeString(std::istream &input) const
{
    std::string::size_type stringLength(readStringLength(input));
    readExpectedChar(input, ':');
    std::string str(readStringOfGivenLength(input, stringLength));
    return BString::create(str);
}

std::string::size_type Decoder::readStringLength(std::istream &input) const
{
    std::string stringLengthInASCII;
    bool stringLengthInASCIIReadCorrectly = readUpTo(input, stringLengthInASCII, ':');
    if (!stringLengthInASCIIReadCorrectly)
    {
        throw DecodingError("error during the decoding of a string near '" + stringLengthInASCII + "'");
    }

    std::string::size_type stringLength;
    bool stringLengthIsValid = strToNum(stringLengthInASCII, stringLength);
    if (!stringLengthIsValid)
    {
        throw DecodingError("invalid string length: '" + stringLengthInASCII + "'");
    }

    return stringLength;
}

std::string Decoder::readStringOfGivenLength(std::istream &input, std::string::size_type length) const
{
    std::string str(length, char());
    input.read(&str[0], length);
    std::string::size_type numOfReadChars(input.gcount());
    if (numOfReadChars != length)
    {
        throw DecodingError("expected a string containing " + std::to_string(length) + " characters, but read only " +
                            std::to_string(numOfReadChars) + " characters");
    }
    return str;
}

void Decoder::validateInputDoesNotContainUndecodedCharacters(std::istream &input)
{
    if (input.peek() != std::char_traits<char>::eof())
    {
        throw DecodingError("input contains undecoded characters");
    }
}
std::unique_ptr<BItem> decode(const std::string &data)
{
    auto decoder = Decoder::create();
    return decoder->decode(data);
}

std::unique_ptr<BItem> decode(std::istream &input)
{
    auto decoder = Decoder::create();
    return decoder->decode(input);
}
