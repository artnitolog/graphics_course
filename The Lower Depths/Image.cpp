#include "Image.h"

#include <iostream>
#include <cstring>
#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

std::ostream& operator<<(std::ostream& os, const Pixel& p) {
    os << "(" << static_cast<unsigned>(p.r) << ", " << static_cast<unsigned>(p.g) << ", "
       << static_cast<unsigned>(p.b) << ", " << static_cast<unsigned>(p.a) << ")";
    return os;
}

Image::Image(const std::string &a_path) {
    // std::cout << "Path constructor!" << std::endl;
    int channels_in_file;
    if (data_ = (Pixel *)stbi_load(a_path.c_str(), &width_, &height_, &channels_in_file, 4)) {
        size_ = width_ * height_ * 4;
        // std::cout << "Successfully loaded " << width_ << "x" << height_ 
        //           << "(originally " << channels_in_file << " channels)"
        //           << " image from " << a_path << std::endl;
    } else {
        std::cerr << "Failed to load " << a_path << std::endl;
    }
}

Image::Image(int a_width, int a_height, Pixel fillColor) : width_(a_width), height_(a_height) {
    // std::cout << "hand-made constructor!" << std::endl;
    data_ = new Pixel[width_ * height_]{};
    if (data_) {
        size_ = width_ * height_ * 4;
        self_allocated_ = true;
        // std::cout << "Successfully allocated " << width_ << "x" << height_ << std::endl;
    } else {
        std::cerr << "Fail to allocate " << width_ * height_ * sizeof(Pixel)
                  << " bytes" << std::endl;
    }
    if (fillColor.a != 0) {
        FillImage(fillColor);
    }
}

Image::~Image() {
    // std::cout << "Destruct!" << std::endl;
    if (size_ > 0) {
        if (self_allocated_) {
            delete[] data_;
        }
        else {
            stbi_image_free(data_);
        }
    }
}

Image::Image(const Image &other) : width_(other.width_), height_(other.height_),
                                   size_(other.size_), self_allocated_(other.self_allocated_) {
    std::cout << "COPY constructor!" << std::endl;
    if (other.data_) {
        data_ = new Pixel[width_ * height_];
        std::memcpy(data_, other.data_, size_);
    }
}

void Image::Swap(Image &other) {
    // std::cout << "Swap!" << std::endl;
    std::swap(width_, other.width_);
    std::swap(height_, other.height_);
    std::swap(size_, other.size_);
    std::swap(self_allocated_, other.self_allocated_);
    std::swap(data_, other.data_);
}

const Image &Image::operator =(Image other) {
    // std::cout << "operator =" << std::endl;
    Swap(other);
    return *this;
}

void Image::FillImage(Pixel fillColor) {
    for (int i = 0; i < width_ * height_; ++i) {
        data_[i] = fillColor;
    }
}

void Image::Save(const char *path) {
    if (stbi_write_png(path, width_, height_, 4, data_, width_ * 4)) {
        std::cout << width_ << "x" << height_ << "image (png, RGBA) written to "
                  << path << std::endl;
    } else {
        std::cerr << "Failed to write " << width_ << "x" << height_ << "image (png, RGBA)"
                  << path << std::endl;
    }
}

bool Image::CheckPixel(int x, int y) const {
    return (x >= 0) && (y >= 0) && (x < width_) && (y < height_);
} 

Pixel Image::GetPixel(int x, int y) const {
    if (!CheckPixel(x, y)) {
        std::cerr << "Trying to get pixel out of bound!\n The window is " << width_ << "x" << height_ << std::endl;
        std::cerr << "(x, y) == (" << x << ", " << y << ")" << std::endl;
        exit(1);
    }
    return data_[width_ * y + x];
}

void Image::PutPixel(int x, int y, const Pixel &pix) {
    if (!CheckPixel(x, y)) {
        std::cerr << "Trying to put pixel out of bound!\n The window is " << width_ << "x" << height_ << std::endl;
        std::cerr << "(x, y) == (" << x << ", " << y << ")" << std::endl;
        return;
    }
    data_[width_ * y + x] = pix;
}

void Image::PutTile(int x, int y, const Image &tile) {
    for (int i = 0; i < tile.height_; ++i) {
        std::memcpy(&data_[(y + i) * width_ + x], &tile.data_[tile.width_ * i],
                    tile.width_ * sizeof(Pixel));
    }
}

void Image::PutTileOver(int x, int y, const Image &tile) {
    for (int i = 0; i < tile.height_; ++i) {
        for (int j = 0; j < tile.width_; ++j) {
            if (tile.data_[i * tile.width_ + j].a) {  // no semitransparent blending on CPU
                data_[(y + i) * width_ + x + j] = tile.data_[i * tile.width_ + j];
            }
        }
    }
}


