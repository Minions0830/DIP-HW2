#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <iostream>

using namespace std;

void GetHeaderInfo(unsigned char *&p, int &width, int &height, int &max_color_value);
void WriteHeader(int width, int height, int max_color_value, FILE *out);
void q1_a(int width, int height, int max_color_value, unsigned char *offset);
void q1_b(int width, int height, int max_color_value, unsigned char *offset);
void q1_c(int width, int height, int max_color_value, unsigned char *offset);
void RGBfrequency(int width, int height, int max_color_value, unsigned char *offset);
void RGBfrequency_line(int width, int height, int max_color_value, unsigned char *offset);
unsigned char *LoadData(const string &filename);


inline int GetPixelAddress(int x, int y, int width) {
    return y * width * 3 + 3 * x;
}

int main() {
    string filename = "/Users/natsuki/Desktop/CSIT463-DIP/cat.ppm";
    unsigned char *data = LoadData(filename);
    unsigned char *p = data;

    if (data == nullptr) {
        fprintf(stderr, "%s not found.\n", filename.c_str());
        return EXIT_FAILURE;
    }

    if (data[0] != 'P' || data[1] != '6') {
        fprintf(stderr, "%s is not a PPM(P6) file\n", filename.c_str());
        return EXIT_FAILURE;
    }

    int width;
    int height;
    int max_color_value;
    GetHeaderInfo(p, width, height, max_color_value);

    cout << "Width = " << width << endl;
    cout << "Height = " << height << endl;
    cout << "Max color value = " << max_color_value << endl;

    unsigned char *offset = p;

    q1_a(width, height, max_color_value, offset);
    q1_b(width, height, max_color_value, offset);
    q1_c(width, height, max_color_value, offset);
    RGBfrequency(width, height, max_color_value, offset);
    RGBfrequency_line(width, height, max_color_value, offset);

    delete[] data;
}

unsigned char *LoadData(const string &filename) {
    FILE *in = fopen(filename.c_str(), "rb");
    if (in == nullptr) {
        return nullptr;
    }

    fseek(in, 0, SEEK_END);
    size_t filesize = ftell(in);
    cout << "Filesize: " << filesize << endl;
    fseek(in, 0, SEEK_SET);

    unsigned char *data = new unsigned char[filesize];
    fread(data, filesize, 1, in);
    fclose(in);

    return data;
}


void normalize(map<unsigned char, int> &color_count, int size) {
    int max_count = 0.0;

    for (auto &it : color_count) {
        max_count = max(max_count, it.second);
    }

    for (auto &it : color_count) {
        it.second = size * it.second / max_count;
    }
}


void rect(int x1, int y1, int x2, int y2, unsigned char *data, int canvas_width, int r, int g, int b) {
    for (int x = min(x1, x2); x <= max(x1, x2); x++) {
        for (int y = min(y1, y2); y <= max(y1, y2); y++) {
            unsigned char *ptr = data + GetPixelAddress(x, y, canvas_width);
            ptr[0] = r;
            ptr[1] = g;
            ptr[2] = b;
        }
    }
}


void drawline(int x1, int y1, int x2, int y2, unsigned char *data, int canvas_width, int r, int g, int b) {
    int E, x, y;
    int dx, dy, sx, sy, i;
    sx = (x2 > x1) ? 1 : -1;
    dx = (x2 > x1) ? x2 - x1 : x1 - x2;
    sy = (y2 > y1) ? 1 : -1;
    dy = (y2 > y1) ? y2 - y1 : y1 - y2;

    x = x1;
    y = y1;
    /* 傾きが1以下の場合 */
    if (dx >= dy) {
        E = -dx;
        for (i = 0; i <= dx; i++) {
            unsigned char *ptr = data + GetPixelAddress(x, y, canvas_width);
            ptr[0] = r;
            ptr[1] = g;
            ptr[2] = b;

            x += sx;
            E += 2 * dy;
            if (E >= 0) {
                y += sy;
                E -= 2 * dx;
            }
        }
        /* 傾きが1より大きい場合 */
    } else {
        E = -dy;
        for (i = 0; i <= dy; i++) {
            unsigned char *ptr = data + GetPixelAddress(x, y, canvas_width);
            ptr[0] = r;
            ptr[1] = g;
            ptr[2] = b;

            y += sy;
            E += 2 * dx;
            if (E >= 0) {
                x += sx;
                E -= 2 * dy;
            }
        }
    }
}


