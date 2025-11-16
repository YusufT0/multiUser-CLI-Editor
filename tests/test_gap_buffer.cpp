#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "gap_buffer.hpp"
#include "file_io.hpp"

TEST_CASE("Check create_gap_buffer") {
    auto gb = create_gap_buffer();
    CHECK(gb.gap_start == 0);
    CHECK(gb.gap_end == 1024);
    CHECK(gb.data.size() == 1024);
}

TEST_CASE("Check insert"){
    auto gb = create_gap_buffer();
    insert_char(gb, 'x');
    CHECK(gb.data[0] == 'x');
    CHECK(gb.gap_start == 1);
}

TEST_CASE("Check move_cursor_left"){
    GapBuffer gb;
    gb.data = {'A', 'B', 'C', '_', '_'};
    gb.gap_start = 3;
    gb.gap_end = 5;
    move_cursor_left(gb);
    CHECK(gb.gap_start == 2);
    CHECK(gb.gap_end == 4);
}

TEST_CASE("Check move_cursor_right"){
    GapBuffer gb;
    gb.data = {'A', 'B','_', 'C'};
    gb.gap_start = 2;
    gb.gap_end = 3;
    move_cursor_right(gb);
    CHECK(gb.gap_start == 3);
    CHECK(gb.gap_end == 4);
}

TEST_CASE("Check move_cursor_up"){
    GapBuffer gb;
    gb.data = {'A','B','\n','C','D','_'};
    gb.gap_start = 5;
    gb.gap_end = 6;

    move_cursor_up(gb);

    CHECK(gb.gap_start == 2);
    CHECK(gb.data[gb.gap_start - 1] == 'B');
}

TEST_CASE("Check move_cursor_down"){
    GapBuffer gb;
    gb.data = {'A','B','_','\n','C','D'};
    gb.gap_start = 2;
    gb.gap_end = 3;

    move_cursor_down(gb);

    CHECK(gb.gap_start == 5);
    CHECK(gb.data[gb.gap_start - 1] == 'D');
}

TEST_CASE("Check grow_gap"){
    GapBuffer gb;
    gb.gap_start = 0;
    gb.data = {'A', 'B', 'C', 'D'};
    gb.gap_start = 4;
    gb.gap_end = 4;
    grow_gap(gb, 10);
    CHECK(gb.data.size() == 14);
    CHECK(gb.gap_start == 4);
    CHECK(gb.gap_end == 14);
}

