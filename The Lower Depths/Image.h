#ifndef MAIN_IMAGE_H
#define MAIN_IMAGE_H

#include <string>
#include <iostream>

struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

std::ostream& operator<<(std::ostream& os, const Pixel& p);

class Image {
public:
    // 32-bit RGBA
    Image(){}
    explicit Image(const std::string &a_path);
    Image(int a_width, int a_height, Pixel fillColor = {});
    ~Image();
    Image(const Image &other);
    void Swap(Image &other);
    const Image &operator =(Image other);
    
    void FillImage(Pixel fillColor);
    void Save(const char *path);
    bool CheckPixel(int x, int y) const;
    Pixel GetPixel(int x, int y) const;
    void PutPixel(int x, int y, const Pixel &pix);

    void PutTile(int x, int y, const Image &tile);
    void PutTileOver(int x, int y, const Image &tile);

    int width() const { return width_; }
    int height() const { return height_; }
    size_t size() const { return size_; }
    Pixel *data() const { return data_; }

private:
    int width_ = -1;
    int height_ = -1;
    Pixel *data_ = nullptr;
    size_t size_{};
    bool self_allocated_ = false;
};

#endif  // MAIN_IMAGE_H