void RGBfrequency_line(int width, int height, int max_color_value, unsigned char *offset) {
    FILE *out = fopen("./cat_2b.ppm", "wb");

    map<unsigned char, int> r_count;
    map<unsigned char, int> g_count;
    map<unsigned char, int> b_count;

    for (int y = 0; y < height; y++) {
        for (int x = width; x > 0; x--) {
            unsigned char *ptr = offset + GetPixelAddress(x, y, width);
            unsigned char r = ptr[0];
            unsigned char g = ptr[1];
            unsigned char b = ptr[2];

            r_count[r]++;
            g_count[g]++;
            b_count[b]++;
        }
    }

    normalize(r_count, 300);
    normalize(g_count, 300);
    normalize(b_count, 300);

    int canvas_width = 1100;
    int canvas_height = 1000;
    WriteHeader(canvas_width, canvas_height, max_color_value, out);
    size_t size = canvas_width * 3 * canvas_height;
    unsigned char *data = new unsigned char[size];
    memset(data, max_color_value, size);

    int prev_x = -1;
    int prev_y = -1;
    for (auto &it : r_count) {
        if (prev_x != -1 && prev_y != -1) {
            drawline(prev_x, prev_y, 30 + 4 * it.first, 330 - it.second, data, canvas_width, 255, 0, 0);
        }
        prev_x = 30 + 4 * it.first;
        prev_y = 330 - it.second;
    }

    prev_x = prev_y = -1;
    for (auto &it : g_count) {
        if (prev_x != -1 && prev_y != -1) {
            drawline(prev_x, prev_y, 30 + 4 * it.first, 660 - it.second, data, canvas_width, 0, 255, 0);
        }
        prev_x = 30 + 4 * it.first;
        prev_y = 660 - it.second;
    }

    prev_x = prev_y = -1;
    for (auto &it : b_count) {
        if (prev_x != -1 && prev_y != -1) {
            drawline(prev_x, prev_y, 30 + 4 * it.first, 990 - it.second, data, canvas_width, 0, 0, 255);
        }
        prev_x = 30 + 4 * it.first;
        prev_y = 990 - it.second;
    }

    fwrite(data, size, 1, out);
    fclose(out);
}


void RGBfrequency(int width, int height, int max_color_value, unsigned char *offset) {
    FILE *out = fopen("./cat_2a.ppm", "wb");

    map<unsigned char, int> r_count;
    map<unsigned char, int> g_count;
    map<unsigned char, int> b_count;

    for (int y = 0; y < height; y++) {
        for (int x = width; x > 0; x--) {
            unsigned char *ptr = offset + GetPixelAddress(x, y, width);
            unsigned char r = ptr[0];
            unsigned char g = ptr[1];
            unsigned char b = ptr[2];

            r_count[r]++;
            g_count[g]++;
            b_count[b]++;
        }
    }

    normalize(r_count, 300);
    normalize(g_count, 300);
    normalize(b_count, 300);

    int canvas_width = 1100;
    int canvas_height = 1000;
    WriteHeader(canvas_width, canvas_height, max_color_value, out);
    size_t size = canvas_width * 3 * canvas_height;
    unsigned char *data = new unsigned char[size];
    memset(data, max_color_value, size);

    for (auto &it : r_count) {
        rect(30 + 4 * it.first, 330, 30 + 4 * it.first, 330 - it.second, data, canvas_width, 255, 192, 203);
        rect(31 + 4 * it.first, 330, 31 + 4 * it.first, 330 - it.second, data, canvas_width, 255, 192, 203);
        rect(32 + 4 * it.first, 330, 32 + 4 * it.first, 330 - it.second, data, canvas_width, 255, 192, 203);
    }

    for (auto &it : g_count) {
        rect(30 + 4 * it.first, 660, 30 + 4 * it.first, 660 - it.second, data, canvas_width, 0, 100, 0);
        rect(31 + 4 * it.first, 660, 31 + 4 * it.first, 660 - it.second, data, canvas_width, 0, 100, 0);
        rect(32 + 4 * it.first, 660, 32 + 4 * it.first, 660 - it.second, data, canvas_width, 0, 100, 0);
    }

    for (auto &it : b_count) {
        rect(30 + 4 * it.first, 990, 30 + 4 * it.first, 990 - it.second, data, canvas_width, 65, 105, 225);
        rect(31 + 4 * it.first, 990, 31 + 4 * it.first, 990 - it.second, data, canvas_width, 65, 105, 225);
        rect(32 + 4 * it.first, 990, 32 + 4 * it.first, 990 - it.second, data, canvas_width, 65, 105, 225);
    }

    fwrite(data, size, 1, out);
    fclose(out);
}


