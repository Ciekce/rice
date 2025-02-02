#pragma once

#include "types.h"
#include "search.h"

#define MAXHISTORY 8196
#define MAXCOUNTERHISTORY 16000

void score_moves(Board &board, Movelist &list, SearchStack *ss, SearchInfo &info, Move tt_move);
void score_moves(Board &board, Movelist &list, Move tt_move);
void pick_nextmove(const int moveNum, Movelist &list);
void update_hist(Board& board, SearchInfo& info, Move bestmove, Movelist &quietList, int depth);
void update_conthist(Board& board, SearchInfo& info, SearchStack *ss, Move bestmove, Movelist &quietList, int depth);
int get_conthist_score(Board &board, SearchInfo &info,
                                       SearchStack *ss, const Move move);

void get_history_scores(int& hus, int& ch, int& fmh, Board &board, SearchInfo &info, SearchStack *ss, const Move move);