#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <unistd.h>

#include "piece.h"
#include "piecemanager.h"
#include "utils.h"

#define BLOCK_SIZE 16384            // Размер блока (2 ^ 14)
#define MAX_PENDING_TIME 5          // Максимальное время ожидания блока (5 секунд)
#define PROGRESS_BAR_WIDTH 40       // Ширина полосы прогресса
#define PROGRESS_DISPLAY_INTERVAL 1 // Интервал отображения прогресса (0.5 секунд)

PieceManager::PieceManager(const TorrentFile &fileParser, const std::string &downloadPath, const int maximumConnections)
    : fileParser(fileParser), maximumConnections(maximumConnections), pieceLength(fileParser.getPieceLength())
{
    missingPieces = initiatePieces();
    downloadedFile.open(downloadPath, std::ios::binary | std::ios::out);
    downloadedFile.seekp(fileParser.getFileSize() - 1);
    downloadedFile.write("", 1);

    startingTime = std::time(nullptr);
    std::thread progressThread([this] { this->trackProgress(); });
    progressThread.detach();
}

PieceManager::~PieceManager()
{
    for (Piece *piece : missingPieces)
    {
        delete piece;
    }

    for (Piece *piece : ongoingPieces)
    {
        delete piece;
    }

    for (PendingRequest *pending : pendingRequests)
    {
        delete pending;
    }

    downloadedFile.close();
}

std::vector<Piece *> PieceManager::initiatePieces()
{
    std::vector<std::string> pieceHashes = fileParser.splitPieceHashes();
    totalPieces = pieceHashes.size();
    std::vector<Piece *> torrentPieces;
    missingPieces.reserve(totalPieces);

    long totalLength = fileParser.getFileSize();

    int blockCount = ceil(pieceLength / BLOCK_SIZE);
    long remLength = pieceLength;

    for (int i = 0; i < totalPieces; i++)
    {
        if (i == totalPieces - 1)
        {
            remLength = totalLength % pieceLength;
            blockCount = std::max((int)ceil(remLength / BLOCK_SIZE), 1);
        }
        std::vector<Block *> blocks;
        blocks.reserve(blockCount);

        for (int offset = 0; offset < blockCount; offset++)
        {
            auto block = new Block;
            block->piece = i;
            block->status = Missing;
            block->offset = offset * BLOCK_SIZE;
            int blockSize = BLOCK_SIZE;
            if (i == totalPieces - 1 && offset == blockCount - 1)
            {
                blockSize = remLength % BLOCK_SIZE;
            }
            block->length = blockSize;
            blocks.push_back(block);
        }
        auto piece = new Piece(i, blocks, pieceHashes[i]);
        torrentPieces.emplace_back(piece);
    }
    return torrentPieces;
}

bool PieceManager::isComplete()
{
    lock.lock();
    bool isComplete = havePieces.size() == totalPieces;
    lock.unlock();
    return isComplete;
}

void PieceManager::addPeer(const std::string &peerId, std::string bitField)
{
    lock.lock();
    peers[peerId] = bitField;
    lock.unlock();
}

void PieceManager::updatePeer(const std::string &peerId, int index)
{
    lock.lock();
    if (peers.find(peerId) != peers.end())
    {
        setPiece(peers[peerId], index);
        lock.unlock();
    }
    else
    {
        lock.unlock();
        throw std::runtime_error("Не соединился с " + peerId);
    }
}

void PieceManager::removePeer(const std::string &peerId)
{
    if (isComplete())
        return;
    lock.lock();
    auto iter = peers.find(peerId);
    if (iter != peers.end())
    {
        peers.erase(iter);
        lock.unlock();
    }
    else
    {
        lock.unlock();
        throw std::runtime_error("Удаление пира " + peerId + " коннект не настреон.");
    }
}

Block *PieceManager::nextRequest(std::string peerId)
{
    lock.lock();
    if (missingPieces.empty())
    {
        lock.unlock();
        return nullptr;
    }

    if (peers.find(peerId) == peers.end())
    {
        lock.unlock();
        return nullptr;
    }

    Block *block = expiredRequest(peerId);
    if (!block)
    {
        block = nextOngoing(peerId);
        if (!block)
            block = getRarestPiece(peerId)->nextRequest();
    }
    lock.unlock();

    return block;
}

Block *PieceManager::expiredRequest(std::string peerId)
{
    time_t currentTime = std::time(nullptr);
    for (PendingRequest *pending : pendingRequests)
    {
        if (hasPiece(peers[peerId], pending->block->piece))
        {
            auto diff = std::difftime(currentTime, pending->timestamp);
            if (diff >= MAX_PENDING_TIME)
            {
                pending->timestamp = currentTime;
                return pending->block;
            }
        }
    }
    return nullptr;
}

Block *PieceManager::nextOngoing(std::string peerId)
{
    for (Piece *piece : ongoingPieces)
    {
        if (hasPiece(peers[peerId], piece->index))
        {
            Block *block = piece->nextRequest();
            if (block)
            {
                auto currentTime = std::time(nullptr);
                auto newPendingRequest = new PendingRequest;
                newPendingRequest->block = block;
                newPendingRequest->timestamp = std::time(nullptr);
                pendingRequests.push_back(newPendingRequest);
                return block;
            }
        }
    }
    return nullptr;
}

