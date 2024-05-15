#ifndef BENCODE_H
#define BENCODE_H

#include <cstdint>
#include <initializer_list>
#include <ios>
#include <istream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>

class BItemVisitor {
    public:
    virtual ~BItemVisitor();

    virtual void visit(class BDictionary *bDictionary) = 0;
    virtual void visit(class BInteger *bInteger) = 0;
    virtual void visit(class BList *blist) = 0;
    virtual void visit(class BString *bString) = 0;

    protected:
    BItemVisitor();
};

class BItem : public std::enable_shared_from_this<BItem> {
    public:
    virtual ~BItem() = 0;
    virtual void accept(BItemVisitor *visitor) = 0;
    template <typename T> std::shared_ptr<T> as()
    {
        static_assert(std::is_base_of<BItem, T>::value, "T has to be a subclass of BItem");

        return std::dynamic_pointer_cast<T>(shared_from_this());
    }

    protected:
    BItem();

    private:
    BItem(const BItem &) = delete;
    BItem &operator=(const BItem &) = delete;
};

class BInteger : public BItem {
    public:
    using ValueType = int64_t;

    static std::unique_ptr<BInteger> create(ValueType value);

    ValueType value() const;
    void setValue(ValueType value);
    virtual void accept(BItemVisitor *visitor) override;

    private:
    explicit BInteger(ValueType value);

    ValueType _value;
};

class BString : public BItem {
    public:
    using ValueType = std::string;

    static std::unique_ptr<BString> create(ValueType value);

    ValueType value() const;
    void setValue(ValueType value);
    ValueType::size_type length() const;

    virtual void accept(BItemVisitor *visitor) override;

    private:
    explicit BString(ValueType value);

    private:
    ValueType _value;
};

class BDictionary : public BItem {
    private:
    class BStringByValueComparator {
        public:
        bool operator()(const std::shared_ptr<BString> &lhs, const std::shared_ptr<BString> &rhs) const;
    };

    using BItemMap = std::map<std::shared_ptr<BString>, std::shared_ptr<BItem>, BStringByValueComparator>;

    public:
    using key_type = BItemMap::key_type;

    using mapped_type = BItemMap::mapped_type;

    using value_type = BItemMap::value_type;

    using size_type = BItemMap::size_type;

    using reference = BItemMap::reference;

    using const_reference = BItemMap::const_reference;

    using iterator = BItemMap::iterator;

    using const_iterator = BItemMap::const_iterator;

    static std::unique_ptr<BDictionary> create();
    static std::unique_ptr<BDictionary> create(std::initializer_list<value_type> items);
    size_type size() const;
    bool empty() const;
    mapped_type &operator[](const key_type &key);

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    virtual void accept(BItemVisitor *visitor) override;
    std::shared_ptr<BItem> getValue(const std::string &key);

    private:
    BDictionary();
    explicit BDictionary(std::initializer_list<value_type> items);

    private:
    BItemMap itemMap;
};

class BList : public BItem {
    private:
    using BItemList = std::list<std::shared_ptr<BItem>>;

    public:
    using value_type = BItemList::value_type;

    using size_type = BItemList::size_type;

    using reference = BItemList::reference;

    using const_reference = BItemList::const_reference;

    using iterator = BItemList::iterator;

    using const_iterator = BItemList::const_iterator;

    static std::unique_ptr<BList> create();
    static std::unique_ptr<BList> create(std::initializer_list<value_type> items);
    size_type size() const;
    bool empty() const;
    void push_back(const value_type &bItem);
    void pop_back();
    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    virtual void accept(BItemVisitor *visitor) override;
    std::shared_ptr<BItem> getValue(const std::string &key);

    private:
    BList();
    explicit BList(std::initializer_list<value_type> items);
    BItemList itemList;
};

template <typename N>
bool strToNum(const std::string &str, N &num, std::ios_base &(*format)(std::ios_base &) = std::dec)
{
    std::istringstream stream(str);
    N convNum = 0;
    stream >> format >> convNum;
    if (!stream.fail() && stream.eof())
    {
        num = convNum;
        return true;
    }
    return false;
}
bool readUpTo(std::istream &stream, std::string &readData, char sentinel);
bool readUntil(std::istream &stream, std::string &readData, char last);

std::string replace(const std::string &str, char what, const std::string &withWhat);
class PrettyPrinter : private BItemVisitor {
    public:
    static std::unique_ptr<PrettyPrinter> create();

    std::string getPrettyRepr(std::shared_ptr<BItem> data, const std::string &indent = "    ");

    private:
    PrettyPrinter();
    virtual void visit(BDictionary *bDictionary) override;
    virtual void visit(BInteger *bInteger) override;
    virtual void visit(BList *bList) override;
    virtual void visit(BString *bString) override;

    void storeCurrentIndent();
    void increaseIndentLevel();
    void decreaseIndentLevel();

    std::string prettyRepr = "";

    std::string indentLevel = "    ";

    std::string currentIndent = "";
};

std::string getPrettyRepr(std::shared_ptr<BItem> data, const std::string &indent = "    ");

class Encoder : private BItemVisitor {
    public:
    static std::unique_ptr<Encoder> create();

    std::string encode(std::shared_ptr<BItem> data);

    private:
    Encoder();
    virtual void visit(BDictionary *bDictionary) override;
    virtual void visit(BInteger *bInteger) override;
    virtual void visit(BList *bList) override;
    virtual void visit(BString *bString) override;

    std::string encodedData;
};

std::string encode(std::shared_ptr<BItem> data);

class DecodingError : public std::runtime_error {
    public:
    explicit DecodingError(const std::string &what);
};
class Decoder {
    public:
    static std::unique_ptr<Decoder> create();

    std::unique_ptr<BItem> decode(const std::string &data);
    std::unique_ptr<BItem> decode(std::istream &input);

    private:
    Decoder();

    void readExpectedChar(std::istream &input, char expected_char) const;
    std::unique_ptr<BDictionary> decodeDictionary(std::istream &input);
    std::unique_ptr<BDictionary> decodeDictionaryItemsIntoDictionary(std::istream &input);
    std::shared_ptr<BString> decodeDictionaryKey(std::istream &input);
    std::unique_ptr<BItem> decodeDictionaryValue(std::istream &input);
    std::unique_ptr<BInteger> decodeInteger(std::istream &input) const;
    std::string readEncodedInteger(std::istream &input) const;
    std::unique_ptr<BInteger> decodeEncodedInteger(const std::string &encodedInteger) const;
    std::unique_ptr<BList> decodeList(std::istream &input);
    std::unique_ptr<BList> decodeListItemsIntoList(std::istream &input);
    std::unique_ptr<BString> decodeString(std::istream &input) const;
    std::string::size_type readStringLength(std::istream &input) const;
    std::string readStringOfGivenLength(std::istream &input, std::string::size_type length) const;

    void validateInputDoesNotContainUndecodedCharacters(std::istream &input);
};
std::unique_ptr<BItem> decode(const std::string &data);
std::unique_ptr<BItem> decode(std::istream &input);

#endif // BENCODE_H
