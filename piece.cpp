#include "sha1.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "piece.h"
#include "utils.h"

// Конструктор класса Piece
Piece::Piece(int index, std::vector<Block *> blocks, std::string hashValue)
    : index(index), hashValue(std::move(hashValue))
{
    this->blocks = std::move(blocks);
}

// Деструктор класса Piece
Piece::~Piece()
{
    for (Block *block : blocks)
    {
        delete block;
    }
}

// Сбрасывает состояние всех блоков фрагмента на Missing
void Piece::reset()
{
    for (Block *block : blocks)
    {
        block->status = Missing;
    }
}

// Возвращает следующий блок для загрузки (состояние блока меняется на Pending)
Block *Piece::nextRequest()
{
    for (Block *block : blocks)
    {
        if (block->status == Missing)
        {
            block->status = Pending;
            return block;
        }
    }
    return nullptr;
}

// Устанавливает состояние блока в Retrieved и сохраняет полученные данные
void Piece::blockReceived(int offset, std::string data)
{
    for (Block *block : blocks)
    {
        if (block->offset == offset)
        {
            block->status = Retrieved;
            block->data = data;
            return;
        }
    }
    throw std::runtime_error("Trying to complete a non-existing block " + std::to_string(offset) + " in piece " +
                             std::to_string(index));
}

// Проверяет, загружены ли все блоки фрагмента
bool Piece::isComplete()
{
    return std::all_of(blocks.begin(), blocks.end(), [](Block *block) { return block->status == Retrieved; });
}

// Проверяет соответствие хэш-значения данных фрагмента ожидаемому значению
bool Piece::isHashMatching()
{
    std::string pieceHash = hexDecode(sha1(getData()));
    return pieceHash == hashValue;
}

// Получает данные всех блоков фрагмента и объединяет их в одну строку
std::string Piece::getData()
{
    assert(isComplete());
    std::stringstream data;
    for (Block *block : blocks)
    {
        data << block->data;
    }
    return data.str();
}
