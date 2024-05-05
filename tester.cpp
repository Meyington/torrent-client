#include "tester.h"
#include "bencode.h"
#include "piece.h"
#include "sha1.h"
#include <cassert>
#include <iostream>
#include <ostream>
#include <unordered_map>

void runTests()
{
    {
        auto bInt = BInteger::create(42);
        assert(bInt->value() == 42);

        std::string encoded = encode(std::move(bInt));
        auto decoded = decode(encoded);

        auto decodedBInt = dynamic_cast<BInteger *>(decoded.get());
        if (decodedBInt)
        {
            std::cout << "Decoded value of BInteger (Bencode): " << encoded << std::endl;
            std::cout << "Decoded value of BInteger (Text): " << decodedBInt->value() << std::endl;
            assert(decodedBInt->value() == 42);
        }
        else
        {
            std::cerr << "Failed to cast to BInteger." << std::endl;
        }
    }

    {
        auto bString = BString::create("Hello");
        assert(bString->value() == "Hello");
        assert(bString->length() == 5);

        std::string encoded = encode(std::move(bString));
        auto decoded = decode(encoded);

        auto decodedBString = dynamic_cast<BString *>(decoded.get());
        if (decodedBString)
        {
            std::cout << "Decoded value of BString (Bencode): " << encoded << std::endl;
            std::cout << "Decoded value of BString (Text): " << decodedBString->value() << std::endl;
            assert(decodedBString->value() == "Hello");
        }
        else
        {
            std::cerr << "Failed to cast to BString." << std::endl;
        }
    }

    {
        std::list<std::shared_ptr<BItem>> items;
        items.push_back(BInteger::create(42));
        items.push_back(BString::create("Hello"));

        auto bList = BList::create();
        for (auto &item : items)
        {
            bList->push_back(std::move(item));
        }

        std::string encoded = encode(std::move(bList));
        auto decoded = decode(encoded);

        auto decodedBList = dynamic_cast<BList *>(decoded.get());
        if (decodedBList)
        {
            std::cout << "Decoded value of BList (Bencode): " << encoded << std::endl;
            std::cout << "Decoded value of BList (Text): ";
            for (const auto &item : *decodedBList)
            {
                if (auto bInt = dynamic_cast<BInteger *>(item.get()))
                {
                    std::cout << bInt->value() << ", ";
                }
                else if (auto bString = dynamic_cast<BString *>(item.get()))
                {
                    std::cout << bString->value() << ", ";
                }
            }
            std::cout << std::endl;
        }
        else
        {
            std::cerr << "Failed to cast to BList." << std::endl;
        }
    }

    {
        std::unordered_map<std::string, std::shared_ptr<BItem>> items;
        items["int"] = BInteger::create(42);
        items["string"] = BString::create("Hello");

        auto bDict = BDictionary::create();
        for (const auto &item : items)
        {
            (*bDict)[BString::create(item.first)] = item.second;
        }

        std::string encoded = encode(std::move(bDict));
        auto decoded = decode(encoded);

        auto decodedBDict = dynamic_cast<BDictionary *>(decoded.get());
        if (decodedBDict)
        {
            std::cout << "Decoded value of BDict (Bencode): " << encoded << std::endl;
            std::cout << "Decoded value of BDict (Text): ";
            for (const auto &pair : *decodedBDict)
            {
                auto bString = std::dynamic_pointer_cast<BString>(pair.first);
                if (bString)
                {
                    std::cout << bString->value() << ": ";
                }

                if (auto bInt = dynamic_cast<BInteger *>(pair.second.get()))
                {
                    std::cout << bInt->value();
                }
                else if (auto bString = dynamic_cast<BString *>(pair.second.get()))
                {
                    std::cout << bString->value();
                }
                std::cout << ", ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cerr << "Failed to cast to BDict." << std::endl;
        }
    }

    std::cout << "All Bencode tests passed successfully!" << std::endl;
}
void runSHA1()
{
    {
        std::string input = "hello";

        std::string hash = sha1(input);

        std::string expected_hash = "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d";

        std::cout << "Input String: " << input << std::endl;
        std::cout << "Hash: " << hash << std::endl;
        std::cout << "Expected Hash: " << expected_hash << std::endl;

        assert(hash == expected_hash);
    }

    std::cout << "All SHA1 tests passed successfully!" << std::endl;
}

void runPiece()
{
    std::vector<Block *> blocks;
    for (int i = 0; i < 5; ++i)
    {
        Block *block = new Block{i, i * 1000, 1000, BlockStatus::Missing, ""};
        blocks.push_back(block);
    }

    Piece piece(0, blocks, "1234567890ABCDEF");

    // Проверяем, что метод GetIndex() работает корректно
    if (piece.index == 0)
    {
        std::cout << "GetIndex() method works correctly!" << std::endl;
    }
    else
    {
        std::cout << "GetIndex() method is incorrect!" << std::endl;
    }

    Block *nextBlock = piece.nextRequest();
    if (nextBlock != nullptr && nextBlock->status == BlockStatus::Pending)
    {
        std::cout << "nextRequest() method works correctly!" << std::endl;
    }
    else
    {
        std::cout << "nextRequest() method is incorrect!" << std::endl;
    }

    for (Block *block : blocks)
    {
        piece.blockReceived(block->offset, "block_data_" + std::to_string(block->offset));
    }

    if (piece.isComplete())
    {
        std::cout << "isComplete() method works correctly!" << std::endl;
    }
    else
    {
        std::cout << "isComplete() method is incorrect!" << std::endl;
    }

    if (piece.isHashMatching())
    {
        std::cout << "isHashMatching() method works correctly!" << std::endl;
    }
    else
    {
        std::cout << "isHashMatching() method is incorrect!" << std::endl;
    }

    piece.reset();
    if (!piece.isComplete())
    {
        std::cout << "Reset() method works correctly!" << std::endl;
    }
    else
    {
        std::cout << "Reset() method is incorrect!" << std::endl;
    }

    for (auto block : blocks)
    {
        delete block;
    }
}
