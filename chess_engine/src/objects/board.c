/**
 * The board of the game is 8 by 8. Every cell can only hold one piece.
 *
 * @author Vasilis Kyriafinis
 * @date 14/7/2022
 * @version 0.1
 */

#include <stdio.h>
#include <string.h>
#include "board.h"

/// Bit operations. square is the 0 to 63 square of the board
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

/// Starting bitboards for black and white
#define BLACK_PAWNS 0b0000000011111111000000000000000000000000000000000000000000000000
#define WHITE_PAWNS 0b0000000000000000000000000000000000000000000000001111111100000000
#define BLACK_BISHOPS 0b0010010000000000000000000000000000000000000000000000000000000000
#define WHITE_BISHOPS 0b0000000000000000000000000000000000000000000000000000000000100100
#define BLACK_KNIGHTS 0b0100001000000000000000000000000000000000000000000000000000000000
#define WHITE_KNIGHTS 0b0000000000000000000000000000000000000000000000000000000001000010
#define BLACK_ROOKS 0b1000000100000000000000000000000000000000000000000000000000000000
#define WHITE_ROOKS 0b0000000000000000000000000000000000000000000000000000000010000001
#define BLACK_QUEEN 0b0000100000000000000000000000000000000000000000000000000000000000
#define WHITE_QUEEN 0b0000000000000000000000000000000000000000000000000000000000001000
#define BLACK_KING 0b0001000000000000000000000000000000000000000000000000000000000000
#define WHITE_KING 0b0000000000000000000000000000000000000000000000000000000000010000

// 12 different pieces 0 to 5 white pieces (pawn, knight, bishop, rook, queen, king), 6 to 11 black pieces same order.

// ASCII pieces
char ascii_pieces[12] = "PNBRQKpnbrqk";
// unicode pieces
char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};


/**
* Initializes the board to the starting positions
* @param board Pointer to the Board struct to be initialized
*/
void initBoard(Board *board) {
    board->white_pawns = WHITE_PAWNS;
    board->white_knights = WHITE_KNIGHTS;
    board->white_bishops = WHITE_BISHOPS;
    board->white_rooks = WHITE_ROOKS;
    board->white_queen = WHITE_QUEEN;
    board->white_king = WHITE_KING;


    board->black_pawns = BLACK_PAWNS;
    board->black_knights = BLACK_KNIGHTS;
    board->black_bishops = BLACK_BISHOPS;
    board->black_rooks = BLACK_ROOKS;
    board->black_queen = BLACK_QUEEN;
    board->black_king = BLACK_KING;
}

/**
    * Prints the board in a way that is easy to see. White pieces are represented with capital letters black pieces with
    * small letters. (p: pawn, n: knight, b: bishop, r: rook, q: queen, k: king)
    * @param board
    */
void printBoard(Board *board) {
    // Array of all the bitboards.
    uint64_t bitboards[12] = {
            board->white_pawns, board->white_knights, board->white_bishops,
            board->white_rooks, board->white_queen, board->white_king,
            board->black_pawns,board->black_knights, board->black_bishops,
            board->black_rooks, board->black_queen, board->black_king
    };

    // New lines and files
    printf("\n\n      a    b    c    d    e    f    g    h  \n");

    // The board is printed from the top to the bottom and from left to right. The ranks must run from 7 to 0.
    for (int rank = 7; rank >= 0; --rank) {
        // And the files must run from 0 to 7
        for (int file = 0; file < 8; ++file) {

            // The square of the board
            int square = rank * 8 + file;

            if(!file){
                printf("   %d", rank + 1);
            }

            // define piece variable
            int piece = -1;

            // loop over all piece bitboards
            for (int bb_piece = 0; bb_piece <= 11; bb_piece++) {
                // If the bit board returns 1 the piece is found
                if (get_bit(bitboards[bb_piece], square)) {
                    piece = bb_piece;
                    break;
                }
            }

            // print different piece set depending on OS
            //printf("  %c  ", (piece == -1) ? '.' : ascii_pieces[piece]);
            printf("  %s  ", (piece == -1) ? "." : unicode_pieces[piece]);

        }

        // print new line
        printf("\n");
    }
}