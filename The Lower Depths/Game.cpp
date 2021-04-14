#include "Game.h"

#include<fstream>
#include<map>
#include<cstring>

Game::Game() {
    path_ = "../map_design/";
    for (int i = 0; i < tiles.size(); ++i) {
        tiles[i] = Image(path_ + "tiles/" + std::to_string(i) + ".png");
    }
    std::array<const char*, 4> dir_str{"up", "down", "left", "right"};
    for (int dir = 0; dir < 4; ++dir) {
        for (int i = 0; i < player_sprite[0].size(); ++i) {
            player_sprite[dir][i] = Image(path_ + "sprites/player_" + 
                    dir_str[dir] + "_" +  std::to_string(i) + ".png");
        }
    }
    for (int dir = 0; dir < 4; ++dir) {
        for (int i = 0; i < guard_sprite[0].size(); ++i) {
            guard_sprite[dir][i] = Image(path_ + "sprites/guard_" + 
                    dir_str[dir] + "_" +  std::to_string(i) + ".png");
        }
    }
    for (int i = 0; i < hole_tile.size(); ++i) {
        hole_tile[i] = Image(path_ + "objects/hole" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < health_bar.size(); ++i) {
        health_bar[i] = Image(path_ + "objects/hb" + std::to_string(i) + ".png");
    }
    for (int i = 0; i < free_pearl_tile.size(); ++i) {
        free_pearl_tile[i] = Image(path_ + "objects/pearl_16_" + std::to_string(i) + ".png");
    }
    pearl_inv_tile = Image(path_ + "objects/pearl_glow.png");
    for (int i = 0; i < lightning_effect.size(); ++i) {
        lightning_effect[i] = Image(path_ + "objects/lightning" + std::to_string(i) + ".png");
    }
    gameover_img = Image(path_ + "objects/game_over.png");
    win_img = Image(path_ + "objects/game_win.png");
    rules_img = Image(path_ + "objects/game_begin.png");
    LabInit();
    RoomInit();
}

void Game::LabInit() {
    std::ifstream fin;
    fin.open(path_ + "Lab.mashgraph");
    for (int i = 0; i < LAB_SIZE; ++i) {
        fin.getline(lab[i], LAB_SIZE + 1);
        auto p_init_room = std::strchr(lab[i], '@');
        if (p_init_room) {
            cur_room_ = {static_cast<int>(p_init_room - lab[i]), i};
        }
    }
    new_room_ = cur_room_;
}

void Game::RoomInit() {
    RoomDraw();
    RoomEquip();
    if (state_ == GameState::PLAY) {
        RoomFindPos();
    } else if (state_ == GameState::NONE) {
        state_ = GameState::PLAY;
    }
    player_pos_real_ = player_pos_;
}

void Game::RoomChangeCheck() {
    room_time_ = time_ - room_change_begin_;
    switch (room_state_) {
    case RoomState::NORMAL: return;
    case RoomState::FADEOUT:
        if (room_time_ > fade_semi_time_) {
            cur_room_ = new_room_;
            RoomInit();
            room_state_ = RoomState::FADEIN;
            return;
        } else return;
    case RoomState::FADEIN:
        if (room_time_ > fade_semi_time_ * 2) {
            room_state_ = RoomState::NORMAL;
        }
    }
}

double Game::RoomFade() const {
    if (room_state_ == RoomState::NORMAL) {
        return -1;
    } else {
        return 1 - std::abs(1 - room_time_ / fade_semi_time_);
    }
}

void Game::RoomFindPos() {
    switch (player_dir_) {
    case Direction::RIGHT:
        player_pos_.x = (std::strchr(objects[ROOM_Y_CENTER], 'x') - objects[ROOM_Y_CENTER]) * TILE_SIZE + 24;
        break;
    case Direction::LEFT:
        player_pos_.x = (std::strrchr(objects[ROOM_Y_CENTER], 'x') - objects[ROOM_Y_CENTER]) * TILE_SIZE - 24;
        break;
    case Direction::DOWN:
        for (int i = 0; i < MAP_HEIGHT; ++i) {
            if (objects[i][ROOM_X_CENTER] == 'x') {
                player_pos_.y = i * TILE_SIZE + 8;
                break;
            }
        }
        break;
    case Direction::UP:
        for (int i = MAP_HEIGHT - 1; i >= 0; --i) {
            if (objects[i][ROOM_X_CENTER] == 'x') {
                player_pos_.y = i * TILE_SIZE - 48;
                break;
            }
        }
        break;
    }
}

void Game::RoomDraw() {
    std::ifstream fin_back, fin_items;
    fin_back.open(path_ + "rooms/" + RoomType() + "_back.csv");
    fin_items.open(path_ + "rooms/" + RoomType() + "_items.csv");
    for (int y = 0; y < MAP_HEIGHT * TILE_SIZE; y += TILE_SIZE) {
        for (int x = 0; x < MAP_WIDTH * TILE_SIZE; x += TILE_SIZE) {
            int tile_num;
            fin_back >> tile_num;
            background_.PutTile(x, y, tiles[tile_num]);
            fin_items >> tile_num;
            if (tile_num > 0 && tile_num != HOLE_MAP_TILE && tile_num != GUARD_MAP_TILE) {
                background_.PutTileOver(x, y, tiles[tile_num]);
            }
        }
    }
    fin_back.close();
    fin_items.close();
}

void Game::RoomEquip() {
    holes.clear();
    guards.clear();
    bool init_pearls = pearls.find(CurRoomMap()) == pearls.end();
    std::ifstream fin;
    fin.open(path_ + "rooms/" + RoomType() + ".mashgraph");
    for (int i = 0; i < MAP_HEIGHT; ++i) {
        fin.getline(objects[i], MAP_WIDTH + 1);
        char *p_hole = std::strchr(objects[i], 'h');
        if (p_hole) {
            int idx = p_hole - objects[i];
            // < 2 holes in a row
            holes.push_back({idx * TILE_SIZE, i * TILE_SIZE});
        }
        char *p_guard = std::strchr(objects[i], 'g');
        // < 2 guards in a row
        if (p_guard) {
            Point<int> guard_pos = {static_cast<int>(p_guard - objects[i]) * TILE_SIZE, i * TILE_SIZE};
            guards.push_back({guard_pos, Direction::DOWN, guard_pos});
        }
        if (init_pearls) {
            char *p_pearl = std::strchr(objects[i], 'p');
            int idx = p_pearl - objects[i];
            if (p_pearl && (pearls[CurRoomMap()].empty() || (pearls[CurRoomMap()].back().first.x != idx * TILE_SIZE - 8))) {
                pearls[CurRoomMap()].push_back({{idx * TILE_SIZE - 8, i * TILE_SIZE - 8}, true});
            }
        }
    }
    fin.close();
}

void Game::Move(Direction dir) {
    if (room_state_ != RoomState::NORMAL) {
        return;
    }
    idle_ = false;
    Point<double> desired_pos_real = player_pos_real_.Shift(dir, mean_delta_ * player_speed_);
    player_dir_ = dir;
    Point<int> desired_pos = desired_pos_real;
    std::map<char, bool> collisions;
    for (int dy : {16, 32, 40}) {
        for (int dx: {0, 16, 18}) {
            char c = objects[(desired_pos.y + dy) / TILE_SIZE][(desired_pos.x + dx) / TILE_SIZE];
            collisions[c] = true;
        }
    }
    if (collisions['#']) {
        return;
    }
    if (collisions['c'] && (time_ > last_coral_hit_ + coral_cd_) && (time_ > last_pearl_activated_ + pearl_cd_)) {
        last_coral_hit_ = time_;
        --health_;
        if (health_ == 0) {
            state_ = GameState::OVER;
            return;
        }
    }
    if (collisions['h'] && (time_ > last_pearl_activated_ + pearl_cd_)) {
        state_ = GameState::OVER;
        return;
    }
    if (collisions['x']) {
        new_room_ = cur_room_.Shift(dir, 1);
        room_time_ = 0;
        room_change_begin_ = time_;
        room_state_ = RoomState::FADEOUT;
        return;
    }
    if (collisions['Q']) {
        state_ = GameState::WIN;
        return;
    }
    if (collisions['p']) {
        auto it = pearls[CurRoomMap()].begin();
        int min_dist = desired_pos.SqrDist(it->first);
        auto min_it = it;
        while (++it != pearls[CurRoomMap()].end()) {
            int cur_dist = desired_pos.SqrDist(it->first);
            if (cur_dist < min_dist) { 
                min_dist = cur_dist;
                min_it = it;
            }
        }
        if (min_it->second && pearl_num_ < max_pearls_) {
            min_it->second = false;
            ++pearl_num_;
        }
    }
    player_pos_real_ = desired_pos_real;
    player_pos_ = desired_pos;
}

void Game::MoveGuards() {
    if (room_state_ != RoomState::NORMAL) {
        return;
    }
    double step = mean_delta_ * guard_speed_;
    if (last_pearl_activated_ + pearl_cd_ > time_) {
        step = -step * 2;
    }
    for (auto &[guard_pos, guard_dir, guard_pos_real]: guards) {
        if (guard_pos_real.y > player_pos_real_.y) {
            guard_pos_real.y -=step;
            guard_dir = Direction::UP;
        } else if (guard_pos_real.y < player_pos_real_.y) {
            guard_pos_real.y += step;
            guard_dir = Direction::DOWN;
        }
        if (guard_pos_real.x > player_pos_real_.x) {
            guard_pos_real.x -= step;
            guard_dir = Direction::LEFT;
        } else if (guard_pos_real.x < player_pos_real_.x) {
            guard_pos_real.x += step;
            guard_dir = Direction::RIGHT;
        }
        guard_pos = guard_pos_real;
        if (player_pos_ == guard_pos) {
            state_ = GameState::OVER;
        }
    }
}

void Game::ActivatePearl() {
    if (pearl_num_ == 0) {
        return;
    }
    if (time_ < last_pearl_activated_ + pearl_cd_){
        return;
    }
    health_ = max_health_;
    last_pearl_activated_ = time_;
    --pearl_num_;
}

void Game::UpdTime(double current_time) {
    if (current_time - last_fps_info_ > 10) {
        std::cout << "Mean FPS: " << 1 / mean_delta_ << "; current delta: " << 1 / (current_time - time_) << std::endl;
        last_fps_info_ = current_time;
    }
    ++counter_;
    time_ = current_time;
    mean_delta_ = current_time / counter_;
}

int discrete_wave(double x, int p, double a) {
    return std::abs(static_cast<int>(std::floor(x * a + p)) % (2 * p) - p);
}

int lightning_idx(double time_, double a) {
    int res = static_cast<int>(a * 6 * time_) % 6;
    if (res == 0 || res == 2) {
        return static_cast<int>(a * time_) % 4 + 1;
    } else {
        return 0;
    }
}

std::list<std::pair<Point<int>, const Image &>> Game::DrawList() {
    std::list<std::pair<Point<int>, const Image &>> draw_list{{{0, 0}, background_}};
    if (time_ > last_pearl_activated_ + pearl_cd_) {
        for (int i = 0; i < holes.size(); ++i) {
            draw_list.push_back({holes[i], hole_tile[discrete_wave(time_, hole_tile.size() - 1, 10 * (i + 1))]});
        }
    }
    unsigned sprite_state = discrete_wave(time_, 2, 2);
    if (state_ != GameState::OVER) {
        if (time_ < last_coral_hit_ + coral_cd_) {
            draw_list.push_back({player_pos_, player_sprite[to_underlying(player_dir_)][discrete_wave(time_, 2, 10)]});
        }
        else {
            draw_list.push_back({player_pos_, player_sprite[to_underlying(player_dir_)][sprite_state]});
        }
    }
    for (auto &[pearl_pos, is_free]: pearls[CurRoomMap()]) {
        if (is_free) {
            draw_list.push_back({pearl_pos, free_pearl_tile[static_cast<int>(time_ * 10) % free_pearl_tile.size()]});
        }
    }
    for (auto &[guard_pos, guard_dir, guard_pos_real]: guards) {
        draw_list.push_back({guard_pos, guard_sprite[to_underlying(guard_dir)][sprite_state]});
    }
    if (time_ < last_pearl_activated_ + pearl_cd_) {
        draw_list.push_back({{player_pos_.x + 9 - MAP_WIDTH * TILE_SIZE, player_pos_.y + 20 - MAP_HEIGHT * TILE_SIZE},
                             lightning_effect[lightning_idx(time_, 8)]});
    }
    draw_list.push_back({{0, 0}, health_bar[health_]});
    for (int i = 0; i < pearl_num_; ++i) {
        draw_list.push_back({{((MAP_WIDTH / 2) + 1 + i * 3) * TILE_SIZE, 0}, pearl_inv_tile});
    }
    if (state_ == GameState::WIN) {
        draw_list.push_back({{0, 0}, win_img});
    } else if (state_ == GameState::OVER) {
        draw_list.push_back({{0, 0}, gameover_img});
    } else if (idle_) {
        draw_list.push_back({{0, 0}, rules_img});
    }
    return draw_list;
}