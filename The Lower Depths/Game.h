#ifndef MAIN_GAME_H
#define MAIN_GAME_H

#include "Image.h"
#include "structs.hpp"

#include<vector>
#include<list>
#include<string>
#include<tuple>
#include<map>

constexpr int TILE_SIZE = 16;
constexpr int MAP_WIDTH = 31, MAP_HEIGHT = 20;
constexpr int ROOM_OFFSET = 3;
constexpr int ROOM_Y_CENTER = ROOM_OFFSET + (MAP_HEIGHT - ROOM_OFFSET) / 2;
constexpr int ROOM_X_CENTER = MAP_WIDTH / 2;
constexpr int HOLE_MAP_TILE = 774, GUARD_MAP_TILE = 780;
constexpr int LAB_SIZE = 8;

enum class GameState {NONE, PLAY, OVER, WIN};
enum class RoomState {NORMAL, FADEOUT, FADEIN};

class Game {
public:
    Game();
    void LabInit();
    void RoomInit();
    void RoomChangeCheck();
    void RoomDraw();
    void RoomEquip();
    void RoomFindPos();
    void ActivatePearl();

    void Move(Direction dir);
    void MoveGuards();

    void UpdTime(double current_time);

    char RoomType() const { return lab[cur_room_.y][cur_room_.x]; }
    double RoomFade() const;

    GameState State() const { return state_; }

    std::string Path() const {return path_; }

    std::list<std::pair<Point<int>, const Image &>> DrawList();

    Point<int> PlayerPos() const {return player_pos_;}

private:
    GameState state_ = GameState::NONE;
    Image gameover_img, win_img, rules_img;
    Point<int> player_pos_{ROOM_X_CENTER * TILE_SIZE, ROOM_Y_CENTER * TILE_SIZE - 20};
    Point<double> player_pos_real_;
    std::array<Image, 864 + 1> tiles;
    std::array<std::array<Image, 3>, 4> player_sprite;
    std::array<std::array<Image, 3>, 4> guard_sprite;
    std::array<Image, 9> hole_tile;
    std::array<Image, 9> health_bar;
    std::array<Image, 10> free_pearl_tile;
    std::array<Image, 5> lightning_effect;
    Image pearl_inv_tile;
    Image background_{MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE};

    Direction player_dir_ = Direction::DOWN;
    double time_ = 0;
    double mean_delta_ = 0;
    double last_coral_hit_ = -coral_cd_ - 1;
    double last_pearl_activated_ = -pearl_cd_ - 1;
    double last_guard_move_ = 0;
    double last_fps_info_ = 0;
    double room_change_begin_ = -fade_semi_time_;
    double room_time_ = fade_semi_time_;
    RoomState room_state_ = RoomState::FADEIN;
    std::array<double, 4> last_player_move_{};
    uint64_t counter_ = 0;

    std::vector<Point<int>> holes{};
    std::map<int, std::vector<std::pair<Point<int>, bool>>> pearls;
    std::vector<std::tuple<Point<int>, Direction, Point<double>>> guards{};

    int health_ = 5;
    int pearl_num_ = 0;
    bool idle_ = true;

    constexpr static double player_speed_ = 90.0, guard_speed_ = 30.0;
    constexpr static double fade_semi_time_ = 0.3;
    constexpr static double coral_cd_ = 1.5;
    constexpr static int max_health_ = 8;
    constexpr static double pearl_cd_ = 3;
    constexpr static int max_pearls_ = 5;

    Point<int> cur_room_{};
    Point<int> new_room_{};

    std::array<char[MAP_WIDTH + 1], MAP_HEIGHT> objects;
    std::array<char[LAB_SIZE + 1], LAB_SIZE> lab;
    int CurRoomMap() {return cur_room_.y  * LAB_SIZE + cur_room_.x; }


    std::string path_;
};

#endif 