Piece *PieceManager::getRarestPiece(std::string peerId)
{
    auto comp = [](const Piece *a, const Piece *b) { return a->index < b->index; };
    std::map<Piece *, int, decltype(comp)> pieceCount(comp);
    for (Piece *piece : missingPieces)
    {
        if (peers.find(peerId) != peers.end())
        {
            if (hasPiece(peers[peerId], piece->index))
                pieceCount[piece] += 1;
        }
    }

    Piece *rarest;
    int leastCount = INT16_MAX;
    for (auto const &[piece, count] : pieceCount)
    {
        if (count < leastCount)
        {
            leastCount = count;
            rarest = piece;
        }
    }

    missingPieces.erase(std::remove(missingPieces.begin(), missingPieces.end(), rarest), missingPieces.end());
    ongoingPieces.push_back(rarest);
    return rarest;
}

void PieceManager::blockReceived(std::string peerId, int pieceIndex, int blockOffset, std::string data)
{
    PendingRequest *requestToRemove = nullptr;
    lock.lock();
    for (PendingRequest *pending : pendingRequests)
    {
        if (pending->block->piece == pieceIndex && pending->block->offset == blockOffset)
        {
            requestToRemove = pending;
            break;
        }
    }

    pendingRequests.erase(std::remove(pendingRequests.begin(), pendingRequests.end(), requestToRemove),
                          pendingRequests.end());

    Piece *targetPiece = nullptr;
    for (Piece *piece : ongoingPieces)
    {
        if (piece->index == pieceIndex)
        {
            targetPiece = piece;
            break;
        }
    }
    lock.unlock();
    if (!targetPiece)
        throw std::runtime_error("Отстуствие куска");

    targetPiece->blockReceived(blockOffset, std::move(data));
    if (targetPiece->isComplete())
    {
        if (targetPiece->isHashMatching())
        {
            write(targetPiece);
            lock.lock();
            ongoingPieces.erase(std::remove(ongoingPieces.begin(), ongoingPieces.end(), targetPiece),
                                ongoingPieces.end());
            havePieces.push_back(targetPiece);
            piecesDownloadedInInterval++;
            lock.unlock();
        }
        else
        {
            targetPiece->reset();
        }
    }
}

void PieceManager::write(Piece *piece)
{
    long position = piece->index * fileParser.getPieceLength();
    downloadedFile.seekp(position);
    downloadedFile << piece->getData();
}

unsigned long PieceManager::bytesDownloaded()
{
    lock.lock();
    unsigned long bytesDownloaded = havePieces.size() * pieceLength;
    lock.unlock();
    return bytesDownloaded;
}

void PieceManager::trackProgress()
{
    usleep(pow(10, 6));
    while (!isComplete())
    {
        displayProgressBar();
        piecesDownloadedInInterval = 0;
        usleep(PROGRESS_DISPLAY_INTERVAL * pow(10, 6));
    }
}

void PieceManager::displayProgressBar()
{
    std::stringstream info;
    lock.lock();
    unsigned long downloadedPieces = havePieces.size();
    unsigned long downloadedLength = pieceLength * piecesDownloadedInInterval;

    double avgDownloadSpeed = (double)downloadedLength / (double)PROGRESS_DISPLAY_INTERVAL;
    double avgDownloadSpeedInMBS = avgDownloadSpeed / pow(10, 6);

    info << "[Peers: " + std::to_string(peers.size()) + "/" + std::to_string(maximumConnections) + ", ";
    info << std::fixed << std::setprecision(2) << avgDownloadSpeedInMBS << " MB/s, ";

    double timePerPiece = (double)PROGRESS_DISPLAY_INTERVAL / (double)piecesDownloadedInInterval;
    long remainingTime = ceil(timePerPiece * (totalPieces - downloadedPieces));
    info << "ВРЕМЯ: " << formatTime(remainingTime) << "]";

    double progress = (double)downloadedPieces / (double)totalPieces;
    int pos = PROGRESS_BAR_WIDTH * progress;
    info << "[";
    for (int i = 0; i < PROGRESS_BAR_WIDTH; i++)
    {
        if (i < pos)
        {
            info << "=";
        }
        else if (i == pos)
        {
            info << ">";
        }
        else
        {
            info << " ";
        }
    }
    info << "] ";
    info << std::to_string(downloadedPieces) + "/" + std::to_string(totalPieces) + " ";
    info << "[" << std::fixed << std::setprecision(2) << (progress * 100) << "%] ";

    time_t currentTime = std::time(nullptr);
    long timeSinceStart = floor(std::difftime(currentTime, startingTime));

    info << "in " << formatTime(timeSinceStart);
    std::cout << info.str() << "\r";
    std::cout.flush();
    lock.unlock();
    if (isComplete())
    {
        std::cout << std::endl;
    }
}
