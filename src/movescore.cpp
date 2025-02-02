#include "movescore.h"
#include "eval.h"
#include "see.h"

int mvv_lva[12][12] = {
    105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605, 104, 204, 304,
    404, 504, 604, 104, 204, 304, 404, 504, 604, 103, 203, 303, 403, 503, 603,
    103, 203, 303, 403, 503, 603, 102, 202, 302, 402, 502, 602, 102, 202, 302,
    402, 502, 602, 101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
    100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600,

    105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605, 104, 204, 304,
    404, 504, 604, 104, 204, 304, 404, 504, 604, 103, 203, 303, 403, 503, 603,
    103, 203, 303, 403, 503, 603, 102, 202, 302, 402, 502, 602, 102, 202, 302,
    402, 502, 602, 101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
    100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600};

int get_conthist_score(Board &board, SearchInfo &info, SearchStack *ss,
                       const Move move) {

    int score = 0;
    for (int i = 1; i <= 2; i++) {
        if ((ss-i)->move) {
            score += info.contHist[(ss - i)->moved_piece][to((ss - i)->move)][board.pieceAtB(from(move))][to(move)];
        }
    }

    return score;
}

// Move scoring
void score_moves(Board &board, Movelist &list, SearchStack *ss,
                 SearchInfo &info, Move tt_move) {

    // Loop through moves in movelist.
    for (int i = 0; i < list.size; i++) {
        Piece victim = board.pieceAtB(to(list[i].move));
        Piece attacker = board.pieceAtB(from(list[i].move));

        // Score tt move the highest
        if (list[i].move == tt_move) {
            list[i].value = PvMoveScore;
        } else if (victim != None) {
            // If it's a capture move, we score using MVVLVA (Most valuable
            // victim, Least Valuable Attacker) and if see move that doesn't
            // lose material, we add additional bonus

            list[i].value = mvv_lva[attacker][victim] +
                            (GoodCaptureScore * see(board, list[i].move, -107));
        } else if (list[i].move == ss->killers[0]) {
            // Score for killer 1
            list[i].value = Killer1Score;
        } else if (list[i].move == ss->killers[1]) {
            // Score for killer 2
            list[i].value = Killer2Score;
        } else {
            // Otherwise, history score.
            list[i].value = info.searchHistory[attacker][to(list[i].move)] +
                            get_conthist_score(board, info, ss, list[i].move);
        }
    }
}

// Used for Qsearch move scoring
void score_moves(Board &board, Movelist &list, Move tt_move) {

    // Loop through moves in movelist.
    for (int i = 0; i < list.size; i++) {
        Piece victim = board.pieceAtB(to(list[i].move));
        Piece attacker = board.pieceAtB(from(list[i].move));
        if (list[i].move == tt_move) {
            list[i].value = 20000000;
        } else if (victim != None) {
            // If it's a capture move, we score using MVVLVA (Most valuable
            // victim, Least Valuable Attacker)
            list[i].value = mvv_lva[attacker][victim] +
                            (GoodCaptureScore * see(board, list[i].move, -107));
        }
    }
}

void pick_nextmove(const int moveNum, Movelist &list) {

    ExtMove temp;
    int index = 0;
    int bestscore = -INF_BOUND;
    int bestnum = moveNum;

    for (index = moveNum; index < list.size; ++index) {

        if (list[index].value > bestscore) {
            bestscore = list[index].value;
            bestnum = index;
        }
    }

    temp = list[moveNum];
    list[moveNum] = list[bestnum]; // Sort the highest score move to highest.
    list[bestnum] = temp;
}

// Update History
void update_hist(Board &board, SearchInfo &info, Move bestmove,
                 Movelist &quietList, int depth) {

    // Update best move score
    int bonus = std::min(depth * depth, 1200);
    int score = std::min(
        info.searchHistory[board.pieceAtB(from(bestmove))][to(bestmove)] +
            bonus,
        MAXHISTORY);

    info.searchHistory[board.pieceAtB(from(bestmove))][to(bestmove)] = score;

    for (int i = 0; i < quietList.size; i++) {
        Move move = quietList[i].move;

        if (move == bestmove)
            continue; // Don't give penalty to our best move

        // Penalize moves that didn't cause a beta cutoff.
        int penalty = std::max(
            info.searchHistory[board.pieceAtB(from(move))][to(move)] - bonus,
            -MAXHISTORY);

        info.searchHistory[board.pieceAtB(from(move))][to(move)] = penalty;
    }
}

static inline void update_ch(Board &board, SearchInfo &info, SearchStack *ss,
                             const Move move, const int score) {

    for (int i = 1; i <= 2; i++) {
        if (ss->ply >= i) {
            info.contHist[(ss - i)->moved_piece][to((ss - i)->move)]
                         [board.pieceAtB(from(move))][to(move)] += score;
        }
    }
}

void update_conthist(Board &board, SearchInfo &info, SearchStack *ss,
                     Move bestmove, Movelist &quietList, int depth) {
    // Update best move score
    int bonus = std::min(depth * depth, 1200);
    int score = bonus - get_conthist_score(board, info, ss, bestmove) *
                            std::abs(bonus) / MAXCOUNTERHISTORY;

    update_ch(board, info, ss, bestmove, score);

    for (int i = 0; i < quietList.size; i++) {
        Move move = quietList[i].move;

        if (move == bestmove)
            continue; // Don't give penalty to our best move, so skip it.

        // Penalize moves that didn't cause a beta cutoff.
        int penalty = bonus - get_conthist_score(board, info, ss, move) *
                                  std::abs(bonus) / MAXCOUNTERHISTORY;
        update_ch(board, info, ss, move, -penalty);
    }
}

void get_history_scores(int &his, int &ch, int &fmh, Board &board,
                        SearchInfo &info, SearchStack *ss, const Move move) {
    Move previous_move = (ss - 1)->move;
    Move previous_previous_move = (ss - 2)->move;
    Piece previous_piece = (ss - 1)->moved_piece;
    Piece previous_previous_piece = (ss - 2)->moved_piece;
    Piece moved_piece = board.pieceAtB(from(move));

    his = info.searchHistory[board.pieceAtB(from(move))][to(move)];

    ch = previous_move ? info.contHist[previous_piece][to(previous_move)]
                                      [moved_piece][to(move)]
                       : 0;
    fmh = previous_previous_move
              ? info.contHist[previous_previous_piece]
                             [to(previous_previous_move)][moved_piece][to(move)]
              : 0;
}
