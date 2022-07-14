/**
 * The board of the game is 8 by 8. Every cell can only hold one piece.
 *
 * @author Vasilis Kyriafinis
 * @date 14/7/2022
 * @version 0.1
 */
#include <stdint.h>


#ifndef BOARD_H
#define BOARD_H

    /**
     * The Board is a set of 12 64bit unsigned integers. Every integer represents one type and one color of a piece and
     * it's position on the board. The least significant bit of each number is the a1 square of the board and the msb is
     * the h8 square in a row major format.
     *
     *         Bit (0 - 63) to square
     *
     *            a    b    c    d    e    f    g    h
     *         8  56   57   58   59   60   61   62   63
     *
     *         7  48   49   50   51   52   53   54   55
     *
     *         6  40   41   42   43   44   45   46   47
     *
     *         5  32   33   34   35   36   37   38   39
     *
     *         4  24   25   26   27   28   29   30   31
     *
     *         3  16   17   18   19   20   21   22   23
     *
     *         2  8    9    10   11   12   13   14   15
     *
     *         1  0    1    2    3    4    5    6    7
     */
    typedef struct{

        // Pawns
        uint64_t white_pawns;
        uint64_t black_pawns;

        // Bishops
        uint64_t white_bishops;
        uint64_t black_bishops;

        // Knights
        uint64_t white_knights;
        uint64_t black_knights;

        // Rooks
        uint64_t white_rooks;
        uint64_t black_rooks;

        // Queens
        uint64_t white_queen;
        uint64_t black_queen;

        // Kings
        uint64_t white_king;
        uint64_t black_king;
    } Board;

    /**
     * Initializes the board to the starting positions
     * @param board Pointer to the Board struct to be initialized
     */
    void initBoard(Board *board);

    /**
     * Prints the board in a way that is easy to see. White pieces are represented with capital letters black pieces with
     * small letters. (p: pawn, n: knight, b: bishop, r: rook, q: queen, k: king)
     * @param board
     */
    void printBoard(Board *board);
#endif