void q1_c(int width, int height, int max_color_value, unsigned char *offset) {
    FILE *out = fopen("./cat_1c.ppm", "wb");
    WriteHeader(width, height, max_color_value, out);

    for (int y = height; y > 0; y--) {
        for (int x = 0; x < width; x++) {
            unsigned char *ptr = offset + GetPixelAddress(x, y, width);
            unsigned char r = ptr[0];
            unsigned char g = ptr[1];
            unsigned char b = ptr[2];

            fputc(r, out);
            fputc(g, out);
            fputc(b, out);
        }
    }

    fclose(out);
}


void q1_b(int width, int height, int max_color_value, unsigned char *offset) {
    FILE *out = fopen("./cat_1b.ppm", "wb");
    WriteHeader(width / 2, height / 2, max_color_value, out);

    int start_y = (height - (height / 2)) / 2;
    int end_y = start_y + height / 2;
    int start_x = (width - (width / 2)) / 2;
    int end_x = start_x + width / 2;

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            unsigned char *ptr = offset + GetPixelAddress(x, y, width);
            unsigned char r = ptr[0];
            unsigned char g = ptr[1];
            unsigned char b = ptr[2];

            fputc(r, out);
            fputc(g, out);
            fputc(b, out);
        }
    }

    fclose(out);
}

void q1_a(int width, int height, int max_color_value, unsigned char *offset) {
    FILE *out = fopen("./cat_1a.ppm", "wb");
    WriteHeader(width / 2, height / 2, max_color_value, out);

    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width / 2; x++) {
            unsigned char *ptr = offset + GetPixelAddress(x, y, width);
            unsigned char r = ptr[0];
            unsigned char g = ptr[1];
            unsigned char b = ptr[2];

            fputc(r, out);
            fputc(g, out);
            fputc(b, out);
        }
    }

    fclose(out);
}

void WriteHeader(int width, int height, int max_color_value, FILE *out) {
    fputc('P', out);
    fputc('6', out);
    fputc(0x0a, out);

    char output_width[16];
    sprintf(output_width, "%d", width);
    char *ptr = output_width;
    while (*ptr != '\0') {
        fputc(*ptr, out);
        ptr++;
    }
    fputc(0x20, out);

    char output_height[16];
    sprintf(output_height, "%d", height);
    ptr = output_height;
    while (*ptr != '\0') {
        fputc(*ptr, out);
        ptr++;
    }
    fputc(0x0a, out);

    char output_color_value[16];
    sprintf(output_color_value, "%d", max_color_value);
    ptr = output_color_value;
    while (*ptr != '\0') {
        fputc(*ptr, out);
        ptr++;
    }
    fputc(0x0a, out);
}

void GetHeaderInfo(unsigned char *&p, int &width, int &height, int &max_color_value) {
    width = 0;
    height = 0;
    max_color_value = 0;
    p += 3;
    while (*p != 0x20) {
        width *= 10;
        width += *p - '0';
        p++;
    }
    p++;

    while (*p != 0x0a) {
        height *= 10;
        height += *p - '0';
        p++;
    }
    p++;
    while (*p != 0x0a) {
        max_color_value *= 10;
        max_color_value += *p - '0';
        p++;
    }
    p++;
}
